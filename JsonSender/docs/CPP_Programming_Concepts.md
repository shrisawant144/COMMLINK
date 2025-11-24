# C++ Concepts Used in JsonSender

This README explains the C++ programming concepts demonstrated in the JsonSender project. Since you're new to C++, we'll break down each concept with examples from the code.

## 1. Basic Program Structure

### Header Files (.h) and Implementation Files (.cpp)

**Header files** declare what a class or module provides:
```cpp
// gui.h - Declaration
#pragma once
#include <QWidget>

class JsonSenderGUI : public QWidget {
    Q_OBJECT
public:
    JsonSenderGUI();
private:
    QComboBox *protocolCombo;
};
```

**Implementation files** contain the actual code:
```cpp
// gui.cpp - Implementation
#include "gui.h"
#include <QComboBox>

JsonSenderGUI::JsonSenderGUI() {
    protocolCombo = new QComboBox();
    // ... setup code
}
```

- `#pragma once`: Prevents multiple inclusion of header files
- `#include`: Brings in declarations from other files
- Separation allows faster compilation and cleaner organization

## 2. Object-Oriented Programming (OOP)

### Classes and Objects

**Class definition:**
```cpp
class JsonSenderGUI : public QWidget {
    // Members and methods
};
```

**Object creation:**
```cpp
JsonSenderGUI window;  // Create an object
window.show();         // Call a method on the object
```

- **Class**: Blueprint defining properties and behaviors
- **Object**: Instance of a class in memory
- **Methods**: Functions that operate on the object's data

### Inheritance

```cpp
class JsonSenderGUI : public QWidget {
    // Inherits all QWidget functionality
    // Can add new features or override existing ones
};
```

- `JsonSenderGUI` inherits from `QWidget`
- Gets all QWidget features (window, painting, events)
- Can add custom behavior (JSON sending, network connection)

### Access Modifiers

```cpp
class JsonSenderGUI : public QWidget {
public:      // Accessible from anywhere
    JsonSenderGUI();
private:     // Only accessible within this class
    QComboBox *protocolCombo;
};
```

- `public`: Interface to the outside world
- `private`: Internal implementation details

## 3. Memory Management

### Pointers

```cpp
private:
    QComboBox *protocolCombo;  // Pointer to QComboBox object
    QLineEdit *hostEdit;        // Pointer to QLineEdit object
```

- `*` indicates a pointer variable
- Stores memory address of an object
- Allows dynamic object creation and management

### Dynamic Memory Allocation

```cpp
protocolCombo = new QComboBox();  // Create object on heap
hostEdit = new QLineEdit("127.0.0.1");  // Create with initial value
```

- `new`: Allocates memory and creates object
- Objects created with `new` must be deleted with `delete`
- Qt handles most memory management automatically

### RAII (Resource Acquisition Is Initialization)

```cpp
Sender::~Sender() {
    disconnect();     // Clean up connections
    stopReceiver();   // Stop receiver thread
}

void Sender::closeSocket(int &fd) {
    if (fd >= 0) {
        close(fd);
        fd = -1;  // Reset to invalid state
    }
}
```

- Destructor automatically called when object is destroyed
- Ensures proper cleanup of all resources (sockets, threads)
- Helper methods provide consistent resource management
- Prevents resource leaks and dangling file descriptors

## 4. Function Pointers and Callbacks

### std::function and Modern Error Handling

```cpp
// sender.h
std::function<void(const QJsonDocument&)> sendJson;

// sender.cpp - TCP lambda with error handling
sendJson = [this](const QJsonDocument &doc) {
    if (tcp_fd < 0) {
        qWarning() << "TCP not connected";
        return;
    }
    QByteArray payload = doc.toJson(QJsonDocument::Compact) + "\n";
    ssize_t written = send(tcp_fd, payload.constData(), payload.size(), 0);
    if (written < 0) {
        qWarning() << "TCP send failed";
    } else if (written == payload.size()) {
        qInfo() << "→ TCP:" << payload.constData();
    }
};
```

- `std::function`: Modern C++ callback mechanism
- Comprehensive error checking for all operations
- Different implementations for TCP vs UDP
- Proper logging for debugging

## 5. Lambda Functions

### Anonymous Functions

```cpp
sendJson = [this](const QJsonDocument &doc) {
    // Lambda body - can access 'this' and local variables
    QByteArray payload = doc.toJson(QJsonDocument::Compact) + "\n";
    ssize_t written = send(tcp_fd, payload.constData(), payload.size(), 0);
};
```

