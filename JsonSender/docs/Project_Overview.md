# JsonSender Project Overview

## What is JsonSender?

JsonSender is a robust desktop application built with Qt and C++ that allows users to send and receive JSON messages over network connections using either TCP or UDP protocols. It features comprehensive error handling, input validation, and a user-friendly interface designed for network testing, API debugging, and educational purposes.

## Project Structure

The project is organized as follows:

```
JsonSender/
├── CMakeLists.txt          # Build configuration
├── README.md               # Project documentation
├── test_app.sh             # Test script
├── robustness_test.sh      # Robustness test script
├── test_valid.json         # Valid JSON test file
├── test_invalid.json       # Invalid JSON test file
├── test_CMakeLists.txt     # Test build configuration
├── docs/                   # Detailed documentation
│   ├── CPP_Programming_Concepts.md
│   ├── Project_Overview.md
│   ├── Qt_Framework_Features.md
│   └── Source_Code_Analysis.md
├── include/                # Header files
│   ├── gui.h               # GUI class header
│   ├── receiverthread.h    # Thread-safe receiver header
│   ├── sender.h            # Network sender class header
├── src/                    # Source files
│   ├── gui.cpp             # GUI class implementation
│   ├── main.cpp            # Application entry point
│   ├── receiverthread.cpp  # Thread-safe receiver implementation
│   ├── sender.cpp          # Network sender class implementation
│   └── test_network.cpp    # Network test implementation
├── build/                  # Build artifacts (generated)
├── cmake/                  # CMake modules
├── test_build/             # Test build artifacts
└── tests/                  # Test files
```

## How It Works

1. **User Interface**: The application provides a comprehensive interface where users can:
   - Select the network protocol (TCP or UDP)
   - Enter target host and port with validation
   - Compose and validate JSON messages
   - Connect/disconnect with proper state management
   - Send JSON messages with error handling
   - Start/stop message receivers on specified ports
   - View timestamped activity logs and received messages

2. **Network Communication**: The application features:
   - Thread-safe TCP/UDP socket management
   - Simultaneous sending and receiving capabilities
   - Proper resource cleanup and error handling
   - JSON validation and parsing
   - Connection state management
   - Comprehensive logging and debugging

## Purpose and Use Cases

- **Learning Tool**: Excellent for learning C++17, Qt5, network programming, and threading
- **API Testing**: Comprehensive tool for testing JSON-based APIs and microservices
- **Network Debugging**: Advanced debugging with bidirectional communication
- **Educational Example**: Demonstrates modern C++ practices, Qt features, and network programming
- **Development Aid**: Useful for testing client-server applications during development

## Technologies Used

- **C++17**: Modern C++ with lambdas, smart pointers, and atomic operations
- **Qt5**: Cross-platform GUI framework with signals/slots and JSON support
- **CMake**: Advanced build system with automatic MOC processing
- **POSIX Sockets**: Low-level network programming with proper error handling
- **QThread**: Thread-safe networking with Qt's threading framework
- **std::function**: Modern C++ callbacks and functional programming

## Building and Running

1. Ensure Qt5 development libraries are installed
2. Create a build directory: `mkdir build && cd build`
3. Configure with CMake: `cmake ..`
4. Build: `make`
5. Run: `./JsonSender`

This project demonstrates best practices in modern C++ development, including proper error handling, resource management, thread safety, and user interface design. It serves as an excellent reference for combining GUI development, network programming, threading, and data serialization in a production-quality application.
