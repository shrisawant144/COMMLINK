# Code Flow - Quick Reference

Visual reference for understanding CommLink's execution flow.

## Application Startup

```
main() → QApplication → MainWindow()
  ├─→ initializeNetworkComponents()  [8 network objects]
  ├─→ setupUI()                      [5 UI panels]
  ├─→ setupConnections()              [Wire signals/slots]
  └─→ loadSettings()                  [Restore preferences]
```

## Connection Flow

```
User clicks "Connect"
  ↓
ConnectionPanel → Signal → MainWindow::onConnectRequested()
  ↓
Network Client → connectToHost() [Async]
  ↓
Network → connected() Signal → MainWindow::updateStatus()
  ↓
UI Updated
```

## Message Sending

```
User types message → Clicks "Send"
  ↓
MessagePanel → Signal → MainWindow::onSendRequested()
  ↓
Validate → Parse → DataMessage
  ↓
Network Client → sendMessage() → serialize() → Bytes
  ↓
Network Transmission [Async]
  ↓
DisplayPanel + Database + Log
```

## Message Receiving

```
Network data arrives
  ↓
Network Component → onReadyRead() → deserialize()
  ↓
emit messageReceived() → MainWindow::onDataReceived()
  ↓
DisplayPanel + Database + Log
```

## Signal-Slot Chain

```
UI Event → Signal → MainWindow Slot → Network Method
  → Network Signal → MainWindow Slot → UI Update
```

## Data Format Pipeline

```
User Input → parseInput() → QVariant → DataMessage
  → serialize() → QByteArray → Network
```

## Key Concepts

- **Asynchronous I/O** - All network operations are non-blocking
- **Signal-Slot** - Decoupled component communication
- **Event-Driven** - Reacts to network events as they occur

## File Locations

- **Startup**: `src/main.cpp`, `src/ui/mainwindow.cpp`
- **Connection**: `src/ui/connectionpanel.cpp`, `src/network/tcpclient.cpp`
- **Sending**: `src/ui/messagepanel.cpp`, `src/ui/mainwindow.cpp::onSendRequested()`
- **Receiving**: `src/network/tcpclient.cpp::onReadyRead()`, `src/ui/mainwindow.cpp::onDataReceived()`

For detailed explanations, see [CODE_FLOW.md](CODE_FLOW.md)
