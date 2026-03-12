package relay

import (
	"context"
	"crypto/rand"
	"encoding/binary"
	"errors"
	"fmt"
	"io"
	"log/slog"
	"net"
	"net/netip"
	"slices"
	"sync"
	"time"

	"goldberg-relay/internal/config"
	"goldberg-relay/internal/network"
	"goldberg-relay/pkg/protocol"
)

type endpointKey struct {
	appID uint32
	ip    uint32
	port  uint16
}

type client struct {
	mu          sync.Mutex
	appID       uint32
	primaryID   uint64
	listenIDs   map[uint64]struct{}
	listenPort  uint16
	token       uint64
	virtualIP   uint32
	virtualPort uint16
	tcpConn     net.Conn
	udpAddr     netip.AddrPort
	lastSeen    time.Time
	closing     bool
}

func (c *client) idsSlice() []uint64 {
	ids := make([]uint64, 0, len(c.listenIDs))
	for id := range c.listenIDs {
		ids = append(ids, id)
	}
	slices.Sort(ids)
	return ids
}

type Server struct {
	cfg     config.Config
	logger  *slog.Logger
	limiter *network.Limiter
	tcpLn   net.Listener
	udpConn *net.UDPConn

	mu         sync.RWMutex
	clients    map[*client]struct{}
	byPrimary  map[string]*client
	bySteamID  map[string]*client
	byToken    map[uint64]*client
	byEndpoint map[endpointKey]*client
	nextIP     uint32
}

func NewServer(cfg config.Config, logger *slog.Logger) (*Server, error) {
	tcpLn, err := net.Listen("tcp", fmt.Sprintf("%s:%d", cfg.ListenAddress, cfg.TCPPort))
	if err != nil {
		return nil, fmt.Errorf("listen tcp: %w", err)
	}

	udpAddr, err := net.ResolveUDPAddr("udp", fmt.Sprintf("%s:%d", cfg.ListenAddress, cfg.UDPPort))
	if err != nil {
		_ = tcpLn.Close()
		return nil, fmt.Errorf("resolve udp: %w", err)
	}

	udpConn, err := net.ListenUDP("udp", udpAddr)
	if err != nil {
		_ = tcpLn.Close()
		return nil, fmt.Errorf("listen udp: %w", err)
	}

	return &Server{
		cfg:        cfg,
		logger:     logger,
		limiter:    network.NewLimiter(cfg.RateLimitPerSecond, cfg.RateBurst),
		tcpLn:      tcpLn,
		udpConn:    udpConn,
		clients:    make(map[*client]struct{}),
		byPrimary:  make(map[string]*client),
		bySteamID:  make(map[string]*client),
		byToken:    make(map[uint64]*client),
		byEndpoint: make(map[endpointKey]*client),
		nextIP:     1,
	}, nil
}

func (s *Server) Run(ctx context.Context) error {
	defer s.tcpLn.Close()
	defer s.udpConn.Close()

	errCh := make(chan error, 3)
	go func() { errCh <- s.acceptLoop(ctx) }()
	go func() { errCh <- s.udpLoop(ctx) }()
	go func() { errCh <- s.cleanupLoop(ctx) }()

	s.logger.Info("relay listening", "tcp_port", s.cfg.TCPPort, "udp_port", s.cfg.UDPPort)

	select {
	case <-ctx.Done():
		return nil
	case err := <-errCh:
		if errors.Is(err, context.Canceled) {
			return nil
		}
		return err
	}
}

func (s *Server) acceptLoop(ctx context.Context) error {
	for {
		if tl, ok := s.tcpLn.(*net.TCPListener); ok {
			_ = tl.SetDeadline(time.Now().Add(time.Second))
		}

		conn, err := s.tcpLn.Accept()
		if err != nil {
			if ne, ok := err.(net.Error); ok && ne.Timeout() {
				if ctx.Err() != nil {
					return nil
				}
				continue
			}
			if ctx.Err() != nil {
				return nil
			}
			return fmt.Errorf("accept tcp: %w", err)
		}

		if ra, ok := remoteAddrPort(conn.RemoteAddr()); ok {
			s.logger.Info("tcp client accepted", "remote", ra.String())
		}
		go s.handleTCPConn(ctx, conn)
	}
}

