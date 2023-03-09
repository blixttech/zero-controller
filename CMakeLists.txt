cmake_minimum_required(VERSION 3.20)

project (zero-controller CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CONAN_DISABLE_CHECK_COMPILER ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(NOT SET_UP_CONFIGURATIONS_DONE)
    set(SET_UP_CONFIGURATIONS_DONE TRUE)
    # No reason to set CMAKE_CONFIGURATION_TYPES if it's not a multiconfig generator
    # Also no reason mess with CMAKE_BUILD_TYPE if it's a multiconfig generator.
    get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if(isMultiConfig)
        set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE) 
    else()
        if(NOT CMAKE_BUILD_TYPE)
            message(STATUS "Defaulting to release build.")
            set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
        endif()
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY HELPSTRING "Choose the type of build")
        # set the valid options for cmake-gui drop-down list
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release;Profile")
    endif()
endif()

#list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Gui)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS OpenGL)
find_package(Qt6 REQUIRED COMPONENTS PrintSupport)
find_package(Qt6 REQUIRED COMPONENTS Concurrent)
find_package(Qt6 REQUIRED COMPONENTS Network)
find_package(Qt6 REQUIRED COMPONENTS StateMachine)
find_package(Qt6 REQUIRED COMPONENTS Svg)
find_package(Qt6 REQUIRED COMPONENTS Coap)
#find_package(Qwt REQUIRED)
find_package(Threads REQUIRED)

qt_standard_project_setup()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)



add_subdirectory(src)
