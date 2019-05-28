IF(UNIX)
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SSI_CFLAG}" )
    set( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${SSI_LD_FLAG}" )
    set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${SSI_LD_FLAG}" )
    set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SSI_LD_FLAG}" )
endif(UNIX)

#LandmarkDetector library
include_directories(../../local/LandmarkDetector/include)

#Utilities library
include_directories(../../local/Utilities/include)

SET(SOURCE
    src/GazeEstimation.cpp
)

SET(HEADERS
    include/GazeEstimation.h
)

add_library( GazeAnalyser ${SOURCE} ${HEADERS})
add_library( OpenFace::GazeAnalyser ALIAS GazeAnalyser)

target_include_directories(GazeAnalyser PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/OpenFace>)

target_include_directories(GazeAnalyser PRIVATE ${GAZEANALYSER_SOURCE_DIR}/include)
target_include_directories(GazeAnalyser PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(GazeAnalyser PUBLIC ${Boost_INCLUDE_DIRS}/boost)
target_include_directories(GazeAnalyser PUBLIC ${OpenCV_INCLUDE_DIRS})

target_link_libraries(GazeAnalyser PUBLIC ${OpenCV_LIBS} ${Boost_LIBRARIES} ${OpenBLAS_LIB})
target_link_libraries(GazeAnalyser PUBLIC dlib)

target_include_directories(GazeAnalyser PRIVATE ${OpenBLAS_INCLUDE_DIR})

#install (TARGETS GazeAnalyser EXPORT OpenFaceTargets LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
#install (FILES ${HEADERS} DESTINATION include/OpenFace)