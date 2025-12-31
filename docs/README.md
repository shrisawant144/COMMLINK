# CommLink Documentation

Welcome to the CommLink documentation. This guide helps you find the right documentation for your needs.

## Getting Started

### New Users
- **[Quick Start](quick-start.md)** - Get up and running in 5 minutes
- **[User Guide](user-guide-simple.md)** - Complete usage instructions
- **[Learning Guide](learning-guide.md)** - Networking concepts explained
- **[FAQ](faq.md)** - Common questions and solutions

### Developers
- **[Developer Guide](developer-guide.md)** - Build, test, and contribute
- **[Architecture](ARCHITECTURE.md)** - System design and architecture
- **[Code Flow](CODE_FLOW.md)** - Detailed execution flows
- **[API Documentation](DOXYGEN.md)** - Doxygen-generated API docs

## Documentation Structure

### User Documentation
- `quick-start.md` - Quick setup guide
- `user-guide-simple.md` - Complete user manual
- `learning-guide.md` - Educational content
- `faq.md` - Frequently asked questions
- `Keyboard_Shortcuts_Reference.md` - Keyboard shortcuts

### Developer Documentation
- `developer-guide.md` - Development setup and guidelines
- `ARCHITECTURE.md` - System architecture and design
- `CODE_FLOW.md` - Detailed code execution flows
- `CODE_FLOW_QUICK_REFERENCE.md` - Quick flow reference
- `DOXYGEN.md` - API documentation guide

### API Documentation
- Generated HTML documentation in `doxygen/html/`
- View by opening `doxygen/html/index.html` in a browser
- Generated from source code comments using Doxygen

## Quick Examples

### Test an API
```
Connection Panel → HTTP → Host: api.github.com → Method: GET → Connect
Message Panel → Send → View response in Display Panel
```

### Create TCP Server
```
Server Panel → TCP Server → Port 5000 → Start Server
Connection Panel → TCP → Host: 127.0.0.1, Port: 5000 → Connect
Message Panel → Type message → Send
```

## Need Help?

1. Check [FAQ](faq.md) for common issues
2. Review logs in the application's Status Panel
3. Open a [GitHub issue](https://github.com/shrisawant144/COMMLINK/issues) with details

## Contributing

See [CONTRIBUTING.md](../CONTRIBUTING.md) for:
- Development setup
- Coding standards
- Testing guidelines
- Pull request process
