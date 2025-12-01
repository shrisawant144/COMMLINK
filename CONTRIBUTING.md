# Contributing to CommLink# Contributing to CommLink



Thank you for your interest in contributing to CommLink! This document provides guidelines and instructions for contributing to the project.## Development Setup



---```bash

# Clone repository

## Table of Contentsgit clone <repository-url>

cd commlink

1. [Code of Conduct](#code-of-conduct)

2. [Getting Started](#getting-started)# Install dependencies

3. [Development Setup](#development-setup)./scripts/install-deps.sh

4. [Project Structure](#project-structure)

5. [Coding Standards](#coding-standards)# Build

6. [Testing Guidelines](#testing-guidelines)./scripts/build.sh

7. [Commit Message Guidelines](#commit-message-guidelines)

8. [Pull Request Process](#pull-request-process)# Run tests

9. [Documentation](#documentation)cd build && ctest

10. [Reporting Issues](#reporting-issues)```



---## Code Style



## Code of Conduct- Follow C++17 standards

- Use clang-format for formatting: `./scripts/format.sh`

### Our Pledge- Run static analysis before committing

- Write unit tests for new features

We are committed to providing a welcoming and inclusive environment for all contributors. We expect:

## Commit Messages

- **Respect**: Treat everyone with respect and dignity

- **Inclusivity**: Welcome diverse perspectives and experiencesFollow conventional commits:

- **Collaboration**: Work together constructively- `feat:` New feature

- **Professionalism**: Maintain professional communication- `fix:` Bug fix

- `docs:` Documentation changes

### Unacceptable Behavior- `test:` Test additions/changes

- `refactor:` Code refactoring

- Harassment, discrimination, or offensive comments- `chore:` Maintenance tasks

- Personal attacks or trolling

- Spam or self-promotion## Pull Request Process

- Violation of privacy or confidentiality

1. Create feature branch from `develop`

---2. Write tests for new functionality

3. Ensure all tests pass

## Getting Started4. Update documentation

5. Submit PR with clear description

### Prerequisites6. Address review comments



Before contributing, ensure you have:## Code Review Guidelines



- **C++ Compiler**: GCC 7+, Clang 5+, or MSVC 2017+- Code must pass CI checks

- **CMake**: Version 3.10 or newer- Maintain test coverage above 80%

- **Qt5**: Version 5.12 or newer with the following modules:- Follow existing architecture patterns

  - Qt5Core- Document public APIs with Doxygen

  - Qt5Widgets
  - Qt5Network
  - Qt5Sql
  - Qt5WebSockets
- **Git**: For version control

### Linux Setup

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential cmake git
sudo apt-get install qt5-default libqt5websockets5-dev

# Fedora/RHEL
sudo dnf install gcc gcc-c++ cmake git
sudo dnf install qt5-qtbase-devel qt5-qtwebsockets-devel
```

### Windows Setup

1. Install [Qt5](https://www.qt.io/download-qt-installer) for Windows
2. Install [CMake](https://cmake.org/download/)
3. Install [Visual Studio](https://visualstudio.microsoft.com/) or [MinGW](https://www.mingw-w64.org/)

### macOS Setup

```bash
# Using Homebrew
brew install cmake qt5
```

---

## Development Setup

### 1. Fork and Clone

```bash
# Fork the repository on GitHub first, then:
git clone https://github.com/YOUR_USERNAME/CommLink.git
cd CommLink

# Add upstream remote
git remote add upstream https://github.com/shrisawant144/COMMLINK.git
```

### 2. Create Build Directory

```bash
mkdir build
cd build
```

### 3. Configure with CMake

```bash
# Linux/macOS
cmake ..

# Windows with Visual Studio
cmake .. -G "Visual Studio 16 2019"

# Windows with MinGW
cmake .. -G "MinGW Makefiles"
```

### 4. Build

```bash
# Linux/macOS
make -j$(nproc)

