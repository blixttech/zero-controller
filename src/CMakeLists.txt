set (BK710_TOOL_SOURCES_GUI
    mainwindow.cpp
    bk710appgui.cpp
    main.cpp
    ${CMAKE_SOURCE_DIR}/resources/ui-style/ui-style.qrc
)

if(MSVC)
    # This is a GUI application without the console
    add_executable(bk710-tool WIN32 ${BK710_TOOL_SOURCES_GUI})
else()
    add_executable(bk710-tool ${BK710_TOOL_SOURCES_GUI})
endif()

set_property(TARGET bk710-tool PROPERTY OUTPUT_NAME "bk710-tool")
set_property(TARGET bk710-tool PROPERTY AUTOMOC ON)
set_property(TARGET bk710-tool PROPERTY AUTOUIC ON)
set_property(TARGET bk710-tool PROPERTY AUTORCC ON)
set_property(TARGET bk710-tool PROPERTY CMAKE_INCLUDE_CURRENT_DIR ON)
set_property(TARGET bk710-tool PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(bk710-tool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(bk710-tool PRIVATE ${Qt5Core_INCLUDE_DIRS})
target_include_directories(bk710-tool PRIVATE ${Qt5SerialPort_INCLUDE_DIRS})
target_include_directories(bk710-tool PRIVATE ${Qwt_INCLUDE_DIRS})

target_link_libraries(bk710-tool Qt5::Core)
target_link_libraries(bk710-tool Qt5::Concurrent)
target_link_libraries(bk710-tool Qt5::Gui)
target_link_libraries(bk710-tool Qt5::Widgets)
target_link_libraries(bk710-tool Qt5::OpenGL)
target_link_libraries(bk710-tool Qt5::PrintSupport)
target_link_libraries(bk710-tool Qt5Svg)
target_link_libraries(bk710-tool ${CMAKE_THREAD_LIBS_INIT})

include(GNUInstallDirs)

install(TARGETS bk710-tool 
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

