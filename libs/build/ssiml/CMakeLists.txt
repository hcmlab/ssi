#-----------------------------------------------------------------------------
# Initialize project.

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	if(NOT DEFINED WITH_IN_SOURCE_BUILD)
		message(FATAL_ERROR
			"CMake generation for openssi is not allowed within the source directory!"
			"\n Remove the CMakeCache.txt file and try again from another folder, e.g.:"
			"\n "
			"\n rm CMakeCache.txt"
			"\n cd .."
			"\n mkdir cmake-make"
			"\n cd cmake-make"
			"\n cmake ../core"
			"\n "
			"\n Alternately define WITH_IN_SOURCE_BUILD to force this option (not recommended!)"
		)		
	endif()
endif()

#http://stackoverflow.com/questions/25609692/how-to-add-source-files-in-another-folder
#file(GLOB Dir1_Sources RELATIVE "Dir1" "*.cpp")
#http://www.opentissue.org/mediawiki/index.php/Using_CMake
cmake_minimum_required(VERSION 2.8)

project(ssiml)

include_directories(include ../../../core/include ../../../plugins/  ..

)



set(COMMON_SRC_FILES


source/Annotation.cpp
#source/AnnotationMongoVisitor.cpp
source/arff_attr.cpp
source/arff_data.cpp
source/arff_instance.cpp
source/arff_lexer.cpp
source/arff_parser.cpp
source/arff_scanner.cpp
source/arff_token.cpp
source/arff_utils.cpp
source/arff_value.cpp
source/ElanDocument.cpp
source/ElanTier.cpp
source/ElanTools.cpp
source/Evaluation2Latex.cpp
source/EvaluationCont.cpp
source/Evaluation.cpp
source/FindNN.cpp
source/ISDuplStrms.cpp
source/ISFlatSample.cpp
source/ISHotClass.cpp
source/ISMergeDim.cpp
source/ISMergeSample.cpp
source/ISMergeStrms.cpp
source/ISMissingData.cpp
source/ISNorm.cpp
source/ISOverSample.cpp
source/ISReClass.cpp
source/ISSelectClass.cpp
source/ISSelectDim.cpp
source/ISSelectSample.cpp
source/ISSelectUser.cpp
source/ISSplitStream.cpp
source/ISTransform.cpp
source/ISTrigger.cpp
source/ISUnderSample.cpp
source/KMeans.cpp
source/KmTree.cpp
source/KmUtils.cpp
source/Machine.cpp
source/ModelTools.cpp
source/SampleArff.cpp
source/SampleList.cpp
source/Selection.cpp
source/Trainer.cpp





)


set(SRC_FILES ${COMMON_SRC_FILES} )

#find_library(OPENSSI_CORE_LIBRARY NAMES openssi-core HINTS ../../cmake-core/corelib/Debug/ ../../cmake-core/corelib/Release/)

IF(MINGW)

  	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ggdb" )
  ELSEIF(UNIX)

  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SSI_CFLAG} -std=c++11 -ggdb -fPIC" )

ELSE(MINGW)

ENDIF(MINGW)

add_library(ssiml STATIC ${SRC_FILES} )

#add_executable(myapp main.c)
target_link_libraries(ssiml ssi )
#rename targets if debug
set_target_properties(ssiml PROPERTIES DEBUG_POSTFIX "d" PREFIX "")

#install target to ssi install path set in base directory
install(TARGETS ssiml DESTINATION ${SSI_INSTALL}/${SSI_PLATFORM}/${SSI_COMPILER})
