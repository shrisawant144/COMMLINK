# Developer Guide

## Building CommLink

### Prerequisites
- **CMake** 3.8.2+
- **Qt5** 5.12+ (Core, Widgets, Network, Sql, WebSockets)
- **C++17** compiler (GCC 7+, Clang 5+, MSVC 2017+)

### Quick Build
```bash
git clone https://github.com/shrisawant144/COMMLINK.git
cd COMMLINK
./scripts/build.sh
```

### Manual Build
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j$(nproc)
```

## Project Structure

```
CommLink/
├── src/
│   ├── main.cpp                    # Application entry point
│   ├── core/                       # Core functionality
│   │   ├── dataformat.cpp          # Format conversions (JSON/XML/CSV/etc)
│   │   ├── filemanager.cpp         # File I/O operations
│   │   ├── exportmanager.cpp       # Export functionality
│   │   ├── logger.cpp              # Logging system
│   │   └── messagehistorymanager.cpp  # SQLite database operations
│   ├── network/                    # Protocol implementations
│   │   ├── tcpclient.cpp           # TCP client
│   │   ├── tcpserver.cpp           # TCP server
│   │   ├── udpclient.cpp           # UDP client
│   │   ├── udpserver.cpp           # UDP server
│   │   ├── httpclient.cpp          # HTTP client
│   │   ├── httpserver.cpp          # HTTP server
│   │   ├── websocketclient.cpp     # WebSocket client
│   │   └── websocketserver.cpp     # WebSocket server
│   └── ui/                         # User interface panels
│       ├── mainwindow.cpp          # Main application window
│       ├── gui.cpp                 # Legacy GUI (being phased out)
│       ├── connectionpanel.cpp     # Client connection controls
│       ├── serverpanel.cpp         # Server management controls
│       ├── messagepanel.cpp        # Message composition
│       ├── displaypanel.cpp        # Message display with tabs
│       ├── statuspanel.cpp         # Status information
│       ├── historytab.cpp          # Message history interface
│       └── thememanager.cpp        # Theme management
├── include/commlink/               # Header files (same structure as src/)
├── tests/
│   └── unit/                       # Unit tests
│       ├── test_dataformat.cpp
│       └── test_filemanager.cpp
├── resources/                      # Application resources
│   ├── CommLink.qrc               # Qt resource file
│   ├── logo/                      # Application icons
│   └── controls/                  # UI control assets
├── scripts/                       # Build and utility scripts
│   ├── build.sh                   # Main build script
│   ├── install-deps.sh            # Dependency installation
│   ├── setup-linux.sh             # Linux setup
│   └── setup-windows.ps1          # Windows setup
├── docs/                          # Documentation
├── cmake/                         # CMake modules
├── CMakeLists.txt                 # Build configuration
└── README.md
```

## Key Classes

### Network Components
All network classes inherit from QObject and follow similar patterns:

```cpp
class TcpClient : public QObject {
    Q_OBJECT
public:
    void connectToHost(const QString& host, quint16 port);
    void sendMessage(const DataMessage& message);
    void setFormat(DataFormatType format);
    bool isConnected() const;
signals:
    void connected();
    void disconnected();
    void messageReceived(const DataMessage& message, const QString& source);
    void errorOccurred(const QString& error);
};
```

### UI Panels
Panel-based architecture with specialized components:

```cpp
class ConnectionPanel : public QWidget {
    Q_OBJECT
public:
    QString getProtocol() const;
    QString getHost() const;
    int getPort() const;
    void setConnectionState(bool connected);
signals:
    void connectRequested();
    void protocolChanged(const QString& protocol);
};
```

### DataFormat
Handles format conversions:
```cpp
enum class DataFormatType {
    JSON, XML, CSV, TEXT, BINARY, HEX
};

class DataMessage {
public:
    DataFormatType type;
    QVariant data;
    
    DataMessage(DataFormatType t = DataFormatType::TEXT, const QVariant& d = QVariant());
    QByteArray serialize() const;
    static DataMessage deserialize(const QByteArray& bytes, DataFormatType type);
};
```

## Adding Features

### New Protocol Support
1. Create client/server classes inheriting from QObject
2. Implement standard signals: connected(), disconnected(), messageReceived(), errorOccurred()
3. Add to MainWindow network component initialization
4. Update ConnectionPanel and ServerPanel protocol lists

### New UI Panel
1. Create class inheriting from QWidget
2. Add to MainWindow setupUI() method
3. Connect signals to MainWindow slots
4. Update layout in MainWindow

## Testing

### Unit Tests
```bash
cd build
ctest --verbose
```

### Manual Testing
1. Build debug version: `cmake -DCMAKE_BUILD_TYPE=Debug ..`
2. Run application: `./bin/commlink`
3. Test each panel thoroughly:
   - Connection Panel: Try all protocols
   - Server Panel: Start/stop servers
   - Message Panel: Test all data formats
   - Display Panel: Check all tabs

## Code Style

### Naming Conventions
- **Classes**: PascalCase (`TcpClient`, `MessagePanel`)
- **Methods**: camelCase (`sendMessage`, `getProtocol`)
- **Variables**: camelCase (`messageCount`, `connectionPanel`)
- **Constants**: UPPER_CASE (`DEFAULT_WIDTH`, `MIN_HEIGHT`)
- **Signals**: camelCase (`connectRequested`, `messageReceived`)

### Qt Conventions
- Use Qt containers (`QList`, `QMap`) over STL
- Prefer `QString` over `std::string`
- Use Qt's signal/slot mechanism for events
- Follow Qt's memory management (parent-child ownership)
- Use `Q_OBJECT` macro for classes with signals/slots

## Debugging Tips

### Network Issues
- Enable Qt logging: `QT_LOGGING_RULES="*.debug=true"`
- Check firewall and port availability
- Use Wireshark for packet analysis

### GUI Issues  
- Test with different themes (Light/Dark/System)
- Verify panel layouts on different screen sizes
- Check signal/slot connections between panels
- Validate input data in panels before processing

### Database Issues
- Check SQLite file permissions in user directory
- Use DB Browser for SQLite to inspect database
- Enable SQL query logging in MessageHistoryManager
- Verify database schema matches current version

## Contributing

1. Fork repository
2. Create feature branch: `git checkout -b feature/new-feature`
3. Follow code style guidelines
4. Add tests for new functionality
5. Update documentation
6. Submit pull request

## Release Process

1. Update version in `VERSION` file
2. Update `CHANGELOG.md`
3. Create git tag: `git tag v1.0.1`
4. Build release binaries
5. Create GitHub release with binaries
