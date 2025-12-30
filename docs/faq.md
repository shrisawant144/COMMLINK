# FAQ - Frequently Asked Questions

## Installation & Setup

**Q: What do I need to run CommLink?**
A: Qt5 (5.12+), CMake (3.8.2+), and a C++17 compiler. Use `./scripts/install-deps.sh` on Linux.

**Q: Build fails with "Qt5 not found"**
A: Install Qt5 development packages:
- Ubuntu: `sudo apt install qt5-default libqt5websockets5-dev`
- Windows: Download from qt.io
- macOS: `brew install qt5`

**Q: How do I run CommLink?**
A: After building: `./build/bin/commlink` (Linux) or `.\bin\Release\commlink.exe` (Windows)

## Using CommLink

**Q: How do I test a REST API?**
A: Connection Panel → HTTP → Set host/method → Connect. Message Panel → Add JSON body if needed → Send. View response in Client Received tab.

**Q: Can I save my messages?**
A: Yes! File → Save (current session) or History tab → Export (all history)

**Q: How do I connect two CommLink instances?**
A: One as server (Server Panel → TCP Server → Start), other as client (Connection Panel → TCP → Connect to server IP:port)

**Q: What's the difference between TCP and UDP?**
A: TCP is reliable (guaranteed delivery), UDP is fast (no guarantee). Use TCP for important data, UDP for speed.

## Troubleshooting

**Q: "Connection refused" error**
A: Check if:
- Server is running and listening on correct port
- Firewall allows the connection
- IP address and port are correct

**Q: Messages not appearing**
A: Verify:
- Connection is established (check status)
- Data format is valid (try plain text first)
- Look at Logs tab for error messages

**Q: Application crashes on startup**
A: Usually missing Qt5 libraries. Install Qt5 completely or run from terminal to see error details.

**Q: Can't see sent messages in history**
A: History saves automatically. Try:
- Refresh History tab
- Check if database file has write permissions
- Look for SQLite errors in Logs

## Features

**Q: What data formats are supported?**
A: JSON, XML, CSV, plain text, binary, and hexadecimal

**Q: Can I use CommLink for WebSocket testing?**
A: Yes! WebSocket tab → Enter ws:// or wss:// URL → Connect → Send messages

**Q: How do I change the theme?**
A: View menu → Theme → Choose Light, Dark, or System

**Q: Is there a command line version?**
A: No, CommLink is GUI-only. For CLI tools, consider curl (HTTP) or netcat (TCP/UDP).

## Advanced Usage

**Q: Can I script CommLink?**
A: Not directly, but you can:
- Save/load message files
- Export history programmatically
- Use the file formats in your scripts

**Q: How do I test HTTPS APIs?**
A: Same as HTTP - just use https:// URLs. CommLink handles SSL automatically.

**Q: Can I monitor network traffic?**
A: CommLink shows sent/received messages. For packet-level analysis, use Wireshark.

**Q: How do I test WebSocket with authentication?**
A: Add authentication headers before connecting, or include auth tokens in your messages.

## Development

**Q: How do I add a new protocol?**
A: Inherit from NetworkBase class, implement required methods, register in protocol factory. See developer-guide.md.

**Q: Can I contribute to CommLink?**
A: Yes! Fork the repo, make changes, add tests, submit a pull request. See CONTRIBUTING.md.

**Q: Where are the unit tests?**
A: In `tests/` directory. Run with `ctest` after building.

## Getting Help

**Q: Where can I report bugs?**
A: GitHub Issues: https://github.com/shrisawant144/COMMLINK/issues

**Q: How do I get support?**
A: Check documentation first, then create a GitHub issue with:
- Your OS and Qt version
- Steps to reproduce the problem
- Error messages from Logs tab

**Q: Is there a user community?**
A: GitHub Discussions: https://github.com/shrisawant144/COMMLINK/discussions
