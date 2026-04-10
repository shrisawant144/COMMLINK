# Architecture

This document explains how CommLink is structured at the system level and how responsibilities are divided across the codebase.

It is written for contributors and maintainers who need to understand where behavior belongs before changing it.

## System Summary

CommLink is a Qt desktop application with:

- a modular default UI
- protocol-specific network handlers
- shared core services for formatting, file I/O, export, logging, and history

The architectural center of the current application is `MainWindow`, which coordinates the UI panels and the network layer without embedding protocol behavior inside the smaller widgets.

## Architectural Goals

The current design aims to provide:

- clear boundaries between UI, transport, and shared logic
- asynchronous network operations that do not block the UI thread
- reusable protocol handlers
- a maintainable migration path away from the older monolithic GUI

## High-Level Component Model

```text
main.cpp
  |
  +-- QApplication
  |
  +-- MainWindow (default) or CommLinkGUI (--legacy)
        |
        +-- UI panels
        |     +-- ConnectionPanel
        |     +-- ServerPanel
        |     +-- MessagePanel
        |     +-- DisplayPanel
        |     +-- StatusPanel
        |
        +-- Network components
        |     +-- TcpClient / TcpServer
        |     +-- UdpClient / UdpServer
        |     +-- WebSocketClient / WebSocketServer
        |     +-- HttpClient / HttpServer
        |
        +-- Core services
              +-- MessageHistoryManager
              +-- FileManager
              +-- ExportManager
              +-- ThemeManager
              +-- DataMessage / DataFormatType
```

## Layer Responsibilities

## 1. Entry Point

`src/main.cpp` is responsible for:

- creating the Qt application
- applying application metadata
- parsing command-line options
- choosing between the modular UI and the legacy UI

This file should stay thin and bootstrap-focused.

## 2. UI Layer

The UI layer lives in `src/ui/` and `include/commlink/ui/`.

### Modular UI

The modular interface is built around:

- `MainWindow`
- `ConnectionPanel`
- `ServerPanel`
- `MessagePanel`
- `DisplayPanel`
- `StatusPanel`

#### `MainWindow` responsibilities

`MainWindow` acts as an orchestrator. It is responsible for:

- creating UI panels
- creating network handlers
- connecting signals and slots
- coordinating send/receive flows
- updating UI status
- persisting settings
- persisting history entries

`MainWindow` should coordinate behavior, not become the implementation owner of protocol internals.

#### Panel responsibilities

Each panel should remain focused:

- `ConnectionPanel`: client-side protocol, host, port, and connection options
- `ServerPanel`: server-side protocol, port, client list, and send mode
- `MessagePanel`: message composition and format selection
- `DisplayPanel`: sent/received/log presentation and history access
- `StatusPanel`: compact status and summary information

Panels should expose user intent via signals rather than calling each other directly.

### Legacy UI

`CommLinkGUI` is the older monolithic implementation. It still exists for compatibility and can be launched with `--legacy`.

It is a larger, more tightly coupled code path than the modular UI. New work should default to the modular architecture unless the legacy path must also be updated for correctness or parity.

## 3. Network Layer

The network layer lives in `src/network/` and `include/commlink/network/`.

Each protocol implementation is encapsulated in a dedicated QObject-based class. These classes handle:

- connect/start behavior
- disconnect/stop behavior
- async send/receive flows
- serialization/deserialization boundaries
- error signaling

They communicate upward by emitting signals such as:

- `connected()`
- `disconnected()`
- `messageReceived(...)`
- `errorOccurred(...)`
- server-side client connection signals where relevant

The network layer should not directly manipulate UI widgets.

## 4. Core Layer

The core layer lives in `src/core/` and `include/commlink/core/`.

### `DataMessage`

This is the shared payload abstraction. It packages:

- the selected `DataFormatType`
- the parsed `QVariant` payload

It is the serialization boundary used by both UI-facing send flows and network receive flows.

### `MessageHistoryManager`

This service manages:

- SQLite database initialization
- schema creation
- message persistence
- message queries
- export helpers
- session identifiers

It is the main persistence component in the application.

### `FileManager`

This provides utility behavior for:

- loading payload text from disk
- saving payload text to disk
- recent file tracking
- default save directory resolution

### `ExportManager`

This handles exporting:

- log output
- message collections

### `ThemeManager`

This centralizes theme choice and application-wide styling behavior.

## Communication Model

The main communication pattern in the modular architecture is Qt signals and slots.

### Typical flow

1. A panel emits a signal describing user intent.
2. `MainWindow` receives that signal.
3. `MainWindow` selects the relevant network/core service.
4. The service emits results or errors asynchronously.
5. `MainWindow` updates the UI and persists history as needed.

This keeps smaller widgets decoupled and avoids direct widget-to-widget orchestration.

## Ownership and Lifetime

The project relies heavily on Qt parent-child ownership.

Typical ownership patterns:

- `MainWindow` owns network components
- `MainWindow` owns major modular UI widgets
- lower-level Qt child objects are owned by their parent widgets/objects

This is why raw pointers are common in the UI and network classes. In Qt, raw pointers plus parent ownership are a normal and correct memory-management pattern.

## Persistence Model

CommLink persists:

- UI settings through `QSettings`
- message history through SQLite

Settings include things like:

- geometry
- selected client/server protocols
- host and port values
- selected data format

History records include:

- direction
- protocol
- host/port
- content
- sender information
- session identifier
- format type

## Current Architectural Realities

This section documents realities that matter when making changes.

### Dual UI paths exist

The repository contains both a modular default UI and a legacy UI path. Not every improvement will automatically apply to both.

### Tests are not yet fully integrated

The codebase contains test source files, but automated tests are not currently enabled through the shipped CMake test configuration. Treat test strategy as an active area for improvement rather than an already-complete subsystem.

### `MainWindow` is still a large coordinator

The modular architecture is meaningfully cleaner than the legacy UI, but `MainWindow` still carries substantial orchestration logic. That is acceptable for now, but new work should avoid making it a dumping ground for protocol-specific details that could live lower in the stack.

## Change Placement Guidelines

When deciding where to implement a change:

- Put transport behavior in `network`
- Put reusable business rules in `core`
- Put UI presentation and small widget behavior in `ui`
- Put cross-component coordination in `MainWindow`

If a change touches multiple layers, try to keep each layer’s contribution narrow and explicit.

## Suggested Future Improvements

High-leverage architectural improvements include:

- re-enabling and modernizing automated tests
- continuing to reduce reliance on the legacy GUI
- extracting reusable orchestration helpers from `MainWindow` where complexity grows
- improving protocol-specific validation consistency across the UI
