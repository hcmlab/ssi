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

cmake_minimum_required(VERSION 2.8)

project(ssifactory_test)

IF(SSI_GUI STREQUAL "SDL")
	IF(UNIX)
		INCLUDE(FindPkgConfig)
		pkg_check_modules(CAIRO REQUIRED cairo)
		pkg_check_modules(SDL2 REQUIRED sdl2)
	endif(UNIX)
endif(SSI_GUI STREQUAL "SDL")

include_directories (
	.
	../../../core/include
	../../../core/include/ioput/socket
	../../../core/include/ioput
	
	../../../plugins/
	${CAIRO_INCLUDE_DIRS}
	${SDL2_INCLUDE_DIRS}
)

set(COMMON_SRC_FILES

 Main.cpp
 MyObject.cpp

 
)


set(SRC_FILES ${COMMON_SRC_FILES} )

IF(MINGW )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ggdb" )
  find_library (MINGW_PTHREAD "pthread" HINTS ${MINGWLIB} )
ELSEIF(UNIX)
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SSI_CFLAG}" )
  if(ANDROID)
  set(MINGW_PTHREAD "")
  else(ANDROID)
  find_library (MINGW_PTHREAD "pthread" HINTS "/usr/lib" )
  endif(ANDROID)
  link_directories(${CAIRO_LIBRARY_DIRS})
  link_directories(${SDL2_LIBRARY_DIRS})
ELSE(MINGW)
  SET(MINGW_PTHREAD "")

ENDIF(MINGW)

add_executable(ssifactory_test ${SRC_FILES})
if(UNIX)
target_link_libraries(ssifactory_test ssi ${MINGW_PTHREAD} ${CAIRO_LIBRARIES} ${SDL2_LIBRARIES})
else(UNIX)
target_link_libraries(ssifactory_test ssi ${MINGW_PTHREAD})
endif(UNIX)

#rename targets if debug
set_target_properties(ssifactory_test PROPERTIES DEBUG_POSTFIX "d")

#install target to ssi install path set in base directory
install(TARGETS ssifactory_test DESTINATION ${SSI_INSTALL}/${SSI_PLATFORM}/${SSI_COMPILER}/)
