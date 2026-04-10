# Developer Guide

This guide is the primary entry point for engineers working on CommLink.

It covers how to build the project, how the repository is structured, what architectural patterns the code uses, and what the current development reality looks like.

## Engineering Overview

CommLink is a Qt5/C++17 desktop application for network communication testing. The codebase is organized around a modular default UI that orchestrates reusable protocol handlers and shared business logic.

At a high level:

- `src/main.cpp` boots the Qt application and selects the default modular UI or the legacy UI.
- `src/ui/mainwindow.cpp` is the orchestration layer for the modular interface.
- `src/network/` contains protocol-specific transport code.
- `src/core/` contains shared abstractions such as payload formatting, history persistence, and export/file helpers.

## Prerequisites

- CMake 3.8.2 or newer
- Qt5 5.12 or newer
- A C++17-capable compiler

Required Qt modules:

- `Core`
- `Widgets`
- `Network`
- `Sql`
- `WebSockets`

## Build

### Recommended build

```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j"$(nproc)"
```

Run the application:

```bash
./bin/commlink
```

Launch the legacy UI explicitly:

```bash
./bin/commlink --legacy
```

### Convenience scripts

The repository includes helper scripts under `scripts/`:

- `install-deps.sh`
- `build.sh`
- `format.sh`
- `setup-linux.sh`
- `setup-windows.ps1`

Use them as shortcuts, but treat CMake as the source of truth for how the project is built.

## Repository Structure

```text
COMMLINK/
├── include/commlink/
│   ├── core/              # Public headers for shared logic
│   ├── network/           # Public headers for protocol handlers
│   └── ui/                # Public headers for UI classes
├── src/
│   ├── core/              # Shared implementation
│   ├── network/           # Protocol implementations
│   ├── ui/                # Modular UI + legacy GUI
│   ├── main.cpp           # Application entry point
│   └── CMakeLists.txt     # Main target definitions
├── resources/             # Qt resource collections and assets
├── docs/                  # Project documentation
├── scripts/               # Setup, build, and formatting helpers
├── tests/                 # Test sources
├── cmake/                 # Shared CMake modules
└── CMakeLists.txt         # Top-level build configuration
```

## Build Targets

The build is split into layered static libraries:

- `commlink_core`
- `commlink_network`
- `commlink_ui`
- `commlink` executable

This separation is useful when making changes because it encourages clear boundaries between shared logic, protocol code, and UI concerns.

## Architectural Model

### Default UI

The modular application is centered on `MainWindow`, which acts as an orchestrator rather than a catch-all implementation bucket.

Its responsibilities are:

- Create major UI panels
- Create network client/server instances
- Connect signals and slots between panels and protocol handlers
- Persist settings and message history
- Translate network events into UI updates

### Legacy UI

The legacy `CommLinkGUI` still exists and can be launched with `--legacy`. It is useful for compatibility and comparison, but new work should generally prefer the modular architecture unless a change must affect both paths.

## Important Modules

### `DataMessage` and format handling

The payload abstraction lives in `include/commlink/core/dataformat.h`.

It is responsible for:

- Validating input for the selected format
- Parsing UI text into an internal `QVariant`
- Serializing data before send
- Deserializing incoming bytes after receive
- Producing a display-friendly string for the UI and history storage

### Message history

`MessageHistoryManager` provides the SQLite-backed local history system.

It handles:

- Database initialization
- Schema creation
- Message persistence
- Querying and export support
- Session identifiers

### Protocol handlers

Each protocol gets its own class pair where relevant:

- `TcpClient` / `TcpServer`
- `UdpClient` / `UdpServer`
- `WebSocketClient` / `WebSocketServer`
- `HttpClient` / `HttpServer`

These classes are asynchronous Qt objects that emit signals upward rather than manipulating UI state directly.

## Typical Development Workflow

1. Build the project.
2. Launch the app from the build directory.
3. Reproduce or exercise the flow you are changing.
4. Make code changes in the appropriate layer.
5. Rebuild and rerun the relevant flows manually.
6. Update docs when behavior, setup, or architecture has changed.

## Testing Status

This section is intentionally explicit because it matters for contributor expectations.

### Current state

- The repository contains test source files under `tests/unit/`.
- The current `tests/CMakeLists.txt` has test targets commented out.
- Running `ctest` against the shipped build configuration currently reports no tests.
- Some checked-in test sources appear to target older APIs and should be treated as stale until refreshed.

### What this means for contributors

For now, validation is primarily:

- Successful build
- Manual runtime verification
- Focused code review

If you add or modernize tests, keep them aligned with the current API surface and re-enable them through CMake rather than relying on ad hoc standalone binaries.

## Debugging Guidance

### UI and orchestration issues

- Start with `src/ui/mainwindow.cpp`.
- Confirm signal-slot wiring in `setupConnections()` and `initializeNetworkComponents()`.
- Verify panel state transitions through `updateStatus()`, `updateSendButtonState()`, and related methods.

### Protocol issues

- Inspect the matching class in `src/network/`.
- Confirm the chosen `DataFormatType` is being propagated correctly.
- Pay attention to asynchronous behavior and protocol-specific connection state.

### Persistence issues

- Inspect `MessageHistoryManager`.
- Verify the SQLite database can be created in the application data directory.
- Check whether the payload was transformed before save via `toDisplayString()`.

## Documentation Expectations

Documentation changes are part of normal engineering work in this repository.

Update documentation when you change:

- Build or runtime requirements
- CLI behavior
- Architecture or ownership boundaries
- Public-facing user workflows
- Contributor workflows

Relevant files usually include:

- `README.md`
- `docs/README.md`
- `docs/developer-guide.md`
- `docs/ARCHITECTURE.md`
- `docs/CODE_FLOW.md`
- `CONTRIBUTING.md`

## Style Guidelines

### C++ and Qt conventions

- Use Qt ownership patterns consistently.
- Prefer signal-slot communication over direct cross-widget coupling.
- Keep protocol logic in `network`, not in UI components.
- Keep business logic in `core` where it can be shared.

### Naming conventions

- Classes: `PascalCase`
- Methods and variables: `camelCase`
- Constants: `UPPER_CASE` or descriptive `static constexpr` members

## Recommended First Files To Read

If you are new to the project, start here:

1. `src/main.cpp`
2. `include/commlink/ui/mainwindow.h`
3. `src/ui/mainwindow.cpp`
4. `include/commlink/core/dataformat.h`
5. `src/core/messagehistorymanager.cpp`
6. One protocol pair under `src/network/`

## Release Hygiene

Before cutting a release or preparing the repository for wider reuse, verify:

- Build succeeds from a clean checkout
- Documentation matches current behavior
- Changelog and version files are updated
- Legacy versus modular UI behavior is clearly described where relevant
