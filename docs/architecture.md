# CommLink Architecture# CommLink Architecture



## System Overview## System Overview



CommLink is a modular, event-driven network communication application built on Qt's signal-slot mechanism. It supports multiple protocols (TCP, UDP, HTTP, WebSocket) and data formats (JSON, XML, CSV, TEXT, BINARY, HEX), providing a unified interface for testing and debugging network communication.CommLink follows a modular, event-driven architecture using Qt's signal-slot mechanism for component communication.



### Key Architectural Principles## Component Diagram

- **Separation of Concerns**: Core logic separated from UI and network layers

- **Modularity**: Independent components communicating through well-defined interfaces```

- **Asynchronous I/O**: Non-blocking network operations keeping UI responsive┌─────────────────────────────────────────────────────────────┐

- **Data Format Agnostic**: Pluggable serialization system supporting multiple formats│                         GUI (Main Window)                    │

│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐   │

## Component Diagram│  │  Send    │  │ Receive  │  │   Logs   │  │ History  │   │

│  │   Tab    │  │   Tab    │  │   Tab    │  │   Tab    │   │

```│  └──────────┘  └──────────┘  └──────────┘  └──────────┘   │

┌───────────────────────────────────────────────────────────────────────────┐└─────────────────────────────────────────────────────────────┘

│                         GUI (Main Window)                                  │         │              │              │              │

│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐       │         ▼              ▼              ▼              ▼

│  │  TCP/UDP │ │   HTTP   │ │WebSocket │ │ History  │ │   Logs   │       │┌──────────────┐ ┌──────────────┐ ┌──────────┐ ┌──────────────┐

│  │   Tab    │ │   Tab    │ │   Tab    │ │   Tab    │ │   Tab    │       ││    Sender    │ │   Receiver   │ │  Logger  │ │   History    │

│  └──────────┘ └──────────┘ └──────────┘ └──────────┘ └──────────┘       ││              │ │              │ │          │ │   Manager    │

└───────────────────────────────────────────────────────────────────────────┘└──────────────┘ └──────────────┘ └──────────┘ └──────────────┘

         │              │             │             │            │         │              │                             │

         ▼              ▼             ▼             ▼            ▼         ▼              ▼                             ▼

┌──────────────┐ ┌──────────────┐ ┌──────────────┐ ┌──────────────┐ ┌──────────┐┌──────────────┐ ┌──────────────┐           ┌──────────────┐

│  TCPClient   │ │  HTTPClient  │ │  WSClient    │ │   History    │ │  Logger  ││ SenderThread │ │ReceiverThread│           │   SQLite DB  │

│  TCPServer   │ │  HTTPServer  │ │  WSServer    │ │   Manager    │ │          │└──────────────┘ └──────────────┘           └──────────────┘

│  UDPClient   │ │              │ │              │ │              │ │          │         │              │

│  UDPServer   │ │              │ │              │ │              │ │          │         └──────┬───────┘

└──────────────┘ └──────────────┘ └──────────────┘ └──────────────┘ └──────────┘                ▼

         │              │             │                    │        ┌──────────────┐

         └──────┬───────┴─────────────┘                    │        │  DataFormat  │

                ▼                                           ▼        │   Handler    │

        ┌──────────────────────┐                   ┌──────────────┐        └──────────────┘

        │    DataFormat        │                   │   SQLite DB  │                │

        │    Handler           │                   │   (History)  │                ▼

        │  (Multi-format       │                   └──────────────┘        ┌──────────────┐

        │   Serialization)     │        │   Network    │

        └──────────────────────┘        │  (TCP/UDP)   │

                │        └──────────────┘

                ▼```

        ┌──────────────────────┐

        │  Network Layer       │## Class Relationships

        │  TCP | UDP           │

        │  HTTP | WebSocket    │```

        └──────────────────────┘GUI

``` ├── Sender

 │    └── SenderThread

## Class Relationships │         └── DataFormat

 ├── Receiver

``` │    └── ReceiverThread

GUI (Main Window) │         └── DataFormat

 ├── Network Components (Core) ├── Logger

 │    ├── TCPClient/TCPServer ├── HistoryTab

 │    ├── UDPClient/UDPServer │    └── MessageHistoryManager

 │    ├── HTTPClient/HTTPServer ├── FileManager

 │    └── WebSocketClient/WebSocketServer ├── ExportManager

 │         └── DataFormat (shared serialization) └── ThemeManager

 │```

 ├── Core Managers

 │    ├── MessageHistoryManager (SQLite persistence)## Data Flow

 │    ├── FileManager (Load/Save operations)

 │    ├── ExportManager (Multi-format export)### Sending Message Flow

 │    ├── Logger (Activity logging)```

 │    └── ThemeManager (UI theming)User Input → GUI → Sender → DataFormat → SenderThread → Network

 │                                                    ↓

 └── UI Components                                            Logger ← ConnectionStats

      ├── HistoryTab (Search/Filter/Export)                                                    ↓

      └── StatusBar (Connection status)                                            MessageHistoryManager

