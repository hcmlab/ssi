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

project(ssimodel)

IF(SSI_GUI STREQUAL "SDL")
	IF(UNIX)
		INCLUDE(FindPkgConfig)
		pkg_check_modules(CAIRO REQUIRED cairo)
		pkg_check_modules(SDL2 REQUIRED sdl2)
	endif(UNIX)
endif(SSI_GUI STREQUAL "SDL")


include_directories(
					include
					../../core/include/ioput/socket
					../../core/include/ioput
					../../core/include
					../../plugins/
                                        ../../libs/build/AlgLib/include
                                        ../../libs/build/ssiml/include
                                        ../../libs/build
					${CAIRO_INCLUDE_DIRS}
					)
if(ANDROID)
else(ANDROID)
#add_subdirectory(tests)
add_subdirectory(tools)
#add_subdirectory(MlpXml)
#add_dependencies(ssimlpxml ssi ssimodel)
endif(ANDROID)



set(COMMON_SRC_FILES

source/AlgLibTools.cpp
#source/BufferWriter.cpp
source/Classifier.cpp
source/OnlineClassifier.cpp
source/ClassifierT.cpp
source/DecisionSmoother.cpp
source/Collector.cpp
source/Dollar1.cpp
source/ExportMain.cpp
#source/FScore.cpp
source/Fisher.cpp
source/FloatingCFS.cpp
source/FloatingSearch.cpp
source/GeometricRecognizer.cpp
source/GestureTemplate.cpp
source/HierarchicalModel.cpp
source/KMeans.cpp
source/KNearestNeighbors.cpp
source/KmTree.cpp
source/KmUtils.cpp
source/LDA.cpp
#source/MlpXml.cpp
#source/MlpXmlExportMain.cpp
#source/MlpXmlTrain.cpp
source/NaiveBayes.cpp
source/OnlineNaiveBayes.cpp
source/PCA.cpp
source/RandomFusion.cpp
source/Rank.cpp
source/Relief.cpp
source/SVM.cpp
source/SimpleFusion.cpp
source/SimpleKNN.cpp
source/libsvm-3.20.cpp



)


set(SRC_FILES ${COMMON_SRC_FILES} )

#find_library(OPENSSI_CORE_LIBRARY NAMES openssi-core HINTS ../../cmake-core/corelib/Debug/ ../../cmake-core/corelib/Release/)
IF(MINGW)

  find_library (MINGW_WSOCKET "wsock32" HINTS ${MINGWLIB} )
  find_library (MINGW_WMM "winmm" HINTS ${MINGWLIB})
  find_library (MINGW_WSOCKET2 "ws2_32" HINTS ${MINGWLIB} )
  find_library (MINGW_PTHREAD "pthread" HINTS ${MINGWLIB} )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ggdb" )
  
  ELSEIF(UNIX)
  
  SET(MINGW_WSOCKET "")
  SET(MINGW_WSOCKET2 "")
  SET(MINGW_WMM "")
  
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SSI_CFLAG}" )
  set( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${SSI_LD_FLAG}" )
  set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${SSI_LD_FLAG}" )
  set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SSI_LD_FLAG}" )
  
  if(ANDROID)
  set(MINGW_PTHREAD "")
  else(ANDROID)
  find_library (MINGW_PTHREAD "pthread" HINTS "/usr/lib" )
  endif(ANDROID)
  
  link_directories(${CAIRO_LIBRARY_DIRS})
ELSE(MINGW)
  SET(MINGW_WSOCKET "")
  SET(MINGW_WSOCKET2 "")
  SET(MINGW_WMM "")
  SET(MINGW_PTHREAD "")
ENDIF(MINGW)




add_library(ssimodel SHARED ${SRC_FILES} )
IF(UNIX)
target_link_libraries(ssimodel ssi ssiml AlgLib ${CAIRO_LIBRARIES} ${MINGW_WSOCKET} ${MINGW_WSOCKET2}  ${MINGW_WMM} ${MINGW_PTHREAD})
ELSE(UNIX)
target_link_libraries(ssimodel ssi ssiml AlgLib ${MINGW_WSOCKET} ${MINGW_WSOCKET2}  ${MINGW_WMM} ${MINGW_PTHREAD})
ENDIF(UNIX)
#add_executable(myapp main.c)

#rename targets if debug
  if(ANDROID)

  else(ANDROID)
set_target_properties(ssimodel PROPERTIES DEBUG_POSTFIX "d" PREFIX "")
  endif(ANDROID)

#install target to ssi install path set in base directory
install(TARGETS ssimodel DESTINATION ${SSI_INSTALL}/${SSI_PLATFORM}/${SSI_COMPILER})
