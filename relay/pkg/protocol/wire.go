package protocol

import (
	"encoding/binary"
	"fmt"
)

const (
	Magic   uint32 = 0x4C524247
	Version uint16 = 1

	MsgHello      uint16 = 1
	MsgRegister   uint16 = 2
	MsgWelcome    uint16 = 3
	MsgHeartbeat  uint16 = 4
	MsgReliable   uint16 = 5
	MsgUnreliable uint16 = 6
	MsgDisconnect uint16 = 7

	FlagBroadcast       uint32 = 1 << 0
	FlagHasDestSteamID  uint32 = 1 << 1
	FlagHasDestEndpoint uint32 = 1 << 2
)

type Envelope struct {
	Type              uint16
	Flags             uint32
	AppID             uint32
	SourceID          uint64
	DestID            uint64
	SourceVirtualIP   uint32
	SourceVirtualPort uint16
	DestVirtualIP     uint32
	DestVirtualPort   uint16
	SessionToken      uint64
	Payload           []byte
}

func EncodeEnvelope(env Envelope) []byte {
	size := 4 + 2 + 2 + 4 + 4 + 8 + 8 + 4 + 2 + 4 + 2 + 8 + 4 + len(env.Payload)
	buf := make([]byte, size)
	off := 0
	binary.LittleEndian.PutUint32(buf[off:], Magic)
	off += 4
	binary.LittleEndian.PutUint16(buf[off:], Version)
	off += 2
	binary.LittleEndian.PutUint16(buf[off:], env.Type)
	off += 2
	binary.LittleEndian.PutUint32(buf[off:], env.Flags)
	off += 4
	binary.LittleEndian.PutUint32(buf[off:], env.AppID)
	off += 4
	binary.LittleEndian.PutUint64(buf[off:], env.SourceID)
	off += 8
	binary.LittleEndian.PutUint64(buf[off:], env.DestID)
	off += 8
	binary.LittleEndian.PutUint32(buf[off:], env.SourceVirtualIP)
	off += 4
	binary.LittleEndian.PutUint16(buf[off:], env.SourceVirtualPort)
	off += 2
	binary.LittleEndian.PutUint32(buf[off:], env.DestVirtualIP)
	off += 4
	binary.LittleEndian.PutUint16(buf[off:], env.DestVirtualPort)
	off += 2
	binary.LittleEndian.PutUint64(buf[off:], env.SessionToken)
	off += 8
	binary.LittleEndian.PutUint32(buf[off:], uint32(len(env.Payload)))
	off += 4
	copy(buf[off:], env.Payload)
	return buf
}

func DecodeEnvelope(data []byte) (Envelope, error) {
	const headerSize = 52
	if len(data) < headerSize {
		return Envelope{}, fmt.Errorf("frame too short")
	}
	off := 0
	magic := binary.LittleEndian.Uint32(data[off:])
	off += 4
	if magic != Magic {
		return Envelope{}, fmt.Errorf("invalid magic")
	}
	version := binary.LittleEndian.Uint16(data[off:])
	off += 2
	if version != Version {
		return Envelope{}, fmt.Errorf("unsupported version")
	}
	env := Envelope{}
	env.Type = binary.LittleEndian.Uint16(data[off:])
	off += 2
	env.Flags = binary.LittleEndian.Uint32(data[off:])
	off += 4
	env.AppID = binary.LittleEndian.Uint32(data[off:])
	off += 4
	env.SourceID = binary.LittleEndian.Uint64(data[off:])
	off += 8
	env.DestID = binary.LittleEndian.Uint64(data[off:])
	off += 8
	env.SourceVirtualIP = binary.LittleEndian.Uint32(data[off:])
	off += 4
	env.SourceVirtualPort = binary.LittleEndian.Uint16(data[off:])
	off += 2
	env.DestVirtualIP = binary.LittleEndian.Uint32(data[off:])
	off += 4
	env.DestVirtualPort = binary.LittleEndian.Uint16(data[off:])
	off += 2
	env.SessionToken = binary.LittleEndian.Uint64(data[off:])
	off += 8
	payloadLen := binary.LittleEndian.Uint32(data[off:])
	off += 4
	if off+int(payloadLen) != len(data) {
		return Envelope{}, fmt.Errorf("invalid payload length")
	}
	env.Payload = append([]byte(nil), data[off:]...)
	return env, nil
}

func FrameTCP(payload []byte) []byte {
	out := make([]byte, 4+len(payload))
	binary.LittleEndian.PutUint32(out[:4], uint32(len(payload)))
	copy(out[4:], payload)
	return out
}

func NextTCPFrame(buffer []byte) (frame []byte, rest []byte, ok bool) {
	if len(buffer) < 4 {
		return nil, buffer, false
	}
	size := binary.LittleEndian.Uint32(buffer[:4])
	if len(buffer) < 4+int(size) {
		return nil, buffer, false
	}
	frame = append([]byte(nil), buffer[4:4+size]...)
	rest = append([]byte(nil), buffer[4+size:]...)
	return frame, rest, true
}

func EncodeIDsPayload(listenPort uint16, ids []uint64) []byte {
	out := make([]byte, 4+8*len(ids))
	binary.LittleEndian.PutUint16(out[:2], listenPort)
	binary.LittleEndian.PutUint16(out[2:4], uint16(len(ids)))
	off := 4
	for _, id := range ids {
		binary.LittleEndian.PutUint64(out[off:], id)
		off += 8
	}
	return out
}

func DecodeIDsPayload(payload []byte) (listenPort uint16, ids []uint64, err error) {
	if len(payload) < 4 {
		return 0, nil, fmt.Errorf("payload too short")
	}
	listenPort = binary.LittleEndian.Uint16(payload[:2])
	count := int(binary.LittleEndian.Uint16(payload[2:4]))
	if len(payload) != 4+count*8 {
		return 0, nil, fmt.Errorf("payload length mismatch")
	}
	ids = make([]uint64, 0, count)
	for off := 4; off < len(payload); off += 8 {
		ids = append(ids, binary.LittleEndian.Uint64(payload[off:off+8]))
	}
	return listenPort, ids, nil
}
