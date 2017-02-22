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

project(ssimodel_fsel_test)

IF(SSI_GUI STREQUAL "SDL")
	IF(UNIX)
		INCLUDE(FindPkgConfig)
		pkg_check_modules(CAIRO REQUIRED cairo)
		pkg_check_modules(SDL2 REQUIRED sdl2)
	endif(UNIX)
endif(SSI_GUI STREQUAL "SDL")





include_directories (
	include
	../../../core/include
	
	../../../plugins/
	../include
	${CAIRO_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS}
        ${PORTAUDIO_INCLUDE_DIRS}
	
)

set(COMMON_SRC_FILES

 MainFsel.cpp

 
)

IF(MINGW)
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
ELSEIF(UNIX)
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
	  find_library (X11 "X11" HINTS "/usr/X11R6/lib")
         find_library (Xi "Xi" HINTS "/usr/X11R6/lib")
         link_directories(${PORTAUDIO_LIBRARY_DIRS})
ENDIF(MINGW)

set(SRC_FILES ${COMMON_SRC_FILES} )


add_executable(ssimodel_fsel_test ${SRC_FILES})
if(UNIX)
target_link_libraries(ssimodel_fsel_test ssi  ssiml AlgLib ${X11} ${Xi} ${CAIRO_LIBRARIES} ${SDL2_LIBRARIES} ${PORTAUDIO_LIBRARIES} )
else(UNIX)
target_link_libraries(ssimodel_fsel_test ssi  ssiml AlgLib )
endif(UNIX)


#rename targets if debug
set_target_properties(ssimodel_fsel_test PROPERTIES DEBUG_POSTFIX "d")

#install target to ssi install path set in base directory
install(TARGETS ssimodel_fsel_test DESTINATION ${SSI_INSTALL}/${SSI_PLATFORM}/${SSI_COMPILER}/)
