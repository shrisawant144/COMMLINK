# CommLink Architecture

## System Overview

CommLink follows a modular, event-driven architecture built on Qt's signal-slot mechanism. The application separates concerns into distinct layers: presentation (UI), business logic (core), and communication (network).

### Design Principles

- **Modularity**: Independent components with well-defined interfaces
- **Asynchronous I/O**: Non-blocking operations for responsive UI
- **Type Safety**: Strong typing with Qt's meta-object compiler
- **Format Agnostic**: Unified interface for multiple data formats

## Component Diagram

```
┌─────────────────────────────────────────┐
│           GUI (Main Window)             │
│  ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐  │
│  │ Send │ │Receive│ │History│ │ Logs │  │
│  └──────┘ └──────┘ └──────┘ └──────┘  │
└─────────────────────────────────────────┘
         │              │              │
         ▼              ▼              ▼
┌──────────────┐ ┌──────────────┐ ┌──────────┐
│   Sender     │ │   Receiver   │ │  Logger  │
│   Thread     │ │   Thread     │ └──────────┘
└──────────────┘ └──────────────┘       │
         │              │                ▼
         └──────┬───────┘         ┌──────────────┐
                ▼                 │   History    │
        ┌──────────────┐          │   Manager    │
        │  DataFormat  │          └──────────────┘
        │   Handler    │                │
        └──────────────┘                ▼
                │                 ┌──────────────┐
                ▼                 │  SQLite DB   │
        ┌──────────────┐          └──────────────┘
        │   Network    │
        │ (TCP/UDP/    │
        │ HTTP/WS)     │
        └──────────────┘
```

## Core Components

### 1. Network Layer

Implements protocol-specific communication logic.

#### TCP Client/Server
- **Purpose**: Reliable, ordered data transmission
- **Features**: Connection management, error recovery, keep-alive
- **Classes**: `TcpClient`, `TcpServer`

#### UDP Client/Server
- **Purpose**: Low-latency, connectionless messaging
- **Features**: Datagram transmission, broadcast support
- **Classes**: `UdpClient`, `UdpServer`

#### HTTP Client/Server
- **Purpose**: RESTful API communication
- **Features**: GET/POST/PUT/DELETE, headers, status codes, long-polling
- **Classes**: `HttpClient`, `HttpServer`

#### WebSocket Client/Server
- **Purpose**: Full-duplex real-time communication
- **Features**: Persistent connections, event-driven messaging
- **Classes**: `WebSocketClient`, `WebSocketServer`

### 2. Data Format Handler

Provides unified serialization interface for multiple formats.

**Core Class**: `DataFormat`

**Capabilities**:
- Format validation (JSON, XML, CSV, HEX)
- Bi-directional conversion (object ↔ bytes)
- Error reporting with line numbers
- Display formatting

**Example**:
```cpp
DataMessage msg(DataFormatType::JSON, jsonObject);
QByteArray serialized = msg.serialize();
// Transmit over network
DataMessage received = DataMessage::deserialize(bytes, DataFormatType::JSON);
```

### 3. Message History Manager

Manages persistent storage and retrieval of messages.

**Features**:
- SQLite database with indexing
- Full-text search
- Advanced filtering (date, protocol, direction)
- Session tracking
- Thread-safe operations

**Database Schema**:
```sql
CREATE TABLE messages (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp TEXT NOT NULL,
    direction TEXT NOT NULL,      -- 'sent' or 'received'
    protocol TEXT NOT NULL,        -- 'TCP', 'UDP', 'HTTP', 'WebSocket'
    format TEXT NOT NULL,          -- 'JSON', 'XML', etc.
    host TEXT NOT NULL,
    port INTEGER NOT NULL,
    content TEXT NOT NULL,
    session_id TEXT NOT NULL,
    status TEXT NOT NULL           -- 'success' or 'error'
);

CREATE INDEX idx_timestamp ON messages(timestamp);
CREATE INDEX idx_search ON messages(content, protocol, direction);
```

### 4. User Interface Layer

Qt Widgets-based GUI with tabbed interface.

**Main Components**:
- **MainWindow**: Application container
- **ConnectionPanel**: Protocol and endpoint configuration
- **MessagePanel**: Message composition and sending
- **DisplayPanel**: Received message display
- **HistoryTab**: Search and browse past messages
- **LogsTab**: Application activity monitoring
- **ThemeManager**: Appearance management (singleton)

## Data Flow

### Sending Message Flow
```
User Input → GUI → Format Validation → DataFormat Serialization
                                              ↓
                                        Sender Thread
                                              ↓
                                      Network Transmission
                                              ↓
                                    Logger + History Manager
```

### Receiving Message Flow
```
Network → Receiver Thread → DataFormat Deserialization
                                     ↓
                              Format Validation
                                     ↓
                            GUI Display Update
                                     ↓
                          Logger + History Manager
```

## Design Patterns

### Observer Pattern (Qt Signals/Slots)
```cpp
// Sender emits signal
emit dataReceived(message, source);

// GUI connects slot
connect(receiver, &Receiver::dataReceived,
        this, &GUI::onDataReceived);
```

