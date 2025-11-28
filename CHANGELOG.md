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

### Planned
- Unit tests for core components
- CI/CD pipeline integration
- Performance benchmarks
- Packaging for multiple platforms