- `[this]`: Capture list - what variables the lambda can access
- `(const QJsonDocument &doc)`: Parameters
- `{ ... }`: Function body
- Lambdas are unnamed functions defined inline

## 6. Error Handling

### Return Values and Logging

```cpp
bool Sender::connectTcp(const QString &host, quint16 port) {
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd < 0) {
        qCritical() << "TCP socket creation failed";
        return false;  // Indicate failure
    }
    // ... success case
    return true;  // Indicate success
}
```

- Functions return `bool` to indicate success/failure
- Logging provides debugging information
- Caller checks return value to handle errors

## 7. Networking with Sockets

### Low-Level Network Programming

```cpp
// Create socket
tcp_fd = socket(AF_INET, SOCK_STREAM, 0);

// Set up address structure
sockaddr_in addr{};
addr.sin_family = AF_INET;
addr.sin_port = htons(port);  // Convert to network byte order
inet_pton(AF_INET, host.toUtf8().constData(), &addr.sin_addr);

// Connect
if (::connect(tcp_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
    // Handle error
}
```

- **Socket**: Endpoint for network communication
- **TCP**: Reliable, connection-oriented protocol
- **UDP**: Fast, connectionless protocol
- `sockaddr_in`: Structure holding IP address and port
- `htons()`: Converts port to network byte order

## 8. String Manipulation

### C++ Strings vs Qt Strings

```cpp
// Qt string handling
QString proto = protocolCombo->currentText();
QString host = hostEdit->text();
int port = portEdit->text().toInt();  // Convert string to number

// Convert to C-style string for POSIX functions
inet_pton(AF_INET, host.toUtf8().constData(), &addr.sin_addr);
```

- `QString`: Qt's Unicode string class
- `.toInt()`: Convert string to integer
- `.toUtf8()`: Convert to UTF-8 byte array
- `.constData()`: Get C-style string pointer

## 9. Namespaces and Scope Resolution

### Avoiding Name Conflicts

```cpp
// In sender.cpp
if (::connect(tcp_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
    // ::connect refers to POSIX connect function
    // (not confused with Qt's connect for signals/slots)
}
```

- `::` is scope resolution operator
- `::connect` calls global POSIX connect function
- Without `::`, it might refer to Qt's connect function

## 10. Constructor Initialization

### Member Initialization

```cpp
JsonSenderGUI::JsonSenderGUI() {
    setWindowTitle("JSON Sender GUI");
    resize(500, 400);
    
    // Create and setup widgets
    protocolCombo = new QComboBox();
    // ...
}
```

- Constructor sets up the object when created
- Initializes UI elements and connects signals/slots
- Called automatically when object is instantiated

## 11. Include Guards and Headers

### Preventing Multiple Definitions

```cpp
#pragma once  // Modern include guard

#include <QWidget>     // Qt headers
#include "sender.h"    // Project headers
```

- `#pragma once`: Ensures header is included only once
- `<>` for system/Qt headers
- `""` for project headers

## 12. Type Conversion and Casting

### Safe Type Conversions

```cpp
// String to number
int port = portEdit->text().toInt();

// Qt string to C string
inet_pton(AF_INET, host.toUtf8().constData(), &addr.sin_addr);

// C-style cast for socket functions
(sockaddr*)&addr
```

- Qt provides safe conversion methods
- C-style casts used with legacy C APIs (sockets)

## 13. Threading and Concurrency

### QThread for Background Operations

```cpp
class ReceiverThread : public QThread {
    Q_OBJECT
public:
    ReceiverThread(int socket_fd, bool is_tcp, QObject *parent = nullptr);
    void stop();
    
signals:
    void jsonReceived(const QJsonDocument &doc, const QString &protocol, const QString &senderInfo);
    
protected:
    void run() override;
    
private:
    std::atomic<bool> running{true};
    int socket_fd;
    bool is_tcp;
};
```

- `QThread`: Qt's threading class for non-blocking operations
- `std::atomic<bool>`: Thread-safe boolean with modern syntax
- `signals`: Thread-safe communication with main thread
- `run()`: Override to define thread behavior

## Key C++ Concepts Summary

1. **OOP**: Classes, objects, inheritance, encapsulation
2. **Memory**: Pointers, dynamic allocation, RAII
3. **Functions**: Lambdas, function pointers, callbacks
4. **Error Handling**: Comprehensive validation and logging
5. **Threading**: QThread, atomic operations, thread safety
6. **Networking**: Sockets, TCP/UDP protocols
7. **Strings**: QString vs C-strings, conversions
8. **Build System**: Headers vs implementation, compilation

These concepts work together to create a robust, maintainable application with modern C++ practices and comprehensive error handling.
