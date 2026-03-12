package relay

import (
	"context"
	"encoding/binary"
	"io"
	"log/slog"
	"net"
	"strconv"
	"testing"
	"time"

	"goldberg-relay/internal/config"
	"goldberg-relay/pkg/protocol"
)

type testClient struct {
	tcp   net.Conn
	udp   *net.UDPConn
	token uint64
	vIP   uint32
	vPort uint16
	id    uint64
}

func TestServerRoutesTrafficAndDisconnects(t *testing.T) {
	tcpPort := freeTCPPort(t)
	udpPort := freeUDPPort(t)

	cfg := config.Default()
	cfg.ListenAddress = "127.0.0.1"
	cfg.TCPPort = tcpPort
	cfg.UDPPort = udpPort
	cfg.SessionTimeout = 30 * time.Second
	cfg.CleanupInterval = 500 * time.Millisecond

	logger := slog.New(slog.NewTextHandler(io.Discard, nil))
	server, err := NewServer(cfg, logger)
	if err != nil {
		t.Fatalf("new server: %v", err)
	}

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	go func() {
		_ = server.Run(ctx)
	}()
	time.Sleep(150 * time.Millisecond)

	c1 := connectClient(t, tcpPort, udpPort, 480, 1001)
	defer c1.tcp.Close()
	defer c1.udp.Close()

	c2 := connectClient(t, tcpPort, udpPort, 480, 2001)
	defer c2.udp.Close()

	reliablePayload := []byte("reliable")
	sendTCPEnvelope(t, c1.tcp, protocol.Envelope{
		Type:     protocol.MsgReliable,
		Flags:    protocol.FlagHasDestSteamID,
		AppID:    480,
		SourceID: c1.id,
		DestID:   c2.id,
		Payload:  reliablePayload,
	})
	gotReliable := readTCPEnvelope(t, c2.tcp)
	if gotReliable.Type != protocol.MsgReliable || string(gotReliable.Payload) != string(reliablePayload) {
		t.Fatalf("unexpected reliable envelope: %+v", gotReliable)
	}
	if gotReliable.SourceVirtualIP != c1.vIP || gotReliable.SourceVirtualPort != c1.vPort {
		t.Fatalf("unexpected reliable source endpoint: %+v", gotReliable)
	}

	broadcastPayload := []byte("broadcast")
	sendUDPEnvelope(t, c1.udp, protocol.Envelope{
		Type:         protocol.MsgUnreliable,
		Flags:        protocol.FlagBroadcast,
		AppID:        480,
		SourceID:     c1.id,
		SessionToken: c1.token,
		Payload:      broadcastPayload,
	})
	gotBroadcast := readUDPEnvelope(t, c2.udp)
	if gotBroadcast.Type != protocol.MsgUnreliable || string(gotBroadcast.Payload) != string(broadcastPayload) {
		t.Fatalf("unexpected broadcast envelope: %+v", gotBroadcast)
	}

	endpointPayload := []byte("endpoint")
	sendUDPEnvelope(t, c2.udp, protocol.Envelope{
		Type:            protocol.MsgUnreliable,
		Flags:           protocol.FlagHasDestEndpoint,
		AppID:           480,
		SourceID:        c2.id,
		DestVirtualIP:   c1.vIP,
		DestVirtualPort: c1.vPort,
		SessionToken:    c2.token,
		Payload:         endpointPayload,
	})
	gotEndpoint := readUDPEnvelope(t, c1.udp)
	if gotEndpoint.Type != protocol.MsgUnreliable || string(gotEndpoint.Payload) != string(endpointPayload) {
		t.Fatalf("unexpected endpoint envelope: %+v", gotEndpoint)
	}

	_ = c2.tcp.Close()
	gotDisconnect := readTCPEnvelope(t, c1.tcp)
	if gotDisconnect.Type != protocol.MsgDisconnect {
		t.Fatalf("expected disconnect, got %+v", gotDisconnect)
	}
	_, ids, err := protocol.DecodeIDsPayload(gotDisconnect.Payload)
	if err != nil {
		t.Fatalf("decode disconnect payload: %v", err)
	}
	if len(ids) != 1 || ids[0] != c2.id {
		t.Fatalf("unexpected disconnect ids: %v", ids)
	}
}