func (s *Server) handleTCPConn(ctx context.Context, conn net.Conn) {
	var bound *client
	defer func() {
		if bound != nil {
			s.removeClient(bound, "tcp closed")
		} else {
			_ = conn.Close()
		}
	}()

	if ra, ok := remoteAddrPort(conn.RemoteAddr()); ok {
		s.logger.Debug("tcp client connected", "remote", ra.String())
	}

	var buffer []byte
	tmp := make([]byte, 4096)
	for {
		if err := conn.SetReadDeadline(time.Now().Add(time.Second)); err != nil {
			return
		}

		n, err := conn.Read(tmp)
		if n > 0 {
			buffer = append(buffer, tmp[:n]...)
			for {
				frame, rest, ok := protocol.NextTCPFrame(buffer)
				if !ok {
					buffer = rest
					break
				}
				buffer = rest

				env, err := protocol.DecodeEnvelope(frame)
				if err != nil {
					s.logger.Warn("drop invalid tcp frame", "remote", conn.RemoteAddr().String(), "error", err)
					continue
				}
				s.logger.Debug("tcp frame received", "remote", conn.RemoteAddr().String(), "type", env.Type, "appid", env.AppID, "source_id", env.SourceID, "dest_id", env.DestID, "flags", env.Flags, "payload_bytes", len(env.Payload))
				next, err := s.handleTCPEnvelope(conn, env)
				if err != nil {
					s.logger.Warn("tcp envelope handling failed", "remote", conn.RemoteAddr().String(), "type", env.Type, "error", err)
					return
				}
				if next != nil {
					bound = next
				}
			}
		}

		if err != nil {
			if ne, ok := err.(net.Error); ok && ne.Timeout() {
				if ctx.Err() != nil {
					return
				}
				continue
			}
			if err == io.EOF || ctx.Err() != nil {
				return
			}
			return
		}
	}
}

func (s *Server) handleTCPEnvelope(conn net.Conn, env protocol.Envelope) (*client, error) {
	addrPort, ok := remoteAddrPort(conn.RemoteAddr())
	if !ok {
		return nil, fmt.Errorf("invalid tcp remote")
	}
	if !s.limiter.Allow(addrPort.Addr(), time.Now()) {
		s.logger.Warn("tcp packet rate limited", "remote", addrPort.String())
		return nil, fmt.Errorf("rate limited")
	}

	switch env.Type {
	case protocol.MsgHello, protocol.MsgRegister:
		return s.registerClient(conn, env)
	case protocol.MsgHeartbeat:
		client := s.lookupClientByConn(conn)
		if client != nil {
			s.touchClient(client, time.Now())
		}
		return client, nil
	case protocol.MsgReliable:
		client := s.lookupClientByConn(conn)
		if client == nil {
			s.logger.Warn("reliable packet before registration", "remote", addrPort.String(), "appid", env.AppID, "source_id", env.SourceID, "dest_id", env.DestID)
			return nil, fmt.Errorf("reliable packet before hello")
		}
		s.touchClient(client, time.Now())
		s.routeEnvelope(client, env, true)
		return client, nil
	default:
		return s.lookupClientByConn(conn), nil
	}
}

