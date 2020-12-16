set (BCBCONTROLLER_SOURCES
    mainwindow.cpp
    bk710appgui.cpp
    main.cpp
    ${CMAKE_SOURCE_DIR}/resources/ui-style/ui-style.qrc
)

if(MSVC)
    # This is a GUI application without the console
    add_executable(bcbcontroller WIN32 ${BCBCONTROLLER_SOURCES})
else()
    add_executable(bcbcontroller ${BCBCONTROLLER_SOURCES})
endif()

set_property(TARGET bcbcontroller PROPERTY OUTPUT_NAME "bcbcontroller")
set_property(TARGET bcbcontroller PROPERTY AUTOMOC ON)
set_property(TARGET bcbcontroller PROPERTY AUTOUIC ON)
set_property(TARGET bcbcontroller PROPERTY AUTORCC ON)
set_property(TARGET bcbcontroller PROPERTY CMAKE_INCLUDE_CURRENT_DIR ON)
set_property(TARGET bcbcontroller PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(bcbcontroller PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(bcbcontroller PRIVATE ${Qt5Core_INCLUDE_DIRS})
target_include_directories(bcbcontroller PRIVATE ${Qt5Svg_INCLUDE_DIRS})

target_compile_definitions(bcbcontroller
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:SOURCE_VERSION=${SOURCE_VERSION}>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:SOURCE_COMMIT=${SOURCE_COMMIT}>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:SOURCE_DIRTY=${SOURCE_DIRTY}>
)

target_link_libraries(bcbcontroller Qt5::Core)
target_link_libraries(bcbcontroller Qt5::Concurrent)
target_link_libraries(bcbcontroller Qt5::Gui)
target_link_libraries(bcbcontroller Qt5::Widgets)
target_link_libraries(bcbcontroller Qt5::OpenGL)
target_link_libraries(bcbcontroller Qt5::PrintSupport)
target_link_libraries(bcbcontroller Qt5Svg)
target_link_libraries(bcbcontroller ${CMAKE_THREAD_LIBS_INIT})

include(GNUInstallDirs)

install(TARGETS bcbcontroller 
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

