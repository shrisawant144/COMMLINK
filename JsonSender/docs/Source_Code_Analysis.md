# JsonSender Code Explanations

This README provides detailed, line-by-line explanations of the JsonSender project code. Each file is broken down section by section to help you understand how the application works.

## main.cpp - Application Entry Point

```cpp
#include <QApplication>
#include "gui.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    JsonSenderGUI window;
    window.show();
    
    return app.exec();
}
```

**Line-by-line explanation:**
- `#include <QApplication>`: Brings in Qt's application class
- `#include "gui.h"`: Includes our custom GUI header
- `QApplication app(argc, argv)`: Creates the Qt application object, passing command-line arguments
- `JsonSenderGUI window`: Creates our main window object
- `window.show()`: Makes the window visible on screen
- `return app.exec()`: Starts Qt's event loop - this is where the application "runs" and waits for user interactions

**Key concept:** Every Qt GUI application needs exactly one QApplication instance to manage the event loop.

## gui.h - GUI Class Declaration

```cpp
#pragma once
#include <QWidget>
#include <QJsonDocument>
#include "sender.h"

class QComboBox;
class QLineEdit;
class QPushButton;
class QTextEdit;
class QIntValidator;

class JsonSenderGUI : public QWidget
{
    Q_OBJECT
public:
    JsonSenderGUI();
    
private slots:
    void onConnect();
    void onSend();
    void onStartReceive();
    void onStopReceive();
    void onJsonReceived(const QJsonDocument &doc, const QString &protocol, const QString &senderInfo);
    
private:
    void setupUI();
    void setupValidators();
    void updateConnectionState(bool connected);
    void updateReceiveState(bool receiving);
    void logMessage(const QString &message, const QString &prefix = "");
    bool validateInputs();
    
    // UI Components
    QComboBox *protocolCombo;
    QLineEdit *hostEdit, *portEdit, *receivePortEdit;
    QPushButton *connectBtn, *sendBtn, *startReceiveBtn, *stopReceiveBtn;
    QTextEdit *jsonEdit, *logEdit, *receivedEdit;
    
    // Validators
    QIntValidator *portValidator;
    
    // Business logic
    Sender sender;
    bool isConnected = false;
    bool isReceiving = false;
};
```

**Section explanations:**

**Forward Declarations:**
```cpp
class QComboBox;
class QIntValidator;
// ...
```
- Tells compiler these classes exist without including full headers
- Reduces compilation time and dependencies
- Includes validation classes for input checking

**Class Declaration:**
- `class JsonSenderGUI : public QWidget`: Inherits from Qt's base widget class
- `Q_OBJECT`: Required macro for Qt's signal/slot system
- `public:` Constructor is public interface
- `private slots:` Methods for handling UI events and network data
- `private:` Helper methods and internal data members

**Helper Methods:**
- `setupUI()`: Organizes UI creation
- `setupValidators()`: Configures input validation
- `updateConnectionState()`: Manages UI state
- `validateInputs()`: Checks user input

**Member Variables:**
- UI widget pointers (created dynamically)
- `QIntValidator`: Input validation for ports
- `Sender sender`: Network communication object
- State tracking variables

## gui.cpp - GUI Implementation

### Constructor - Setting Up the Interface

```cpp
JsonSenderGUI::JsonSenderGUI() {
    setWindowTitle("JSON Sender/Receiver");
    resize(700, 700);

    auto *layout = new QVBoxLayout(this);
```

- `setWindowTitle()`: Sets text in window's title bar
- `resize()`: Sets initial window size in pixels
- `QVBoxLayout`: Vertical layout manager
- `this`: Passes the JsonSenderGUI as parent to layout

### Creating Connection Controls

```cpp
auto *connLayout = new QHBoxLayout();
protocolCombo = new QComboBox();
protocolCombo->addItems({"tcp", "udp"});
hostEdit = new QLineEdit("127.0.0.1");
portEdit = new QLineEdit("5000");
connectBtn = new QPushButton("Connect");
```

