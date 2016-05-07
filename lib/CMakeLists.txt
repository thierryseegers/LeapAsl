find_package(Boost REQUIRED COMPONENTS system thread)
find_package(BZip2 REQUIRED)
find_package(KenLM REQUIRED)
find_package(Leap REQUIRED)
find_package(ZLIB REQUIRED)

add_library(LeapAsl
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapAsl/Analyzer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapAsl/Database.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapAsl/LeapAsl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapAsl/Recognizer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapAsl/Utility.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Analyzer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Database.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Recognizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Utility.cpp
    ${Leap_INCLUDE_DIR}/LeapSDK/util/LeapUtilGL.cpp
)

add_library(LeapAsl::LeapAsl ALIAS LeapAsl)

set_target_properties(LeapAsl::LeapAsl PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(LeapAsl::LeapAsl
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC ${KenLM_INCLUDE_DIRS}
    PUBLIC ${Leap_INCLUDE_DIR}
    PUBLIC ${Leap_INCLUDE_DIR}/LeapSDK
)

target_link_libraries(LeapAsl::LeapAsl
    PRIVATE ${Boost_LIBRARIES}
    PRIVATE ${BZIP2_LIBRARIES}
    PRIVATE ${KenLM_LIBRARIES}
    PRIVATE ${Leap_BUILD_LIBRARIES}
    PRIVATE ${ZLIB_LIBRARIES}
)