# Windows
cmake --build . --config Release
```

### 5. Run Application

```bash
# Linux/macOS
./bin/commlink

# Windows
.\bin\Release\commlink.exe
```

### 6. Run Tests

```bash
# From build directory
ctest --verbose

# Or run specific test
./tests/unit/test_dataformat
```

---

## Project Structure

```
CommLink/
├── src/
│   ├── core/              # Core functionality
│   │   ├── dataformat.cpp       # Multi-format serialization
│   │   ├── filemanager.cpp      # File I/O operations
│   │   ├── exportmanager.cpp    # Export functionality
│   │   ├── logger.cpp           # Logging system
│   │   └── messagehistorymanager.cpp  # Database operations
│   │
│   ├── network/           # Protocol implementations
│   │   ├── tcpclient.cpp        # TCP client
│   │   ├── tcpserver.cpp        # TCP server
│   │   ├── udpclient.cpp        # UDP client
│   │   ├── udpserver.cpp        # UDP server
│   │   ├── httpclient.cpp       # HTTP client
│   │   ├── httpserver.cpp       # HTTP server
│   │   ├── websocketclient.cpp  # WebSocket client
│   │   └── websocketserver.cpp  # WebSocket server
│   │
│   ├── ui/                # User interface
│   │   ├── gui.cpp              # Main window
│   │   ├── historytab.cpp       # History interface
│   │   └── thememanager.cpp     # Theme management
│   │
│   └── main.cpp           # Application entry point
│
├── include/               # Header files
│   └── commlink/
│       ├── core/
│       ├── network/
│       └── ui/
│
├── tests/                 # Test suite
│   └── unit/
│       ├── test_dataformat.cpp
│       └── test_filemanager.cpp
│
├── docs/                  # Documentation
├── scripts/               # Build and utility scripts
├── CMakeLists.txt         # Build configuration
└── README.md              # Project overview
```

---

## Coding Standards

### C++ Style Guide

#### Naming Conventions

```cpp
// Classes: PascalCase
class HTTPClient { };
class MessageHistoryManager { };

// Functions/Methods: camelCase
void sendMessage();
void connectToServer();

// Member variables: camelCase with 'm_' prefix (optional)
QString m_hostname;
int m_port;

// Constants: UPPER_SNAKE_CASE
const int MAX_BUFFER_SIZE = 8192;
const QString DEFAULT_HOST = "127.0.0.1";

// Local variables: camelCase
QString message;
int statusCode;
```

#### Code Formatting

```cpp
// Braces on new line (Allman style)
void functionName()
{
    if (condition)
    {
        doSomething();
    }
    else
    {
        doSomethingElse();
    }
}

// Or same line (K&R style) - be consistent within file
void functionName() {
    if (condition) {
        doSomething();
    } else {
        doSomethingElse();
    }
}

// Use spaces around operators
int result = a + b;
bool isValid = (x > 0) && (y < 100);

// Indent with 4 spaces (not tabs)
void example() {
    if (true) {
        doSomething();
    }
}
```

#### Modern C++ Features

```cpp
// ✅ Use auto for type deduction
auto manager = new QNetworkAccessManager(this);
auto data = socket->readAll();

// ✅ Use smart pointers
std::unique_ptr<HTTPClient> client = std::make_unique<HTTPClient>();

// ✅ Use range-based for loops
for (const auto &message : messages) {
    processMessage(message);
}

// ✅ Use nullptr instead of NULL
QTcpSocket *socket = nullptr;

// ✅ Use override keyword
void sendMessage(const QString &msg) override;

// ✅ Use const correctness
QString getMessage() const;
void processData(const QByteArray &data);
```

#### Header Guards

```cpp
// Use #pragma once
#pragma once

#include <QObject>

class MyClass : public QObject {
    // ...
};
```

#### Include Order

```cpp
// 1. Related header (for .cpp files)
#include "httpclient.h"

// 2. Qt headers
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTcpSocket>

