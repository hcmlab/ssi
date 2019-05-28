IF(UNIX)
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SSI_CFLAG}" )
	set( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${SSI_LD_FLAG}" )
	set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${SSI_LD_FLAG}" )
	set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SSI_LD_FLAG}" )
endif(UNIX)

#Utilities library
include_directories(../../local/Utilities/include)

SET(SOURCE
    src/CCNF_patch_expert.cpp
	src/CEN_patch_expert.cpp
	src/CNN_utils.cpp
	src/FaceDetectorMTCNN.cpp
	src/LandmarkDetectionValidator.cpp
    src/LandmarkDetectorFunc.cpp
	src/LandmarkDetectorModel.cpp
    src/LandmarkDetectorUtils.cpp
	src/LandmarkDetectorParameters.cpp
	src/Patch_experts.cpp
	src/PAW.cpp
    src/PDM.cpp
	src/SVR_patch_expert.cpp
	src/stdafx.cpp
)

SET(HEADERS
    include/CCNF_patch_expert.h	
	include/CEN_patch_expert.h
    include/CNN_utils.h
	include/FaceDetectorMTCNN.h
    include/LandmarkCoreIncludes.h
	include/LandmarkDetectionValidator.h
    include/LandmarkDetectorFunc.h
	include/LandmarkDetectorModel.h
	include/LandmarkDetectorParameters.h
	include/LandmarkDetectorUtils.h
	include/Patch_experts.h	
    include/PAW.h
	include/PDM.h
	include/SVR_patch_expert.h		
	include/stdafx.h
)

add_library( LandmarkDetector ${SOURCE} ${HEADERS} )
add_library( OpenFace::LandmarkDetector ALIAS LandmarkDetector)

target_include_directories(LandmarkDetector PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/OpenFace>)

target_include_directories(LandmarkDetector PRIVATE ${LandmarkDetector_SOURCE_DIR}/include)
target_include_directories(LandmarkDetector PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(LandmarkDetector PUBLIC ${Boost_INCLUDE_DIRS}/boost)
target_include_directories(LandmarkDetector PUBLIC ${OpenCV_INCLUDE_DIRS})

target_link_libraries(LandmarkDetector PUBLIC ${OpenCV_LIBS} ${Boost_LIBRARIES} ${OpenBLAS_LIB})
target_link_libraries(LandmarkDetector PUBLIC dlib)

target_include_directories(LandmarkDetector PRIVATE ${OpenBLAS_INCLUDE_DIR})

#install (TARGETS LandmarkDetector EXPORT OpenFaceTargets LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
#install (FILES ${HEADERS} DESTINATION include/OpenFace)