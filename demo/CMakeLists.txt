find_package(Boost REQUIRED COMPONENTS system)
#find_package(FFT)
find_package(Leap REQUIRED)
find_package(SFML REQUIRED COMPONENTS graphics system window)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/lib/include
    ${BOOST_INCLUDEDIR}
    ${Leap_INCLUDE_DIR}
    ${Leap_INCLUDE_DIR}/LeapSDK)

add_executable(demo
    ${Leap_INCLUDE_DIR}/LeapSDK/util/LeapUtil.cpp
    ${Leap_INCLUDE_DIR}/LeapSDK/util/LeapUtilGL.cpp
    main.cpp)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set_source_files_properties(${Leap_INCLUDE_DIR}/LeapSDK/util/LeapUtilGL.cpp PROPERTIES
        COMPILE_FLAGS "-Wno-deprecated-declarations -Wno-unused-private-field")
    set_source_files_properties(main.cpp PROPERTIES
        COMPILE_FLAGS "-Wno-comment -Wno-deprecated-declarations -Wno-unused-private-field -Wno-unused-variable")
endif()

target_link_libraries(demo
    LearnedGestures
    ${Leap_BUILD_LIBRARIES}
    ${SFML_LIBRARIES})

if(APPLE)
    target_link_libraries(demo
        "-framework GLUT"
        "-framework OpenGL")
endif()

add_custom_command(TARGET demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/resources" $<TARGET_FILE_DIR:demo>/resources)