# CommLink

[![CI](https://github.com/shrisawant144/COMMLINK/workflows/CI/badge.svg)](https://github.com/shrisawant144/COMMLINK/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](VERSION)
[![Qt](https://img.shields.io/badge/Qt-5.12%2B-green.svg)](https://www.qt.io/)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)

**CommLink** is a cross-platform desktop application for testing and debugging network communications. It provides a unified panel-based interface for working with multiple network protocols and data formats.

## Features

### Multi-Protocol Support
- 🔌 **TCP Client/Server** - Reliable, connection-oriented communication
- 📡 **UDP Client/Server** - Fast, connectionless messaging
- 🌐 **HTTP Client/Server** - RESTful API testing (GET, POST, PUT, DELETE)
- 💬 **WebSocket Client/Server** - Real-time bidirectional communication

### Data Format Flexibility
- 📊 **JSON** - JavaScript Object Notation for APIs
- 📄 **XML** - Extensible Markup Language for legacy systems
- 📈 **CSV** - Comma-Separated Values for tabular data
- 📝 **TEXT** - Plain text messages
- 🔢 **BINARY** - Raw byte data
- 🔤 **HEX** - Hexadecimal representation

### Core Capabilities
- 💾 **Persistent History** - SQLite database stores all messages
- 🔍 **Advanced Search** - Full-text search with filtering by protocol, date, format
- 📤 **Export Options** - Export logs and messages in TXT, CSV, JSON formats
- 📁 **File Operations** - Load/save messages with auto-format detection
- 📋 **Real-Time Logging** - Monitor all activity with timestamps
- 🎨 **Theme Support** - Light, Dark, and System themes
- 📊 **Session Tracking** - Group related messages together
- ⚡ **Asynchronous I/O** - Non-blocking operations keep UI responsive

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
- 🎓 [Learning Guide](docs/learning-guide.md) - Networking concepts explained
- 💻 [Developer Guide](docs/developer-guide.md) - Build and contribute
- ❓ [FAQ](docs/faq.md) - Common questions answered

## Requirements

### Build Requirements
- **CMake**: 3.8.2 or newer
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

## Building from Source

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

### Build with Tests
```bash
mkdir build && cd build
cmake -DBUILD_TESTING=ON ..
make -j$(nproc)
ctest
```

## Use Cases

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

## Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for:

- Development setup instructions
- Coding standards and style guide
- Testing guidelines
- Pull request process

### Quick Contribution Steps

1. Fork the repository
2. Create feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'feat: add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

### Third-Party Software

CommLink uses the following open-source software:

- **Qt Framework** (LGPL v3) - GUI and networking components
  - Copyright © The Qt Company Ltd.
  - Website: [qt.io](https://www.qt.io/)
  
- **SQLite** (Public Domain) - Embedded database
  - Website: [sqlite.org](https://www.sqlite.org/)

## Author

**Shrikrishna Sawant**

- GitHub: [@shrisawant144](https://github.com/shrisawant144)
- Project: [CommLink](https://github.com/shrisawant144/COMMLINK)

## Support

- **Documentation**: Check [docs/](docs/) folder
- **Issues**: [GitHub Issues](https://github.com/shrisawant144/COMMLINK/issues)
- **Discussions**: [GitHub Discussions](https://github.com/shrisawant144/COMMLINK/discussions)

---

**Happy Testing!** 🚀

Built with ❤️ using Qt and C++17
