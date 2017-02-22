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

project(ssimouse_test)

IF(UNIX)
INCLUDE(FindPkgConfig)
pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(SDL2 REQUIRED sdl2)
endif(UNIX)


include_directories (
	include
	../../../core/include
	
	../../../plugins/
	
	${CAIRO_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS}
	
)

set(COMMON_SRC_FILES

 Main.cpp

 
)

IF(MINGW)
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
ELSEIF(UNIX)
	  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SSI_CFLAG}" )
  set( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${SSI_LD_FLAG}" )
  set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${SSI_LD_FLAG}" )
  set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SSI_LD_FLAG}" )
	  find_library (X11 "X11" HINTS "/usr/X11R6/lib")
  find_library (Xi "Xi" HINTS "/usr/X11R6/lib")
ENDIF(MINGW)

set(SRC_FILES ${COMMON_SRC_FILES} )


add_executable(ssimouse_test ${SRC_FILES})
if(UNIX)
target_link_libraries(ssimouse_test ssi ${X11} ${Xi} ${CAIRO_LIBRARIES} ${SDL2_LIBRARIES})
else(UNIX)
target_link_libraries(ssimouse_test ssi )
endif(UNIX)


#rename targets if debug
set_target_properties(ssimouse_test PROPERTIES DEBUG_POSTFIX "d")

#install target to ssi install path set in base directory
install(TARGETS ssimouse_test DESTINATION ${SSI_INSTALL}/${SSI_PLATFORM}/${SSI_COMPILER}/)
