INCLUDE (CheckIncludeFiles)

add_executable(zero-controller
    networkinterfacelist.cpp
    main.cpp
    mainwindow.cpp
    mainwindowui.cpp
    coapresourcediscovery.cpp
    coapmessage.cpp
    zeroproxy.cpp
    zerolist.cpp
    zerocoapscanner.cpp
    zeroliveviewmodel.cpp
    zeroliveviewtab.cpp
    openclosebuttondelegate.cpp
    zeromanagementviewmodel.cpp
    zeromanagementviewtab.cpp
    smpimagemgmt.cpp
    smpheader.cpp
    smprequest.cpp
    smpclient.cpp
    smpreply.cpp
    ${CMAKE_SOURCE_DIR}/resources/ui-style/ui-style.qrc
    ${CMAKE_SOURCE_DIR}/resources/icons/icons.qrc
)

set_property(TARGET zero-controller PROPERTY OUTPUT_NAME "zero-controller")
set_property(TARGET zero-controller PROPERTY AUTOMOC ON)
set_property(TARGET zero-controller PROPERTY AUTOUIC ON)
set_property(TARGET zero-controller PROPERTY AUTORCC ON)
set_property(TARGET zero-controller PROPERTY CMAKE_INCLUDE_CURRENT_DIR ON)
set_property(TARGET zero-controller PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(zero-controller PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(zero-controller PRIVATE ${Qt6Core_INCLUDE_DIRS})
target_include_directories(zero-controller PRIVATE ${Qt6Network_INCLUDE_DIRS})
target_include_directories(zero-controller PRIVATE ${Qt6Coap_INCLUDE_DIRS})
target_include_directories(zero-controller PRIVATE ${Qt6Svg_INCLUDE_DIRS})
target_include_directories(zero-controller PRIVATE ${Qt6Network_INCLUDE_DIRS})


CHECK_INCLUDE_FILES(winsock2.h USE_WINSOCK2_H)

target_compile_definitions(zero-controller
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:SOURCE_VERSION="${SOURCE_VERSION}">
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:SOURCE_COMMIT="${SOURCE_COMMIT}">
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:SOURCE_DIRTY="${SOURCE_DIRTY}">
    PRIVATE USE_${CMAKE_CXX_BYTE_ORDER}
)

if(USE_WINSOCK2_H)
    target_compile_definitions(zero-controller
        PRIVATE USE_WINSOCK2
    )
endif()

target_link_libraries(zero-controller Qt6::Core)
target_link_libraries(zero-controller Qt6::Concurrent)
target_link_libraries(zero-controller Qt6::Gui)
target_link_libraries(zero-controller Qt6::Widgets)
target_link_libraries(zero-controller Qt6::OpenGL)
target_link_libraries(zero-controller Qt6::PrintSupport)
target_link_libraries(zero-controller Qt6::Network)
target_link_libraries(zero-controller Qt6::StateMachine)
target_link_libraries(zero-controller Qt6::Svg)
target_link_libraries(zero-controller Qt6::Coap)
target_link_libraries(zero-controller ${CMAKE_THREAD_LIBS_INIT})

include(GNUInstallDirs)

set_target_properties(zero-controller PROPERTIES
    WIN32_EXECUTABLE ON
)

install(TARGETS zero-controller 
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