func (s *Server) udpLoop(ctx context.Context) error {
	buf := make([]byte, s.cfg.MaxPacketSize*2)
	for {
		if err := s.udpConn.SetReadDeadline(time.Now().Add(time.Second)); err != nil {
			return fmt.Errorf("set udp deadline: %w", err)
		}

		n, addr, err := s.udpConn.ReadFromUDPAddrPort(buf)
		if err != nil {
			if ne, ok := err.(net.Error); ok && ne.Timeout() {
				if ctx.Err() != nil {
					return nil
				}
				continue
			}
			if ctx.Err() != nil {
				return nil
			}
			return fmt.Errorf("read udp: %w", err)
		}

		if !s.limiter.Allow(addr.Addr(), time.Now()) {
			s.logger.Warn("udp packet rate limited", "remote", addr.String())
			continue
		}

		env, err := protocol.DecodeEnvelope(buf[:n])
		if err != nil {
			s.logger.Warn("drop invalid udp frame", "remote", addr.String(), "error", err)
			continue
		}
		s.logger.Debug("udp frame received", "remote", addr.String(), "type", env.Type, "appid", env.AppID, "source_id", env.SourceID, "dest_id", env.DestID, "flags", env.Flags, "payload_bytes", len(env.Payload))

		client := s.lookupClientByToken(env.SessionToken)
		if client == nil {
			s.logger.Warn("udp packet with unknown token", "remote", addr.String(), "type", env.Type, "appid", env.AppID, "source_id", env.SourceID)
			continue
		}
		s.updateUDPAddr(client, addr, time.Now())

		switch env.Type {
		case protocol.MsgHeartbeat:
		case protocol.MsgUnreliable:
			s.routeEnvelope(client, env, false)
		}
	}
}

func (s *Server) cleanupLoop(ctx context.Context) error {
	ticker := time.NewTicker(s.cfg.CleanupInterval)
	defer ticker.Stop()

	for {
		select {
		case <-ctx.Done():
			return nil
		case <-ticker.C:
			now := time.Now()
			var expired []*client
			s.mu.RLock()
			for c := range s.clients {
				if now.Sub(c.lastSeen) > s.cfg.SessionTimeout {
					expired = append(expired, c)
				}
			}
			s.mu.RUnlock()

			for _, c := range expired {
				s.removeClient(c, "timeout")
			}

			s.limiter.Cleanup(now.Add(-2 * s.cfg.SessionTimeout))
		}
	}
}