### Strategy Pattern (DataFormat)
```cpp
class DataFormat {
    virtual QByteArray serialize() const = 0;
    virtual void deserialize(const QByteArray&) = 0;
};

class JSONFormat : public DataFormat { /*...*/ };
class XMLFormat : public DataFormat { /*...*/ };
```

### Singleton Pattern (ThemeManager)
```cpp
ThemeManager& ThemeManager::instance() {
    static ThemeManager instance;  // Thread-safe in C++11
    return instance;
}
```

### Factory Pattern (Network Components)
```cpp
std::unique_ptr<NetworkClient> createClient(Protocol protocol) {
    switch (protocol) {
        case TCP: return std::make_unique<TcpClient>();
        case UDP: return std::make_unique<UdpClient>();
        case HTTP: return std::make_unique<HttpClient>();
        case WS: return std::make_unique<WebSocketClient>();
    }
}
```

## Threading Model

### Main Thread
- Qt event loop
- UI rendering and interactions
- Signal/slot coordination

### Sender Thread
- Outgoing connection management
- Message transmission
- Connection state monitoring

### Receiver Thread
- Incoming connection acceptance
- Message reception
- Background listening

### Database Thread (Future)
- Asynchronous database operations
- Background indexing

## Performance Considerations

### Optimizations
- Asynchronous I/O prevents UI blocking
- Database indexing for fast searches
- Connection pooling for repeated operations
- Lazy loading of history data
- Efficient format conversion with minimal copying

### Resource Management
- Qt parent-child automatic memory management
- RAII for network socket cleanup
- Smart pointers for non-Qt objects
- Prepared SQL statements for performance

## Security Architecture

### Current Posture
⚠️ **Development/Testing Focus**
- No encryption (plaintext transmission)
- No authentication mechanisms
- Suitable for trusted networks only

✅ **Implemented**
- Input validation prevents injection
- Resource limits prevent DoS
- Safe SQL queries (parameterized)

### Future Enhancements
- TLS/SSL support for encrypted communication
- API key authentication
- Certificate validation
- Rate limiting
- Secure WebSocket (WSS)

## Error Handling

### Hierarchical Strategy

1. **Network Layer**: Connection failures, timeouts, protocol errors
2. **Format Layer**: Serialization errors, validation failures
3. **UI Layer**: User feedback, graceful degradation
4. **Database Layer**: Transaction rollback, connection recovery

### Error Recovery
- Automatic reconnection with exponential backoff
- Transaction isolation and rollback
- Detailed error logging for debugging
- User-friendly error messages

## Build System (CMake)

### Structure
```cmake
add_library(commlink_core STATIC
    src/core/dataformat.cpp
    src/core/logger.cpp
    # ...
)

add_library(commlink_network STATIC
    src/network/tcpclient.cpp
    src/network/tcpserver.cpp
    # ...
)

add_library(commlink_ui STATIC
    src/ui/gui.cpp
    src/ui/historytab.cpp
    # ...
)

add_executable(commlink
    src/main.cpp
)

target_link_libraries(commlink
    commlink_core
    commlink_network
    commlink_ui
    Qt5::Widgets
    Qt5::Network
    Qt5::Sql
    Qt5::WebSockets
)
```

### Benefits
- Modular compilation
- Faster incremental builds
- Clear dependency management
- Easy testing of individual components

## Testing Strategy

### Unit Tests
- Protocol serialization/deserialization
- Format conversion accuracy
- Database CRUD operations
- Input validation logic

### Integration Tests
- End-to-end message flow
- Multi-protocol communication
- History persistence and retrieval
- Export functionality

### Manual GUI Testing
- Usability and responsiveness
- Theme switching
- Cross-platform compatibility
- Error handling and recovery

## Extensibility Points

### Adding New Protocols
1. Implement client/server classes
2. Integrate with DataFormat system
3. Register in GUI protocol selector
4. Update database schema if needed

### Adding New Formats
1. Extend DataFormat handler
2. Add validation logic
3. Update UI format dropdown
4. Implement export capability

### Plugin System (Future)
- Dynamic protocol loading
- Custom format handlers
- Theme plugins
- Automation scripts

## Dependencies

### Core Dependencies
- **Qt5**: 5.12+ (Widgets, Network, SQL, WebSockets)
- **SQLite3**: Embedded via Qt SQL
- **C++17**: Modern language features

### Build Dependencies
- **CMake**: 3.10+
- **C++ Compiler**: GCC 7+, Clang 5+, MSVC 2017+

### Optional Dependencies
- **Doxygen**: API documentation generation
- **CTest**: Test framework integration
- **clang-format**: Code formatting

---

**For Implementation Details**: See [Source_Code_Analysis.md](Source_Code_Analysis.md)

**For Usage Instructions**: See [user-guide.md](user-guide.md)

**For Learning Path**: See [Beginners_Complete_Guide.md](Beginners_Complete_Guide.md)