func connectClient(t *testing.T, tcpPort, udpPort int, appID uint32, id uint64) testClient {
	t.Helper()

	tcpConn, err := net.Dial("tcp", net.JoinHostPort("127.0.0.1", itoa(tcpPort)))
	if err != nil {
		t.Fatalf("dial tcp: %v", err)
	}
	udpAddr, err := net.ResolveUDPAddr("udp", net.JoinHostPort("127.0.0.1", itoa(udpPort)))
	if err != nil {
		t.Fatalf("resolve udp: %v", err)
	}
	udpConn, err := net.DialUDP("udp", nil, udpAddr)
	if err != nil {
		t.Fatalf("dial udp: %v", err)
	}

	sendTCPEnvelope(t, tcpConn, protocol.Envelope{
		Type:     protocol.MsgHello,
		AppID:    appID,
		SourceID: id,
		Payload:  protocol.EncodeIDsPayload(47584, []uint64{id}),
	})
	welcome := readTCPEnvelope(t, tcpConn)
	if welcome.Type != protocol.MsgWelcome {
		t.Fatalf("expected welcome, got %+v", welcome)
	}

	sendUDPEnvelope(t, udpConn, protocol.Envelope{
		Type:         protocol.MsgHeartbeat,
		AppID:        appID,
		SourceID:     id,
		SessionToken: welcome.SessionToken,
	})

	return testClient{
		tcp:   tcpConn,
		udp:   udpConn,
		token: welcome.SessionToken,
		vIP:   welcome.SourceVirtualIP,
		vPort: welcome.SourceVirtualPort,
		id:    id,
	}
}

func sendTCPEnvelope(t *testing.T, conn net.Conn, env protocol.Envelope) {
	t.Helper()
	frame := protocol.FrameTCP(protocol.EncodeEnvelope(env))
	if err := conn.SetWriteDeadline(time.Now().Add(2 * time.Second)); err != nil {
		t.Fatalf("set tcp write deadline: %v", err)
	}
	if _, err := conn.Write(frame); err != nil {
		t.Fatalf("write tcp frame: %v", err)
	}
}

func sendUDPEnvelope(t *testing.T, conn *net.UDPConn, env protocol.Envelope) {
	t.Helper()
	payload := protocol.EncodeEnvelope(env)
	if err := conn.SetWriteDeadline(time.Now().Add(2 * time.Second)); err != nil {
		t.Fatalf("set udp write deadline: %v", err)
	}
	if _, err := conn.Write(payload); err != nil {
		t.Fatalf("write udp frame: %v", err)
	}
}

func readTCPEnvelope(t *testing.T, conn net.Conn) protocol.Envelope {
	t.Helper()
	if err := conn.SetReadDeadline(time.Now().Add(3 * time.Second)); err != nil {
		t.Fatalf("set tcp read deadline: %v", err)
	}
	var lenBuf [4]byte
	if _, err := io.ReadFull(conn, lenBuf[:]); err != nil {
		t.Fatalf("read tcp frame len: %v", err)
	}
	size := binary.LittleEndian.Uint32(lenBuf[:])
	frame := make([]byte, size)
	if _, err := io.ReadFull(conn, frame); err != nil {
		t.Fatalf("read tcp frame: %v", err)
	}
	env, err := protocol.DecodeEnvelope(frame)
	if err != nil {
		t.Fatalf("decode tcp envelope: %v", err)
	}
	return env
}

func readUDPEnvelope(t *testing.T, conn *net.UDPConn) protocol.Envelope {
	t.Helper()
	if err := conn.SetReadDeadline(time.Now().Add(3 * time.Second)); err != nil {
		t.Fatalf("set udp read deadline: %v", err)
	}
	buf := make([]byte, 4096)
	n, err := conn.Read(buf)
	if err != nil {
		t.Fatalf("read udp frame: %v", err)
	}
	env, err := protocol.DecodeEnvelope(buf[:n])
	if err != nil {
		t.Fatalf("decode udp envelope: %v", err)
	}
	return env
}

func freeTCPPort(t *testing.T) int {
	t.Helper()
	ln, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatalf("reserve tcp port: %v", err)
	}
	defer ln.Close()
	return ln.Addr().(*net.TCPAddr).Port
}

func freeUDPPort(t *testing.T) int {
	t.Helper()
	addr, err := net.ResolveUDPAddr("udp", "127.0.0.1:0")
	if err != nil {
		t.Fatalf("resolve udp port: %v", err)
	}
	conn, err := net.ListenUDP("udp", addr)
	if err != nil {
		t.Fatalf("reserve udp port: %v", err)
	}
	defer conn.Close()
	return conn.LocalAddr().(*net.UDPAddr).Port
}

func itoa(v int) string {
	return strconv.Itoa(v)
}
