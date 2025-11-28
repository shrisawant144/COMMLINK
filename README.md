# CommLink

[![CI](https://github.com/username/commlink/workflows/CI/badge.svg)](https://github.com/username/commlink/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](VERSION)

Professional network communication tool with multi-format message support.

## Features

- 🔄 TCP/UDP protocol support
- 📊 Multiple data formats (JSON, XML, CSV, TEXT, BINARY, HEX)
- 📝 Real-time logging and monitoring
- 💾 Persistent message history with SQLite
- 🎨 Customizable themes
- 📤 Export capabilities (TXT, CSV, JSON)

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

- [Architecture](docs/architecture.md)
- [User Guide](docs/user-guide.md)
- [API Documentation](docs/api/)
- [Contributing](CONTRIBUTING.md)

## Requirements

- CMake 3.16+
- Qt5 (Core, Widgets, Sql)
- C++17 compiler (GCC 7+, Clang 5+)

## Building

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j$(nproc)
```

## Testing

```bash
cd build
ctest --output-on-failure
```

## Project Structure

```
commlink/
├── include/commlink/    # Public headers
├── src/                 # Implementation
├── tests/               # Unit & integration tests
├── docs/                # Documentation
├── resources/           # Assets
└── scripts/             # Build utilities
```

## License

MIT License - see [LICENSE](LICENSE) file

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for development guidelines.
