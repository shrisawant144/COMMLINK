# CommLink

CommLink is a cross-platform desktop application for testing, inspecting, and debugging network communication across multiple protocols from a single Qt-based interface.

It is designed for developers, QA engineers, students, and integrators who need one tool that can act as both a client and a server, switch data formats quickly, and keep a searchable record of message traffic during a session.

## Highlights

- Named workspaces with payloads, variables, framing and protocol options.
- Original payload inspection in text, hex and decoded views, with conversation export.
- Timed replay with text, exact-byte and JSON-field assertions.
- Multi-protocol support: TCP, UDP, WebSocket, and HTTP in both client and server workflows where applicable.
- Multiple payload formats: JSON, XML, CSV, plain text, binary, and hex.
- Modular desktop UI: a panel-based interface centered around a `MainWindow` orchestrator.
- Local history storage: SQLite-backed message history with session tracking.
- File and export utilities: load/save payloads and export messages or logs.
- Cross-platform build: CMake + Qt5 + C++17.

## Project Status

CommLink builds successfully in its current form and includes both:

- A modular default UI launched by `MainWindow`
- A legacy monolithic UI still available through `--legacy`

The modular UI is the primary architecture going forward. The legacy UI remains in the repository for compatibility and reference.

## Who This Is For

- Developers testing local services or custom protocols
- QA teams validating request/response flows
- Students learning network programming concepts
- Engineers troubleshooting integration issues between services

## Core Capabilities

### Protocol workflows

- TCP client/server
- UDP client/server
- WebSocket client/server
- HTTP client/server

### Payload handling

- Compose and send structured or raw messages
- Switch formats without leaving the UI
- Serialize and deserialize through a shared `DataMessage` abstraction

### Productivity features

- Real-time activity log
- Searchable message history
- Export to JSON, CSV, or text
- Theme support
- Keyboard shortcuts for common actions

## Architecture At A Glance

The application is organized into three main layers:

- `core`: shared business logic such as formatting, file I/O, exports, logging, and history management
- `network`: protocol-specific client/server implementations
- `ui`: the modular desktop interface and the legacy GUI

The default application flow is:

1. `src/main.cpp` creates the Qt application.
2. `MainWindow` initializes the database, network objects, and UI panels.
3. UI panels emit signals for user actions.
4. `MainWindow` orchestrates protocol handlers and updates the display/history layers.

For a deeper walkthrough, see [Architecture](docs/ARCHITECTURE.md) and [Code Flow](docs/CODE_FLOW.md).

## Repository Layout

```text
COMMLINK/
├── include/commlink/      # Public headers
├── src/                   # Application source
│   ├── core/              # Shared logic and persistence
│   ├── network/           # Protocol implementations
│   └── ui/                # Modular and legacy UI code
├── resources/             # Qt resources, logos, UI assets
├── docs/                  # User, developer, and architecture docs
├── scripts/               # Build and environment helper scripts
├── tests/                 # Test sources
└── CMakeLists.txt         # Top-level build entry point
```

## Build Requirements

- CMake 3.16 or newer
- A C++17 compiler
- Qt 5.15 or Qt 6.2+ with `Core`, `Widgets`, `Network`, `Sql`, and `WebSockets` (`Test` for automated tests)

## Quick Start

### Linux

```bash
./scripts/install-deps.sh
./scripts/build.sh
./build/bin/commlink
```

### Manual build

```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j"$(nproc)"
./bin/commlink
```

When a native Linux build inherits Snap schema paths from an editor such as VS Code, CommLink uses the host GNOME settings schemas to avoid GTK startup crashes. This also covers terminals without `SNAP` or `GSETTINGS_SCHEMA_DIR` when `XDG_DATA_DIRS` still contains Snap paths. The adjustment applies only to the CommLink process.

## Workbench workflow

1. Configure the client and server, then choose the send destination in **Send Mode**: Remote Server, Broadcast, or Selected Client.
2. In **Workspace → Options**, choose TCP framing (raw chunks, newline, custom delimiter in hex, or a network-order 32-bit length prefix), keep-alive, HTTP headers/timeouts/authentication, and WebSocket ping interval. Disconnect before changing transport options.
3. Define variables such as `HOST=127.0.0.1` and use `${HOST}` in endpoints, headers and payloads. Variables are expanded once; missing variables are reported.
4. Enter a workspace name and **Save**. **Open** restores its connection settings, payload, variables, framing, HTTP method and polling preference. The active workspace name and its debugging session are restored on startup, including retained conversation bytes, original timestamps and response details.
5. Open **Traffic** to filter messages and inspect original bytes, decoded content, endpoints, UTC timestamps and HTTP status/headers/timing. **Use Payload** fills the composer without transmitting. **Export Conversation** writes byte-safe Base64 payloads to JSON.
6. **Replay** repeats the current payload to the selected destination. Choose a count, interval and optional response assertion: contains text, equals hexadecimal bytes, or JSON field equals a value. JSON paths use dots, including numeric array indices. **Stop** cancels pending sends. HTTP assertions are matched to the replay request, so older requests and background polling cannot satisfy them. Assertions use the first response from the selected transport after each send; raw TCP mode observes chunks rather than complete messages.
7. History survives restarts, preserves new payloads as bytes and supports **Use Payload** in message details. Older records remain available after migration; original bytes cannot be recovered for records created before byte capture was added.

