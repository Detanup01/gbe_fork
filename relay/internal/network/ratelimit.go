package network

import (
	"net/netip"
	"sync"
	"time"
)

type Limiter struct {
	mu        sync.Mutex
	perSecond int
	burst     int
	entries   map[netip.Addr]*limitEntry
}

type limitEntry struct {
	tokens float64
	last   time.Time
}

func NewLimiter(perSecond, burst int) *Limiter {
	return &Limiter{
		perSecond: perSecond,
		burst:     burst,
		entries:   make(map[netip.Addr]*limitEntry),
	}
}

func (l *Limiter) Allow(ip netip.Addr, now time.Time) bool {
	l.mu.Lock()
	defer l.mu.Unlock()

	entry, ok := l.entries[ip]
	if !ok {
		l.entries[ip] = &limitEntry{
			tokens: float64(l.burst - 1),
			last:   now,
		}
		return true
	}

	elapsed := now.Sub(entry.last).Seconds()
	entry.tokens += elapsed * float64(l.perSecond)
	if entry.tokens > float64(l.burst) {
		entry.tokens = float64(l.burst)
	}
	entry.last = now

	if entry.tokens < 1 {
		return false
	}

	entry.tokens--
	return true
}

func (l *Limiter) Cleanup(before time.Time) {
	l.mu.Lock()
	defer l.mu.Unlock()

	for ip, entry := range l.entries {
		if entry.last.Before(before) {
			delete(l.entries, ip)
		}
	}
}
