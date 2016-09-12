find_package(Boost REQUIRED COMPONENTS serialization system thread)
find_package(BZip2 REQUIRED)
find_package(GLUT REQUIRED)
find_package(KenLM REQUIRED)
find_package(Leap REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ZLIB REQUIRED)

find_package(Armadillo)
find_package(mlpack)
if(Armadillo_FOUND AND mlpack_FOUND)
	set(ENABLE_MLPACK TRUE)
endif()

add_library(LeapAsl
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapAsl/Analyzer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapAsl/Labels.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapAsl/LeapAsl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapAsl/Lexicon.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapAsl/Predictors.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapAsl/Recognizer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapAsl/Recorder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapAsl/RecordPlayer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LeapAsl/Utility.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Analyzer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Lexicon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Predictors.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Recognizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Recorder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RecordPlayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Utility.cpp
    ${Leap_INCLUDE_DIR}/LeapSDK/util/LeapUtilGL.cpp
)

add_library(LeapAsl::LeapAsl ALIAS LeapAsl)

set_target_properties(LeapAsl PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

target_compile_options(LeapAsl
    PUBLIC -DKENLM_MAX_ORDER=6
    PUBLIC -Wno-unused-private-field
    PUBLIC -Wno-unused-local-typedef
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
   CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(LeapAsl
        PUBLIC -Wno-deprecated-declarations
        PUBLIC -Wno-redeclared-class-member
    )
endif()

if(ENABLE_MLPACK)
	target_compile_options(LeapAsl
		PUBLIC -DENABLE_MLPACK
	)
endif()


target_include_directories(LeapAsl
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PUBLIC $<INSTALL_INTERFACE:include>
    PUBLIC ${Boost_INCLUDE_DIRS}
    PUBLIC ${KenLM_INCLUDE_DIRS}
    PUBLIC ${Leap_INCLUDE_DIR}
    PUBLIC ${Leap_INCLUDE_DIR}/LeapSDK
)

if(ENABLE_MLPACK)
	target_include_directories(LeapAsl
		PUBLIC ${mlpack_INCLUDE_DIR}
	)
endif()

target_link_libraries(LeapAsl
    PRIVATE ${Boost_LIBRARIES}
    PRIVATE ${BZIP2_LIBRARIES}
    PRIVATE ${GLUT_LIBRARY}
    PUBLIC ${KenLM_LIBRARIES}
    PUBLIC ${Leap_BUILD_LIBRARIES}
    PRIVATE ${OPENGL_LIBRARIES}
    PRIVATE ${ZLIB_LIBRARIES}
)

if(ENABLE_MLPACK)
	target_link_libraries(LeapAsl
		PRIVATE ${ARMADILLO_LIBRARIES}
		PRIVATE ${mlpack_LIBRARIES}
	)
endif()
