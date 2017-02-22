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

project(MlpXml)

if(UNIX)
	if(SSI_GUI STREQUAL "SDL")
		INCLUDE(FindPkgConfig)
		pkg_check_modules(CAIRO REQUIRED cairo)
		pkg_check_modules(SDL2 REQUIRED sdl2)
	endif(SSI_GUI STREQUAL "SDL")
endif(UNIX)

include_directories(include
					../../../core/include
					../../../plugins/
					../../../plugins/model/include
					../ssiml/include
					
					${CAIRO_INCLUDE_DIRS}
					
					)



set(COMMON_SRC_FILES


source/MlpXml.cpp
source/MlpXmlExportMain.cpp
source/MlpXmlTrain.cpp
source/BufferWriter.cpp




)


IF(MINGW)

  	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ggdb" )
ELSEIF(UNIX)

  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ggdb" )
  link_directories(${CAIRO_LIBRARY_DIRS})

ELSE(MINGW)

ENDIF(MINGW)


set(SRC_FILES ${COMMON_SRC_FILES} )

#find_library(OPENSSI_CORE_LIBRARY NAMES openssi-core HINTS ../../cmake-core/corelib/Debug/ ../../cmake-core/corelib/Release/)



add_library(MlpXml STATIC ${SRC_FILES} )

IF(UNIX)
target_link_libraries(MlpXml ssi ssiml${CAIRO_LIBRARIES} )
ELSE(UNIX)
target_link_libraries(MlpXml ssi ssi ssiml )
ENDIF(UNIX)

#add_executable(myapp main.c)

#rename targets if debug
set_target_properties(MlpXml  PROPERTIES DEBUG_POSTFIX "d" PREFIX "")

#install target to ssi install path set in base directory
install(TARGETS MlpXml DESTINATION ${SSI_INSTALL}/${SSI_PLATFORM}/${SSI_COMPILER})