func (s *Server) registerClient(conn net.Conn, env protocol.Envelope) (*client, error) {
	listenPort, ids, err := protocol.DecodeIDsPayload(env.Payload)
	if err != nil {
		return nil, err
	}
	if len(ids) == 0 && env.SourceID != 0 {
		ids = append(ids, env.SourceID)
	}
	if len(ids) == 0 {
		return nil, fmt.Errorf("registration missing ids")
	}
	primaryID := env.SourceID
	if primaryID == 0 {
		primaryID = ids[0]
	}
	if listenPort == 0 {
		listenPort = 47584
	}

	now := time.Now()
	key := steamKey(env.AppID, primaryID)
	var replaced []*client
	sendWelcome := env.Type == protocol.MsgHello
	created := false

	s.mu.Lock()
	current := s.byPrimary[key]
	if current != nil && current.tcpConn != conn {
		replaced = append(replaced, current)
		current = nil
	}
	if current == nil {
		created = true
		current = &client{
			appID:       env.AppID,
			primaryID:   primaryID,
			listenIDs:   make(map[uint64]struct{}),
			token:       randomUint64(),
			virtualIP:   s.allocateVirtualIPLocked(),
			virtualPort: listenPort,
			tcpConn:     conn,
			lastSeen:    now,
		}
		s.clients[current] = struct{}{}
	}
	sendWelcome = sendWelcome || created

	for id := range current.listenIDs {
		if owner, ok := s.bySteamID[steamKey(current.appID, id)]; ok && owner == current {
			delete(s.bySteamID, steamKey(current.appID, id))
		}
	}

	current.appID = env.AppID
	current.primaryID = primaryID
	current.listenPort = listenPort
	current.virtualPort = listenPort
	current.tcpConn = conn
	current.lastSeen = now
	current.closing = false
	current.listenIDs = make(map[uint64]struct{}, len(ids))

	for _, id := range ids {
		idKey := steamKey(env.AppID, id)
		current.listenIDs[id] = struct{}{}
		if owner, ok := s.bySteamID[idKey]; ok && owner != current {
			if id == primaryID {
				replaced = append(replaced, owner)
				s.bySteamID[idKey] = current
			}
			continue
		}
		s.bySteamID[idKey] = current
	}

	s.byPrimary[key] = current
	s.byToken[current.token] = current
	s.byEndpoint[endpointKey{appID: current.appID, ip: current.virtualIP, port: current.virtualPort}] = current
	s.mu.Unlock()

	for _, old := range replaced {
		if old != current {
			s.removeClient(old, "replaced")
		}
	}

	if sendWelcome {
		welcome := protocol.Envelope{
			Type:              protocol.MsgWelcome,
			AppID:             current.appID,
			SourceID:          current.primaryID,
			SourceVirtualIP:   current.virtualIP,
			SourceVirtualPort: current.virtualPort,
			SessionToken:      current.token,
		}
		if err := s.sendTCP(current, welcome); err != nil {
			return nil, err
		}
	}
	s.logger.Info(
		"client registered",
		"remote", conn.RemoteAddr().String(),
		"appid", current.appID,
		"primary_id", current.primaryID,
		"listen_ids", current.idsSlice(),
		"listen_port", current.listenPort,
		"virtual_endpoint", formatVirtualEndpoint(current.virtualIP, current.virtualPort),
		"welcome_sent", sendWelcome,
		"app_clients", s.appClientIDs(current.appID),
		"shared_ids", s.sharedListenIDs(current.appID),
	)
	return current, nil
}

func (s *Server) routeEnvelope(sender *client, env protocol.Envelope, reliable bool) {
	env.AppID = sender.appID
	if env.SourceID == 0 {
		env.SourceID = sender.primaryID
	}
	env.SourceVirtualIP = sender.virtualIP
	env.SourceVirtualPort = sender.virtualPort
	env.SessionToken = 0

	if env.Flags&protocol.FlagBroadcast != 0 {
		recipients := 0
		for _, target := range s.clientsForApp(sender.appID) {
			if target == sender {
				continue
			}
			s.deliver(target, env, false)
			recipients++
		}
		s.logger.Info("broadcast routed", "appid", sender.appID, "source_id", env.SourceID, "payload_bytes", len(env.Payload), "recipients", recipients, "app_clients", s.appClientIDs(sender.appID), "shared_ids", s.sharedListenIDs(sender.appID))
		return
	}

	target := s.resolveTarget(sender.appID, env)
	if target == nil {
		s.logger.Warn(
			"route target not found",
			"appid", sender.appID,
			"source_id", env.SourceID,
			"dest_id", env.DestID,
			"dest_endpoint", formatVirtualEndpoint(env.DestVirtualIP, env.DestVirtualPort),
			"flags", env.Flags,
			"reliable", reliable,
		)
		return
	}
	s.logger.Info(
		"packet routed",
		"appid", sender.appID,
		"source_id", env.SourceID,
		"dest_id", target.primaryID,
		"dest_endpoint", formatVirtualEndpoint(target.virtualIP, target.virtualPort),
		"payload_bytes", len(env.Payload),
		"reliable", reliable,
	)
	s.deliver(target, env, reliable)
}

func (s *Server) resolveTarget(appID uint32, env protocol.Envelope) *client {
	if env.Flags&protocol.FlagHasDestSteamID != 0 {
		return s.lookupClientBySteamID(appID, env.DestID)
	}
	if env.Flags&protocol.FlagHasDestEndpoint != 0 {
		return s.lookupClientByEndpoint(appID, env.DestVirtualIP, env.DestVirtualPort)
	}
	return nil
}

