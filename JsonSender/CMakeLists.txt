cmake_minimum_required(VERSION 3.8)
project(JsonSender LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Using Qt5 with Widgets for GUI
find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

set(CMAKE_AUTOMOC ON)

add_executable(JsonSender main.cpp sender.cpp gui.cpp)

target_link_libraries(JsonSender 
    Qt5::Core 
    Qt5::Widgets
)