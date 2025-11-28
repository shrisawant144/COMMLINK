# Core library
add_library(commlink_core STATIC
    core/dataformat.cpp
    core/filemanager.cpp
    core/exportmanager.cpp
    core/logger.cpp
    core/messagehistorymanager.cpp
    ${CMAKE_SOURCE_DIR}/include/commlink/core/logger.h
    ${CMAKE_SOURCE_DIR}/include/commlink/core/messagehistorymanager.h
)
target_include_directories(commlink_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(commlink_core Qt5::Core Qt5::Widgets Qt5::Sql)

# Network library
add_library(commlink_network STATIC
    network/tcpclient.cpp
    network/tcpserver.cpp
    network/udpclient.cpp
    network/udpserver.cpp
    network/websocketclient.cpp
    network/websocketserver.cpp
    ${CMAKE_SOURCE_DIR}/include/commlink/network/tcpclient.h
    ${CMAKE_SOURCE_DIR}/include/commlink/network/tcpserver.h
    ${CMAKE_SOURCE_DIR}/include/commlink/network/udpclient.h
    ${CMAKE_SOURCE_DIR}/include/commlink/network/udpserver.h
    ${CMAKE_SOURCE_DIR}/include/commlink/network/websocketclient.h
    ${CMAKE_SOURCE_DIR}/include/commlink/network/websocketserver.h
)
target_include_directories(commlink_network PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(commlink_network Qt5::Core Qt5::Network Qt5::WebSockets commlink_core)

# UI library
add_library(commlink_ui STATIC
    ui/gui.cpp
    ui/historytab.cpp
    ui/thememanager.cpp
    ${CMAKE_SOURCE_DIR}/include/commlink/ui/gui.h
    ${CMAKE_SOURCE_DIR}/include/commlink/ui/historytab.h
    ${CMAKE_SOURCE_DIR}/include/commlink/ui/thememanager.h
)
target_include_directories(commlink_ui PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(commlink_ui Qt5::Widgets Qt5::Sql commlink_core commlink_network)

# Main executable
add_executable(commlink main.cpp)
target_include_directories(commlink PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(commlink 
    commlink_ui
    commlink_network
    commlink_core
    Qt5::Widgets
)

install(TARGETS commlink
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
