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

project(liblinear)

include_directories(include ../../../core/include )



set(COMMON_SRC_FILES



source/daxpy.c
source/ddot.c
source/dnrm2.c
source/dscal.c
source/linear.cpp
source/tron.cpp


)
set_source_files_properties(source/daxpy.c
							source/ddot.c
							source/dnrm2.c
							source/dscal.c
 PROPERTIES LANGUAGE CXX )

set(SRC_FILES ${COMMON_SRC_FILES} )

#find_library(OPENSSI_CORE_LIBRARY NAMES openssi-core HINTS ../../cmake-core/corelib/Debug/ ../../cmake-core/corelib/Release/)

IF(UNIX)
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ggdb  -fPIC" )
ENDIF(UNIX)

add_library(liblinear STATIC ${SRC_FILES} )

#add_executable(myapp main.c)

#rename targets if debug
set_target_properties(liblinear PROPERTIES DEBUG_POSTFIX "d" PREFIX "")

#install target to ssi install path set in base directory
install(TARGETS liblinear DESTINATION ${SSI_INSTALL}/${SSI_PLATFORM}/${SSI_COMPILER})