// 3. Standard library headers
#include <memory>
#include <vector>
#include <string>

// 4. Third-party headers
// (if any)
```

### Qt-Specific Guidelines

```cpp
// ✅ Use Qt parent-child memory management
auto *button = new QPushButton("Send", this);  // 'this' is parent

// ✅ Use new signal-slot syntax
connect(button, &QPushButton::clicked, this, &GUI::onSendClicked);

// ❌ Avoid old SIGNAL/SLOT macros
// connect(button, SIGNAL(clicked()), this, SLOT(onSendClicked()));

// ✅ Mark slots with 'private slots' or 'public slots'
private slots:
    void onDataReceived(const QByteArray &data);

// ✅ Use Q_OBJECT macro for classes with signals/slots
class MyClass : public QObject {
    Q_OBJECT
    // ...
};
```

---

## Testing Guidelines

### Writing Unit Tests

Create tests in `tests/unit/` directory:

```cpp
// tests/unit/test_dataformat.cpp
#include <gtest/gtest.h>
#include "commlink/core/dataformat.h"

class DataFormatTest : public ::testing::Test {
protected:
    void SetUp() override {
        formatter = new DataFormat();
    }
    
    void TearDown() override {
        delete formatter;
    }
    
    DataFormat *formatter;
};

TEST_F(DataFormatTest, JsonSerialization) {
    QString input = R"({"name": "Alice", "age": 28})";
    QByteArray result = formatter->serialize(input, DataFormat::JSON);
    
    ASSERT_FALSE(result.isEmpty());
    EXPECT_TRUE(result.contains("name"));
    EXPECT_TRUE(result.contains("Alice"));
}

TEST_F(DataFormatTest, InvalidJsonHandling) {
    QString invalid = "{invalid json}";
    EXPECT_THROW(formatter->serialize(invalid, DataFormat::JSON), 
                 std::runtime_error);
}
```

### Running Tests

```bash
# From build directory
ctest --verbose

# Run specific test
./tests/unit/test_dataformat

# Run with coverage (if configured)
cmake -DCMAKE_BUILD_TYPE=Coverage ..
make coverage
```

### Test Coverage Goals

- **New features**: 80%+ code coverage
- **Bug fixes**: Add regression test
- **Critical paths**: 90%+ coverage
- **UI code**: Integration tests preferred

---

## Commit Message Guidelines

### Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

### Types

- **feat**: New feature
- **fix**: Bug fix
- **docs**: Documentation changes
- **style**: Code style changes (formatting, no logic change)
- **refactor**: Code refactoring
- **test**: Adding or updating tests
- **chore**: Build process, dependencies, tooling
- **perf**: Performance improvements

### Examples

```
feat(http): add support for custom headers

Added ability to specify custom HTTP headers in requests.
Users can now add authentication tokens and custom headers.

Closes #123
```

```
fix(tcp): resolve connection timeout issue

Fixed bug where TCP connections would timeout prematurely.
Increased default timeout from 5s to 30s.

Fixes #456
```

```
docs(readme): update build instructions

Updated README with clearer build steps for Windows users.
Added troubleshooting section for common Qt issues.
```

### Best Practices

- **Subject line**: 50 characters or less
- **Subject**: Use imperative mood ("add" not "added")
- **Body**: Wrap at 72 characters
- **Body**: Explain what and why, not how
- **Footer**: Reference issues and PRs

---

## Pull Request Process

### 1. Create Feature Branch

```bash
# Update main branch
git checkout main
git pull upstream main

# Create feature branch
git checkout -b feature/your-feature-name
```

### 2. Make Changes

- Write code following coding standards
- Add tests for new functionality
- Update documentation
- Test your changes thoroughly

### 3. Commit Changes

```bash
# Stage changes
git add .

# Commit with descriptive message
git commit -m "feat(websocket): add reconnection logic"
```

### 4. Keep Branch Updated

```bash
# Fetch upstream changes
git fetch upstream