- `QHBoxLayout`: Horizontal layout for connection settings
- `QComboBox`: Dropdown with TCP/UDP options
- `QLineEdit`: Text input fields with default values
- `QPushButton`: Clickable button

### Adding Widgets to Layout

```cpp
connLayout->addWidget(new QLabel("Protocol:"));
connLayout->addWidget(protocolCombo);
// ... more widgets added
layout->addLayout(connLayout);
```

- `QLabel`: Text labels for user guidance
- `addWidget()`: Places widgets in layout
- `addLayout()`: Nests horizontal layout inside vertical layout

### JSON Input and Log Areas

```cpp
jsonEdit = new QTextEdit();
jsonEdit->setPlainText(R"({"type":"hello","from":"gui","value":42})");
sendBtn = new QPushButton("Send JSON");
sendBtn->setEnabled(false);

logEdit = new QTextEdit();
logEdit->setReadOnly(true);
```

- `QTextEdit`: Multi-line text editor
- `setPlainText()`: Sets default JSON example
- `setEnabled(false)`: Disables send button until connected
- `setReadOnly(true)`: Makes log area non-editable

### Signal-Slot Connections

```cpp
connect(connectBtn, &QPushButton::clicked, this, &JsonSenderGUI::onConnect);
connect(sendBtn, &QPushButton::clicked, this, &JsonSenderGUI::onSend);
```

- `connect()`: Links button clicks to handler methods
- `&QPushButton::clicked`: Signal emitted when button pressed
- `this, &JsonSenderGUI::onConnect`: Slot method to call

## onConnect() Slot Implementation

```cpp
void JsonSenderGUI::onConnect() {
    QString proto = protocolCombo->currentText();
    QString host = hostEdit->text();
    int port = portEdit->text().toInt();
    
    bool connected = false;
    if (proto == "tcp") {
        connected = sender.connectTcp(host, port);
    } else {
        connected = sender.connectUdp(host, port);
    }
```

- Gets values from UI widgets
- Calls appropriate connection method on sender object
- `QString::toInt()`: Converts string to integer

### Connection Result Handling

```cpp
if (connected) {
    logEdit->append(QString("✅ Connected to %1:%2 via %3").arg(host).arg(port).arg(proto.toUpper()));
    sendBtn->setEnabled(true);
    connectBtn->setText("Disconnect");
} else {
    logEdit->append("❌ Connection failed");
}
```

- `logEdit->append()`: Adds text to log area
- `QString::arg()`: String formatting with placeholders
- `setEnabled(true)`: Enables send button
- `setText()`: Changes button label

## onSend() Slot Implementation

```cpp
void JsonSenderGUI::onSend() {
    QString jsonText = jsonEdit->toPlainText();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonText.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        logEdit->append("❌ Invalid JSON: " + error.errorString());
        return;
    }
    
    sender.sendJson(doc);
    logEdit->append("→ Sent: " + jsonText);
}
```

- Gets JSON text from editor
- `QJsonDocument::fromJson()`: Parses JSON string
- Checks for parsing errors
- Calls sender to transmit JSON
- Logs the sent message

## sender.h - Network Sender Declaration

```cpp
#pragma once
#include <QObject>
#include <QJsonDocument>
#include <functional>
#include <sys/socket.h>
#include <netinet/in.h>

class ReceiverThread;

class Sender : public QObject
{
    Q_OBJECT
public:
    enum class ConnectionType { TCP, UDP };
    
    Sender() = default;
    ~Sender();

    std::function<void(const QJsonDocument&)> sendJson;

    bool connectTcp(const QString &host, quint16 port);
    bool connectUdp(const QString &host, quint16 port);
    void disconnect();

    bool startTcpReceiver(quint16 port);
    bool startUdpReceiver(quint16 port);
    void stopReceiver();
    
    bool isConnected() const { return tcp_fd >= 0 || udp_fd >= 0; }
    ConnectionType getConnectionType() const;

signals:
    void jsonReceived(const QJsonDocument &doc, const QString &protocol, const QString &senderInfo);

private:
    bool validatePort(quint16 port) const;
    bool createSocket(int &fd, int type) const;
    bool setupAddress(const QString &host, quint16 port, sockaddr_in &addr) const;
    void closeSocket(int &fd);
    
    int tcp_fd = -1;
    int udp_fd = -1;
    int listen_fd = -1;
    sockaddr_in udp_addr{};
    ReceiverThread *receiverThread = nullptr;
};
```

