package main

import (
	"context"
	"flag"
	"log/slog"
	"os"
	"os/signal"
	"syscall"

	"goldberg-relay/internal/config"
	"goldberg-relay/internal/relay"
)

func main() {
	configPath := flag.String("config", "", "path to JSON config file")
	flag.Parse()

	cfg, err := config.Load(*configPath)
	if err != nil {
		slog.Error("load config", "error", err)
		os.Exit(1)
	}

	logger, err := config.NewLogger(cfg)
	if err != nil {
		slog.Error("build logger", "error", err)
		os.Exit(1)
	}

	ctx, stop := signal.NotifyContext(context.Background(), syscall.SIGINT, syscall.SIGTERM)
	defer stop()

	server, err := relay.NewServer(cfg, logger)
	if err != nil {
		logger.Error("create relay server", "error", err)
		os.Exit(1)
	}

	if err := server.Run(ctx); err != nil {
		logger.Error("relay exited", "error", err)
		os.Exit(1)
	}
}
