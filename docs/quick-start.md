# Quick Start Guide

## What is CommLink?
CommLink is a simple tool for testing network communication. Think of it as a universal messenger that can speak different network languages.

## Install & Run

### Linux
```bash
./scripts/install-deps.sh
./scripts/build.sh
./build/bin/commlink
```

### Windows
1. Install Qt5 from qt.io
2. Run: `mkdir build && cd build && cmake .. && cmake --build .`
3. Run: `.\bin\Release\commlink.exe`

## Basic Usage

### 1. TCP Chat
- **Server**: Server Panel → TCP Server → Port 5000 → Start Server
- **Client**: Connection Panel → TCP → Host: 127.0.0.1, Port: 5000 → Connect
- **Send**: Message Panel → Type message → Send
- **View**: Check Client/Server Received tabs

### 2. Test APIs
- **Connect**: Connection Panel → HTTP → Host: api.github.com → Method: GET → Connect
- **Send**: Message Panel → Send (or with JSON body for POST)
- **View**: Response in Client Received tab

### 3. WebSocket Chat
- **Connect**: Connection Panel → WebSocket → URL: ws://echo.websocket.org → Connect
- **Send**: Message Panel → `{"message": "Hello!"}` → Send
- **View**: Real-time responses in Client Received tab

### 4. View History
- **Display Panel → History tab** → Search and filter messages
- Export to CSV/JSON if needed

## Data Formats
- **JSON**: `{"name": "Alice", "age": 30}`
- **XML**: `<user><name>Alice</name></user>`
- **TEXT**: Plain text messages
- **HEX**: Raw bytes in hexadecimal

## Need Help?
- Check the **Logs tab** for errors
- Use **File → Load** to import saved messages
- Switch themes in **View → Theme**