**Key elements:**
- Inherits from `QObject` for Qt integration
- `std::function<>`: Flexible callback type for sending
- Socket file descriptors (`tcp_fd`, `udp_fd`)
- `sockaddr_in`: Structure for network addresses

## sender.cpp - Network Implementation

### Destructor - Resource Cleanup

```cpp
Sender::~Sender() {
    disconnect();
    stopReceiver();
}
```

- Automatically called when Sender object is destroyed
- Closes all network sockets and stops receiver thread
- Prevents resource leaks and ensures clean shutdown

### TCP Connection Setup

```cpp
bool Sender::connectTcp(const QString &host, quint16 port)
{
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd < 0) {
        qCritical() << "TCP socket creation failed";
        return false;
    }
```

- `socket()`: Creates TCP socket (SOCK_STREAM)
- `AF_INET`: IPv4 address family
- Error checking with `qCritical()` logging

### Address Configuration

```cpp
sockaddr_in addr{};
addr.sin_family = AF_INET;
addr.sin_port = htons(port);
inet_pton(AF_INET, host.toUtf8().constData(), &addr.sin_addr);
```

- `sockaddr_in`: IPv4 address structure
- `htons()`: Converts port to network byte order
- `inet_pton()`: Converts IP string to binary format

### TCP Connection

```cpp
if (::connect(tcp_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
    close(tcp_fd);
    qCritical() << "TCP connect failed";
    return false;
}
```

- `::connect`: POSIX connect function (global scope)
- Casts address structure to generic sockaddr
- Closes socket on failure

### TCP Send Function Assignment

```cpp
sendJson = [this](const QJsonDocument &doc) {
    QByteArray payload = doc.toJson(QJsonDocument::Compact) + "\n";
    ssize_t written = send(tcp_fd, payload.constData(), payload.size(), 0);
    if (written == payload.size()) qInfo() << "→ TCP:" << payload.constData();
    else qWarning() << "TCP partial send!";
};
```

- Lambda function assigned to `sendJson`
- `[this]`: Captures Sender object for accessing `tcp_fd`
- `doc.toJson()`: Converts JSON to compact string
- `send()`: POSIX function to send data over TCP

### UDP Connection Setup

```cpp
bool Sender::connectUdp(const QString &host, quint16 port)
{
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    // ... similar address setup
    sendJson = [this](const QJsonDocument &doc) {
        QByteArray payload = doc.toJson(QJsonDocument::Compact) + "\n";
        ssize_t written = sendto(udp_fd, payload.constData(), payload.size(), 0, 
                               (sockaddr*)&udp_addr, sizeof(udp_addr));
        // ... logging
    };
    return true;  // UDP doesn't need connection, just address
}
```

- `SOCK_DGRAM`: Datagram socket for UDP
- `sendto()`: Sends to specific address (no connection needed)
- UDP is connectionless - always returns true

## CMakeLists.txt - Build Configuration

```cpp
cmake_minimum_required(VERSION 3.8)
project(JsonSender LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Using Qt5 with Widgets for GUI
find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

set(CMAKE_AUTOMOC ON)

add_executable(JsonSender main.cpp sender.cpp gui.cpp)

target_link_libraries(JsonSender 
    Qt5::Core 
    Qt5::Widgets
)
```