``````



## Data Flow### Receiving Message Flow

```

### TCP/UDP Communication FlowNetwork → ReceiverThread → DataFormat → Receiver → GUI Display

```                                            ↓

User Input → GUI → Protocol Selection → DataFormat → Network Client/Server                                        Logger

                                                            ↓                                            ↓

                                                      Logger ← Stats                                    MessageHistoryManager

                                                            ↓```

                                                    MessageHistoryManager

```## Key Design Patterns



### HTTP Communication Flow1. **Model-View-Controller (MVC)**: GUI separates presentation from logic

```2. **Observer Pattern**: Qt signals/slots for event handling

HTTP Request → GUI → HTTPClient → DataFormat → HTTP Server3. **Strategy Pattern**: DataFormat supports multiple serialization strategies

                                                      ↓4. **Singleton Pattern**: Logger and ThemeManager (implicit)

HTTP Response ← DataFormat ← HTTPServer ← Network Response5. **Thread Pool**: Background threads for non-blocking I/O

         ↓

    GUI Display & Logger & MessageHistoryManager## Threading Model

```

- **Main Thread**: GUI event loop and user interactions

### WebSocket Communication Flow- **Sender Thread**: Handles outgoing network connections

```- **Receiver Thread**: Listens for incoming messages without blocking UI

WS Connect → GUI → WSClient/WSServer → DataFormat → WebSocket Protocol

                                                            ↓## Database Schema

Messages ←→ Bidirectional Stream ←→ Logger & MessageHistoryManager

``````sql

CREATE TABLE messages (

## Core Components    id INTEGER PRIMARY KEY AUTOINCREMENT,

    timestamp TEXT NOT NULL,

### 1. Network Layer    direction TEXT NOT NULL,  -- 'sent' or 'received'

    protocol TEXT NOT NULL,   -- 'TCP' or 'UDP'

#### TCP Client/Server    format TEXT NOT NULL,     -- 'JSON', 'XML', etc.

- **Purpose**: Reliable, connection-oriented communication    host TEXT NOT NULL,

- **Use Cases**: File transfer, API communication, reliable messaging    port INTEGER NOT NULL,

- **Features**: Connection management, error detection, retry logic    message TEXT NOT NULL,

    status TEXT NOT NULL      -- 'success' or 'error'

#### UDP Client/Server);

- **Purpose**: Fast, connectionless communication```

- **Use Cases**: Real-time data, broadcasting, low-latency applications

- **Features**: Datagram-based, no connection overhead## Performance Considerations



#### HTTP Client/Server- Asynchronous I/O prevents UI freezing

- **Purpose**: RESTful API communication and web services- Database indexing on timestamp for fast queries

- **Use Cases**: REST APIs, webhooks, web service testing- Message size limits to prevent memory issues

- **Features**: GET/POST/PUT/DELETE methods, headers, status codes- Connection pooling for repeated sends



#### WebSocket Client/Server## Security Notes

- **Purpose**: Full-duplex, bidirectional communication

- **Use Cases**: Real-time chat, live updates, streaming data- No encryption implemented (plaintext transmission)

- **Features**: Persistent connections, low overhead, event-driven- No authentication mechanism

- Suitable for trusted networks only

### 2. Data Format Handler- Input validation prevents injection attacks


**Supported Formats**:
- **JSON**: Structured data, API communication
- **XML**: Legacy systems, configuration files
- **CSV**: Tabular data, spreadsheet compatible
- **TEXT**: Human-readable messages
- **BINARY**: Raw byte data, efficient transmission
- **HEX**: Hexadecimal representation, debugging

**Features**:
- Automatic format detection
- Format validation and conversion
- Error handling and recovery
- Serialization/deserialization

### 3. Core Managers

#### MessageHistoryManager
- **Database**: SQLite for persistent storage
- **Features**: 
  - Full-text search across messages
  - Advanced filtering (date, protocol, format, host)
  - Session tracking and audit trails
  - Thread-safe operations
  - Automatic cleanup of old records

#### FileManager
- **Purpose**: File I/O operations for all formats
- **Features**:
  - Load/save messages in any supported format
  - Automatic format detection
  - Directory management
  - Validation and error handling

#### ExportManager
- **Purpose**: Export logs and messages for analysis
- **Features**:
  - Multi-format export (TXT, CSV, JSON)
  - Batch export operations
  - Data filtering and transformation
  - Timestamp formatting

#### Logger
- **Purpose**: Application-wide activity logging
- **Features**:
  - Timestamped entries
  - Severity levels (INFO, WARNING, ERROR)
  - Thread-safe logging
  - Export capabilities

#### ThemeManager
- **Purpose**: UI appearance management
- **Features**:
  - Light/Dark/System theme support
  - Dynamic theme switching
  - Persistent theme preferences
  - Consistent styling across components

## Database Schema

