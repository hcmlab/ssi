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

project(kissFFT)

include_directories(include ../../../core/include )



set(COMMON_SRC_FILES


source/FFT.cpp
source/FilterTools.cpp
source/IFFT.cpp
source/kiss_fft.c
source/kiss_fftr.c



)
 set_source_files_properties(source/kiss_fft.c
								source/kiss_fftr.c
 PROPERTIES LANGUAGE CXX )

set(SRC_FILES ${COMMON_SRC_FILES} )

#find_library(OPENSSI_CORE_LIBRARY NAMES openssi-core HINTS ../../cmake-core/corelib/Debug/ ../../cmake-core/corelib/Release/)

IF(UNIX)
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ggdb  -fPIC" )
ENDIF(UNIX)

add_library(kissFFT STATIC ${SRC_FILES} )
target_link_libraries(kissFFT ssi  )
#add_executable(myapp main.c)

#rename targets if debug
set_target_properties(kissFFT PROPERTIES DEBUG_POSTFIX "d" PREFIX "")

#install target to ssi install path set in base directory
install(TARGETS kissFFT DESTINATION ${SSI_INSTALL}/${SSI_PLATFORM}/${SSI_COMPILER})
