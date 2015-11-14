find_package(Leap REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapLearnedGestures
    ${Leap_INCLUDE_DIR}
    ${Leap_INCLUDE_DIR}/LeapSDK)

set(public_headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapLearnedGestures/NormalizedHandTransform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapLearnedGestures/LearnedGestures.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapLearnedGestures/detail/Poses.h)

set(sources
    ${CMAKE_CURRENT_SOURCE_DIR}/NormalizedHandTransform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Poses.cpp)

add_library(LearnedGestures ${public_headers} ${sources})
