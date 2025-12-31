# Code Flow

Detailed explanation of code execution in CommLink, including startup, user interactions, data flow, and event handling.

## Table of Contents

1. [Application Startup Flow](#application-startup-flow)
2. [Connection Flow](#connection-flow)
3. [Message Sending Flow](#message-sending-flow)
4. [Message Receiving Flow](#message-receiving-flow)
5. [Server Operations Flow](#server-operations-flow)
6. [Signal-Slot Communication](#signal-slot-communication)
7. [Data Format Processing](#data-format-processing)
8. [Database Operations Flow](#database-operations-flow)

---

## Application Startup Flow

### Sequence Diagram

```
main() → QApplication → MainWindow Constructor
                           │
                           ├─→ initializeNetworkComponents()
                           │     ├─→ Create TcpClient
                           │     ├─→ Create TcpServer
                           │     ├─→ Create UdpClient
                           │     ├─→ Create UdpServer
                           │     ├─→ Create WebSocketClient
                           │     ├─→ Create WebSocketServer
                           │     ├─→ Create HttpClient
                           │     ├─→ Create HttpServer
                           │     └─→ Connect all signals to MainWindow slots
                           │
                           ├─→ setupUI()
                           │     ├─→ Create ConnectionPanel
                           │     ├─→ Create ServerPanel
                           │     ├─→ Create MessagePanel
                           │     ├─→ Create DisplayPanel
                           │     └─→ Create StatusPanel
                           │
                           ├─→ setupMenuBar()
                           ├─→ setupShortcuts()
                           ├─→ setupConnections()
                           │     └─→ Wire UI panel signals to MainWindow slots
                           │
                           ├─→ loadSettings()
                           ├─→ Initialize Database (MessageHistoryManager)
                           └─→ Apply Theme
```

### Step-by-Step Execution

#### 1. Entry Point (`main.cpp`)

```cpp
int main(int argc, char* argv[]) {
    // 1. Enable High DPI scaling
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    
    // 2. Create Qt application instance
    QApplication app(argc, argv);
    
    // 3. Set application metadata
    QApplication::setApplicationName("CommLink");
    
    // 4. Parse command line arguments
    QCommandLineParser parser;
    // ... parse options ...
    
    // 5. Create and show MainWindow
    MainWindow mainWindow;
    mainWindow.show();
    
    // 6. Start event loop
    return QApplication::exec();
}
```

**Key Points:**
- `QApplication` initializes Qt framework
- `QApplication::exec()` starts the event loop (blocks until app exits)
- All UI interactions happen within this event loop

---

#### 2. MainWindow Construction (`mainwindow.cpp`)

```cpp
MainWindow::MainWindow(QWidget *parent) {
    // 1. Initialize database
    historyManager.initializeDatabase();
    
    // 2. Create all network components
    initializeNetworkComponents();
    
    // 3. Create UI panels
    setupUI();
    
    // 4. Create menu bar
    setupMenuBar();
    
    // 5. Set up keyboard shortcuts
    setupShortcuts();
    
    // 6. Wire signals and slots
    setupConnections();
    
    // 7. Load saved settings
    loadSettings();
    
    // 8. Initialize theme
    ThemeManager::instance().loadSettings();
}
```

**Execution Order:**
1. **Database Initialization**: Creates SQLite database and tables
2. **Network Components**: Creates all 8 network objects (4 clients + 4 servers)
3. **UI Creation**: Creates all 5 UI panels
4. **Signal Wiring**: Connects UI signals to MainWindow slots
5. **Settings**: Restores window size, position, etc.

---

#### 3. Network Component Initialization

```cpp
void MainWindow::initializeNetworkComponents() {
    // Create network objects
    tcpClient = new TcpClient(this);
    tcpServer = new TcpServer(this);
    // ... create all 8 components ...
    
    // Wire network signals to MainWindow slots
    connect(tcpClient, &TcpClient::connected, 
            this, &MainWindow::updateStatus);
    connect(tcpClient, &TcpClient::messageReceived, 
            this, &MainWindow::onDataReceived);
    // ... connect all signals ...
}
```

**Key Points:**
- All network components are created with `MainWindow` as parent
- Qt's parent-child ownership ensures automatic cleanup
- Signals are connected immediately, ready for use

---

#### 4. UI Setup

```cpp
void MainWindow::setupUI() {
    // Create panels
    connectionPanel = new ConnectionPanel();
    serverPanel = new ServerPanel();
    messagePanel = new MessagePanel();
    displayPanel = new DisplayPanel(&historyManager);
    statusPanel = new StatusPanel();
    
    // Layout panels in splitter
    // ...
}
```

**Layout Structure:**
```
MainWindow
└── Central Widget
    └── Horizontal Splitter
        ├── Left Panel (Controls)
        │   ├── ConnectionPanel
        │   ├── ServerPanel
        │   └── MessagePanel
        └── Right Panel (Display)
            ├── DisplayPanel (stretchable)
            └── StatusPanel
```

---

#### 5. Signal-Slot Connections

```cpp
void MainWindow::setupConnections() {
    // UI Panel → MainWindow
    connect(connectionPanel, &ConnectionPanel::connectRequested,
            this, &MainWindow::onConnectRequested);
    
    connect(messagePanel, &MessagePanel::sendRequested,
            this, &MainWindow::onSendRequested);
    
    // ... more connections ...
}
```

**Connection Pattern:**
- UI panels emit signals (user actions)
- MainWindow slots handle these signals
- MainWindow calls network component methods
- Network components emit signals (network events)
- MainWindow forwards to UI panels

---

## Connection Flow

### Client Connection Sequence

```
User clicks "Connect" button
    │
    ▼
ConnectionPanel::onConnectClicked()
    │
    ▼
emit connectRequested() signal
    │
    ▼
MainWindow::onConnectRequested() slot
    │
    ├─→ Validate inputs
    ├─→ Get protocol from ConnectionPanel
    ├─→ Get host/port from ConnectionPanel
    │
    ├─→ [If TCP]
    │     └─→ tcpClient->connectToHost(host, port)
    │           │
    │           ├─→ QTcpSocket::connectToHost() (async)
    │           ├─→ Start connection timer
    │           │
    │           └─→ [When connected]
    │                 ├─→ TcpClient::onConnected()
    │                 ├─→ emit connected() signal
    │                 └─→ MainWindow::updateStatus()
    │
    ├─→ [If UDP]
    │     └─→ udpClient->connectToHost(host, port)
    │
    ├─→ [If WebSocket]
    │     └─→ wsClient->connectToHost(url)
    │
    └─→ [If HTTP]
          └─→ httpClient->connectToHost(url)
```

### Detailed TCP Connection Flow

#### Step 1: User Action
```cpp
// ConnectionPanel::onConnectClicked()
void ConnectionPanel::onConnectClicked() {
    if (connected) {
        emit disconnectRequested();
    } else {
        emit connectRequested();  // Signal emitted
    }
}
```

#### Step 2: MainWindow Handler
```cpp
// MainWindow::onConnectRequested()
void MainWindow::onConnectRequested() {
    QString protocol = connectionPanel->getProtocol();
    QString host = connectionPanel->getHost();
    int port = connectionPanel->getPort();
    
    if (protocol == "TCP") {
        tcpClient->connectToHost(host, port);
    }
    // ...
}
```

#### Step 3: Network Component
```cpp
// TcpClient::connectToHost()
void TcpClient::connectToHost(const QString& host, quint16 port) {
    m_socket->connectToHost(host, port);  // Asynchronous!
    m_connectionTimer->start(3000);       // 3 second timeout
}
```

**Important:** `connectToHost()` is **asynchronous**. It returns immediately and the connection happens in the background.

#### Step 4: Connection Established
```cpp
// TcpClient::onConnected() - called by Qt when socket connects
void TcpClient::onConnected() {
    m_connectionTimer->stop();
    m_connected = true;
    emit connected();  // Signal to MainWindow
}
```

#### Step 5: MainWindow Updates UI
```cpp
// MainWindow::updateStatus() - connected to tcpClient::connected signal
void MainWindow::updateStatus() {
    connectionPanel->setConnectionState(true);
    statusPanel->updateConnectionStatus("Connected");
    // ...
}
```

### Connection Timeout Handling

```cpp
// TcpClient::onConnectionTimeout()
void TcpClient::onConnectionTimeout() {
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        m_socket->abort();
        emit errorOccurred("Connection timeout");
    }
}
```

---

## Message Sending Flow

### Complete Send Flow Diagram

```
User types message → Clicks "Send" (or Ctrl+Return)
    │
    ▼
MessagePanel::onSendClicked()
    │
    ▼
emit sendRequested() signal
    │
    ▼
MainWindow::onSendRequested()
    │
    ├─→ Get message text from MessagePanel
    ├─→ Get format from MessagePanel
    ├─→ Validate input format
    │
    ├─→ Parse input → QVariant
    ├─→ Create DataMessage(format, data)
    │
    ├─→ [Check send mode]
    │     │
    │     ├─→ [Client Mode]
    │     │     ├─→ Get protocol from ConnectionPanel
    │     │     │
    │     │     ├─→ [If TCP]
    │     │     │     └─→ tcpClient->sendMessage(msg)
    │     │     │           │
    │     │     │           ├─→ msg.serialize() → QByteArray
    │     │     │           ├─→ socket->write(bytes)
    │     │     │           └─→ socket->flush()
    │     │     │
    │     │     ├─→ [If UDP]
    │     │     │     └─→ udpClient->sendMessage(msg)
    │     │     │
    │     │     ├─→ [If WebSocket]
    │     │     │     └─→ wsClient->sendMessage(msg)
    │     │     │
    │     │     └─→ [If HTTP]
    │     │           └─→ httpClient->sendRequest(method, url, msg)
    │     │
    │     └─→ [Server Mode]
    │           ├─→ [Broadcast]
    │           │     └─→ server->sendToAll(msg)
    │           │
    │           └─→ [To Selected Client]
    │                 └─→ server->sendToClient(client, msg)
    │
    ├─→ Display message in DisplayPanel
    ├─→ Log message
    └─→ Save to database (MessageHistoryManager)
```

### Step-by-Step: Sending a TCP Message

#### Step 1: User Input
```cpp
// User types in MessagePanel::messageEdit (QTextEdit)
QString messageText = messageEdit->toPlainText();
// Example: '{"name": "John", "age": 30}'
```

#### Step 2: Send Button Clicked
```cpp
// MessagePanel::onSendClicked()
void MessagePanel::onSendClicked() {
    emit sendRequested();  // Signal emitted
}
```

#### Step 3: MainWindow Handler
```cpp
// MainWindow::onSendRequested()
void MainWindow::onSendRequested() {
    // 1. Get message text
    QString messageText = messagePanel->getMessage();
    
    // 2. Get format
    DataFormatType format = messagePanel->getFormat();
    
    // 3. Validate
    if (!DataMessage::validateInput(messageText, format)) {
        QMessageBox::warning(this, "Error", "Invalid format");
        return;
    }
    
    // 4. Parse input
    QVariant parsedData = DataMessage::parseInput(messageText, format);
    
    // 5. Create DataMessage
    DataMessage msg(format, parsedData);
    
    // 6. Send via appropriate protocol
    if (protocol == "TCP" && tcpClient->isConnected()) {
        tcpClient->sendMessage(msg);
    }
}
```

#### Step 4: Format Parsing
```cpp
// DataMessage::parseInput() - for JSON format
QVariant DataMessage::parseInput(const QString& input, DataFormatType type) {
    if (type == DataFormatType::JSON) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(input.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError) {
            return doc;  // Returns QJsonDocument as QVariant
        }
    }
    // ...
}
```

#### Step 5: Serialization
```cpp
// TcpClient::sendMessage()
void TcpClient::sendMessage(const DataMessage& message) {
    // Serialize DataMessage to bytes
    QByteArray data = message.serialize();
    
    // Write to socket
    qint64 bytesWritten = m_socket->write(data);
    
    // Flush to ensure immediate send
    m_socket->flush();
}
```

#### Step 6: Serialization Details
```cpp
// DataMessage::serialize() - for JSON
QByteArray DataMessage::serialize() const {
    if (type == DataFormatType::JSON) {
        QJsonDocument doc = data.value<QJsonDocument>();
        return doc.toJson(QJsonDocument::Compact);
        // Returns: '{"name":"John","age":30}'
    }
    // ...
}
```

#### Step 7: Network Transmission
```cpp
// QTcpSocket::write() - Qt's async I/O
m_socket->write(data);  // Queues data for transmission
m_socket->flush();      // Forces immediate send
```

**Important:** `write()` is **asynchronous**. Data is queued and sent by Qt's event loop.

#### Step 8: UI Update & Persistence
```cpp
// MainWindow::onSendRequested() - after sending
displayPanel->appendSentMessage(timestampedMessage);
logMessage("Sent via TCP", "[SEND] ");
historyManager.saveMessage("sent", "TCP", host, port, msg);
```

---

## Message Receiving Flow

### Complete Receive Flow

```
Network socket receives data
    │
    ▼
QTcpSocket::readyRead() signal (Qt event)
    │
    ▼
TcpClient::onReadyRead() slot
    │
    ├─→ socket->readAll() → QByteArray
    ├─→ DataMessage::deserialize(bytes, format)
    ├─→ Create timestamp
    ├─→ Get source address
    │
    └─→ emit messageReceived(msg, source, timestamp)
        │
        ▼
MainWindow::onDataReceived() slot
    │
    ├─→ Display message in DisplayPanel
    ├─→ Log message
    └─→ Save to database
```

### Detailed TCP Receive Flow

#### Step 1: Network Event
```cpp
// Qt detects data available on socket
// Emits: QTcpSocket::readyRead() signal
```

#### Step 2: Network Component Handler
```cpp
// TcpClient::onReadyRead() - connected to socket::readyRead
void TcpClient::onReadyRead() {
    // 1. Read all available data
    QByteArray data = m_socket->readAll();
    
    // 2. Deserialize to DataMessage
    DataMessage msg = DataMessage::deserialize(data, m_format);
    
    // 3. Create metadata
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString source = m_socket->peerAddress().toString() + ":" + 
                     QString::number(m_socket->peerPort());
    
    // 4. Emit signal
    emit messageReceived(msg, source, timestamp);
}
```

#### Step 3: Deserialization
```cpp
// DataMessage::deserialize() - for JSON
DataMessage DataMessage::deserialize(const QByteArray& bytes, DataFormatType type) {
    DataMessage msg(type);
    
    if (type == DataFormatType::JSON) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(bytes, &error);
        if (error.error == QJsonParseError::NoError) {
            msg.data = doc;  // Store QJsonDocument
        } else {
            // Fallback to text if JSON invalid
            msg.data = QString::fromUtf8(bytes);
        }
    }
    
    return msg;
}
```

#### Step 4: MainWindow Handler
```cpp
// MainWindow::onDataReceived() - connected to all network components
void MainWindow::onDataReceived(const DataMessage &msg, 
                                const QString &source, 
                                const QString &timestamp) {
    // 1. Convert to display string
    QString displayText = msg.toDisplayString();
    
    // 2. Display in UI
    displayPanel->appendReceivedMessage(
        QString("[%1] ← %2:\n%3")
            .arg(timestamp, source, displayText)
    );
    
    // 3. Log
    logMessage(QString("Received from %1").arg(source), "[RECV] ");
    
    // 4. Save to database
    historyManager.saveMessage("received", protocol, host, port, msg);
    
    // 5. Update status
    updateStatus();
}
```

#### Step 5: Display Formatting
```cpp
// DataMessage::toDisplayString() - for JSON
QString DataMessage::toDisplayString() const {
    if (type == DataFormatType::JSON) {
        QJsonDocument doc = data.value<QJsonDocument>();
        return doc.toJson(QJsonDocument::Indented);
        // Pretty-printed JSON for display
    }
    // ...
}
```

---

## Server Operations Flow

### Starting a Server

```
User clicks "Start Server"
    │
    ▼
ServerPanel::onStartServerClicked()
    │
    ▼
emit startServerRequested() signal
    │
    ▼
MainWindow::onStartServerRequested()
    │
    ├─→ Get protocol from ServerPanel
    ├─→ Get port from ServerPanel
    │
    ├─→ [If TCP Server]
    │     └─→ tcpServer->startServer(port)
    │           │
    │           ├─→ QTcpServer::listen(port)
    │           ├─→ [When client connects]
    │           │     └─→ TcpServer::onNewConnection()
    │           │           ├─→ Accept connection
    │           │           ├─→ Store client socket
    │           │           └─→ emit clientConnected()
    │           │
    │           └─→ emit serverStarted()
    │
    ├─→ [If UDP Server]
    │     └─→ udpServer->startServer(port)
    │
    ├─→ [If WebSocket Server]
    │     └─→ wsServer->startServer(port)
    │
    └─→ [If HTTP Server]
          └─→ httpServer->startServer(port)
```

### Server Receiving Messages

```
Client sends data to server
    │
    ▼
Server socket receives data
    │
    ▼
[TCP Server]
    │
    ├─→ QTcpSocket::readyRead() signal
    ├─→ TcpServer::onClientReadyRead()
    ├─→ Read data from client socket
    ├─→ Deserialize to DataMessage
    └─→ emit messageReceived(msg, clientInfo, timestamp)
        │
        ▼
MainWindow::onDataReceived()
```

### Server Broadcasting

```
User sends message in "Broadcast" mode
    │
    ▼
MainWindow::onSendRequested()
    │
    ├─→ Check send mode = "Broadcast"
    ├─→ Get server protocol
    │
    └─→ [If TCP Server]
          └─→ tcpServer->sendToAll(msg)
                │
                ├─→ Iterate all connected clients
                └─→ For each client:
                      └─→ clientSocket->write(msg.serialize())
```

---

## Signal-Slot Communication

### Communication Pattern

```
┌──────────────┐         Signal          ┌──────────────┐
│ UI Panel     │ ────────────────────→ │  MainWindow  │
│              │                         │              │
│ (User Action)│                         │ (Orchestrator)│
└──────────────┘                         └──────────────┘
                                                  │
                                                  │ Method Call
                                                  ▼
                                         ┌──────────────┐
                                         │   Network    │
                                         │  Component   │
                                         │              │
                                         │ (Network I/O)│
                                         └──────────────┘
                                                  │
                                                  │ Signal
                                                  ▼
                                         ┌──────────────┐
                                         │  MainWindow  │
                                         │              │
                                         │ (Event Handler)│
                                         └──────────────┘
                                                  │
                                                  │ Method Call
                                                  ▼
                                         ┌──────────────┐
                                         │   UI Panel   │
                                         │              │
                                         │ (Update UI)  │
                                         └──────────────┘
```

### Example: Complete Signal Chain

#### 1. User Clicks Connect
```cpp
// ConnectionPanel
connectBtn->clicked() 
    → ConnectionPanel::onConnectClicked()
    → emit connectRequested()
```

#### 2. MainWindow Receives Signal
```cpp
// MainWindow (connected in setupConnections())
connect(connectionPanel, &ConnectionPanel::connectRequested,
        this, &MainWindow::onConnectRequested);

// Slot called automatically
MainWindow::onConnectRequested() {
    tcpClient->connectToHost(host, port);
}
```

#### 3. Network Component Emits Signal
```cpp
// TcpClient
void TcpClient::onConnected() {
    emit connected();  // Signal emitted
}

// MainWindow receives (connected in initializeNetworkComponents())
connect(tcpClient, &TcpClient::connected,
        this, &MainWindow::updateStatus);
```

#### 4. MainWindow Updates UI
```cpp
// MainWindow::updateStatus()
void MainWindow::updateStatus() {
    connectionPanel->setConnectionState(true);  // Direct method call
    statusPanel->updateConnectionStatus("Connected");
}
```

---

## Data Format Processing

### Format Conversion Pipeline

```
User Input (String)
    │
    ▼
DataMessage::parseInput()
    │
    ├─→ [JSON] QJsonDocument::fromJson()
    ├─→ [XML]  QString (as-is)
    ├─→ [CSV]  QString (as-is)
    ├─→ [TEXT] QString (as-is)
    ├─→ [BINARY] QByteArray::fromHex()
    └─→ [HEX]   QByteArray::fromHex()
    │
    ▼
QVariant (parsed data)
    │
    ▼
DataMessage(format, data)
    │
    ▼
DataMessage::serialize()
    │
    ├─→ [JSON] QJsonDocument::toJson()
    ├─→ [XML]  QString::toUtf8()
    ├─→ [CSV]  QString::toUtf8()
    ├─→ [TEXT] QString::toUtf8()
    ├─→ [BINARY] QByteArray (as-is)
    └─→ [HEX]   QByteArray::toHex()
    │
    ▼
QByteArray (network bytes)
    │
    ▼
Network Transmission
```

### Format Validation Flow

```cpp
// Before sending
bool isValid = DataMessage::validateInput(messageText, format);

// Validation per format:
// - JSON: Valid JSON syntax
// - XML:  Non-empty string
// - CSV:  Non-empty string
// - TEXT: Always valid
// - HEX:  Only hex characters (0-9, A-F)
```

---

## Database Operations Flow

### Saving a Message

```
Message sent/received
    │
    ▼
MainWindow::onSendRequested() or onDataReceived()
    │
    ▼
historyManager.saveMessage(direction, protocol, host, port, msg)
    │
    ├─→ Acquire database mutex (thread safety)
    ├─→ Prepare SQL INSERT statement
    ├─→ Serialize DataMessage to JSON
    ├─→ Execute INSERT query
    ├─→ Release mutex
    └─→ Return success/failure
```

### Querying Messages

```
User searches history
    │
    ▼
DisplayPanel::searchMessages()
    │
    ▼
historyManager.getMessages(filter, startDate, endDate, limit, offset)
    │
    ├─→ Acquire database mutex
    ├─→ Build SQL SELECT query with filters
    ├─→ Execute query
    ├─→ Convert results to QList<QVariantMap>
    ├─→ Release mutex
    └─→ Return results
```

### Database Thread Safety

```cpp
// MessageHistoryManager uses QMutex for thread safety
QMutex dbMutex;

bool MessageHistoryManager::saveMessage(...) {
    QMutexLocker locker(&dbMutex);  // Lock mutex
    
    // Database operations...
    
    // Mutex automatically unlocked when locker goes out of scope
}
```

---

## Event Loop and Asynchronous Operations

### Qt Event Loop

```
QApplication::exec() starts event loop
    │
    ├─→ Process UI events (mouse clicks, key presses)
    ├─→ Process network events (socket readyRead, connected)
    ├─→ Process timer events
    ├─→ Process signal-slot connections
    └─→ Repeat until application exits
```

### Asynchronous Network I/O

**Key Point:** All network operations are **non-blocking**

```cpp
// This returns immediately, doesn't block
tcpClient->connectToHost(host, port);

// Connection happens asynchronously
// When complete, Qt emits connected() signal
// Your slot is called automatically
```

### Why Asynchronous?

- **UI Responsiveness**: Main thread never blocks
- **Multiple Connections**: Can handle many connections simultaneously
- **Event-Driven**: Reacts to network events as they occur

---

## Error Handling Flow

### Connection Errors

```
Network error occurs
    │
    ▼
Network component detects error
    │
    ├─→ [TcpClient] QTcpSocket::error() signal
    │     └─→ TcpClient::onError()
    │           └─→ emit errorOccurred(errorString)
    │
    ▼
MainWindow::onNetworkError()
    │
    ├─→ Display error in StatusPanel
    ├─→ Log error
    └─→ Update connection state
```

### Validation Errors

```
User input invalid
    │
    ▼
MainWindow::onSendRequested()
    │
    ├─→ DataMessage::validateInput() returns false
    │
    └─→ QMessageBox::warning() - show error dialog
```

---

## Summary: Key Flow Patterns

### 1. User Action → Network Operation
```
UI Panel → Signal → MainWindow Slot → Network Component Method
```

### 2. Network Event → UI Update
```
Network Component → Signal → MainWindow Slot → UI Panel Update
```

### 3. Data Transformation
```
User Input → Parse → DataMessage → Serialize → Network Bytes
```

### 4. Persistence
```
Message → HistoryManager → SQLite Database
```

### 5. Thread Safety
```
Database Operations → Mutex Lock → Execute → Unlock
```

---

## Debugging Tips

### Tracing Signal Flow

1. **Add qDebug() statements** in slots:
```cpp
void MainWindow::onConnectRequested() {
    qDebug() << "onConnectRequested() called";
    // ...
}
```

2. **Use Qt Creator's Signal/Slot Inspector** to see all connections

3. **Check signal emissions**:
```cpp
connect(tcpClient, &TcpClient::connected, 
        []() { qDebug() << "TCP connected signal emitted"; });
```

### Common Issues

1. **Signal not received**: Check if `connect()` was called
2. **Slot not called**: Verify signal is actually emitted
3. **UI not updating**: Ensure updates happen in main thread
4. **Data not saved**: Check database mutex and error returns

---

This document provides a comprehensive view of how code flows through the CommLink application. Each operation follows the patterns described above, ensuring a consistent and maintainable architecture.

