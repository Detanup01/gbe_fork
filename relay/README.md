# Goldberg Relay

`goldberg-relay` is a public TCP/UDP broker for Goldberg's internal networking transport. It replaces LAN-only discovery/direct peer transport with a relay-issued virtual endpoint model while keeping Goldberg's internal `Common_Message` payloads unchanged.

## Transport Model

- TCP `tcp_port`: session registration, heartbeats, disconnects, and reliable payload routing
- UDP `udp_port`: announce broadcasts, unreliable payload routing, and NAT endpoint refresh
- routing scope: `appid`
- authoritative identity: primary SteamID from the Goldberg client
- auxiliary listen IDs: accepted for lookup, but collisions do not evict an existing client

Each connected Goldberg client gets:

- a persistent TCP session
- a relay session token used to authenticate UDP datagrams
- a stable virtual IPv4/port endpoint for the lifetime of that relay session

## Config

The current JSON schema only includes fields used by the broker:

```json
{
  "listen_address": "0.0.0.0",
  "tcp_port": 23010,
  "udp_port": 23011,
  "session_timeout": 120000000000,
  "cleanup_interval": 10000000000,
  "max_packet_size": 4096,
  "rate_limit_per_second": 5000,
  "rate_burst": 10000,
  "log_level": "debug",
  "log_format": "json"
}
```

Notes:

- durations are Go nanoseconds because the config is unmarshaled directly into `time.Duration`
- `max_packet_size` should stay comfortably above Goldberg announce payload size; `4096` is the current safe default
- use `log_level=debug` while diagnosing connectivity, and `info` once stable

## Run

```bash
go run ./cmd/server -config ./config.example.json
```

## Docker

```bash
docker build -t goldberg-relay .
docker run --rm \
  -p 23010:23010/tcp \
  -p 23011:23011/udp \
  -v "$(pwd)/config.example.json:/app/config.json:ro" \
  goldberg-relay
```

The included [compose.yaml](/c:/Users/czach/Desktop/gbe_fork/relay/compose.yaml) mirrors the same runtime shape.

## Goldberg Client Config

Set these in `configs.main.ini`:

```ini
[main::connectivity]
disable_lan_only=1
enable_relay=1
relay_host=relay.example.com
relay_tcp_port=23010
relay_udp_port=23011
```

Important behavior notes:

- relay mode is transport replacement, not an addition to the LAN path
- `custom_broadcasts.txt` is not used for relay transport
- if `matchmaking_server_details_via_source_query=1`, Goldberg falls back because source-query proxying is not implemented in relay mode

## Useful Logs

Server-side log lines that matter most:

- `client registered`
- `udp endpoint learned`
- `broadcast routed`
- `packet routed`
- `client removed`

If `broadcast routed` shows `recipients=0`, the relay did not have another online client in the same `appid` at that moment.

## Development

Run tests with:

```bash
go test ./...
```

If you need to rebase the Goldberg-side relay integration later, follow [RELAY_REBASE.md](/c:/Users/czach/Desktop/gbe_fork/RELAY_REBASE.md).
