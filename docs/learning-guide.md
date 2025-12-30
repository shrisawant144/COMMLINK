# Learning Guide

## New to Networking?

This guide explains networking concepts using CommLink examples.

## Chapter 1: Basic Concepts

### What is a Network Protocol?
A **protocol** is like a language that computers use to talk to each other.

**Real-world analogy**: Phone conversation rules
- Dial number (establish connection)
- Say "Hello" (handshake)
- Take turns talking (data exchange)
- Say "Goodbye" (close connection)

### IP Address and Port
- **IP Address**: Like a street address (`192.168.1.100`)
- **Port**: Like an apartment number (`5000`)
- **Together**: Complete address (`192.168.1.100:5000`)

## Chapter 2: Protocol Types

### TCP (Reliable)
**Like registered mail** - guaranteed delivery, in order

**Use TCP when:**
- Data must arrive completely
- Order matters
- Examples: Web browsing, file transfer

**CommLink example:**
```
TCP Server: Port 5000
TCP Client: Connect to 127.0.0.1:5000
Send: "Hello TCP!"
```

### UDP (Fast)
**Like regular mail** - fast but no guarantee

**Use UDP when:**
- Speed is important
- Some data loss is okay
- Examples: Video streaming, gaming

**CommLink example:**
```
UDP Server: Port 5001  
UDP Client: Send to 127.0.0.1:5001
Send: "Hello UDP!"
```

### HTTP (Web)
**Like filling out forms** - request and response

**Use HTTP for:**
- Web APIs
- REST services
- Getting web data

**CommLink example:**
```
Method: GET
URL: https://api.github.com/users/octocat
Response: User information in JSON
```

### WebSocket (Real-time)
**Like a phone call** - continuous conversation

**Use WebSocket for:**
- Chat applications
- Live updates
- Real-time data

**CommLink example:**
```
URL: ws://echo.websocket.org
Send: {"message": "Hello!"}
Receive: {"message": "Hello!"} (echoed back)
```

## Chapter 3: Data Formats

### JSON (Most Popular)
**JavaScript Object Notation** - easy to read

```json
{
  "name": "Alice",
  "age": 30,
  "hobbies": ["reading", "coding"]
}
```

**When to use**: Web APIs, configuration files

### XML (Structured)
**Extensible Markup Language** - like HTML

```xml
<person>
  <name>Alice</name>
  <age>30</age>
  <hobbies>
    <hobby>reading</hobby>
    <hobby>coding</hobby>
  </hobbies>
</person>
```

**When to use**: Legacy systems, complex data structures

### CSV (Spreadsheet)
**Comma-Separated Values** - like Excel

```csv
name,age,city
Alice,30,New York
Bob,25,London
```

**When to use**: Data export, simple tables

## Chapter 4: Hands-On Exercises

### Exercise 1: TCP Chat
**Goal**: Create a simple chat between two CommLink instances

1. **Instance 1** (Server):
   - Server Panel → TCP Server → Port: 5000 → Start Server

2. **Instance 2** (Client):  
   - Connection Panel → TCP → Host: 127.0.0.1, Port: 5000 → Connect

3. **Chat**:
   - Message Panel → Type messages → Send
   - View in Client/Server Received tabs

**What you learned**: TCP connection, client-server model

### Exercise 2: API Testing
**Goal**: Test a real web API

1. **Get user info**:
   - Connection Panel → HTTP → Host: jsonplaceholder.typicode.com → Method: GET → Connect
   - Message Panel → Send (URL: /users/1 will be appended)
   - View JSON response in Client Received tab

2. **Create new post**:
   - Connection Panel → HTTP → Method: POST
   - Message Panel → Format: JSON → Message: `{"title": "My Post", "body": "Hello World", "userId": 1}` → Send

**What you learned**: HTTP methods, JSON APIs

### Exercise 3: WebSocket Echo
**Goal**: Real-time communication

1. **Connect**:
   - Connection Panel → WebSocket → URL: ws://echo.websocket.org → Connect

2. **Send messages**:
   - Message Panel → Send: `Hello WebSocket!`
   - View echo in Client Received tab

3. **Try JSON**:
   - Message Panel → Format: JSON → Send: `{"type": "greeting", "message": "Hi there!"}`
   - See it echoed back

**What you learned**: Real-time communication, WebSocket protocol

## Chapter 5: Common Patterns

### Request-Response (HTTP)
```
Client → Request → Server
Client ← Response ← Server
```

### Publish-Subscribe (WebSocket)
```
Client A → Message → Server → Message → Client B
                           → Message → Client C
```

### Peer-to-Peer (TCP)
```
Client A ↔ Direct Connection ↔ Client B
```

## Chapter 6: Troubleshooting

### "Connection Refused"
- Check if server is running
- Verify IP address and port
- Check firewall settings

### "Invalid JSON"
- Use JSON validator online
- Check for missing quotes or commas
- Look at Logs tab for details

### "Timeout"
- Server might be slow or down
- Try increasing timeout
- Check network connection

## Next Steps

1. **Practice**: Try all exercises multiple times
2. **Experiment**: Change ports, formats, messages
3. **Build**: Create your own simple server/client
4. **Learn more**: Study network programming books
5. **Contribute**: Help improve CommLink documentation
