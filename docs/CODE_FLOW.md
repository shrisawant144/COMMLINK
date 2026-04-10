# Code Flow

This document explains how control moves through the CommLink codebase during the most important runtime scenarios.

It complements the architecture guide by focusing on execution paths rather than static ownership.

## Scope

This guide focuses on the modular default UI path centered on `MainWindow`.

The legacy `CommLinkGUI` path still exists, but it is not the preferred reference path for understanding current design and contributor workflows.

## Main Runtime Flows

- Application startup
- Client connection and disconnection
- Server start and stop
- Message send
- Message receive
- Settings persistence
- History persistence

## 1. Application Startup

### Entry point

Startup begins in `src/main.cpp`.

High-level sequence:

1. Enable high-DPI behavior.
2. Create `QApplication`.
3. Register application metadata.
4. Parse command-line options.
5. Launch `MainWindow` by default or `CommLinkGUI` when `--legacy` is provided.
6. Enter the Qt event loop.

### Modular startup sequence

When the modular UI is used, `MainWindow` performs this initialization pattern:

1. Set window title, icon, and size.
2. Initialize the message history database.
3. Create all network components.
4. Build the modular UI.
5. Create the menu bar and shortcuts.
6. Connect UI signals and network signals.
7. Load saved settings.
8. Load and apply theme settings.
9. Write an initial log entry.

This order matters because the UI depends on services and signal wiring being ready before the window becomes interactive.

## 2. Client Connection Flow

The client connection flow starts in `ConnectionPanel`.

### High-level sequence

1. User clicks the connect button.
2. `ConnectionPanel` emits `connectRequested()`.
3. `MainWindow::onConnectRequested()` validates the current inputs.
4. `MainWindow` selects the active protocol handler.
5. The chosen client object performs an asynchronous connection or readiness step.
6. The client emits connection-related signals.
7. `MainWindow` updates UI state and logs the result.

### Protocol-specific behavior

#### TCP

- `MainWindow` passes host, port, and format into `TcpClient`.
- `TcpClient` calls `QTcpSocket::connectToHost()`.
- On success, it emits `connected()`.
- `MainWindow` responds by refreshing status.

#### UDP

- UDP is treated as a lightweight connected workflow for convenience.
- `MainWindow` calls `UdpClient::connectToHost(...)`.
- UI state is updated immediately if the operation succeeds.

#### WebSocket

- `MainWindow` forwards the URL-like host field to `WebSocketClient`.
- Connection completion is asynchronous.

#### HTTP

- HTTP is modeled as a readiness state rather than a persistent socket in the same sense as TCP/WebSocket.
- `MainWindow` marks the HTTP client as connected and enables optional polling.

## 3. Client Disconnect Flow

1. User clicks disconnect, or the connect button is toggled while connected.
2. `ConnectionPanel` emits `disconnectRequested()`.
3. `MainWindow::onDisconnectRequested()` selects the active protocol.
4. The chosen client is disconnected or polling is stopped.
5. UI state is reset and status is refreshed.

## 4. Server Start Flow

The server control flow begins in `ServerPanel`.

### High-level sequence

1. User requests server start.
2. `ServerPanel` emits `startServerRequested()`.
3. `MainWindow::onStartServerRequested()` reads the selected server protocol and port.
4. `MainWindow` applies the current message format to the server object.
5. The chosen server starts listening.
6. UI state, logs, and status are updated.

### Server responsibilities

Depending on protocol, the server may also:

- track connected clients
- emit connection/disconnection signals
- broadcast to all connected clients
- queue responses for HTTP workflows

## 5. Server Stop Flow

1. User requests stop.
2. `ServerPanel` emits `stopServerRequested()`.
3. `MainWindow::onStopServerRequested()` stops the selected protocol server.
4. Client lists, counters, and status labels are reset.

## 6. Message Send Flow

The send flow begins in `MessagePanel`.

### Validation and transformation

