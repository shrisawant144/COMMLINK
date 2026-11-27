cmake_minimum_required(VERSION 3.16)

project(CommLink VERSION 1.0.0 LANGUAGES CXX)

# Project-wide settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Options
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_DOCS "Build documentation" OFF)
option(ENABLE_WARNINGS "Enable compiler warnings" ON)

# Qt configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(COMMLINK_QT_MAJOR_VERSION "" CACHE STRING "Qt major version (5 or 6; empty selects automatically)")
set_property(CACHE COMMLINK_QT_MAJOR_VERSION PROPERTY STRINGS "" 5 6)
if(COMMLINK_QT_MAJOR_VERSION)
    if(NOT COMMLINK_QT_MAJOR_VERSION MATCHES "^[56]$")
        message(FATAL_ERROR "COMMLINK_QT_MAJOR_VERSION must be 5 or 6")
    endif()
    set(QT_VERSION_MAJOR "${COMMLINK_QT_MAJOR_VERSION}")
else()
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Sql WebSockets Network)
endif()
if(QT_VERSION_MAJOR EQUAL 5)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core Widgets Sql WebSockets Network)
else()
    find_package(Qt6 6.2 REQUIRED COMPONENTS Core Widgets Sql WebSockets Network)
endif()
if(ANDROID AND NOT QT_VERSION_MAJOR EQUAL 6)
    message(FATAL_ERROR "The Android package requires Qt 6")
endif()

set(COMMLINK_HOST_QT_BIN "" CACHE PATH "Native Qt code-generator directory for cross builds")
if(CMAKE_CROSSCOMPILING AND COMMLINK_HOST_QT_BIN)
    foreach(tool moc rcc uic)
        if(NOT EXISTS "${COMMLINK_HOST_QT_BIN}/${tool}")
            message(FATAL_ERROR "Missing native Qt tool: ${COMMLINK_HOST_QT_BIN}/${tool}")
        endif()
        set(tool_target "Qt${QT_VERSION_MAJOR}::${tool}")
        get_target_property(alias_target ${tool_target} ALIASED_TARGET)
        if(alias_target)
            set(tool_target "${alias_target}")
        endif()
        set_property(TARGET ${tool_target} PROPERTY IMPORTED_LOCATION "${COMMLINK_HOST_QT_BIN}/${tool}")
        get_target_property(tool_configs ${tool_target} IMPORTED_CONFIGURATIONS)
        foreach(config IN LISTS tool_configs)
            string(TOUPPER "${config}" config)
            set_property(TARGET ${tool_target} PROPERTY IMPORTED_LOCATION_${config} "${COMMLINK_HOST_QT_BIN}/${tool}")
        endforeach()
    endforeach()
endif()
option(COMMLINK_MOBILE_UI "Use the phone interface" ${ANDROID})
if(COMMLINK_MOBILE_UI)
    add_compile_definitions(COMMLINK_MOBILE_UI)
endif()

# Compiler warnings
if(ENABLE_WARNINGS)
    include(cmake/CompilerWarnings.cmake)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Subdirectories
add_subdirectory(src)

# Installation
install(DIRECTORY include/commlink
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Package configuration
set(CPACK_PACKAGE_NAME "CommLink")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "CommLink Project")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_CONTACT "CommLink contributors")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Desktop network communication workbench")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "CommLink contributors")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
if(QT_VERSION_MAJOR EQUAL 6)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6sql6-sqlite")
else()
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5sql5-sqlite")
endif()
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
include(CPack)
