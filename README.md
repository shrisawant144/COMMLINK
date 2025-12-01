# CommLink# CommLink



[![CI](https://github.com/shrisawant144/COMMLINK/workflows/CI/badge.svg)](https://github.com/shrisawant144/COMMLINK/actions)[![CI](https://github.com/shrisawant144/COMMLINK/workflows/CI/badge.svg)](https://github.com/shrisawant144/COMMLINK/actions)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](VERSION)[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](VERSION)

[![Qt](https://img.shields.io/badge/Qt-5.12%2B-green.svg)](https://www.qt.io/)

[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)Professional network communication tool with multi-format message support.



**CommLink** is a cross-platform desktop application for testing and debugging network communications. It provides a unified interface for working with multiple network protocols (TCP, UDP, HTTP, WebSocket) and data formats (JSON, XML, CSV, TEXT, BINARY, HEX).## Features



---- 🔄 TCP/UDP protocol support

- 📊 Multiple data formats (JSON, XML, CSV, TEXT, BINARY, HEX)

## ✨ Features- 📝 Real-time logging and monitoring

- 💾 Persistent message history with SQLite

### Multi-Protocol Support- 🎨 Customizable themes

- 🔌 **TCP Client/Server** - Reliable, connection-oriented communication- 📤 Export capabilities (TXT, CSV, JSON)

- 📡 **UDP Client/Server** - Fast, connectionless messaging

- 🌐 **HTTP Client/Server** - RESTful API testing (GET, POST, PUT, DELETE)## Quick Start

- 💬 **WebSocket Client/Server** - Real-time bidirectional communication

```bash

### Data Format Flexibility# Install dependencies

- 📊 **JSON** - JavaScript Object Notation for APIs./scripts/install-deps.sh

- 📄 **XML** - Extensible Markup Language for legacy systems

- 📈 **CSV** - Comma-Separated Values for tabular data# Build

- 📝 **TEXT** - Plain text messages./scripts/build.sh

- 🔢 **BINARY** - Raw byte data

- 🔤 **HEX** - Hexadecimal representation# Run

./build/bin/commlink

### Core Capabilities```

- 💾 **Persistent History** - SQLite database stores all messages

- 🔍 **Advanced Search** - Full-text search with filtering by protocol, date, format## Documentation

- 📤 **Export Options** - Export logs and messages in TXT, CSV, JSON formats

- 📁 **File Operations** - Load/save messages with auto-format detection- [Architecture](docs/architecture.md)

- 📋 **Real-Time Logging** - Monitor all activity with timestamps- [User Guide](docs/user-guide.md)

- 🎨 **Theme Support** - Light, Dark, and System themes- [API Documentation](docs/api/)

- 📊 **Session Tracking** - Group related messages together- [Contributing](CONTRIBUTING.md)

- ⚡ **Asynchronous I/O** - Non-blocking operations keep UI responsive

## Requirements

---

- CMake 3.16+

## 🚀 Quick Start- Qt5 (Core, Widgets, Sql)

- C++17 compiler (GCC 7+, Clang 5+)

### Prerequisites

## Building

- **CMake** 3.10 or newer

- **Qt5** 5.12 or newer (Core, Widgets, Network, Sql, WebSockets)```bash

- **C++17 Compiler** (GCC 7+, Clang 5+, MSVC 2017+)mkdir build && cd build

cmake -DCMAKE_BUILD_TYPE=Release ..

### Installationcmake --build . -j$(nproc)

```

#### Linux (Ubuntu/Debian)

## Testing

```bash

# Install dependencies```bash

sudo apt-get updatecd build

sudo apt-get install build-essential cmake gitctest --output-on-failure

sudo apt-get install qt5-default libqt5websockets5-dev```



# Clone repository## Project Structure

git clone https://github.com/shrisawant144/COMMLINK.git

cd CommLink```

commlink/

# Build├── include/commlink/    # Public headers

mkdir build && cd build├── src/                 # Implementation

cmake ..├── tests/               # Unit & integration tests

make -j$(nproc)├── docs/                # Documentation

├── resources/           # Assets

# Run└── scripts/             # Build utilities

./bin/commlink```

```

## License

#### Linux (Fedora/RHEL)

MIT License - see [LICENSE](LICENSE) file

```bash

# Install dependenciesCopyright (c) 2025 Shrikrishna Sawant

sudo dnf install gcc gcc-c++ cmake git

sudo dnf install qt5-qtbase-devel qt5-qtwebsockets-devel## Third-Party Software



# Build and run (same as above)This project uses the Qt Framework (LGPL v3) for GUI components. Qt is a trademark of The Qt Company Ltd. For more information, visit [qt.io](https://www.qt.io/).

```

## Author

#### Windows

**Shrikrishna Sawant**

```powershell- GitHub: [@shrisawant144](https://github.com/shrisawant144)

# Install Qt5 from https://www.qt.io/download-qt-installer

# Install CMake from https://cmake.org/download/## Contributing



# Clone repositorySee [CONTRIBUTING.md](CONTRIBUTING.md) for development guidelines.

git clone https://github.com/shrisawant144/COMMLINK.git
cd CommLink

# Configure with CMake GUI or command line
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release

# Run
.\bin\Release\commlink.exe
```

#### macOS

```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake qt5

# Build and run (same as Linux)
```

### Using Build Scripts

```bash
# Install dependencies (Linux)
./scripts/install-deps.sh

# Build application
./scripts/build.sh

# Run application
./build/bin/commlink
```

---

## 📖 Usage Examples

### TCP Communication

```
1. Select TCP/UDP tab
2. Protocol: TCP, Mode: Server, Port: 5000
3. Click "Start Server"
4. In another instance: Mode: Client, Host: 127.0.0.1, Port: 5000
5. Click "Connect"
6. Send messages back and forth
```

### HTTP API Testing

```
1. Select HTTP tab
2. Method: POST
3. URL: http://api.example.com/users
4. Headers: Content-Type: application/json
5. Body: {"name": "Alice", "email": "alice@example.com"}
6. Click "Send Request"
7. View response with status code and body
```

### WebSocket Real-Time Chat

```
1. Select WebSocket tab
2. URL: ws://localhost:8080/chat
3. Click "Connect"
4. Send JSON messages: {"type": "message", "text": "Hello!"}
5. Receive real-time responses
```

### Message History

```
1. Go to History tab
2. Use search box to find specific messages
3. Filter by protocol, date range, format
4. Export filtered results to CSV or JSON
```

---

## 📁 Project Structure

```
CommLink/
├── src/
│   ├── core/                      # Core functionality
│   │   ├── dataformat.cpp         # Multi-format serialization
│   │   ├── filemanager.cpp        # File I/O operations
│   │   ├── exportmanager.cpp      # Export functionality
│   │   ├── logger.cpp             # Logging system
│   │   └── messagehistorymanager.cpp  # Database operations
│   │
│   ├── network/                   # Protocol implementations
│   │   ├── tcpclient.cpp          # TCP client
│   │   ├── tcpserver.cpp          # TCP server
│   │   ├── udpclient.cpp          # UDP client
│   │   ├── udpserver.cpp          # UDP server
│   │   ├── httpclient.cpp         # HTTP client
│   │   ├── httpserver.cpp         # HTTP server
│   │   ├── websocketclient.cpp    # WebSocket client
│   │   └── websocketserver.cpp    # WebSocket server
│   │
│   ├── ui/                        # User interface
│   │   ├── gui.cpp                # Main window and tabs
│   │   ├── historytab.cpp         # History interface
│   │   └── thememanager.cpp       # Theme management
│   │
│   └── main.cpp                   # Application entry point
│
├── include/commlink/              # Public headers
│   ├── core/
│   ├── network/
│   └── ui/
│
├── tests/                         # Test suite
│   ├── unit/                      # Unit tests
│   └── selenium/                  # Integration tests
│
├── docs/                          # Comprehensive documentation
│   ├── architecture.md            # System design
│   ├── Project_Overview.md        # Project overview
│   ├── user-guide.md              # Complete user manual
│   ├── Beginners_Complete_Guide.md    # Learning guide
│   ├── CPP_Programming_Concepts.md    # C++ concepts
│   ├── Qt_Framework_Features.md       # Qt usage
│   ├── GUI_Implementation_Guide.md    # GUI details
│   └── Source_Code_Analysis.md        # Code walkthrough
│
├── resources/                     # Application resources
│   ├── CommLink.qrc               # Qt resource file
│   └── logo/                      # Application icons
│
├── scripts/                       # Build and utility scripts
│   ├── build.sh                   # Build script
│   ├── format.sh                  # Code formatting
│   ├── install-deps.sh            # Dependency installation
│   ├── setup-linux.sh             # Linux setup
│   └── setup-windows.ps1          # Windows setup
│
├── CMakeLists.txt                 # Build configuration
├── CONTRIBUTING.md                # Contribution guidelines
├── LICENSE                        # MIT License
├── README.md                      # This file
└── VERSION                        # Version information
```

---

## 🧪 Testing

### Run Unit Tests

```bash
cd build
ctest --verbose

# Or run specific test
./tests/unit/test_dataformat
```

### Manual GUI Testing

```bash
# Follow test scenarios in docs/GUI_Implementation_Guide.md
./bin/commlink
```

---

## 📚 Documentation

### User Documentation
- **[User Guide](docs/user-guide.md)** - Complete usage instructions with examples
- **[Beginner's Guide](docs/Beginners_Complete_Guide.md)** - Step-by-step learning path
- **[Project Overview](docs/Project_Overview.md)** - High-level project understanding

### Developer Documentation
- **[Architecture](docs/architecture.md)** - System design and patterns
- **[C++ Concepts](docs/CPP_Programming_Concepts.md)** - C++ features used
- **[Qt Framework](docs/Qt_Framework_Features.md)** - Qt components explained
- **[GUI Implementation](docs/GUI_Implementation_Guide.md)** - UI architecture
- **[Source Code Analysis](docs/Source_Code_Analysis.md)** - Detailed code walkthrough

### Contributing
- **[Contributing Guidelines](CONTRIBUTING.md)** - How to contribute to the project

---

## 🎯 Use Cases

### Development & Testing
- ✅ Test REST APIs during development
- ✅ Debug microservice communication
- ✅ Validate WebSocket implementations
- ✅ Test IoT device protocols

### Learning & Education
- ✅ Understand network protocols (TCP vs UDP)
- ✅ Learn data formats (JSON, XML, CSV)
- ✅ Experiment with client-server architecture
- ✅ Practice HTTP REST API concepts

### Professional Use
- ✅ QA testing of network features
- ✅ Integration testing between services
- ✅ Performance analysis and debugging
- ✅ Protocol compliance verification

---

## 🔧 Requirements

### Build Requirements
- **CMake**: 3.10 or newer
- **C++ Compiler**: Supporting C++17
  - GCC 7+ (Linux)
  - Clang 5+ (macOS)
  - MSVC 2017+ (Windows)

### Qt5 Modules Required
- Qt5Core
- Qt5Widgets
- Qt5Network
- Qt5Sql
- Qt5WebSockets

### Runtime Requirements
- Operating System: Linux, Windows 10+, macOS 10.14+
- Minimum RAM: 512 MB
- Disk Space: 100 MB

---

## 🛠️ Building from Source

### Standard Build

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Debug Build

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

### Release Build with Optimizations

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### Build with Tests

```bash
mkdir build && cd build
cmake -DBUILD_TESTING=ON ..
make -j$(nproc)
ctest
```

---

## 🤝 Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for:

- Development setup instructions
- Coding standards and style guide
- Testing guidelines
- Pull request process
- Code of conduct

### Quick Contribution Steps

1. Fork the repository
2. Create feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'feat: add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

---

## 📝 Changelog

See [CHANGELOG.md](CHANGELOG.md) for a list of notable changes in each version.

**Current Version**: 1.0.0 (December 2025)
- Initial release with TCP, UDP, HTTP, WebSocket support
- Multi-format data handling (JSON, XML, CSV, TEXT, BINARY, HEX)
- SQLite message history with advanced search
- Theme support (Light/Dark/System)
- Export capabilities

---

## 🐛 Reporting Issues

Found a bug or have a feature request? Please:

1. Check [existing issues](https://github.com/shrisawant144/COMMLINK/issues)
2. Create a new issue with:
   - Clear title and description
   - Steps to reproduce (for bugs)
   - Expected vs actual behavior
   - System information (OS, Qt version, etc.)
   - Relevant logs or screenshots

---

## 📄 License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

### Third-Party Software

CommLink uses the following open-source software:

- **Qt Framework** (LGPL v3) - GUI and networking components
  - Copyright © The Qt Company Ltd.
  - Website: [qt.io](https://www.qt.io/)
  
- **SQLite** (Public Domain) - Embedded database
  - Website: [sqlite.org](https://www.sqlite.org/)

---

## 👨‍💻 Author

**Shrikrishna Sawant**

- GitHub: [@shrisawant144](https://github.com/shrisawant144)
- Project: [CommLink](https://github.com/shrisawant144/COMMLINK)

---

## 🌟 Show Your Support

If you find CommLink useful, please consider:

- ⭐ Starring the repository
- 🐛 Reporting bugs or requesting features
- 🤝 Contributing code or documentation
- 📢 Sharing with others

---

## 🙏 Acknowledgments

- Qt Framework team for excellent cross-platform toolkit
- Open-source community for inspiration and support
- All contributors who help improve CommLink

---

## 📞 Support

- **Documentation**: Check [docs/](docs/) folder
- **Issues**: [GitHub Issues](https://github.com/shrisawant144/COMMLINK/issues)
- **Discussions**: [GitHub Discussions](https://github.com/shrisawant144/COMMLINK/discussions)

---

**Happy Testing!** 🚀

Built with ❤️ using Qt and C++17
