find_package(Leap REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Leap_INCLUDE_DIR}
    ${Leap_INCLUDE_DIR}/LeapSDK)

set(headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapLearnedGestures/detail/Gestures.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapLearnedGestures/Listener.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapLearnedGestures/Trainer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapLearnedGestures/LearnedGestures.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapLearnedGestures/Utility.h)

set(sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Listener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Trainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Gestures.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Utility.cpp)

add_library(LearnedGestures ${headers} ${sources})
