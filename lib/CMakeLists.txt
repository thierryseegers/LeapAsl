find_package(KenLM REQUIRED)
find_package(Leap REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${KenLM_INCLUDE_DIRS}
    ${Leap_INCLUDE_DIR}
    ${Leap_INCLUDE_DIR}/LeapSDK)

set(headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapAsl/Analyzer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapAsl/Database.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapAsl/LeapAsl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapAsl/Recognizer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapAsl/Utility.h)

set(sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Analyzer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Database.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Recognizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Utility.cpp)

add_library(LeapAsl ${headers} ${sources})