func (s *Server) deliver(target *client, env protocol.Envelope, reliable bool) {
	if reliable {
		_ = s.sendTCP(target, env)
		return
	}
	_ = s.sendUDP(target, env)
}

func (s *Server) sendTCP(target *client, env protocol.Envelope) error {
	frame := protocol.FrameTCP(protocol.EncodeEnvelope(env))
	target.mu.Lock()
	defer target.mu.Unlock()
	if target.tcpConn == nil {
		s.logger.Warn("tcp delivery skipped, missing tcp connection", "target_id", target.primaryID, "appid", target.appID)
		return fmt.Errorf("missing tcp conn")
	}
	if err := target.tcpConn.SetWriteDeadline(time.Now().Add(2 * time.Second)); err != nil {
		return err
	}
	_, err := target.tcpConn.Write(frame)
	if err != nil {
		s.logger.Warn("tcp delivery failed", "target_id", target.primaryID, "appid", target.appID, "type", env.Type, "error", err)
		go s.removeClient(target, "tcp write failed")
	}
	return err
}

func (s *Server) sendUDP(target *client, env protocol.Envelope) error {
	target.mu.Lock()
	addr := target.udpAddr
	target.mu.Unlock()
	if !addr.IsValid() {
		s.logger.Warn("udp delivery skipped, missing udp address", "target_id", target.primaryID, "appid", target.appID, "type", env.Type)
		return fmt.Errorf("missing udp addr")
	}
	payload := protocol.EncodeEnvelope(env)
	if err := s.udpConn.SetWriteDeadline(time.Now().Add(2 * time.Second)); err != nil {
		return err
	}
	_, err := s.udpConn.WriteToUDPAddrPort(payload, addr)
	if err != nil {
		s.logger.Warn("udp delivery failed", "target_id", target.primaryID, "appid", target.appID, "type", env.Type, "remote", addr.String(), "error", err)
	}
	return err
}

func (s *Server) removeClient(target *client, reason string) {
	if target == nil {
		return
	}

	var notify []*client
	var payload []byte
	var tcpConn net.Conn

	s.mu.Lock()
	if _, ok := s.clients[target]; !ok || target.closing {
		s.mu.Unlock()
		return
	}
	target.closing = true
	delete(s.clients, target)
	delete(s.byPrimary, steamKey(target.appID, target.primaryID))
	delete(s.byToken, target.token)
	delete(s.byEndpoint, endpointKey{appID: target.appID, ip: target.virtualIP, port: target.virtualPort})
	for id := range target.listenIDs {
		key := steamKey(target.appID, id)
		if owner, ok := s.bySteamID[key]; ok && owner == target {
			delete(s.bySteamID, key)
		}
	}
	for client := range s.clients {
		if client.appID == target.appID {
			notify = append(notify, client)
		}
	}
	payload = protocol.EncodeIDsPayload(target.listenPort, target.idsSlice())
	tcpConn = target.tcpConn
	s.mu.Unlock()

	if tcpConn != nil {
		_ = tcpConn.Close()
	}

	disconnect := protocol.Envelope{
		Type:              protocol.MsgDisconnect,
		AppID:             target.appID,
		SourceID:          target.primaryID,
		SourceVirtualIP:   target.virtualIP,
		SourceVirtualPort: target.virtualPort,
		Payload:           payload,
	}
	for _, peer := range notify {
		_ = s.sendTCP(peer, disconnect)
	}

	s.logger.Info("client removed", "appid", target.appID, "steam_id", target.primaryID, "reason", reason)
}

func (s *Server) clientsForApp(appID uint32) []*client {
	s.mu.RLock()
	defer s.mu.RUnlock()
	out := make([]*client, 0, len(s.clients))
	for c := range s.clients {
		if c.appID == appID {
			out = append(out, c)
		}
	}
	return out
}