Workspaces live under the application's data directory in `workspaces/` and are written atomically with owner-only file permissions. Authentication fields are session-only and cleared when opening a workspace. Explicit headers and variables are saved as entered, so avoid saving credentials there unless intended.

Traffic retains at most 2,000 records and 64 MiB of captured payload/wire bytes. Text previews are bounded and may be truncated; retained originals remain available in Traffic and History. TCP frames and HTTP bodies are limited to 8 MiB. TCP/WebSocket outgoing queues are limited to 16 MiB per peer. HTTP permits up to 16 concurrent client requests, with a 32 MiB total limit on queued mock-server payloads. TCP bursts are decoded in batches of at most 64 frames per event-loop turn so other UI work can run. Optional history retention is configured in Options (0 keeps all records; positive values delete older history on startup).

TCP captures include configured framing; UDP captures contain the datagram payload. HTTP and WebSocket captures contain application payloads provided by Qt, with HTTP response metadata shown separately; they are not packet captures of TLS or protocol framing. The built-in HTTP server is a bounded local mock server: it supports Content-Length requests and explicitly rejects transfer encoding, including chunked requests. HTTPS/WSS clients use Qt's normal certificate verification; local servers currently use plain TCP/HTTP/WS.

## Acceptance checks

| Area | Verification |
| --- | --- |
| Exact traffic inspection | Binary payload/wire round trips, HTTP bodies as transmitted, incomplete-response capture, byte-safe export. GET/HEAD have no request body; DELETE/OPTIONS preserve composed bodies. |
| Correct TCP handling | Fragmented/coalesced raw, newline, delimiter and length-prefixed traffic, with bounded receive batches. |
| Saved workspaces | Names, settings, variables, payloads and retained conversation sessions survive restart; sessions remain separate. |
| Clear conversations | Original timestamps, normalized endpoints, bytes, protocol, response details and errors; selecting a row opens its views. |
| Explicit sending controls | Typed remote, broadcast and selected-client destinations; selected-client routing tested with two peers. |
| Protocol controls | HTTP headers/authentication/status/timing, cancellation and polling validation; WebSocket binary/pong diagnostics; TCP framing and keep-alive. |
| Repeatable testing | Counts, intervals, exact-byte/text/JSON assertions, failure/timeout/cancellation, and HTTP request correlation. |
| Dependable operation | Small-window layouts in both themes, startup compatibility, migration, display/queue limits, TCP event-loop fairness and Debian packaging. |

HTTP/WebSocket inspection is at the application-payload layer; it does not capture encrypted TLS packets or Qt's protocol framing. Linux builds and native startup are verified; Windows/macOS still require native verification.

## Tests and packages

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
cmake --build build --parallel 4
ctest --test-dir build --output-on-failure
# Create an archive; it requires the platform's Qt runtime packages.
(cd build && cpack -G TGZ -B packages)
# Debian/Ubuntu package with desktop entry, icon and runtime dependencies:
(cd build && cpack -G DEB -B packages)
```

GitHub Actions builds Debug and Release on Linux, runs the headless and loopback suites, and builds a Debian package. Remote workflow runs start when the branches are pushed.

See [Platform Packaging](docs/platform-packaging.md) to build the Windows single-file EXE and the signed Android APK.

## Development Notes

- The default executable target is `commlink`.
- Build outputs are generated locally in `build/`.
- With `BUILD_TESTS=ON` (the default), run `ctest --test-dir build --output-on-failure` to check startup environment handling and UI layouts across themes, window sizes, protocols, and tabs. See the developer guide for the status of other test sources.

## Documentation Map

- [Documentation Home](docs/README.md)
- [Quick Start](docs/quick-start.md)
- [User Guide](docs/user-guide-simple.md)
- [Developer Guide](docs/developer-guide.md)
- [Platform Packaging](docs/platform-packaging.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Code Flow](docs/CODE_FLOW.md)
- [Doxygen Guide](docs/DOXYGEN.md)
- [Contributing](CONTRIBUTING.md)

## Roadmap Priorities

Current high-value improvement areas for the project:

- Re-enable and modernize automated tests
- Continue converging on the modular UI as the primary code path
- Strengthen protocol-level validation and UX consistency
- Expand contributor and API documentation

## Contributing

Contributions are welcome. Start with [CONTRIBUTING.md](CONTRIBUTING.md) for setup, coding expectations, documentation standards, and pull request guidance.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
