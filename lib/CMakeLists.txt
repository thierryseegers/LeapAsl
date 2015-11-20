find_package(Leap REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapLearnedGestures
    ${Leap_INCLUDE_DIR}
    ${Leap_INCLUDE_DIR}/LeapSDK)

set(headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapLearnedGestures/NormalizedHandTransform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapLearnedGestures/Listener.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapLearnedGestures/Trainer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapLearnedGestures/LearnedGestures.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapLearnedGestures/detail/Poses.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapLearnedGestures/detail/Utils.h)

set(sources
    ${CMAKE_CURRENT_SOURCE_DIR}/NormalizedHandTransform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Listener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Trainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Poses.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils.cpp)

add_library(LearnedGestures ${headers} ${sources})
