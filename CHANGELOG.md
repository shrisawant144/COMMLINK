# Changelog

All notable changes to the CommLink project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-11-27

### Added
- Logger widget with customizable log levels and display
- Logo assets and resource file for application branding
- SenderThread and ConnectionStats components
- Multi-format message support (JSON, XML, CSV, TEXT, BINARY, HEX)
- DataFormat module for message serialization
- FileManager for saving/loading messages
- ExportManager for exporting logs and messages in multiple formats
- MessageHistoryManager with SQLite database for persistent storage
- HistoryTab for viewing and searching message history
- ThemeManager for UI customization
- Support for both TCP and UDP protocols
- Real-time message logging and monitoring
- Comprehensive documentation suite

### Changed
- Updated GUI to support DataFormat selection
- Enhanced Receiver components to use DataFormat module
- Updated Sender component to use DataFormat module
- Improved .gitignore to include build artifacts and IDE files

### Fixed
- Build configuration for new source files in CMakeLists.txt

## [Unreleased]

### Added
- Named workspaces with payloads, variables and protocol settings.
- Buffered TCP framing, original payload capture, traffic inspection and byte-safe exports.
- Explicit remote, broadcast and selected-client destinations.
- Timed replay with text, byte and JSON-field assertions, cancellation and timeouts.
- HTTP authentication, headers, response metadata, WebSocket ping/close diagnostics and UDP peer tracking.
- Eight test suites, Linux Debug/Release CI and Debian packaging with desktop integration.

### Fixed
- Workspace names and saved conversation sessions now restore original timestamps, bytes and response details.
- HTTP DELETE/OPTIONS preserve bodies; GET/HEAD inspection reflects their empty request bodies.
- Polling uses validated headers and request limits, and stopping it cancels outstanding poll responses.
- Replay assertions ignore unrelated HTTP requests and incomplete responses.
- TCP receive bursts yield to other event-loop work between bounded batches.
- Overlapping panels and headers, including small-window layouts.
- Startup failures from inherited Snap desktop settings schemas.
- History migrations now preserve records and store original payload bytes.
- Binary/hex wire encoding, malformed HTTP handling and blocking network-error dialogs.

### Planned
- Performance benchmarks
- Native Windows and macOS packaging and verification
