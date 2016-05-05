cmake_minimum_required(VERSION 2.6)

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.10")

project(LeapAsl)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
list(APPEND CMAKE_PREFIX_PATH "/opt/local")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(CMAKE_GENERATOR STREQUAL Xcode)
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14")
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
	list(APPEND CMAKE_CXX_FLAGS "-stdlib=libc++")
endif()

add_subdirectory(lib)
add_subdirectory(demo)
add_subdirectory(recorder)
add_subdirectory(transcriber)
