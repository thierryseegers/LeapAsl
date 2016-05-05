find_package(Leap REQUIRED)
find_package(SFML REQUIRED COMPONENTS graphics system window)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/lib/include
    ${Leap_INCLUDE_DIR}
    ${Leap_INCLUDE_DIR}/LeapSDK)

add_executable(recorder
    ${Leap_INCLUDE_DIR}/LeapSDK/util/LeapUtilGL.cpp
    main.cpp)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set_source_files_properties(${Leap_INCLUDE_DIR}/LeapSDK/util/LeapUtilGL.cpp PROPERTIES
        COMPILE_FLAGS "-Wno-deprecated-declarations -Wno-unused-private-field")
    set_source_files_properties(main.cpp PROPERTIES
        COMPILE_FLAGS "-Wno-comment -Wno-deprecated-declarations -Wno-unused-private-field -Wno-unused-variable")
endif()

set_target_properties(recorder PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON)

target_link_libraries(recorder
    LeapAsl
    ${Leap_BUILD_LIBRARIES}
    ${SFML_LIBRARIES}
)

if(APPLE)
    target_link_libraries(recorder
        "-framework GLUT"
        "-framework OpenGL")
endif()

add_custom_command(TARGET recorder POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/resources" $<TARGET_FILE_DIR:recorder>/resources
)