# Rebase on main
git rebase upstream/main

# Resolve conflicts if any
```

### 5. Push to Fork

```bash
git push origin feature/your-feature-name
```

### 6. Create Pull Request

On GitHub:
1. Navigate to your fork
2. Click "New Pull Request"
3. Select your feature branch
4. Fill in PR template:
   - **Title**: Clear, descriptive summary
   - **Description**: What changed and why
   - **Related Issues**: Link to issues (e.g., "Closes #123")
   - **Testing**: How you tested the changes
   - **Screenshots**: If UI changes

### 7. Address Review Comments

- Respond to all comments
- Make requested changes
- Push updates to same branch
- Request re-review when ready

### PR Checklist

- [ ] Code follows project style guidelines
- [ ] Tests pass locally (`ctest`)
- [ ] New tests added for new functionality
- [ ] Documentation updated (if applicable)
- [ ] Commit messages follow guidelines
- [ ] No merge conflicts with main branch
- [ ] Code builds without warnings
- [ ] Changes are focused and atomic

---

## Documentation

### Code Documentation

Use Doxygen-style comments for public APIs:

```cpp
/**
 * @brief Sends an HTTP request to the specified URL
 * 
 * @param url The target URL
 * @param method HTTP method (GET, POST, PUT, DELETE)
 * @param body Request body (optional)
 * 
 * @return true if request sent successfully, false otherwise
 * 
 * @throws std::invalid_argument if URL is empty
 * 
 * @example
 * client->sendRequest("http://api.example.com", "GET");
 */
bool sendRequest(const QString &url, const QString &method, 
                const QByteArray &body = QByteArray());
```

### README and Guides

When updating documentation:

- **README.md**: Keep concise, focus on quick start
- **docs/user-guide.md**: Comprehensive user documentation
- **docs/architecture.md**: System design and architecture
- **CHANGELOG.md**: Document notable changes

---

## Reporting Issues

### Bug Reports

When reporting bugs, include:

1. **Title**: Clear, descriptive summary
2. **Environment**:
   - OS and version
   - Qt version
   - CommLink version
3. **Steps to Reproduce**:
   - Detailed steps
   - Expected behavior
   - Actual behavior
4. **Logs**: Include relevant log output
5. **Screenshots**: If applicable

### Feature Requests

When requesting features:

1. **Use Case**: Describe the problem it solves
2. **Proposed Solution**: How it might work
3. **Alternatives**: Other approaches considered
4. **Priority**: How important is this feature?

### Issue Labels

- `bug`: Something isn't working
- `enhancement`: New feature or request
- `documentation`: Documentation improvements
- `good first issue`: Good for newcomers
- `help wanted`: Community help needed
- `question`: Further information requested
- `wontfix`: This will not be worked on

---

## Development Workflow

### Branch Strategy

```
main           - Stable releases
 ├── develop   - Integration branch (default)
 │    ├── feature/http-interface
 │    ├── feature/websocket-support
 │    └── fix/connection-timeout
 └── hotfix/critical-bug
```

### Release Process

1. Features merged to `develop`
2. Testing on `develop` branch
3. Create `release/x.y.z` branch
4. Final testing and bug fixes
5. Merge to `main` and tag version
6. Update CHANGELOG.md

---

## Communication Channels

- **GitHub Issues**: Bug reports, feature requests
- **Pull Requests**: Code review and discussion
- **Discussions**: General questions and ideas

---

## Recognition

Contributors will be acknowledged in:
- **AUTHORS** file
- Release notes
- Project README

---

## Questions?

If you have questions about contributing:

1. Check existing documentation
2. Search GitHub Issues
3. Open a new discussion
4. Ask in a pull request

---

## License

By contributing to CommLink, you agree that your contributions will be licensed under the project's license (see LICENSE file).

---

**Thank you for contributing to CommLink!** Your efforts help make this project better for everyone. 🚀
