cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

project(LeapAsl VERSION 0.9 LANGUAGES CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
# This is just to shush a warning from FindLeap.cmake.
if(NOT MSVC)
    list(APPEND CMAKE_CXX_FLAGS "-stdlib=libc++")
endif()

add_subdirectory(lib)
add_subdirectory(recorder)
add_subdirectory(transcriber)
