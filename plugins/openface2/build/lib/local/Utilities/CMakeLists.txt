IF(UNIX)
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SSI_CFLAG}" )
	set( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${SSI_LD_FLAG}" )
	set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${SSI_LD_FLAG}" )
	set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SSI_LD_FLAG}" )
endif(UNIX)

SET(SOURCE
    src/ImageCapture.cpp
	src/RecorderCSV.cpp
    src/RecorderHOG.cpp
	src/RecorderOpenFace.cpp
    src/RecorderOpenFaceParameters.cpp
	src/SequenceCapture.cpp
	src/VisualizationUtils.cpp
	src/Visualizer.cpp
)

SET(HEADERS
    include/ImageCapture.h	
    include/RecorderCSV.h
	include/RecorderHOG.h
    include/RecorderOpenFace.h
	include/RecorderOpenFaceParameters.h
	include/SequenceCapture.h
	include/VisualizationUtils.h
	include/Visualizer.h	
)

add_library( Utilities ${SOURCE} ${HEADERS})
add_library( OpenFace::Utilities ALIAS Utilities)

target_include_directories(Utilities PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/OpenFace>)

target_include_directories(Utilities PRIVATE ${UTILITIES_SOURCE_DIR}/include)
target_include_directories(Utilities PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(Utilities PUBLIC ${Boost_INCLUDE_DIRS}/boost)
target_include_directories(Utilities PUBLIC ${OpenCV_INCLUDE_DIRS})

target_link_libraries(Utilities PUBLIC ${OpenCV_LIBS} ${Boost_LIBRARIES})
target_link_libraries(Utilities PUBLIC dlib)

#install (TARGETS Utilities EXPORT OpenFaceTargets LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
#install (FILES ${HEADERS} DESTINATION include/OpenFace)