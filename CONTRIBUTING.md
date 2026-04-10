# Contributing to CommLink

Thank you for contributing to CommLink.

This document explains how to set up the project, how to approach changes, and what maintainers should expect from a contribution. The aim is to keep the repository approachable while maintaining a professional engineering standard.

## Guiding Principles

We value contributions that are:

- Accurate: changes should reflect the current behavior of the codebase.
- Focused: each pull request should have a clear purpose.
- Maintainable: code, tests, and docs should be understandable by the next contributor.
- Respectful: collaboration should stay constructive and professional.

## Before You Start

Please read these first:

1. [README.md](README.md)
2. [docs/README.md](docs/README.md)
3. [docs/developer-guide.md](docs/developer-guide.md)
4. [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)

## Development Environment

### Requirements

- CMake 3.8.2 or newer
- Qt5 5.12 or newer
- C++17-capable compiler
- Git

Qt modules required:

- `Core`
- `Widgets`
- `Network`
- `Sql`
- `WebSockets`

### Basic setup

```bash
git clone https://github.com/shrisawant144/COMMLINK.git
cd COMMLINK
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j"$(nproc)"
```

Run the app:

```bash
./bin/commlink
```

Run the legacy UI:

```bash
./bin/commlink --legacy
```

## How To Approach Changes

### Prefer the modular architecture

The default modular UI built around `MainWindow` is the primary architecture for the project. Prefer extending or improving that path unless:

- the bug exists only in the legacy UI
- the requested change explicitly applies to the legacy UI
- parity between both paths is necessary for the feature

### Keep responsibilities clear

- UI state and orchestration belong in `src/ui/`
- Shared business logic belongs in `src/core/`
- Transport/protocol behavior belongs in `src/network/`

Avoid adding protocol logic directly into small UI widgets when it belongs in reusable handlers or the orchestration layer.

## Build, Validate, Document

Every contribution should aim to do three things:

1. Build cleanly
2. Validate the changed behavior
3. Update documentation when needed

### Validation expectations

At the time of writing:

- the project builds successfully
- automated tests are not fully enabled in the current CMake test configuration
- some checked-in test sources are outdated relative to the current APIs

Because of that, contributors should include:

- the build result they verified
- the manual test scenarios they ran
- any known gaps that remain

If you add or restore automated tests, that is strongly encouraged.

## Documentation Requirements

Documentation is part of the deliverable, not a follow-up task.

Update docs when your change affects:

- setup or prerequisites
- architecture or ownership boundaries
- user-facing behavior
- contributor workflow
- command-line behavior
- protocol support or limitations

Common files to update:

- `README.md`
- `docs/README.md`
- `docs/developer-guide.md`
- `docs/ARCHITECTURE.md`
- `docs/CODE_FLOW.md`

## Coding Expectations

### General

- Follow the existing code style and naming patterns.
- Keep methods focused and responsibilities clear.
- Prefer readability over cleverness.
- Avoid broad refactors unless they are part of the change scope.

### Qt-specific

- Use Qt parent-child ownership correctly.
- Prefer signals and slots for component communication.
- Keep widgets decoupled where possible.
- Treat asynchronous protocol behavior carefully.

### Public interfaces

If you change a public header or a core workflow, document the change with comments or updated docs where appropriate.

## Pull Request Guidance

### Good pull requests usually have

- one clear purpose
- a concise summary
- rationale for the change
- validation notes
- documentation updates where needed

### In your PR description, include

- what changed
- why it changed
- how you validated it
- any known limitations or follow-up work

## Commit Style

Conventional commit prefixes are recommended:

- `feat:`
- `fix:`
- `docs:`
- `refactor:`
- `test:`
- `chore:`

They are not mandatory, but consistent commit messages help project history stay readable.

## Issues and Discussion

When reporting a bug or proposing a change, include:

- platform and environment
- build details
- protocol involved
- steps to reproduce
- expected behavior
- actual behavior

## Review Checklist

Before opening a pull request, confirm:

- the project builds successfully
- the modified behavior was validated
- docs were updated if needed
- no stale debug code or commented-out experiments remain
- the change fits the intended layer of the architecture

## Maintainer Notes

If you review contributions, optimize for:

- correctness
- regression risk
- clarity of ownership
- documentation quality
- honesty about testing and limitations

That last point matters. Clear known limitations are better than implied confidence.
