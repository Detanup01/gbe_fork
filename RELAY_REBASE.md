# Relay Rebase Guide

This document explains how to reapply the public relay transport after rebasing this fork onto a newer upstream Goldberg Emulator revision.

## Goal

Keep Goldberg's Steam-facing behavior intact while replacing LAN-only networking transport with:

- a Goldberg-side relay transport backend in C++
- a public Go relay broker in `relay/`

The relay is responsible for discovery broadcasts, reliable routing, unreliable routing, disconnect propagation, and virtual endpoint assignment.

## Files Touched By The Relay Integration

Goldberg C++ side:

- `dll/dll/settings.h`
- `dll/settings_parser.cpp`
- `dll/dll/network.h`
- `dll/network.cpp`
- `dll/steam_client.cpp`
- `dll/dll/common_includes.h`
- `dll/dll/relay_transport.h`
- `dll/relay_transport.cpp`
- `post_build/steam_settings.EXAMPLE/configs.main.EXAMPLE.ini`
- `post_build/README.release.md`

Go relay side:

- `relay/cmd/server/main.go`
- `relay/internal/config/config.go`
- `relay/internal/network/ratelimit.go`
- `relay/internal/relay/server.go`
- `relay/internal/relay/server_test.go`
- `relay/pkg/protocol/endpoint.go`
- `relay/pkg/protocol/wire.go`
- `relay/config.example.json`
- `relay/compose.yaml`
- `relay/Dockerfile`
- `relay/README.md`

## Rebase Strategy

Prefer replaying the relay in layers instead of trying to resolve everything at once.

1. Rebase onto upstream.
2. Restore buildability without relay-specific conflict resolution.
3. Reapply relay config surface.
4. Reapply the relay transport backend.
5. Reapply the Go relay broker and protocol package.
6. Re-run tests and do a two-client smoke test.

This is faster and safer than resolving every merge conflict by hand in one pass.

## Goldberg-Side Responsibilities

### 1. Config Surface

These settings must continue to exist in `configs.main.ini` under `[main::connectivity]`:

- `enable_relay`
- `relay_host`
- `relay_tcp_port`
- `relay_udp_port`

They are stored in `Settings` and parsed in `settings_parser.cpp`.

### 2. Transport Split In `Networking`

`Networking` must keep the LAN path untouched and branch into the relay path when:

- `enable_relay=1`
- `relay_host` is non-empty
- both relay ports are non-zero

The relay path is centered in:

- `Relay_Transport`
- `Networking::relay_dispatch_messages()`
- `Networking::relay_mark_peer_online()`
- `Networking::relay_mark_peer_offline()`

### 3. Relay Session Semantics

The Goldberg relay transport must:

- open one TCP connection to the relay
- open one UDP socket to the relay
- send `HELLO`, then `REGISTER` updates when listen IDs change
- receive `WELCOME` with virtual IP/port plus session token
- use relay virtual endpoints for `source_ip`, `source_port`, `getIP()`, `getPort()`, and matchmaking data

### 4. Announce And Rediscovery

The announce flow is easy to lose during refactors. Preserve these rules:

- `create_announce()` must publish relay virtual TCP port when relay is ready
- `send_announce_broadcasts()` must route through `Relay_Transport::SendBroadcast()` in relay mode
- relay rediscovery must be triggered when:
  - relay session becomes ready again
  - a relay peer goes offline
  - a relay peer times out
  - a relay peer disconnects

The current implementation uses a short cooldown in `Networking::trigger_relay_rediscovery()`.

### 5. Source Query Behavior

Source query proxying is not part of the relay transport.

Keep this behavior:

- relay mode logs that source query is unsupported
- matchmaking falls back to non-query data

## Go Relay Responsibilities

### 1. Protocol Contract

The Go relay only understands the relay envelope, not Goldberg gameplay semantics.

Keep the protocol package responsible for:

- TCP framing
- envelope encode/decode
- ID payload encode/decode
- virtual endpoint helpers

