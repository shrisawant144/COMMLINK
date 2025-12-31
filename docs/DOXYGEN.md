# API Documentation

CommLink uses [Doxygen](https://www.doxygen.nl/) to generate comprehensive API documentation from source code comments.

## Quick Start

### Generate Documentation

```bash
doxygen Doxyfile
```

Documentation is generated in `docs/doxygen/html/`

### View Documentation

Open `docs/doxygen/html/index.html` in your web browser.

**Linux:**
```bash
xdg-open docs/doxygen/html/index.html
```

**macOS:**
```bash
open docs/doxygen/html/index.html
```

**Windows:**
```bash
start docs/doxygen/html/index.html
```

## What's Documented

The Doxygen documentation includes:

- **Class Reference** - All classes with detailed descriptions
- **Method Documentation** - Function signatures, parameters, return values
- **Code Flow** - Step-by-step execution flows for key operations
- **Architecture Overview** - System design and component relationships
- **Class Diagrams** - Visual representation of class hierarchies (requires Graphviz)

## Finding Code Flow Documentation

1. **Main Page** - Overview with links to flow documentation
2. **Class Pages** - Each class includes flow sections in its description
3. **Method Pages** - Methods include "Flow:" sections showing execution steps
4. **Search** - Use the search box to find specific methods or classes

### Key Classes with Flow Documentation

- **MainWindow** - Application orchestrator with connection, send, and receive flows
- **TcpClient** - TCP connection and message handling flows
- **DataMessage** - Data format processing pipeline

## Requirements

- **Doxygen** 1.8.17 or newer
- **Graphviz** (optional) - For class diagrams

### Installation

**Linux:**
```bash
sudo apt-get install doxygen graphviz
```

**macOS:**
```bash
brew install doxygen graphviz
```

**Windows:**
Download from [doxygen.nl](https://www.doxygen.nl/download.html)

## Configuration

The Doxygen configuration is in `Doxyfile` at the project root. Key settings:

- **INPUT**: `include/commlink`, `src`, documentation files
- **OUTPUT_DIRECTORY**: `docs/doxygen`
- **GENERATE_HTML**: Enabled
- **HAVE_DOT**: Enabled (for diagrams, requires Graphviz)

## Writing Documentation Comments

### Class Documentation

```cpp
/**
 * @brief Brief description of the class
 * 
 * Detailed description explaining the class purpose and usage.
 */
class MyClass {
    // ...
};
```

### Method Documentation

```cpp
/**
 * @brief Brief description
 * 
 * @param param1 Description of parameter
 * @return Description of return value
 * 
 * Flow:
 * 1. Step one
 * 2. Step two
 * 3. Step three
 */
void myMethod(int param1);
```

## Integration with Build System

Add to `CMakeLists.txt`:

```cmake
find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating API documentation"
    )
endif()
```

Build with:
```bash
cmake --build build --target docs
```

## Troubleshooting

**Missing Diagrams?**
- Install Graphviz: `sudo apt-get install graphviz` or `brew install graphviz`
- Verify: `dot -V`
- Regenerate: `doxygen Doxyfile`

**Documentation Not Updating?**
- Ensure source files have Doxygen comments
- Check `EXTRACT_ALL = YES` in Doxyfile
- Regenerate after code changes

## Additional Resources

- [Doxygen Manual](https://www.doxygen.nl/manual/index.html)
- [Doxygen Commands](https://www.doxygen.nl/manual/commands.html)
- [Code Flow Documentation](CODE_FLOW.md) - Detailed flow explanations
