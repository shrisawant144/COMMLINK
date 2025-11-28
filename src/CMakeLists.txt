# Core library
add_library(commlink_core STATIC
    core/dataformat.cpp
    core/filemanager.cpp
    core/exportmanager.cpp
    core/logger.cpp
)
target_link_libraries(commlink_core Qt5::Core)

# Network library
add_library(commlink_network STATIC
    network/sender.cpp
    network/receiver.cpp
    network/senderthread.cpp
    network/receiverthread.cpp
    network/connectionstats.cpp
)
target_link_libraries(commlink_network Qt5::Core commlink_core)

# UI library
add_library(commlink_ui STATIC
    ui/gui.cpp
    ui/historytab.cpp
    ui/thememanager.cpp
)
target_link_libraries(commlink_ui Qt5::Widgets Qt5::Sql commlink_core commlink_network)

# Main executable
add_executable(commlink main.cpp)
target_link_libraries(commlink 
    commlink_ui
    commlink_network
    commlink_core
    Qt5::Widgets
)
