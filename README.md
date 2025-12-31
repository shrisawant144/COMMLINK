# CommLink

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](VERSION)
[![Qt](https://img.shields.io/badge/Qt-5.12%2B-green.svg)](https://www.qt.io/)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)

**CommLink** is a cross-platform desktop application for testing and debugging network communications. It provides a unified interface for working with multiple network protocols and data formats.

## Features

### Multi-Protocol Support
- **TCP Client/Server** - Reliable, connection-oriented communication
- **UDP Client/Server** - Fast, connectionless messaging
- **HTTP Client/Server** - RESTful API testing (GET, POST, PUT, DELETE)
- **WebSocket Client/Server** - Real-time bidirectional communication

### Data Format Flexibility
- **JSON, XML, CSV, TEXT, BINARY, HEX** - Multiple format support with automatic conversion

### Core Capabilities
- **Persistent History** - SQLite database with full-text search
- **Export Options** - Export messages in TXT, CSV, JSON formats
- **File Operations** - Load/save messages with auto-format detection
- **Real-Time Logging** - Monitor all activity with timestamps
- **Theme Support** - Light, Dark, and System themes
- **Session Tracking** - Group related messages together

## Quick Start

```bash
# Install dependencies
./scripts/install-deps.sh

# Build
./scripts/build.sh

# Run
./build/bin/commlink
```

## Documentation

📚 **[Complete Documentation](docs/README.md)** - Start here for all guides

**Quick Links:**
- 🚀 [Quick Start](docs/quick-start.md) - Get running in 5 minutes
- 📖 [User Guide](docs/user-guide-simple.md) - How to use CommLink
- 💻 [Developer Guide](docs/developer-guide.md) - Build and contribute
- 🏗️ [Architecture](docs/ARCHITECTURE.md) - System design
- 🔄 [Code Flow](docs/CODE_FLOW.md) - Execution flows
- 📚 [API Documentation](docs/DOXYGEN.md) - Doxygen API docs

## Requirements

### Build Requirements
- **CMake**: 3.8.2 or newer
- **C++ Compiler**: Supporting C++17 (GCC 7+, Clang 5+, MSVC 2017+)
- **Qt5**: Core, Widgets, Network, Sql, WebSockets

### Runtime Requirements
- Linux, Windows 10+, or macOS 10.14+
- 512 MB RAM minimum
- 100 MB disk space

## Building from Source

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

For debug builds:
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

## Use Cases

- **Development & Testing** - Test REST APIs, debug microservice communication
- **Learning & Education** - Understand network protocols and data formats
- **Professional Use** - QA testing, integration testing, protocol verification

## Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for:
- Development setup instructions
- Coding standards and style guide
- Testing guidelines
- Pull request process

## License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

## Author

**Shrikrishna Sawant**
- GitHub: [@shrisawant144](https://github.com/shrisawant144)
- Project: [CommLink](https://github.com/shrisawant144/COMMLINK)

## Support

- **Documentation**: [docs/](docs/)
- **Issues**: [GitHub Issues](https://github.com/shrisawant144/COMMLINK/issues)
- **Discussions**: [GitHub Discussions](https://github.com/shrisawant144/COMMLINK/discussions)

---

Built with ❤️ using Qt and C++17
