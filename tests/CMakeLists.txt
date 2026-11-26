# UI layout regression
add_executable(test_layout ui/test_layout.cpp)
target_link_libraries(test_layout PRIVATE commlink_ui)
add_test(NAME UILayoutTest COMMAND test_layout)
set_tests_properties(UILayoutTest PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# Unit tests
add_executable(test_startupenvironment unit/test_startupenvironment.cpp
    ${CMAKE_SOURCE_DIR}/src/startupenvironment.cpp)
target_include_directories(test_startupenvironment PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_startupenvironment PRIVATE Qt5::Core)
add_test(NAME StartupEnvironmentTest COMMAND test_startupenvironment)

# add_executable(test_dataformat unit/test_dataformat.cpp)
# target_include_directories(test_dataformat PRIVATE ${CMAKE_SOURCE_DIR}/include)
# target_link_libraries(test_dataformat commlink_core Qt5::Core)
# add_test(NAME DataFormatTest COMMAND test_dataformat)

# add_executable(test_filemanager unit/test_filemanager.cpp)
# target_include_directories(test_filemanager PRIVATE ${CMAKE_SOURCE_DIR}/include)
# target_link_libraries(test_filemanager commlink_core Qt5::Core)
# add_test(NAME FileManagerTest COMMAND test_filemanager)

# add_executable(test_sender unit/test_sender.cpp)
# target_link_libraries(test_sender commlink_network Qt5::Core)
# add_test(NAME SenderTest COMMAND test_sender)

# Integration tests
# add_executable(integration_test integration/test_full_flow.cpp)
# target_link_libraries(integration_test 
#     commlink_ui 
#     commlink_network 
#     commlink_core 
#     Qt5::Widgets
# )
# add_test(NAME IntegrationTest COMMAND integration_test)
