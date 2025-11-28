# CommLink Architecture

## System Overview

CommLink follows a modular, event-driven architecture using Qt's signal-slot mechanism for component communication.

## Component Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                         GUI (Main Window)                    │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐   │
│  │  Send    │  │ Receive  │  │   Logs   │  │ History  │   │
│  │   Tab    │  │   Tab    │  │   Tab    │  │   Tab    │   │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘   │
└─────────────────────────────────────────────────────────────┘
         │              │              │              │
         ▼              ▼              ▼              ▼
┌──────────────┐ ┌──────────────┐ ┌──────────┐ ┌──────────────┐
│    Sender    │ │   Receiver   │ │  Logger  │ │   History    │
│              │ │              │ │          │ │   Manager    │
└──────────────┘ └──────────────┘ └──────────┘ └──────────────┘
         │              │                             │
         ▼              ▼                             ▼
┌──────────────┐ ┌──────────────┐           ┌──────────────┐
│ SenderThread │ │ReceiverThread│           │   SQLite DB  │
└──────────────┘ └──────────────┘           └──────────────┘
         │              │
         └──────┬───────┘
                ▼
        ┌──────────────┐
        │  DataFormat  │
        │   Handler    │
        └──────────────┘
                │
                ▼
        ┌──────────────┐
        │   Network    │
        │  (TCP/UDP)   │
        └──────────────┘
```

## Class Relationships

```
GUI
 ├── Sender
 │    └── SenderThread
 │         └── DataFormat
 ├── Receiver
 │    └── ReceiverThread
 │         └── DataFormat
 ├── Logger
 ├── HistoryTab
 │    └── MessageHistoryManager
 ├── FileManager
 ├── ExportManager
 └── ThemeManager
```

## Data Flow

### Sending Message Flow
```
User Input → GUI → Sender → DataFormat → SenderThread → Network
                                                    ↓
                                            Logger ← ConnectionStats
                                                    ↓
                                            MessageHistoryManager
```

### Receiving Message Flow
```
Network → ReceiverThread → DataFormat → Receiver → GUI Display
                                            ↓
                                        Logger
                                            ↓
                                    MessageHistoryManager
```

## Key Design Patterns

1. **Model-View-Controller (MVC)**: GUI separates presentation from logic
2. **Observer Pattern**: Qt signals/slots for event handling
3. **Strategy Pattern**: DataFormat supports multiple serialization strategies
4. **Singleton Pattern**: Logger and ThemeManager (implicit)
5. **Thread Pool**: Background threads for non-blocking I/O

## Threading Model

- **Main Thread**: GUI event loop and user interactions
- **Sender Thread**: Handles outgoing network connections
- **Receiver Thread**: Listens for incoming messages without blocking UI

## Database Schema

```sql
CREATE TABLE messages (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp TEXT NOT NULL,
    direction TEXT NOT NULL,  -- 'sent' or 'received'
    protocol TEXT NOT NULL,   -- 'TCP' or 'UDP'
    format TEXT NOT NULL,     -- 'JSON', 'XML', etc.
    host TEXT NOT NULL,
    port INTEGER NOT NULL,
    message TEXT NOT NULL,
    status TEXT NOT NULL      -- 'success' or 'error'
);
```

## Performance Considerations

- Asynchronous I/O prevents UI freezing
- Database indexing on timestamp for fast queries
- Message size limits to prevent memory issues
- Connection pooling for repeated sends

## Security Notes

- No encryption implemented (plaintext transmission)
- No authentication mechanism
- Suitable for trusted networks only
- Input validation prevents injection attacks
