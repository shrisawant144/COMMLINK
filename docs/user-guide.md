# CommLink User Guide

## Table of Contents
1. [Getting Started](#getting-started)
2. [TCP/UDP Communication](#tcpudp-communication)
3. [HTTP Communication](#http-communication)
4. [WebSocket Communication](#websocket-communication)
5. [Message History](#message-history)
6. [File Operations](#file-operations)
7. [Theme Customization](#theme-customization)
8. [Troubleshooting](#troubleshooting)

---

## Getting Started

### Installation

#### Linux
```bash
# Install dependencies
sudo apt-get install qt5-default libqt5websockets5-dev

# Build from source
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run
./bin/commlink
```

#### Windows
```powershell
# Install Qt5 from qt.io
# Open CMake GUI, configure and generate
# Build with Visual Studio or MinGW
```

### First Launch

When you launch CommLink, you'll see a tabbed interface with five main sections:

1. **TCP/UDP Tab**: Traditional socket communication
2. **HTTP Tab**: RESTful API testing
3. **WebSocket Tab**: Real-time bidirectional communication
4. **History Tab**: Browse all past messages
5. **Logs Tab**: Monitor application activity

---

## TCP/UDP Communication

### TCP Client (Reliable, Connection-Oriented)

#### Step 1: Configure Connection
- **Protocol**: Select **TCP**
- **Mode**: Select **Client**
- **Host**: Enter server IP (e.g., `127.0.0.1` for localhost)
- **Port**: Enter server port (e.g., `5000`)

#### Step 2: Connect
Click **Connect** button. Status will show:
- ✅ **Connected** (green) - Ready to send messages
- ❌ **Disconnected** (red) - Connection failed or not established

#### Step 3: Send Message
1. **Select Format**: Choose JSON, XML, CSV, TEXT, BINARY, or HEX
2. **Enter Message**:
   - **JSON**: `{"message": "Hello", "from": "client"}`
   - **XML**: `<message from="client">Hello</message>`
   - **CSV**: `field1,field2,field3`
   - **TEXT**: `Plain text message`
   - **BINARY**: Binary data (hex input)
   - **HEX**: `48656C6C6F` (Hello in hex)
3. **Click Send**: Message transmitted immediately
4. **Check Logs**: Verify successful transmission

**Example TCP Client Session**:
```
Format: JSON
Message: {"action": "login", "username": "alice", "password": "secret123"}

[LOG] 2024-12-01 14:30:22 - TCP Connected to 127.0.0.1:5000
[LOG] 2024-12-01 14:30:25 - Sent: {"action": "login", ...}
[LOG] 2024-12-01 14:30:25 - Received: {"status": "success", "token": "abc123"}
```

### TCP Server (Listen for Connections)

#### Step 1: Configure Server
- **Protocol**: Select **TCP**
- **Mode**: Select **Server**
- **Port**: Enter listening port (e.g., `5000`)

#### Step 2: Start Listening
Click **Start Server** button. Status shows:
- 🟢 **Listening on port 5000** - Ready to accept connections
- ⚪ **Stopped** - Server not running

#### Step 3: Handle Incoming Connections
- Incoming connections automatically accepted
- Received messages appear in **Received Messages** area
- Each message shows: timestamp, client IP, port, and content

**Use Cases**:
- Test client applications connecting to your server
- Simulate server responses
- Debug protocol implementations

### UDP Client/Server (Fast, Connectionless)

UDP operates similarly to TCP but without connection establishment:

#### UDP Client
1. Set **Protocol** to **UDP**
2. Set **Mode** to **Client**
3. Enter **Host** and **Port**
4. Click **Send** directly (no Connect needed)

#### UDP Server
1. Set **Protocol** to **UDP**
2. Set **Mode** to **Server**
3. Enter **Port** to listen on
4. Click **Start Server**

**Key Differences**:
- **No Connection**: UDP sends immediately without handshake
- **No Guaranteed Delivery**: Messages may be lost (check logs)
- **Lower Latency**: Faster than TCP for real-time applications

**Example Use Cases**:
- Gaming network protocols
- Video streaming
- DNS queries
- IoT sensor data

---

## HTTP Communication

### HTTP Client (REST API Testing)

#### Basic GET Request
1. **Select HTTP Tab**
2. **Method**: Choose **GET**
3. **URL**: Enter endpoint (e.g., `http://api.example.com/users`)
4. **Headers** (optional):
   ```
   Content-Type: application/json
   Authorization: Bearer your-token-here
   ```
5. **Click Send Request**

**Response Display**:
- **Status Code**: 200 OK, 404 Not Found, etc.
- **Response Headers**: Server info, content type, etc.
- **Response Body**: JSON, XML, HTML, or text content

#### POST Request with JSON Body
1. **Method**: Select **POST**
2. **URL**: `http://api.example.com/users`
3. **Headers**:
   ```
   Content-Type: application/json
   ```
4. **Body Format**: Select **JSON**
5. **Body Content**:
   ```json
   {
     "name": "Alice Johnson",
     "email": "alice@example.com",
     "age": 28
   }
   ```
6. **Click Send Request**

#### PUT and DELETE Requests
- **PUT**: Update existing resources
  ```
  PUT http://api.example.com/users/123
  Body: {"name": "Alice Smith"}
  ```
  
- **DELETE**: Remove resources
  ```
  DELETE http://api.example.com/users/123
  ```

**Supported HTTP Methods**:
- **GET**: Retrieve data
- **POST**: Create new resources
- **PUT**: Update resources
- **DELETE**: Remove resources
- **PATCH**: Partial updates (if supported)
- **HEAD**: Get headers only
- **OPTIONS**: Check allowed methods

### HTTP Server (Simulate API Endpoints)

#### Setup Mock Server
1. **Select HTTP Tab**
2. **Mode**: Select **Server**
3. **Port**: Enter port (e.g., `8080`)
4. **Click Start Server**

#### Configure Mock Responses
Define how the server should respond to different requests:

**Example: Simple API**
```
Endpoint: /api/hello
Method: GET
Response: {"message": "Hello, World!", "timestamp": "2024-12-01T14:30:00Z"}
```

**Use Cases**:
- Mock backend APIs during frontend development
- Test client error handling (return 404, 500 errors)
- Simulate slow responses (add delays)
- Webhook testing

---

## WebSocket Communication

### WebSocket Client

#### Connect to WebSocket Server
1. **Select WebSocket Tab**
2. **Mode**: **Client**
3. **URL**: Enter WebSocket URL
   - **ws://**: Unencrypted WebSocket
   - **wss://**: Secure WebSocket (TLS)
   - Example: `ws://localhost:8080/chat`
4. **Click Connect**

#### Send Messages
1. **Select Format**: JSON, TEXT, etc.
2. **Enter Message**:
   ```json
   {
     "type": "chat",
     "user": "alice",
     "message": "Hello everyone!"
   }
   ```
3. **Click Send**: Message sent over persistent connection

#### Receive Real-Time Messages
- Messages from server appear instantly
- Two-way communication maintained
- Connection stays open until closed manually

**Example: Chat Application**
```
[14:30:22] Connected to ws://localhost:8080/chat
[14:30:25] Sent: {"type": "join", "user": "alice"}
[14:30:25] Received: {"type": "welcome", "message": "Welcome alice!"}
[14:30:30] Sent: {"type": "message", "text": "Hi everyone!"}
[14:30:30] Received: {"type": "message", "from": "bob", "text": "Hey alice!"}
```

### WebSocket Server

#### Start WebSocket Server
1. **Mode**: **Server**
2. **Port**: e.g., `8080`
3. **Path**: e.g., `/chat` (optional)
4. **Click Start Server**

#### Handle Client Connections
- Multiple clients can connect simultaneously
- Broadcast messages to all clients
- Send to specific clients
- Monitor connections in Logs tab

**Use Cases**:
- Real-time chat applications
- Live dashboards and monitoring
- Collaborative editing
- Gaming multiplayer features
- Live notifications

---

## Message History

### Searching Messages

#### Quick Search
Type in the search box to search across all message content:
```
Search: "error"          # Find all messages containing "error"
Search: "127.0.0.1"      # Find messages to/from specific IP
Search: "login"          # Find login-related messages
```

#### Advanced Filtering

**Filter by Date Range**:
- Start Date: `2024-12-01`
- End Date: `2024-12-31`

**Filter by Protocol**:
- Select: TCP, UDP, HTTP, or WebSocket

**Filter by Direction**:
- **Sent**: Only messages you sent
- **Received**: Only messages you received
- **Both**: All messages (default)

**Filter by Format**:
- JSON, XML, CSV, TEXT, BINARY, HEX

**Combined Filters Example**:
```
Search: "api"
Protocol: HTTP
Date Range: Last 7 days
Direction: Both
Format: JSON
```

### Viewing Message Details

Click any message in the history table to see:
- **Full Message Content**: Complete message body
- **Timestamp**: Exact send/receive time
- **Protocol**: TCP, UDP, HTTP, or WebSocket
- **Format**: Data format used
- **Endpoint**: Host and port
- **Status**: Success or error
- **Session Info**: Related messages grouped by session

### Exporting History

#### Export Filtered Results
1. Apply desired filters
2. Click **Export** button
3. Choose format:
   - **JSON**: Machine-readable, structured data
   - **CSV**: Spreadsheet-compatible, tabular format
   - **TXT**: Human-readable, plain text
4. Select save location

**Example CSV Export**:
```csv
Timestamp,Protocol,Direction,Format,Host,Port,Message,Status
2024-12-01 14:30:22,TCP,Sent,JSON,127.0.0.1,5000,"{""action"":""login""}",Success
2024-12-01 14:30:25,TCP,Received,JSON,127.0.0.1,5000,"{""status"":""ok""}",Success
```

---

## File Operations

### Loading Messages

#### Load from File
1. Click **Load Message** button (📁 icon)
2. **Select File**: Choose JSON, XML, CSV, TEXT, BINARY, or HEX file
3. **Automatic Detection**: Format auto-detected from file extension
4. **Message Loaded**: Content appears in message editor

**Supported File Extensions**:
- `.json` - JSON format
- `.xml` - XML format
- `.csv` - CSV format
- `.txt` - Plain text
- `.bin` - Binary data
- `.hex` - Hexadecimal

### Saving Messages

#### Save Current Message
1. Compose message in editor
2. Click **Save Message** button (💾 icon)
3. **Choose Format**: JSON, XML, CSV, TEXT, BINARY, HEX
4. **Select Location**: Choose directory and filename
5. **File Saved**: Message persisted for reuse

**Use Cases**:
- Save frequently used test messages
- Create message templates
- Share test cases with team
- Document API requests

### Exporting Logs

#### Export Application Logs
1. Navigate to **Logs Tab**
2. Click **Export Logs** button
3. Choose format: **TXT** or **CSV**
4. Select save location

**Log Export Includes**:
- All connection events
- Message send/receive events
- Error messages and warnings
- Timestamps for every entry

---

## Theme Customization

### Changing Theme

#### Theme Options
1. Click **Settings** or **Theme** menu
2. Select theme:
   - **Light**: Bright theme for well-lit environments
   - **Dark**: Easy on the eyes, ideal for low light
   - **System**: Match your operating system theme

#### Theme Persistence
Your theme choice is automatically saved and restored when you relaunch the application.

### Custom Styling (Advanced)

For advanced users, Qt stylesheets can be customized:
```css
/* Example: Custom button colors */
QPushButton {
    background-color: #007bff;
    color: white;
    border-radius: 4px;
}
```

---

## Troubleshooting

### Connection Issues

#### "Connection Refused"
**Cause**: No server listening on specified host:port  
**Solution**:
- Verify server is running
- Check host and port are correct
- Ensure firewall allows connection

#### "Connection Timeout"
**Cause**: Server unreachable or network issue  
**Solution**:
- Ping the host to verify network connectivity
- Check for firewall or router blocking
- Verify correct IP address

#### "Address Already in Use"
**Cause**: Port already bound by another application  
**Solution**:
- Choose a different port
- Stop conflicting application
- On Linux: `sudo lsof -i :PORT` to find process

### Message Format Errors

#### "Invalid JSON"
**Cause**: Malformed JSON syntax  
**Solution**:
- Check for missing quotes: `"key": "value"`
- Ensure proper commas between properties
- Use online JSON validators
- Check bracket/brace matching

**Common JSON Errors**:
```json
❌ Bad:  {name: "Alice"}           # Missing quotes around key
✅ Good: {"name": "Alice"}

❌ Bad:  {"a": 1, "b": 2,}         # Trailing comma
✅ Good: {"a": 1, "b": 2}

❌ Bad:  {"text": 'hello'}         # Single quotes
✅ Good: {"text": "hello"}
```

#### "Invalid XML"
**Cause**: Malformed XML structure  
**Solution**:
- Ensure opening and closing tags match
- Escape special characters: `&lt;`, `&gt;`, `&amp;`
- Validate with XML validator

### Performance Issues

#### Slow Message History Search
**Cause**: Large message history database  
**Solution**:
- Apply more specific filters
- Clear old messages periodically
- Rebuild database indexes (Settings → Maintenance)

#### UI Freezing
**Cause**: Sending/receiving very large messages  
**Solution**:
- Reduce message size
- Use BINARY format for large data
- Check network speed

### Database Errors

#### "Database Locked"
**Cause**: SQLite database accessed by multiple processes  
**Solution**:
- Close other CommLink instances
- Check for file locks: `lsof | grep commlink.db`
- Restart application

#### "Database Corrupted"
**Cause**: Unexpected shutdown or disk error  
**Solution**:
- Backup database: `~/.commlink/messages.db`
- Restore from backup
- Use SQLite recovery tools
- In worst case, delete and recreate database

---

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl+T` | New TCP connection |
| `Ctrl+H` | New HTTP request |
| `Ctrl+W` | New WebSocket connection |
| `Ctrl+S` | Save current message |
| `Ctrl+O` | Open/Load message file |
| `Ctrl+F` | Focus search in History |
| `Ctrl+L` | Clear logs |
| `Ctrl+,` | Open settings |
| `Ctrl+Q` | Quit application |
| `F5` | Refresh history |

---

## Tips & Best Practices

### Testing Best Practices
1. **Start Local**: Test with `127.0.0.1` before remote hosts
2. **Use History**: Reference past successful messages
3. **Export Logs**: Keep records of test sessions
4. **Format Validation**: Verify message format before sending
5. **Error Handling**: Test both success and failure scenarios

### Performance Tips
1. **Batch Export**: Export multiple messages at once for efficiency
2. **Filter Early**: Apply filters before searching large histories
3. **Close Unused Connections**: Free resources when done
4. **Clean History**: Periodically remove old test data

### Security Reminders
- ⚠️ **No Encryption**: Don't send sensitive data over plain TCP/UDP/HTTP
- ⚠️ **Localhost Testing**: Keep testing on local network when possible
- ⚠️ **Firewall Rules**: Be cautious when opening ports
- ✅ **Use wss://**: For WebSocket, prefer secure WebSocket when available

---

## Getting Help

### Documentation
- **architecture.md**: System design and technical details
- **Beginners_Complete_Guide.md**: Learning path for newcomers
- **Source_Code_Analysis.md**: In-depth code explanation

### Support
- **GitHub Issues**: Report bugs or request features
- **Community**: Join discussions and share experiences
- **Logs**: Include log exports when reporting issues

### Contributing
Contributions welcome! See CONTRIBUTING.md for guidelines.

---

**Happy Testing!** 🚀
