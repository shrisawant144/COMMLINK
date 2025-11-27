cmake_minimum_required(VERSION 3.8)
project(CommLink VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt5
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Sql)

# Include directories
include_directories(include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/sender.cpp
    src/receiver.cpp
    src/gui.cpp
    src/receiverthread.cpp
    src/senderthread.cpp
    src/connectionstats.cpp
    src/dataformat.cpp
    src/filemanager.cpp
    src/exportmanager.cpp
    src/messagehistorymanager.cpp
    src/historytab.cpp
    src/thememanager.cpp
    src/logger.cpp
)

set(HEADERS
    include/sender.h
    include/receiver.h
    include/gui.h
    include/receiverthread.h
    include/senderthread.h
    include/connectionstats.h
    include/dataformat.h
    include/filemanager.h
    include/exportmanager.h
    include/messagehistorymanager.h
    include/historytab.h
    include/thememanager.h
    include/logger.h
)

# Resource files (icons, assets, etc.)
set(RESOURCES
    CommLink.qrc
)

# Create executable
add_executable(CommLink ${SOURCES} ${HEADERS} ${RESOURCES})

# Link libraries
target_link_libraries(CommLink
    Qt5::Core
    Qt5::Widgets
    Qt5::Sql
)

# Set target properties
set_target_properties(CommLink PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