1. User triggers send.
2. `MessagePanel` emits `sendRequested()`.
3. `MainWindow::onSendRequested()` reads the text and selected format.
4. Empty input is rejected.
5. `DataMessage::validateInput(...)` validates the text for the chosen format.
6. `DataMessage::parseInput(...)` converts UI text into the stored `QVariant`.
7. `MainWindow` creates a `DataMessage`.

### Branch: server-side send

If the server panel is in a broadcast or selected-client send mode:

1. `MainWindow` checks the active server protocol.
2. It sends to all clients or one target client.
3. The display panel is updated with a sent message entry.
4. A history record is saved.
5. A log entry is appended.

### Branch: client-side send

If the user is sending through a client connection:

1. `MainWindow` checks the selected client protocol.
2. The chosen client sends or requests with the `DataMessage`.
3. The display panel is updated with a sent entry.
4. A history record is saved.
5. A log entry is appended.

## 7. Message Receive Flow

Incoming data originates from one of the network handlers.

### High-level sequence

1. A protocol object receives bytes or a response.
2. The protocol handler converts raw input into a `DataMessage`.
3. The protocol handler emits `messageReceived(...)` or `responseReceived(...)`.
4. `MainWindow::onDataReceived(...)` receives the event.
5. `MainWindow` identifies the protocol based on the sender object.
6. The message is converted to display text.
7. The display panel is updated.
8. A log entry is appended.
9. The message is persisted to history.

### Important detail

Deserialization happens in the protocol layer, not in the display widgets. By the time `MainWindow::onDataReceived(...)` runs, the payload has already crossed the transport boundary and is represented as a `DataMessage`.

## 8. Format Synchronization Flow

When the payload format changes:

1. `MessagePanel` emits `formatChanged(...)`.
2. `MainWindow::onFormatChanged(...)` reads the selected `DataFormatType`.
3. `MainWindow` pushes that format into every client and server object.

This allows a single format selection in the UI to drive serialization and deserialization behavior across the active protocol handlers.

## 9. File Load and Save Flow

### Load

1. User triggers load.
2. `MainWindow` opens a file dialog.
3. `FileManager::loadMessageFromFile(...)` reads the file.
4. The loaded content is validated against the selected format.
5. On success, the message panel text is updated.

### Save

1. User triggers save.
2. `MainWindow` opens a save dialog.
3. `FileManager::saveMessageToFile(...)` writes the current message text.
4. The recent-files list is updated by `FileManager`.

## 10. History Persistence Flow

History persistence is coordinated by `MainWindow` and implemented by `MessageHistoryManager`.

### On send

- `MainWindow` writes a `sent` record after a successful send path is triggered.

### On receive

- `MainWindow::onDataReceived(...)` writes a `received` record after formatting display content and extracting source information.

Stored attributes include:

- direction
- protocol
- host
- port
- display content
- sender information
- session identifier
- format type

## 11. Settings Flow

Settings are stored through `QSettings`.

### Load

During startup, `MainWindow::loadSettings()` restores:

- geometry
- client protocol
- client host
- client port
- server protocol
- server port
- selected data format

### Save

Settings are saved:

- when the window closes
- during certain runtime operations such as message receive

This behavior is part of the current implementation and may be refined in future cleanups.

## 12. Error Flow

Network handlers emit `errorOccurred(...)` signals upward.

The modular UI handles them by:

1. logging the error
2. showing a user-facing message box

This centralizes the error presentation path in `MainWindow`.

## Execution Boundaries Worth Remembering

When tracing bugs, these boundaries matter:

- UI widgets emit intent, they should not perform network work directly.
- `MainWindow` coordinates flows, but protocol handlers own transport details.
- `DataMessage` is the serialization/deserialization abstraction.
- `MessageHistoryManager` owns persistence concerns.

## Practical Trace Strategy

If you are debugging a runtime issue, a good tracing order is:

1. Find the originating user action or protocol event.
2. Confirm which signal was emitted.
3. Check the `MainWindow` slot that handles it.
4. Check the selected protocol or core service.
5. Confirm the UI update and persistence side effects.

That path mirrors how the modular application is designed to move work through the system.
