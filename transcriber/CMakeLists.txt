find_package(Boost REQUIRED COMPONENTS system thread)
find_package(BZip2 REQUIRED)
find_package(KenLM REQUIRED)
find_package(Leap REQUIRED)
find_package(SFML REQUIRED COMPONENTS graphics system window)
find_package(ZLIB REQUIRED)

add_executable(transcriber
    main.cpp)

target_include_directories(transcriber
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${PROJECT_SOURCE_DIR}/lib/include
    PRIVATE ${BOOST_INCLUDEDIR}
    PRIVATE ${KenLM_INCLUDE_DIRS}
    PRIVATE ${Leap_INCLUDE_DIR}
    PRIVATE ${Leap_INCLUDE_DIR}/LeapSDK
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set_source_files_properties(main.cpp PROPERTIES
        COMPILE_FLAGS "-Wno-comment -Wno-deprecated-declarations -Wno-unused-private-field -Wno-unused-variable")
endif()

set_target_properties(transcriber PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON)

target_link_libraries(transcriber
    PRIVATE LeapAsl::LeapAsl
    PRIVATE ${Boost_LIBRARIES}
    PRIVATE ${BZIP2_LIBRARIES}
    PRIVATE ${KenLM_LIBRARIES}
    PRIVATE ${Leap_BUILD_LIBRARIES}
    PRIVATE ${SFML_LIBRARIES}
    PRIVATE ${ZLIB_LIBRARIES})

if(APPLE)
    target_link_libraries(transcriber
        PRIVATE "-framework GLUT"
        PRIVATE "-framework OpenGL")
endif()

add_custom_command(TARGET transcriber POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:recorder>/gestures $<TARGET_FILE_DIR:transcriber>
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/aspell_en_expanded" $<TARGET_FILE_DIR:transcriber>
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/romeo_and_juliet_corpus.mmap" $<TARGET_FILE_DIR:transcriber>
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/resources" $<TARGET_FILE_DIR:transcriber>/resources
)