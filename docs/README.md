# CommLink Documentation

## Start Here

### New Users
- **[Quick Start Guide](quick-start.md)** - Get up and running in 5 minutes
- **[Learning Guide](learning-guide.md)** - Understand networking concepts with examples
- **[FAQ](faq.md)** - Common questions and solutions

### Regular Users  
- **[User Guide](user-guide-simple.md)** - Complete usage instructions
- **[Keyboard Shortcuts](Keyboard_Shortcuts_Reference.md)** - Speed up your workflow

### Developers
- **[Developer Guide](developer-guide.md)** - Build, test, and contribute
- **[Architecture](architecture-simple.md)** - System design overview

## What is CommLink?

CommLink is a **network communication testing tool** that helps you:

✅ Test TCP/UDP socket connections  
✅ Debug REST APIs (HTTP)  
✅ Test WebSocket real-time communication  
✅ Work with multiple data formats (JSON, XML, CSV, etc.)  
✅ Save and export message history  

## Quick Examples

### Test an API
```
Connection Panel → HTTP → Host: api.github.com → Method: GET → Connect
Message Panel → Send → View response in Client Received tab
```

### Create TCP Chat
```
Instance 1: Server Panel → TCP Server → Port 5000 → Start Server
Instance 2: Connection Panel → TCP → Host: 127.0.0.1, Port: 5000 → Connect
Message Panel → Type message → Send
```

### WebSocket Echo Test
```
Connection Panel → WebSocket → URL: ws://echo.websocket.org → Connect
Message Panel → Format: JSON → Message: {"hello": "world"} → Send
```

## Need Help?

1. **Check [FAQ](faq.md)** for common issues
2. **Look at Logs tab** in CommLink for error details  
3. **Create GitHub issue** with details if problem persists

## File Organization

```
docs/
├── README.md                       # This file - start here
├── quick-start.md                  # 5-minute setup guide
├── user-guide-simple.md            # Complete user manual
├── learning-guide.md               # Networking concepts explained
├── developer-guide.md              # Build and contribute
├── architecture-simple.md          # System design
├── faq.md                          # Common questions
└── Keyboard_Shortcuts_Reference.md # Shortcuts list
```
