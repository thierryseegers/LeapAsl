find_package(Boost REQUIRED COMPONENTS filesystem program_options)

add_executable(analyzer
    levhenstein_distance.h
    main.cpp
)

set_target_properties(analyzer PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
   CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(analyzer
        PUBLIC -Wno-comment
        PUBLIC -Wno-deprecated-declarations
        PUBLIC -Wno-unused-private-field
        PUBLIC -Wno-unused-variable
    )
endif()

target_include_directories(analyzer
	PRIVATE ${Boost_INCLUDE_DIRS}
    PRIVATE ${Leap_INCLUDE_DIR}
)

target_link_libraries(analyzer
	PRIVATE ${Boost_LIBRARIES}
    PRIVATE LeapAsl::LeapAsl
)

add_custom_command(TARGET analyzer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../data/aspell_en_expanded" $<TARGET_FILE_DIR:analyzer>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../data/lexicon.sample" $<TARGET_FILE_DIR:analyzer>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../data/romeo_and_juliet_corpus.mmap" $<TARGET_FILE_DIR:analyzer>
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/../data/captures" $<TARGET_FILE_DIR:analyzer>/captures
)
