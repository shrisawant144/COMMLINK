# Qt Features Used in JsonSender

This README explains the Qt (Qt5) framework features demonstrated in the JsonSender project. Qt is a cross-platform application development framework that simplifies GUI programming, networking, and more.

## 1. QApplication and Event Loop

### Application Entry Point
**From main.cpp:**
```cpp
#include <QApplication>
#include "gui.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);  // Create Qt application
    
    JsonSenderGUI window;          // Create main window
    window.show();                 // Show the window
    
    return app.exec();             // Start event loop
}
```

- `QApplication`: Manages the application's event loop and main settings
- `app.exec()`: Runs the event loop, processing user interactions until app quits
- Every Qt GUI application needs exactly one QApplication instance

## 2. QWidget and GUI Hierarchy

### Base Class for UI Elements
**From gui.h:**
```cpp
class JsonSenderGUI : public QWidget
{
    Q_OBJECT  // Required for signals/slots
public:
    JsonSenderGUI();
    
private slots:
    void onConnect();
    void onSend();
    
private:
    // UI element pointers
    QComboBox *protocolCombo;
    // ... other widgets
};
```

- `QWidget`: Base class for all UI elements
- All custom GUI classes inherit from QWidget
- `Q_OBJECT`: Macro that enables Qt's meta-object features (signals/slots, properties)

## 3. Layout Management

### Automatic Widget Arrangement
**From gui.cpp:**
```cpp
auto *layout = new QVBoxLayout(this);  // Vertical layout for main window

// Horizontal layout for connection settings
auto *connLayout = new QHBoxLayout();
connLayout->addWidget(new QLabel("Protocol:"));
connLayout->addWidget(protocolCombo);
// ... add more widgets

layout->addLayout(connLayout);  // Add horizontal layout to vertical
```

- `QVBoxLayout`: Arranges widgets vertically
- `QHBoxLayout`: Arranges widgets horizontally
- Layouts automatically resize and position widgets
- No manual coordinate calculation needed

## 4. Common Widgets

### Input and Display Widgets with Validation
```cpp
QComboBox *protocolCombo;     // Dropdown selection
QLineEdit *hostEdit;          // Single-line text input with validation
QPushButton *connectBtn;      // Clickable button with state management
QTextEdit *jsonEdit;          // Multi-line text editor
QIntValidator *portValidator; // Input validation for port numbers
```

- Each widget serves a specific UI purpose
- Input validation prevents invalid data entry
- State management provides user feedback
- Seamless integration with layout system

## 5. Signals and Slots (Event Handling)

### Connecting Events to Actions
**From gui.cpp:**
```cpp
connect(connectBtn, &QPushButton::clicked, this, &JsonSenderGUI::onConnect);
connect(sendBtn, &QPushButton::clicked, this, &JsonSenderGUI::onSend);
connect(startReceiveBtn, &QPushButton::clicked, this, &JsonSenderGUI::onStartReceive);
connect(&sender, &Sender::jsonReceived, this, &JsonSenderGUI::onJsonReceived);
```

- **Signals**: Events emitted by objects (button clicks, data received)
- **Slots**: Functions that respond to signals
- `connect()`: Links signals to slots with type safety
- Cross-thread signal/slot connections are automatically queued

### Slot Implementation
```cpp
private slots:
    void onConnect();  // Declaration in header
    void onSend();     // Implementation in cpp
```

Slots are normal member functions marked with `slots:`.

## 6. JSON Handling

### Qt's JSON Classes
**From gui.cpp:**
```cpp
#include <QJsonDocument>

QJsonParseError error;
QJsonDocument doc = QJsonDocument::fromJson(jsonText.toUtf8(), &error);

if (error.error != QJsonParseError::NoError) {
    logEdit->append("❌ Invalid JSON: " + error.errorString());
    return;
}
```

- `QJsonDocument`: Represents a JSON document
- `QJsonParseError`: Contains parsing error information
- Automatic conversion between JSON text and structured data

## 7. String and Byte Array Classes

### QString and QByteArray
```cpp
QString proto = protocolCombo->currentText();  // Qt string class
QString host = hostEdit->text();
int port = portEdit->text().toInt();  // Convert string to int

QByteArray payload = doc.toJson(QJsonDocument::Compact) + "\n";
```

- `QString`: Unicode-aware string class with many utility methods
- `QByteArray`: Efficient byte array for binary data
- Automatic encoding/decoding between formats

## 8. Logging and Debugging

### Qt Logging Macros
**From sender.cpp:**
```cpp
#include <QDebug>

qCritical() << "TCP socket creation failed";  // Error messages
qInfo() << "✅ RAW TCP:" << host << ":" << port;  // Info messages
qWarning() << "TCP partial send!";  // Warning messages
```

- `qCritical()`: Critical errors
- `qInfo()`: Informational messages
- `qWarning()`: Warnings
- Automatically includes timestamps and can be filtered

## 9. QObject Inheritance

### Qt Object System
**From sender.h:**
```cpp
class Sender : public QObject
{
    Q_OBJECT
    // ...
};
```

- `QObject`: Base class providing signals/slots, properties, and memory management
- Required for classes that need Qt's advanced features
- Automatic memory management through parent-child relationships

## 10. Meta-Object Compiler (moc)

### Code Generation
Qt uses a code generator called moc (Meta-Object Compiler) to implement:
- Signal/slot mechanism
- Runtime type information
- Properties system

Files with `Q_OBJECT` are processed by moc to generate additional C++ code.

## 11. Build System Integration

### CMake Configuration
**From CMakeLists.txt:**
```cmake
find_package(Qt5 REQUIRED COMPONENTS Core Widgets)
set(CMAKE_AUTOMOC ON)  # Enable automatic moc processing

target_link_libraries(JsonSender 
    Qt5::Core 
    Qt5::Widgets
)
```

- `CMAKE_AUTOMOC`: Automatically runs moc on files with Q_OBJECT
- Links against Qt libraries
- Cross-platform build configuration

## 12. Input Validation

### QValidator Classes
```cpp
void JsonSenderGUI::setupValidators() {
    portValidator = new QIntValidator(1, 65535, this);
    portEdit->setValidator(portValidator);
    receivePortEdit->setValidator(portValidator);
}
```

- `QIntValidator`: Restricts input to valid integer ranges
- Prevents invalid port numbers (1-65535)
- Real-time validation as user types
- Integrated with Qt's input system

## Key Qt Advantages Demonstrated

1. **Cross-platform**: Same code runs on Windows, macOS, Linux
2. **Event-driven**: Clean separation of UI and logic
3. **Thread-safe**: Automatic queuing of cross-thread signals
4. **Input validation**: Built-in validators for data integrity
5. **Rich widget set**: Pre-built UI components with state management
6. **Integrated features**: JSON, networking, threading, logging
7. **Type safety**: Compile-time signal/slot checking
8. **Memory management**: Parent-child object trees with automatic cleanup

Qt provides a comprehensive framework that simplifies complex GUI, networking, and threading tasks while maintaining high performance and native look-and-feel.
