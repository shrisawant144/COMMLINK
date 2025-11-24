cmake_minimum_required(VERSION 3.8)
project(JsonSender VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt5
find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

# Include directories
include_directories(include)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/sender.cpp
    src/gui.cpp
    src/receiverthread.cpp
)

set(HEADERS
    include/sender.h
    include/gui.h
    include/receiverthread.h
)

# Create executable
add_executable(JsonSender ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(JsonSender 
    Qt5::Core 
    Qt5::Widgets
)

# Set target properties
set_target_properties(JsonSender PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)