find_package(Leap REQUIRED)
find_package(SFML REQUIRED COMPONENTS graphics system window)

add_executable(recorder
    main.cpp
)

target_include_directories(recorder
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${PROJECT_SOURCE_DIR}/lib/include
    PRIVATE ${Leap_INCLUDE_DIR}
    PRIVATE ${Leap_INCLUDE_DIR}/LeapSDK
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set_source_files_properties(main.cpp PROPERTIES
        COMPILE_FLAGS "-Wno-comment -Wno-deprecated-declarations -Wno-unused-private-field -Wno-unused-variable")
endif()

set_target_properties(recorder PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON)

target_link_libraries(recorder
    PRIVATE LeapAsl::LeapAsl
    PRIVATE ${Leap_BUILD_LIBRARIES}
    PRIVATE ${SFML_LIBRARIES}
)

if(APPLE)
    target_link_libraries(recorder
        PRIVATE "-framework GLUT"
        PRIVATE "-framework OpenGL")
endif()

add_custom_command(TARGET recorder POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/resources" $<TARGET_FILE_DIR:recorder>/resources
)