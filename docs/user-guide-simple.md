# User Guide

## Getting Started

### What CommLink Does
CommLink helps you test network communication. It supports:
- **TCP/UDP**: Direct socket communication
- **HTTP**: REST API testing  
- **WebSocket**: Real-time messaging
- **Multiple formats**: JSON, XML, CSV, text, binary, hex

### Installation
```bash
# Linux - install dependencies and build
./scripts/install-deps.sh && ./scripts/build.sh

# Windows - install Qt5 first, then:
mkdir build && cd build && cmake .. && cmake --build .
```

## Interface Layout

CommLink has a **panel-based interface** with two main sections:

### Left Panel (Controls)
- **Connection Panel**: Connect to servers (TCP, UDP, HTTP, WebSocket)
- **Server Panel**: Start your own servers
- **Message Panel**: Compose and send messages

### Right Panel (Display)
- **Display Panel**: View messages in tabs:
  - Client Received
  - Server Received  
  - Sent Messages
  - All Messages
  - Logs
  - History
- **Status Panel**: Connection status and statistics

## Using Each Panel

### Connection Panel
**For connecting to remote servers**

1. **Protocol**: Choose TCP, UDP, HTTP, or WebSocket
2. **Host**: Enter server IP (e.g., 127.0.0.1)
3. **Port**: Enter port number (e.g., 5000)
4. **Method**: For HTTP, choose GET/POST/PUT/DELETE
5. Click **Connect**

### Server Panel
**For starting your own servers**

1. **Protocol**: Choose TCP Server, UDP Server, WebSocket Server, or HTTP Server
2. **Port**: Set listening port (e.g., 5000)
3. **Send Mode**: Choose Individual or Broadcast
4. Click **Start Server**

### Message Panel
**For composing messages**

1. **Format**: Choose JSON, XML, CSV, TEXT, BINARY, or HEX
2. **Message**: Type your message content
3. **Load/Save**: Import/export message files
4. Click **Send**

### Display Panel Tabs
**View different message types**

- **📥 Client Received**: Messages received as client
- **📨 Server Received**: Messages received as server
- **📤 Sent Messages**: Messages you sent
- **📬 All Messages**: Combined view of all messages
- **Logs**: Application activity and errors
- **History**: Searchable database of past messages

## Data Formats

### JSON (Most Common)
```json
{
  "name": "Alice",
  "age": 30,
  "active": true
}
```

### XML
```xml
<user>
  <name>Alice</name>
  <age>30</age>
</user>
```

### CSV
```
name,age,active
Alice,30,true
Bob,25,false
```

### Text
Plain text messages

### Binary/Hex
Raw byte data in hexadecimal format

## Common Workflows

### TCP Chat Between Two CommLink Instances
1. **Instance 1**: Server Panel → TCP Server → Port 5000 → Start Server
2. **Instance 2**: Connection Panel → TCP → Host: 127.0.0.1, Port: 5000 → Connect
3. **Both**: Message Panel → Type message → Send
4. **View**: Check Client/Server Received tabs

### Test REST API
1. **Connection Panel**: HTTP → Host: api.example.com → Method: GET → Connect
2. **Message Panel**: Format: JSON → Message: `{"key": "value"}` → Send
3. **View**: Check Client Received tab for response

### WebSocket Real-Time
1. **Connection Panel**: WebSocket → URL: ws://echo.websocket.org → Connect
2. **Message Panel**: Format: JSON → Message: `{"hello": "world"}` → Send
3. **View**: Real-time responses in Client Received tab

## File Operations

### Save Messages
- **Message Panel → Save** - Save current message
- **Display Panel → Export** - Export all messages

### Load Messages  
- **Message Panel → Load** - Import message file
- Auto-detects format (JSON, CSV, TXT)

### Export History
- **History tab → Export** - Export filtered history
- Choose date range and format

## Troubleshooting

### Connection Issues
- Check Connection Panel status indicator
- Verify host/port in Connection Panel
- Look at Logs tab for error details

### Message Not Sending
- Ensure connection established (green status)
- Validate message format in Message Panel
- Check Logs tab for send errors

### Server Not Starting
- Check if port is already in use
- Verify port number in Server Panel
- Look at Logs tab for server errors

## Keyboard Shortcuts
- **Ctrl+S**: Save current message
- **Ctrl+O**: Load message file
- **Ctrl+Q**: Quit application
- **F11**: Toggle fullscreen
