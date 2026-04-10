# CommLink

CommLink is a cross-platform desktop application for testing, inspecting, and debugging network communication across multiple protocols from a single Qt-based interface.

It is designed for developers, QA engineers, students, and integrators who need one tool that can act as both a client and a server, switch data formats quickly, and keep a searchable record of message traffic during a session.

## Highlights

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

- CMake 3.8.2 or newer
- A C++17 compiler
- Qt5 with `Core`, `Widgets`, `Network`, `Sql`, and `WebSockets`

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

## Development Notes

- The default executable target is `commlink`.
- The project currently builds successfully with the checked-in `build/` directory configuration.
- Test sources exist, but the test targets are not currently enabled in CMake. See the developer guide for the current testing status and recommended next steps.

## Documentation Map

- [Documentation Home](docs/README.md)
- [Quick Start](docs/quick-start.md)
- [User Guide](docs/user-guide-simple.md)
- [Developer Guide](docs/developer-guide.md)
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
