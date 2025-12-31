# Architecture

Technical overview of CommLink's internal architecture and design principles.

## Core Design Philosophy: A Modular Orchestrator

The application is built around a central **orchestrator** pattern to avoid the pitfalls of a monolithic design. The `MainWindow` class (`include/commlink/ui/mainwindow.h`) serves as this central orchestrator.

Instead of containing all application logic, `MainWindow`'s primary responsibilities are:
1.  **Instantiation**: It creates and owns all major sub-components (UI panels, network handlers).
2.  **Connection**: It wires components together using Qt's **Signals and Slots** mechanism.
3.  **State Management**: It tracks the overall application state (e.g., which protocol is active, connection status).

This modular approach makes the system easier to maintain, test, and extend. For example, adding a new protocol like `MQTT` would involve creating a new `MqttClient` and adding it to `MainWindow` without needing to modify the existing UI panels directly.

---

## The `MainWindow` Class

`MainWindow` is the heart of the application. It inherits from `QMainWindow`, giving it the standard features of a desktop application window.

### Key C++ & Qt Concepts in Use

A review of `MainWindow.h` reveals several important C++ and Qt concepts that are fundamental to the application's design.

#### 1. Inheritance
```cpp
class MainWindow : public QMainWindow
```
By inheriting from `QMainWindow`, our class gets a vast amount of built-in functionality for free, including support for menu bars, toolbars, a status bar, and proper window management within the operating system.

#### 2. The `Q_OBJECT` Macro
```cpp
Q_OBJECT
```
This macro is mandatory for any Qt object that defines its own signals or slots. It enables Qt's **Meta-Object System**, which provides the underlying power for the signal-slot mechanism.

#### 3. Signals and Slots
This is the core communication mechanism in the application. It allows for complete decoupling between components. The `private slots:` section of `MainWindow` defines the functions that will react to events.

**Example Flow:**
1.  A user clicks the "Connect" button inside the `ConnectionPanel` widget.
2.  The `ConnectionPanel` object emits a signal (e.g., `connectClicked()`).
3.  `MainWindow` connects this signal to its `onConnectRequested()` slot during setup.
4.  The `onConnectRequested()` function is automatically executed, where it can read data from the `ConnectionPanel` and instruct the appropriate network client to connect.

#### 4. Pointers and Object Ownership
```cpp
// UI Panels
ConnectionPanel *connectionPanel;
// ...
// Network components
TcpClient *tcpClient;
```
`MainWindow` holds **pointers** to its sub-components. This is because these objects are created dynamically on the heap during the `setupUI()` and `initializeNetworkComponents()` phases. `MainWindow` is the **owner** of these objects and is responsible for their lifecycle. They are destroyed when `MainWindow` is destroyed.

#### 5. Event Overriding
```cpp
protected:
    void closeEvent(QCloseEvent *event) override;
```
`MainWindow` overrides the `closeEvent` virtual function from `QMainWindow`. This allows it to intercept the user's request to close the window. Inside this function, we can execute cleanup code, such as calling `saveSettings()` to persist the window size and other user preferences before allowing the application to exit.

---

## Component Breakdown

The application is logically divided into three main categories of components, all managed by `MainWindow`.

### 1. UI Panels
*(e.g., `ConnectionPanel`, `MessagePanel`, `DisplayPanel`)*

*   **Responsibility**: Each panel is a self-contained `QWidget` that manages a specific part of the user interface.
*   **Communication**: They do not talk to each other directly. Instead, they emit signals to report user actions (e.g., `protocolChanged`, `sendRequested`).
*   **State**: They hold UI-specific state but rely on `MainWindow` for application-level state.

### 2. Network Components
*(e.g., `TcpClient`, `UdpServer`, `WebSocketClient`)*

*   **Responsibility**: Each class encapsulates all the logic for a single network protocol. They handle the low-level details of connecting, sending, receiving, and disconnecting.
*   **Communication**: They run asynchronously to prevent blocking the UI. They emit signals for network events like `onDataReceived`, `onClientConnected`, or `onNetworkError`.

### 3. Core/Business Logic Components
*(e.g., `MessageHistoryManager`, `ThemeManager`, `FileManager`)*

*   **Responsibility**: These classes provide shared services that are used across the application.
*   `MessageHistoryManager`: Handles the logic for storing and retrieving messages from the SQLite database.
*   `ThemeManager`: Manages the application's look and feel (Light/Dark/Auto modes).
*   `FileManager`: Provides utility functions for loading and saving message payloads from/to files.