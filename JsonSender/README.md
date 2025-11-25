# JSON Sender/Receiver - A Beginner-Friendly Network Communication Tool

## What Is This Application?

This is a **desktop application** that lets you send and receive **JSON messages** (a common data format) over computer networks. Think of it like a specialized messaging app for computers to talk to each other.

### Real-World Example
Imagine you have two computers:
- **Computer A** wants to send a message: `{"temperature": 25, "humidity": 60}`
- **Computer B** wants to receive this weather data
- This application makes that communication possible!

## What You Can Do With This App

### 📤 **Sending Messages**
- Choose how to send (TCP or UDP - different delivery methods)
- Enter the destination computer's address (like a postal address)
- Type your JSON message
- Click send and watch it go!

### 📥 **Receiving Messages**
- Set up your computer to listen for incoming messages
- See messages arrive in real-time
- View who sent each message and when

### 📋 **Monitoring Everything**
- See a detailed log of all activities
- Track successful sends and receives
- Spot any errors or connection problems

## Project Structure (What Files Do What)

```
JsonSender/
├── 📁 src/                    # The main program code
│   ├── main.cpp              # Starts the application
│   ├── gui.cpp               # Creates the windows and buttons
│   ├── sender.cpp            # Handles sending messages
│   ├── receiver.cpp          # Handles receiving messages
│   └── receiverthread.cpp    # Manages background listening
├── 📁 include/               # Code blueprints (headers)
│   ├── gui.h                 # GUI blueprint
│   ├── sender.h              # Sender blueprint
│   ├── receiver.h            # Receiver blueprint
│   └── receiverthread.h      # Background thread blueprint
├── 📁 docs/                  # Detailed explanations
│   ├── Project_Overview.md   # Big picture explanation
│   ├── CPP_Programming_Concepts.md  # C++ concepts used
│   ├── Qt_Framework_Features.md     # GUI framework features
│   └── Source_Code_Analysis.md      # Code deep-dive
├── CMakeLists.txt            # Build instructions for computer
├── README.md                 # This file you're reading
└── 📁 test_build/           # Compiled application (generated)
```

## Key Features Explained Simply

### 🔄 **Two-Way Communication**
- **Send**: Your computer talks to another computer
- **Receive**: Your computer listens for messages from others
- **Simultaneous**: Can do both at the same time!

### 🛡️ **Safety Features**
- **Input Validation**: Prevents you from entering invalid data
- **Error Handling**: Shows helpful messages when something goes wrong
- **Connection Management**: Properly connects and disconnects

### 🎯 **User-Friendly Interface**
- **Tabbed Layout**: Organized sections for sending, receiving, and logs
- **Real-time Feedback**: See what's happening as it happens
- **Status Updates**: Always know if you're connected or not

## How to Build and Run

### Prerequisites (What You Need First)
1. **Qt5 Development Libraries** - The GUI framework
2. **CMake** - The build system
3. **C++ Compiler** - To compile the code (GCC or Clang)

### Building Steps
```bash
# 1. Create a build folder
mkdir test_build && cd test_build

# 2. Configure the build
cmake ..

# 3. Compile the application
make

# 4. Run the application
./bin/JsonSender
```

## How to Use the Application

### Step 1: Sending Your First Message
1. **Open the "Sending" tab**
2. **Choose Protocol**: 
   - TCP = Reliable delivery (like registered mail)
   - UDP = Fast delivery (like regular mail)
3. **Enter Destination**:
   - Host: `127.0.0.1` (your own computer for testing)
   - Port: `5000` (like a specific mailbox number)
4. **Click "Connect"** - Should show green checkmark
5. **Type JSON Message**: `{"hello": "world"}`
6. **Click "Send JSON"** - Message sent!

### Step 2: Receiving Messages
1. **Open the "Receiving" tab**
2. **Set Listen Port**: `5001` (different from sending port)
3. **Click "Start Receiving"** - Now listening!
4. **Watch for Messages**: They'll appear in the "Received Messages" area

### Step 3: Monitoring Activity
1. **Open the "Logs" tab**
2. **See Everything**: All connections, sends, receives, and errors
3. **Timestamps**: Know exactly when each event happened

## Technical Details (For the Curious)

### Programming Language: C++17
- **Modern C++**: Uses latest features for safety and performance
- **Object-Oriented**: Code organized into logical classes
- **Memory Safe**: Automatic cleanup prevents crashes

### GUI Framework: Qt5
- **Cross-Platform**: Works on Windows, Mac, and Linux
- **Native Look**: Matches your operating system's style
- **Event-Driven**: Responds to clicks, typing, and network events

### Networking: POSIX Sockets
- **Low-Level**: Direct control over network communication
- **Reliable**: Industry-standard networking approach
- **Flexible**: Supports both TCP and UDP protocols

### Threading: Background Processing
- **Non-Blocking**: GUI stays responsive while networking happens
- **Thread-Safe**: Multiple operations can happen simultaneously
- **Proper Cleanup**: Resources are properly managed

## Common Use Cases

### 🎓 **Learning and Education**
- Understand how network communication works
- Learn JSON data format
- See real-time network activity

### 🔧 **Development and Testing**
- Test APIs and web services
- Debug network communication issues
- Simulate client-server interactions

### 🏢 **Professional Applications**
- IoT device communication
- Microservice testing
- Network protocol development

## Troubleshooting Common Issues

### "Connection Failed"
- **Check if target computer is running**
- **Verify IP address and port number**
- **Make sure no firewall is blocking**

### "Invalid JSON"
- **Check for missing quotes around strings**
- **Ensure proper comma placement**
- **Use online JSON validators to check format**

### "Port Already in Use"
- **Choose a different port number**
- **Close other applications using that port**
- **Wait a moment and try again**

This application is designed to be both educational and practical - perfect for learning networking concepts while having a useful tool for development work!