package config

import (
	"encoding/json"
	"fmt"
	"log/slog"
	"net/netip"
	"os"
	"strings"
	"time"
)

type Config struct {
	ListenAddress      string        `json:"listen_address"`
	TCPPort            int           `json:"tcp_port"`
	UDPPort            int           `json:"udp_port"`
	SessionTimeout     time.Duration `json:"session_timeout"`
	CleanupInterval    time.Duration `json:"cleanup_interval"`
	MaxPacketSize      int           `json:"max_packet_size"`
	RateLimitPerSecond int           `json:"rate_limit_per_second"`
	RateBurst          int           `json:"rate_burst"`
	LogLevel           string        `json:"log_level"`
	LogFormat          string        `json:"log_format"`
}

func Default() Config {
	return Config{
		ListenAddress:      "0.0.0.0",
		TCPPort:            23010,
		UDPPort:            23011,
		SessionTimeout:     120 * time.Second,
		CleanupInterval:    10 * time.Second,
		MaxPacketSize:      4096,
		RateLimitPerSecond: 5000,
		RateBurst:          10000,
		LogLevel:           "info",
		LogFormat:          "json",
	}
}

func Load(path string) (Config, error) {
	cfg := Default()
	if path == "" {
		return cfg, cfg.Validate()
	}

	data, err := os.ReadFile(path)
	if err != nil {
		return Config{}, fmt.Errorf("read config: %w", err)
	}

	if err := json.Unmarshal(data, &cfg); err != nil {
		return Config{}, fmt.Errorf("parse config: %w", err)
	}

	return cfg, cfg.Validate()
}

func (c Config) Validate() error {
	if _, err := netip.ParseAddr(c.ListenAddress); err != nil {
		return fmt.Errorf("invalid listen_address: %w", err)
	}
	if c.TCPPort <= 0 || c.TCPPort > 65535 {
		return fmt.Errorf("invalid tcp_port")
	}
	if c.UDPPort <= 0 || c.UDPPort > 65535 {
		return fmt.Errorf("invalid udp_port")
	}
	if c.SessionTimeout <= 0 || c.CleanupInterval <= 0 {
		return fmt.Errorf("timeouts and cleanup interval must be positive")
	}
	if c.MaxPacketSize < 64 || c.MaxPacketSize > 65507 {
		return fmt.Errorf("max_packet_size must be between 64 and 65507")
	}
	if c.RateLimitPerSecond <= 0 || c.RateBurst <= 0 {
		return fmt.Errorf("rate limit settings must be positive")
	}
	return nil
}

func NewLogger(cfg Config) (*slog.Logger, error) {
	var level slog.Level
	switch strings.ToLower(cfg.LogLevel) {
	case "debug":
		level = slog.LevelDebug
	case "info":
		level = slog.LevelInfo
	case "warn":
		level = slog.LevelWarn
	case "error":
		level = slog.LevelError
	default:
		return nil, fmt.Errorf("unsupported log level %q", cfg.LogLevel)
	}

	opts := &slog.HandlerOptions{Level: level}
	switch strings.ToLower(cfg.LogFormat) {
	case "json":
		return slog.New(slog.NewJSONHandler(os.Stdout, opts)), nil
	case "text":
		return slog.New(slog.NewTextHandler(os.Stdout, opts)), nil
	default:
		return nil, fmt.Errorf("unsupported log format %q", cfg.LogFormat)
	}
}
