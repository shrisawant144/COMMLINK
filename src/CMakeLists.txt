# Core library
add_library(commlink_core STATIC
    core/dataformat.cpp
    core/streamframer.cpp
    core/workspacestore.cpp
    core/replayengine.cpp
    ${CMAKE_SOURCE_DIR}/include/commlink/core/replayengine.h
    core/filemanager.cpp
    core/exportmanager.cpp
    core/logger.cpp
    core/messagehistorymanager.cpp
    ${CMAKE_SOURCE_DIR}/include/commlink/core/logger.h
    ${CMAKE_SOURCE_DIR}/include/commlink/core/messagehistorymanager.h
)
target_include_directories(commlink_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(commlink_core Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql)

# Network library
add_library(commlink_network STATIC
    network/tcpclient.cpp
    network/tcpserver.cpp
    network/udpclient.cpp
    network/udpserver.cpp
    network/websocketclient.cpp
    network/websocketserver.cpp
    network/httpclient.cpp
    network/httpserver.cpp
    ${CMAKE_SOURCE_DIR}/include/commlink/network/tcpclient.h
    ${CMAKE_SOURCE_DIR}/include/commlink/network/tcpserver.h
    ${CMAKE_SOURCE_DIR}/include/commlink/network/udpclient.h
    ${CMAKE_SOURCE_DIR}/include/commlink/network/udpserver.h
    ${CMAKE_SOURCE_DIR}/include/commlink/network/websocketclient.h
    ${CMAKE_SOURCE_DIR}/include/commlink/network/websocketserver.h
    ${CMAKE_SOURCE_DIR}/include/commlink/network/httpclient.h
    ${CMAKE_SOURCE_DIR}/include/commlink/network/httpserver.h
)
target_include_directories(commlink_network PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(commlink_network Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::WebSockets commlink_core)

# UI library - Modular components
add_library(commlink_ui STATIC
    # Original GUI (kept for reference)
    ui/gui.cpp
    ui/historytab.cpp
    ui/thememanager.cpp
    ${CMAKE_SOURCE_DIR}/include/commlink/ui/gui.h
    ${CMAKE_SOURCE_DIR}/include/commlink/ui/historytab.h
    ${CMAKE_SOURCE_DIR}/include/commlink/ui/thememanager.h
    
    # New modular components
    ui/connectionpanel.cpp
    ui/serverpanel.cpp
    ui/messagepanel.cpp
    ui/displaypanel.cpp
    ui/statuspanel.cpp
    ui/networkinfopanel.cpp
    ui/mainwindow.cpp
    ui/workbenchpanel.cpp
    ui/trafficinspector.cpp
    ${CMAKE_SOURCE_DIR}/include/commlink/ui/workbenchpanel.h
    ${CMAKE_SOURCE_DIR}/include/commlink/ui/trafficinspector.h
    ${CMAKE_SOURCE_DIR}/include/commlink/ui/connectionpanel.h
    ${CMAKE_SOURCE_DIR}/include/commlink/ui/serverpanel.h
    ${CMAKE_SOURCE_DIR}/include/commlink/ui/messagepanel.h
    ${CMAKE_SOURCE_DIR}/include/commlink/ui/displaypanel.h
    ${CMAKE_SOURCE_DIR}/include/commlink/ui/statuspanel.h
    ${CMAKE_SOURCE_DIR}/include/commlink/ui/networkinfopanel.h
    ${CMAKE_SOURCE_DIR}/include/commlink/ui/mainwindow.h
)
target_include_directories(commlink_ui PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(commlink_ui Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql commlink_core commlink_network)

# Main executable
if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_executable(commlink MANUAL_FINALIZATION main.cpp startupenvironment.cpp ${CMAKE_SOURCE_DIR}/resources/CommLink.qrc)
else()
    add_executable(commlink main.cpp startupenvironment.cpp ${CMAKE_SOURCE_DIR}/resources/CommLink.qrc)
endif()
if(WIN32)
    set_target_properties(commlink PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
target_include_directories(commlink PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(commlink PRIVATE
    commlink_ui
    commlink_network
    commlink_core
    Qt${QT_VERSION_MAJOR}::Widgets
)

if(ANDROID)
    set(COMMLINK_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/android" CACHE PATH "Android manifest, resources and assets")
    set(COMMLINK_ANDROID_OPENSSL_DIR "" CACHE PATH "Directory containing Android OpenSSL shared libraries")
    if(NOT EXISTS "${COMMLINK_ANDROID_OPENSSL_DIR}/libcrypto_3.so" OR
       NOT EXISTS "${COMMLINK_ANDROID_OPENSSL_DIR}/libssl_3.so")
        message(FATAL_ERROR "Set COMMLINK_ANDROID_OPENSSL_DIR to the Android OpenSSL library directory")
    endif()
    set_target_properties(commlink PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${COMMLINK_ANDROID_PACKAGE_SOURCE_DIR}"
        QT_ANDROID_PACKAGE_NAME "app.commlink.android"
        QT_ANDROID_MIN_SDK_VERSION 28
        QT_ANDROID_TARGET_SDK_VERSION 35
        QT_ANDROID_VERSION_NAME "${PROJECT_VERSION}"
        QT_ANDROID_VERSION_CODE 1
        QT_ANDROID_APP_NAME "CommLink")
    set_property(TARGET commlink PROPERTY QT_ANDROID_EXTRA_LIBS
        "${COMMLINK_ANDROID_OPENSSL_DIR}/libcrypto_3.so;${COMMLINK_ANDROID_OPENSSL_DIR}/libssl_3.so")
    target_link_options(commlink PRIVATE "-Wl,-z,max-page-size=16384")
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(commlink)
endif()

install(TARGETS commlink
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

if(UNIX AND NOT ANDROID)
    install(FILES ${CMAKE_SOURCE_DIR}/resources/commlink.desktop DESTINATION share/applications)
    install(FILES ${CMAKE_SOURCE_DIR}/resources/logo/CommLink_256.png DESTINATION share/icons/hicolor/256x256/apps RENAME commlink.png)
endif()
install(FILES ${CMAKE_SOURCE_DIR}/LICENSE ${CMAKE_SOURCE_DIR}/README.md DESTINATION share/doc/commlink)
