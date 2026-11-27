# UI layout regression
if(COMMLINK_MOBILE_UI)
    add_executable(test_layout ui/test_phone_layout.cpp)
else()
    add_executable(test_layout ui/test_layout.cpp)
endif()
target_link_libraries(test_layout PRIVATE commlink_ui)
add_test(NAME UILayoutTest COMMAND test_layout)
set_tests_properties(UILayoutTest PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# Unit tests
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)
add_executable(test_traffic ui/test_traffic.cpp)
target_link_libraries(test_traffic PRIVATE commlink_ui Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME TrafficTest COMMAND test_traffic)
set_tests_properties(TrafficTest PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
add_executable(test_replay unit/test_replay.cpp)
target_link_libraries(test_replay PRIVATE commlink_core Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME ReplayTest COMMAND test_replay)
add_executable(test_protocols unit/test_protocols.cpp)
target_link_libraries(test_protocols PRIVATE commlink_network Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME ProtocolTest COMMAND test_protocols)
add_executable(test_workflow ui/test_workflow.cpp)
target_link_libraries(test_workflow PRIVATE commlink_ui Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME WorkflowTest COMMAND test_workflow)
set_tests_properties(WorkflowTest PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
add_executable(test_workspace unit/test_workspace.cpp)
target_link_libraries(test_workspace PRIVATE commlink_core Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME WorkspaceTest COMMAND test_workspace)
add_executable(test_transport unit/test_transport.cpp)
target_link_libraries(test_transport PRIVATE commlink_network Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME TransportTest COMMAND test_transport)
add_executable(test_startupenvironment unit/test_startupenvironment.cpp
    ${CMAKE_SOURCE_DIR}/src/startupenvironment.cpp)
target_include_directories(test_startupenvironment PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_startupenvironment PRIVATE Qt${QT_VERSION_MAJOR}::Core)
add_test(NAME StartupEnvironmentTest COMMAND test_startupenvironment)

# add_executable(test_dataformat unit/test_dataformat.cpp)
# target_include_directories(test_dataformat PRIVATE ${CMAKE_SOURCE_DIR}/include)
# target_link_libraries(test_dataformat commlink_core Qt${QT_VERSION_MAJOR}::Core)
# add_test(NAME DataFormatTest COMMAND test_dataformat)

# add_executable(test_filemanager unit/test_filemanager.cpp)
# target_include_directories(test_filemanager PRIVATE ${CMAKE_SOURCE_DIR}/include)
# target_link_libraries(test_filemanager commlink_core Qt${QT_VERSION_MAJOR}::Core)
# add_test(NAME FileManagerTest COMMAND test_filemanager)

# add_executable(test_sender unit/test_sender.cpp)
# target_link_libraries(test_sender commlink_network Qt${QT_VERSION_MAJOR}::Core)
# add_test(NAME SenderTest COMMAND test_sender)

# Integration tests
# add_executable(integration_test integration/test_full_flow.cpp)
# target_link_libraries(integration_test 
#     commlink_ui 
#     commlink_network 
#     commlink_core 
#     Qt${QT_VERSION_MAJOR}::Widgets
# )
# add_test(NAME IntegrationTest COMMAND integration_test)
