set (BZCONTROLLER_SOURCES
    main.cpp
    controllerapp.cpp
    mainwindow.cpp
    mainwindowui.cpp
    ${CMAKE_SOURCE_DIR}/resources/ui-style/ui-style.qrc
    ${CMAKE_SOURCE_DIR}/resources/icons/icons.qrc
)

if(MSVC)
    # This is a GUI application without the console
    add_executable(bzcontroller WIN32 ${BZCONTROLLER_SOURCES})
else()
    add_executable(bzcontroller ${BZCONTROLLER_SOURCES})
endif()

set_property(TARGET bzcontroller PROPERTY OUTPUT_NAME "bzcontroller")
set_property(TARGET bzcontroller PROPERTY AUTOMOC ON)
set_property(TARGET bzcontroller PROPERTY AUTOUIC ON)
set_property(TARGET bzcontroller PROPERTY AUTORCC ON)
set_property(TARGET bzcontroller PROPERTY CMAKE_INCLUDE_CURRENT_DIR ON)
set_property(TARGET bzcontroller PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(bzcontroller PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(bzcontroller PRIVATE ${Qt5Core_INCLUDE_DIRS})
target_include_directories(bzcontroller PRIVATE ${Qt5Svg_INCLUDE_DIRS})

target_compile_definitions(bzcontroller
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:SOURCE_VERSION="${SOURCE_VERSION}">
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:SOURCE_COMMIT="${SOURCE_COMMIT}">
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:SOURCE_DIRTY="${SOURCE_DIRTY}">
)

target_link_libraries(bzcontroller Qt5::Core)
target_link_libraries(bzcontroller Qt5::Concurrent)
target_link_libraries(bzcontroller Qt5::Gui)
target_link_libraries(bzcontroller Qt5::Widgets)
target_link_libraries(bzcontroller Qt5::OpenGL)
target_link_libraries(bzcontroller Qt5::PrintSupport)
target_link_libraries(bzcontroller Qt5Svg)
target_link_libraries(bzcontroller Qt5Coap)
target_link_libraries(bzcontroller ${CMAKE_THREAD_LIBS_INIT})

include(GNUInstallDirs)

install(TARGETS bzcontroller 
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

