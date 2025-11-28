# Migration Guide: CommLink → commlink

## What Changed

The project has been restructured to follow professional industry standards:

### Directory Structure

**Old Structure:**
```
QT_PROJECTS/
├── CommLink/
│   ├── src/          # All source files mixed
│   ├── include/      # All headers mixed
│   ├── build/
│   └── test_build/
```

**New Structure:**
```
QT_PROJECTS/
├── commlink/
│   ├── include/commlink/
│   │   ├── core/      # Data handling
│   │   ├── network/   # Network layer
│   │   └── ui/        # User interface
│   ├── src/
│   │   ├── core/
│   │   ├── network/
│   │   └── ui/
│   ├── tests/
│   │   ├── unit/
│   │   └── integration/
│   ├── docs/
│   ├── scripts/
│   ├── resources/
│   └── .github/
```

## Key Improvements

1. **Modular Architecture**: Code organized into core, network, and UI modules
2. **Professional Build System**: Modern CMake with proper targets and installation
3. **CI/CD Ready**: GitHub Actions workflow included
4. **Better Testing**: Separate unit and integration test directories
5. **Developer Tools**: Scripts for building, formatting, dependency installation
6. **Documentation**: Structured docs with architecture and contribution guides
7. **Code Quality**: clang-format and clang-tidy configurations

## How to Use New Structure

### Building
```bash
cd commlink
./scripts/build.sh
```

### Running Tests
```bash
cd commlink/build
ctest --output-on-failure
```

### Installing Dependencies
```bash
cd commlink
./scripts/install-deps.sh
```

### Formatting Code
```bash
cd commlink
./scripts/format.sh
```

## Include Path Changes

**Old:**
```cpp
#include "../include/gui.h"
#include "../include/sender.h"
```

**New:**
```cpp
#include "commlink/ui/gui.h"
#include "commlink/network/sender.h"
```

## Build System Changes

**Old CMakeLists.txt:**
- Single monolithic build
- All sources in one list
- No modular libraries

**New CMakeLists.txt:**
- Modular library targets (commlink_core, commlink_network, commlink_ui)
- Proper dependency management
- Installation targets
- CPack packaging support

## What to Keep

The old `CommLink/` directory is preserved. You can:
1. Continue using it as-is
2. Gradually migrate to new structure
3. Use new structure for future development

## Recommended Workflow

1. Use `commlink/` for new development
2. Keep `CommLink/` as reference
3. Test new structure thoroughly
4. Once stable, archive old structure

## Benefits

- **Scalability**: Easy to add new modules
- **Maintainability**: Clear separation of concerns
- **Collaboration**: Standard structure familiar to developers
- **CI/CD**: Ready for automated testing and deployment
- **Documentation**: Professional documentation structure
- **Packaging**: Easy to create installers and packages
