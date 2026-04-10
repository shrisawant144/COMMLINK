# Documentation

This directory contains the primary documentation set for CommLink.

The goal of this documentation is to support three audiences:

- End users learning how to install and use the application
- Contributors who want to build, debug, and extend the codebase
- Maintainers who need architectural context and operational guidance

## Start Here

### If you want to run the app

- [Quick Start](quick-start.md)
- [User Guide](user-guide-simple.md)
- [FAQ](faq.md)

### If you want to understand the codebase

- [Developer Guide](developer-guide.md)
- [Architecture](ARCHITECTURE.md)
- [Code Flow](CODE_FLOW.md)
- [Code Flow Quick Reference](CODE_FLOW_QUICK_REFERENCE.md)

### If you want generated API references

- [Doxygen Guide](DOXYGEN.md)
- `docs/doxygen/html/index.html`

## Documentation Set

### Product and usage

- `quick-start.md`
- `user-guide-simple.md`
- `learning-guide.md`
- `faq.md`
- `Keyboard_Shortcuts_Reference.md`

### Engineering and maintenance

- `developer-guide.md`
- `ARCHITECTURE.md`
- `CODE_FLOW.md`
- `CODE_FLOW_QUICK_REFERENCE.md`
- `DOXYGEN.md`

## Documentation Principles

CommLink documentation should aim to be:

- Accurate: it must match the current codebase, not an older design.
- Actionable: readers should know what to do next after reading a section.
- Layered: top-level guides should orient, while deeper docs provide detail.
- Honest about status: if something is incomplete, legacy, or not yet automated, say so clearly.

## Current Reality

The project includes both a modular default UI and a legacy UI path. Documentation should prefer the modular architecture when describing the main system design, while still acknowledging the legacy code where it affects maintenance or runtime behavior.

The repository also contains test source files, but automated test targets are not currently enabled in the shipped CMake configuration. Developer-facing docs should reflect that accurately.

## Maintaining The Docs

When code changes affect setup, behavior, architecture, or contributor workflow:

1. Update the nearest relevant document in this directory.
2. Update the root [README](../README.md) if the project-facing summary changed.
3. Update [CONTRIBUTING.md](../CONTRIBUTING.md) if contributor expectations changed.
4. Regenerate Doxygen output if public API comments were updated and generated docs are part of the change.

## Suggested Reading Order For New Contributors

1. [README](../README.md)
2. [Developer Guide](developer-guide.md)
3. [Architecture](ARCHITECTURE.md)
4. [Code Flow](CODE_FLOW.md)
5. Source files under `src/ui/mainwindow.cpp`, `src/core/`, and `src/network/`