Message types in use:

- `MsgHello`
- `MsgRegister`
- `MsgWelcome`
- `MsgHeartbeat`
- `MsgReliable`
- `MsgUnreliable`
- `MsgDisconnect`

### 2. Session Ownership Rules

The most important server-side identity rule is:

- `primary_id` is authoritative
- non-primary `listen_ids` may collide and must not evict an already registered client

This prevents two clients from kicking each other out when they expose the same auxiliary listen ID.

### 3. Routing Rules

The server must continue to support:

- broadcast fan-out by `appid`
- direct routing by SteamID
- direct routing by relay-issued virtual IP/port
- disconnect notifications over TCP
- UDP NAT endpoint refresh keyed by session token

### 4. Config Shape

The current broker only uses these JSON fields:

- `listen_address`
- `tcp_port`
- `udp_port`
- `session_timeout`
- `cleanup_interval`
- `max_packet_size`
- `rate_limit_per_second`
- `rate_burst`
- `log_level`
- `log_format`

Do not reintroduce the old discovery/session port-pool config unless the server architecture actually returns to that model.

## Practical Reapply Checklist

After rebasing, verify these code-level checkpoints in order.

1. `Settings` still contains the relay fields.
2. `settings_parser.cpp` still reads those fields.
3. `Networking` constructor still instantiates `Relay_Transport`.
4. `steam_client.cpp` still passes `settings_server` into `Networking`.
5. `create_announce()` still exposes relay virtual port when ready.
6. `send_announce_broadcasts()` still uses relay broadcast in relay mode.
7. `relay_dispatch_messages()` still calls:
   - `relay_mark_peer_online()`
   - `handle_announce()`
   - `do_callbacks_message()`
8. relay peer-offline paths still trigger rediscovery.
9. Go relay still registers clients by primary SteamID and does not evict on shared secondary IDs.
10. Go relay still routes broadcast/reliable/unreliable payloads.

## Regression Tests

### Go

Run:

```bash
cd relay
go test ./...
```

### Goldberg Manual Smoke Test

Use two clients on different networks or at least separate relay-mode instances.

Verify:

1. Both clients log `WELCOME`.
2. Relay logs `client registered` for both.
3. Relay log `app_clients` contains both primary SteamIDs.
4. Relay log `broadcast routed` shows `recipients=1` when one client announces.
5. Goldberg friends/presence callbacks mark the peer online.
6. Killing one client triggers offline callback on the other.
7. Restarting a client causes rediscovery without restarting the relay.

## Common Failure Modes

### Repeated `WELCOME` / `REGISTER` loop

Cause:

- server sends `WELCOME` on every `REGISTER`

Correct behavior:

- `WELCOME` only for `HELLO` or a genuinely new session

### Clients Replace Each Other

Cause:

- server treats any shared `listen_id` as authoritative ownership

Correct behavior:

- only `primary_id` collision can replace an existing client

### Relay Sees Only One Client

Cause:

- second client not registered
- second client replaced the first
- clients not using same `appid`
- relay mode enabled but LAN-only restrictions still blocking the intended path

Useful log fields:

- `app_clients`
- `shared_ids`
- `recipients`

### Clients Do Not Recover After Network Loss

Cause:

- no forced re-announce after relay reconnection or peer loss

Correct behavior:

- `trigger_relay_rediscovery()` forces a fresh announce cycle with cooldown

## Suggested Workflow For Future Rebases

If upstream changes `Networking` heavily:

1. reapply only relay config fields first
2. reintroduce `Relay_Transport` as an isolated compile step
3. reconnect announce path
4. reconnect callback/offline behavior
5. test with one client
6. test with two clients

Avoid trying to restore matchmaking, announces, direct send, and disconnect handling all at once.

## Final Validation

Before declaring the relay restored after a rebase, confirm:

- Goldberg builds
- relay builds
- `go test ./...` passes
- two clients see each other
- disconnect and reconnect both work
