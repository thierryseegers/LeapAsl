find_package(KenLM REQUIRED)
find_package(Leap REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${KenLM_INCLUDE_DIRS}
    ${Leap_INCLUDE_DIR}
    ${Leap_INCLUDE_DIR}/LeapSDK)

set(headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapLearnedGestures/Analyzer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapLearnedGestures/Database.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapLearnedGestures/LearnedGestures.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapLearnedGestures/Recognizer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapLearnedGestures/Utility.h)

set(sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Analyzer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Database.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Recognizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Utility.cpp)

add_library(LeapLearnedGestures ${headers} ${sources})
