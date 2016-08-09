find_package(Leap REQUIRED)

if(USE_MLPACK)
	find_package(mlpack REQUIRED)
endif()

add_executable(lexicon_to_model
    main.cpp
)

set_target_properties(lexicon_to_model PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
   CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(lexicon_to_model
        PUBLIC -Wno-comment
        PUBLIC -Wno-deprecated-declarations
        PUBLIC -Wno-unused-private-field
        PUBLIC -Wno-unused-variable
    )
endif()

target_include_directories(lexicon_to_model
    PRIVATE ${Leap_INCLUDE_DIR}
    PRIVATE ${Leap_INCLUDE_DIR}/LeapSDK
    PRIVATE ${mlpack_INCLUDE_DIR}
)

if(USE_MLPACK)
	target_include_directories(lexicon_to_model
		PUBLIC ${mlpack_INCLUDE_DIR}
	)
endif()

target_link_libraries(lexicon_to_model
    PRIVATE LeapAsl::LeapAsl
    PRIVATE ${ARMADILLO_LIBRARIES}
    PRIVATE ${Leap_BUILD_LIBRARIES}
    PRIVATE ${mlpack_LIBRARIES}
)

if(USE_MLPACK)
	target_link_libraries(lexicon_to_model
		PRIVATE ${ARMADILLO_LIBRARIES}
		PRIVATE ${mlpack_LIBRARIES}
	)
endif()

add_custom_command(TARGET lexicon_to_model POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../data/lexicon.sample" $<TARGET_FILE_DIR:lexicon_to_model>
)