```sql
CREATE TABLE messages (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp TEXT NOT NULL,
    direction TEXT NOT NULL,      -- 'sent' or 'received'
    protocol TEXT NOT NULL,        -- 'TCP', 'UDP', 'HTTP', 'WebSocket'
    format TEXT NOT NULL,          -- 'JSON', 'XML', 'CSV', 'TEXT', 'BINARY', 'HEX'
    host TEXT NOT NULL,
    port INTEGER NOT NULL,
    message TEXT NOT NULL,
    status TEXT NOT NULL,          -- 'success' or 'error'
    session_id TEXT,               -- For grouping related messages
    metadata TEXT                  -- JSON field for additional protocol-specific data
);

CREATE INDEX idx_timestamp ON messages(timestamp);
CREATE INDEX idx_protocol ON messages(protocol);
CREATE INDEX idx_direction ON messages(direction);
CREATE INDEX idx_session ON messages(session_id);
```

## Key Design Patterns

1. **Model-View-Controller (MVC)**: Separates data, presentation, and logic
2. **Observer Pattern**: Qt signals/slots for event handling and loose coupling
3. **Strategy Pattern**: Pluggable DataFormat handlers for different serialization formats
4. **Factory Pattern**: Creating appropriate network clients/servers based on protocol
5. **Singleton Pattern**: Logger, ThemeManager, and MessageHistoryManager for global access
6. **Thread Pool**: Background workers for non-blocking network I/O

## Threading Model

- **Main Thread**: Qt event loop, UI rendering, user interactions
- **Network Threads**: Asynchronous I/O operations for each active connection
- **Database Thread**: Background database operations to prevent UI blocking
- **Thread Safety**: Mutexes and Qt's thread-safe signals for inter-thread communication

## Performance Considerations

### Optimization Strategies
- **Asynchronous I/O**: Prevents UI freezing during network operations
- **Database Indexing**: Fast queries on timestamp, protocol, and direction
- **Message Size Limits**: Configurable limits to prevent memory exhaustion
- **Connection Pooling**: Reuse connections for repeated operations
- **Lazy Loading**: Load history data on-demand rather than at startup
- **Efficient Serialization**: Minimal overhead in format conversion

### Resource Management
- **Memory**: Automatic cleanup of completed network operations
- **File Handles**: Proper closure of all file descriptors
- **Database Connections**: Connection pooling and prepared statements
- **Network Sockets**: Graceful shutdown and resource release

## Security Considerations

### Current Security Posture
- ⚠️ **No Encryption**: All data transmitted in plaintext
- ⚠️ **No Authentication**: No user or connection verification
- ⚠️ **Trusted Networks Only**: Suitable for development/testing environments
- ✅ **Input Validation**: Prevents injection attacks and malformed data
- ✅ **Resource Limits**: Protects against DoS through size limits

### Future Security Enhancements
- TLS/SSL support for encrypted communication
- API key authentication for HTTP endpoints
- Rate limiting to prevent abuse
- Secure WebSocket (WSS) support
- Certificate validation

## Error Handling Strategy

### Hierarchical Error Handling
1. **Network Layer**: Connection failures, timeouts, protocol errors
2. **Data Layer**: Serialization errors, format validation failures
3. **UI Layer**: User input validation, display of error messages
4. **Persistence Layer**: Database errors, file I/O failures

### Error Recovery
- Automatic reconnection attempts with exponential backoff
- Transaction rollback for database operations
- Graceful degradation when optional features fail
- Detailed error logging for debugging

## Extensibility Points

### Adding New Protocols
1. Implement client/server classes following existing interface
2. Register with GUI tab system
3. Add protocol-specific serialization if needed
4. Update database schema for protocol-specific metadata

### Adding New Data Formats
1. Implement serializer/deserializer in DataFormat
2. Add format validation logic
3. Update UI format selection dropdown
4. Add format-specific export capabilities

### Adding New Features
- Plugin system (future): Load external protocol handlers
- Scripting support (future): Automate testing scenarios
- Custom themes (future): User-defined color schemes
- Advanced analytics (future): Message statistics and visualization

## Build System

### CMake Configuration
- Modular source organization (core, network, ui)
- Automatic Qt MOC (Meta-Object Compiler) integration
- Unit test integration with CTest
- Cross-platform build support (Linux, Windows, macOS)

### Dependencies
- **Qt5**: Core, Widgets, Network, Sql, WebSockets
- **SQLite3**: Embedded database (via Qt Sql)
- **C++17**: Modern language features and standard library

## Testing Strategy

### Unit Tests
- Protocol-specific serialization/deserialization
- DataFormat conversion accuracy
- Database operations and query correctness
- File I/O operations

### Integration Tests
- End-to-end message flow
- Multi-protocol communication
- Database persistence and retrieval
- Export functionality

### Manual Testing
- GUI usability and responsiveness
- Theme switching and persistence
- Error handling and recovery
- Cross-platform compatibility

---

This architecture provides a solid foundation for network communication testing while maintaining flexibility for future enhancements and protocol additions.