func (s *Server) appClientIDs(appID uint32) []uint64 {
	s.mu.RLock()
	defer s.mu.RUnlock()
	out := make([]uint64, 0, len(s.clients))
	for c := range s.clients {
		if c.appID == appID {
			out = append(out, c.primaryID)
		}
	}
	slices.Sort(out)
	return out
}

func (s *Server) sharedListenIDs(appID uint32) []uint64 {
	s.mu.RLock()
	defer s.mu.RUnlock()
	counts := make(map[uint64]int)
	for c := range s.clients {
		if c.appID != appID {
			continue
		}
		for id := range c.listenIDs {
			counts[id]++
		}
	}
	out := make([]uint64, 0)
	for id, count := range counts {
		if count > 1 {
			out = append(out, id)
		}
	}
	slices.Sort(out)
	return out
}

func (s *Server) lookupClientByConn(conn net.Conn) *client {
	s.mu.RLock()
	defer s.mu.RUnlock()
	for c := range s.clients {
		if c.tcpConn == conn {
			return c
		}
	}
	return nil
}

func (s *Server) lookupClientByToken(token uint64) *client {
	s.mu.RLock()
	defer s.mu.RUnlock()
	return s.byToken[token]
}

func (s *Server) lookupClientBySteamID(appID uint32, steamID uint64) *client {
	s.mu.RLock()
	defer s.mu.RUnlock()
	return s.bySteamID[steamKey(appID, steamID)]
}

func (s *Server) lookupClientByEndpoint(appID uint32, ip uint32, port uint16) *client {
	s.mu.RLock()
	defer s.mu.RUnlock()
	return s.byEndpoint[endpointKey{appID: appID, ip: ip, port: port}]
}

func (s *Server) updateUDPAddr(c *client, addr netip.AddrPort, now time.Time) {
	s.mu.Lock()
	defer s.mu.Unlock()
	changed := c.udpAddr != addr
	first := !c.udpAddr.IsValid()
	c.udpAddr = addr
	c.lastSeen = now
	if first || changed {
		s.logger.Info("udp endpoint learned", "appid", c.appID, "steam_id", c.primaryID, "remote", addr.String())
	}
}

func (s *Server) touchClient(c *client, now time.Time) {
	s.mu.Lock()
	defer s.mu.Unlock()
	c.lastSeen = now
}

func (s *Server) allocateVirtualIPLocked() uint32 {
	ip := uint32(0x0AC80000) + s.nextIP
	s.nextIP++
	return ip
}

func steamKey(appID uint32, steamID uint64) string {
	return fmt.Sprintf("%d:%d", appID, steamID)
}

func randomUint64() uint64 {
	var buf [8]byte
	if _, err := rand.Read(buf[:]); err != nil {
		return uint64(time.Now().UnixNano())
	}
	token := binary.LittleEndian.Uint64(buf[:])
	if token == 0 {
		return uint64(time.Now().UnixNano())
	}
	return token
}

func remoteAddrPort(addr net.Addr) (netip.AddrPort, bool) {
	switch v := addr.(type) {
	case *net.TCPAddr:
		ip, ok := netip.AddrFromSlice(v.IP)
		if !ok {
			return netip.AddrPort{}, false
		}
		return netip.AddrPortFrom(ip.Unmap(), uint16(v.Port)), true
	case *net.UDPAddr:
		ip, ok := netip.AddrFromSlice(v.IP)
		if !ok {
			return netip.AddrPort{}, false
		}
		return netip.AddrPortFrom(ip.Unmap(), uint16(v.Port)), true
	default:
		return netip.AddrPort{}, false
	}
}

func formatVirtualEndpoint(ip uint32, port uint16) string {
	addr := netip.AddrFrom4([4]byte{
		byte(ip >> 24),
		byte(ip >> 16),
		byte(ip >> 8),
		byte(ip),
	})
	return netip.AddrPortFrom(addr, port).String()
}
