# Architecture Overview

## System Design

CommLink uses a **modular architecture** with clear separation of concerns:

```
┌─────────────────┐
│   GUI Layer     │  ← User Interface (Qt Widgets)
├─────────────────┤
│ Network Layer   │  ← TCP/UDP/HTTP/WebSocket
├─────────────────┤
│  Core Layer     │  ← Data formats, File I/O, History
├─────────────────┤
│ Storage Layer   │  ← SQLite Database
└─────────────────┘
```

## Key Components

### 1. GUI Layer (`src/ui/`)
- **MainWindow**: Tabbed interface
- **HistoryTab**: Message browsing and search
- **ThemeManager**: Light/Dark themes

### 2. Network Layer (`src/network/`)
- **TCPClient/Server**: Reliable connection-based communication
- **UDPClient/Server**: Fast connectionless messaging  
- **HTTPClient/Server**: REST API testing
- **WebSocketClient/Server**: Real-time bidirectional communication

### 3. Core Layer (`src/core/`)
- **DataFormat**: JSON/XML/CSV/Text/Binary/Hex conversion
- **FileManager**: Load/save messages
- **MessageHistoryManager**: SQLite database operations
- **Logger**: Application logging

## Data Flow

```
User Input → GUI → Network Component → Data Format → Storage
                ↓
            Response ← Network ← Format ← Retrieve ← Database
```

## Design Patterns Used

### Observer Pattern
- GUI components observe network events
- Real-time updates without polling

### Factory Pattern  
- Create network clients/servers based on protocol type
- Extensible for new protocols

### Singleton Pattern
- Database connection management
- Theme manager instance

## Threading Model

- **Main Thread**: GUI and user interactions
- **Network Threads**: Each connection runs in separate thread
- **Database Thread**: Background SQLite operations

## Database Schema

```sql
CREATE TABLE messages (
    id INTEGER PRIMARY KEY,
    timestamp TEXT,
    protocol TEXT,
    direction TEXT,
    content TEXT,
    format TEXT,
    session_id TEXT
);
```

## Extension Points

### Adding New Protocols
1. Inherit from `NetworkBase` class
2. Implement `connect()`, `send()`, `receive()` methods
3. Register in protocol factory

### Adding New Data Formats
1. Add format enum to `DataFormat::Type`
2. Implement `serialize()` and `deserialize()` methods
3. Update GUI format selector

## Performance Considerations

- **Asynchronous I/O**: Non-blocking network operations
- **Connection Pooling**: Reuse HTTP connections
- **Lazy Loading**: Load history on demand
- **Memory Management**: Smart pointers prevent leaks
