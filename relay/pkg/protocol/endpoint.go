package protocol

import (
	"net"
	"net/netip"
)

func AddrPortFromUDP(addr *net.UDPAddr) (netip.AddrPort, bool) {
	if addr == nil {
		return netip.AddrPort{}, false
	}
	ip, ok := netip.AddrFromSlice(addr.IP)
	if !ok {
		return netip.AddrPort{}, false
	}
	return netip.AddrPortFrom(ip.Unmap(), uint16(addr.Port)), true
}