**Build system explanation:**
- `cmake_minimum_required`: Minimum CMake version
- `project()`: Defines project name and language
- `set(CMAKE_CXX_STANDARD 17)`: Uses C++17 features
- `find_package(Qt5 ...)`: Locates Qt5 libraries
- `CMAKE_AUTOMOC ON`: Enables automatic MOC processing
- `add_executable()`: Creates executable from source files
- `target_link_libraries()`: Links Qt libraries

## receiverthread.h - Thread-Safe Receiver Declaration

```cpp
#pragma once
#include <QThread>
#include <QJsonDocument>
#include <QString>
#include <sys/socket.h>
#include <netinet/in.h>
#include <atomic>

class ReceiverThread : public QThread
{
    Q_OBJECT
public:
    ReceiverThread(int socket_fd, bool is_tcp, QObject *parent = nullptr);
    ~ReceiverThread();

    void stop();

signals:
    void jsonReceived(const QJsonDocument &doc, const QString &protocol, const QString &senderInfo);

protected:
    void run() override;

private:
    QString formatSenderInfo(const sockaddr_in &addr) const;
    bool processReceivedData(const char *buffer, ssize_t size, const QString &senderInfo);
    
    int socket_fd;
    bool is_tcp;
    std::atomic<bool> running{true};
    static constexpr size_t BUFFER_SIZE = 4096;
};
```

**Key elements:**
- Inherits from `QThread` for background network listening
- Uses `std::atomic<bool>` for thread-safe stopping
- Emits signals when JSON messages are received
- Supports both TCP and UDP protocols
- Non-blocking socket operations to prevent hanging

## receiverthread.cpp - Thread Implementation

### Constructor and Setup

```cpp
ReceiverThread::ReceiverThread(int socket_fd, bool is_tcp, QObject *parent)
    : QThread(parent), socket_fd(socket_fd), is_tcp(is_tcp)
{
}
```

- Takes ownership of the listening socket
- Stores protocol type (TCP/UDP)

### Main Receive Loop

```cpp
void ReceiverThread::run()
{
    char buffer[BUFFER_SIZE];
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    // Set socket to non-blocking mode
    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);

    while (running) {
        // ... receive logic for TCP/UDP
        if (received > 0) {
            buffer[received] = '\0';
            QString senderInfo = formatSenderInfo(client_addr);
            processReceivedData(buffer, received, senderInfo);
        }
    }
}
```

- Non-blocking receive to avoid thread blocking
- Processes incoming data and emits signals for valid JSON
- Handles both TCP connections and UDP datagrams

## How It All Works Together

1. **Application Start**: `main()` creates QApplication and shows GUI
2. **UI Setup**: GUI creates widgets, layouts, and validators with group boxes
3. **Input Validation**: Port validators ensure valid input ranges with error messages
4. **Connection**: GUI validates input and calls `sender.connectTcp/Udp()`
5. **Socket Creation**: Low-level sockets created with comprehensive error handling
6. **Send Callback**: Lambda functions assigned for protocol-specific sending
7. **Receiver Setup**: Background thread created for incoming messages on separate port
8. **JSON Processing**: Messages parsed, validated, and transmitted with detailed error reporting
9. **State Management**: UI updates reflect connection and receiver status with disabled controls
10. **Logging**: Timestamped logs with prefixes provide debugging information
11. **Resource Cleanup**: RAII ensures proper cleanup on exit, including thread stopping

This architecture demonstrates:
- **Separation of Concerns**: GUI, networking, and threading are separate
- **Error Handling**: Comprehensive validation and error reporting with QMessageBox
- **Thread Safety**: Atomic operations and Qt's thread-safe signals
- **Resource Management**: Proper cleanup and state management with disconnect/stop methods
- **Modern C++**: Lambdas, smart pointers, RAII patterns, and atomic variables
- **User Experience**: Grouped UI elements, real-time feedback, and input validation
