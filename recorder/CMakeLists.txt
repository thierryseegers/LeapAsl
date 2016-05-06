find_package(Leap REQUIRED)
find_package(SFML REQUIRED COMPONENTS graphics system window)

add_executable(recorder
    main.cpp
)

set_target_properties(recorder PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
   CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(recorder
        PUBLIC -Wno-comment
        PUBLIC -Wno-deprecated-declarations
        PUBLIC -Wno-unused-private-field
        PUBLIC -Wno-unused-variable
    )
endif()

target_include_directories(recorder
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${PROJECT_SOURCE_DIR}/lib/include
    PRIVATE ${Leap_INCLUDE_DIR}
    PRIVATE ${Leap_INCLUDE_DIR}/LeapSDK
)

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
