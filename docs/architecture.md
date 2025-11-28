# CommLink Architecture

## Overview

CommLink follows a layered architecture with clear separation of concerns:

```
┌─────────────────────────────────────┐
│         Presentation Layer          │
│            (UI Module)              │
├─────────────────────────────────────┤
│         Business Logic Layer        │
│      (Network + Core Modules)       │
├─────────────────────────────────────┤
│          Data Access Layer          │
│     (SQLite + File Operations)      │
└─────────────────────────────────────┘
```

## Module Structure

### Core Module (`commlink_core`)
- **Purpose**: Data handling, serialization, file I/O
- **Components**: DataFormat, FileManager, ExportManager, Logger
- **Dependencies**: Qt5::Core

### Network Module (`commlink_network`)
- **Purpose**: Network communication, protocol handling
- **Components**: Sender, Receiver, Threads, ConnectionStats
- **Dependencies**: Qt5::Core, commlink_core

### UI Module (`commlink_ui`)
- **Purpose**: User interface, visualization
- **Components**: GUI, HistoryTab, ThemeManager
- **Dependencies**: Qt5::Widgets, Qt5::Sql, commlink_network, commlink_core

## Design Patterns

- **Layered Architecture**: Clear separation between UI, business logic, and data
- **Observer Pattern**: Qt signals/slots for event-driven communication
- **Strategy Pattern**: Multiple data format handlers
- **Repository Pattern**: MessageHistoryManager for data persistence
- **Factory Pattern**: DataFormat creation based on type

## Threading Model

- Main thread handles UI events
- Separate threads for network I/O to prevent blocking
- Thread-safe communication via Qt signals

## Data Flow

```
User Input → GUI → Network Layer → DataFormat → Socket
                                                    ↓
                                              Network
                                                    ↓
Socket → DataFormat → Network Layer → GUI → Display
```

## Build System

CMake-based modular build:
1. Core library (static)
2. Network library (static)
3. UI library (static)
4. Main executable (links all libraries)

## Testing Strategy

- Unit tests for individual components
- Integration tests for module interactions
- Fixtures for test data management
