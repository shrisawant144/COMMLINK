# JSON Sender/Receiver

A professional Qt-based GUI application for sending and receiving JSON messages over TCP/UDP networks with modern C++ architecture.

## Project Structure

```
JsonSender/
├── CMakeLists.txt          # Build configuration
├── README.md               # This file
├── .gitignore             # Version control ignore rules
├── include/               # Header files
│   ├── gui.h             # GUI class declaration
│   ├── sender.h          # Network sender class
│   └── receiverthread.h  # Thread-safe receiver
├── src/                   # Source files
│   ├── main.cpp          # Application entry point
│   ├── gui.cpp           # GUI implementation
│   ├── sender.cpp        # Network implementation
│   └── receiverthread.cpp # Receiver implementation
├── docs/                  # Documentation
│   ├── Project_Overview.md
│   ├── CPP_Programming_Concepts.md
│   ├── Qt_Framework_Features.md
│   └── Source_Code_Analysis.md
├── tests/                 # Test files (future)
└── build/                 # Build artifacts (generated)
```

## Features

- **Dual Protocol Support**: Send and receive JSON messages via TCP or UDP
- **Real-time Communication**: Simultaneous sending and receiving capabilities
- **Input Validation**: Port number validation and JSON syntax checking
- **Connection Management**: Connect/disconnect functionality with state management
- **Activity Logging**: Timestamped logs of all network activities
- **User-friendly GUI**: Intuitive interface with proper error handling

## Architecture

### Core Components

- **Sender**: Main networking class handling TCP/UDP connections and receivers
- **ReceiverThread**: Thread-safe message receiving with proper resource management
- **JsonSenderGUI**: Qt-based user interface with validation and state management

### Key Features

1. **Modern C++17**: Lambdas, atomic operations, RAII patterns
2. **Thread Safety**: Atomic operations for thread-safe communication
3. **Error Handling**: Comprehensive validation and error reporting
4. **Resource Management**: Proper socket cleanup and thread management
5. **Code Organization**: Professional project structure with separated concerns
6. **Input Validation**: Port range validation and JSON syntax checking

## Building

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
make -j$(nproc)

# Run the application
./bin/JsonSender
```

## Usage

1. **Connect**: Choose protocol (TCP/UDP), enter host and port, click Connect
2. **Send**: Enter JSON message and click Send JSON
3. **Receive**: Set listen port and click Start Receiving
4. **Monitor**: View all activities in the Activity Log

## Technical Specifications

- **C++ Standard**: C++17 with modern features
- **Qt Version**: Qt5 (Core, Widgets)
- **Threading**: QThread for non-blocking network operations
- **Networking**: POSIX sockets with comprehensive error handling
- **JSON**: Qt's JSON parsing with validation
- **Build System**: CMake with automatic MOC processing
- **Architecture**: Clean separation of GUI, networking, and threading