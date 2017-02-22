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
# add project
project(ssimouse)

#add subdirectory for text
#add_subdirectory(test)
#add tests dependencies
#add_dependencies(ssimouse_test ssi ssimouse)


#add include directories
include_directories (
	include
	../../core/include
	../../core/include/ioput/socket
	../../core/include/ioput
	
	../event/include
	
	../frame/include
	
	../graphic/include
	
	../ioput/include
	../ioput/include/ioput/socket
	../ioput/include/ioput
	../
)

#set source files
set(COMMON_SRC_FILES

 source/ExportMain.cpp
 source/Mouse.cpp
 source/CursorMover.cpp
 
)

#find librarys

IF(MINGW)
  find_library (MINGW_WSOCKET "wsock32" HINTS ${MINGWLIB} )
  find_library (MINGW_WMM "winmm" HINTS ${MINGWLIB})
  find_library (MINGW_WSOCKET2 "ws2_32" HINTS ${MINGWLIB} )
  find_library (MINGW_PTHREAD "pthread" HINTS ${MINGWLIB} )
  #set compiler flags for c++11 threading and debug
  #todo create gcc/make debug target?
  	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ggdb" )
  	
  ELSEIF(UNIX)
  SET(MINGW_WSOCKET "")
  SET(MINGW_WSOCKET2 "")
  SET(MINGW_WMM "")
  
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SSI_CFLAG}" )
  set( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${SSI_LD_FLAG}" )
  set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${SSI_LD_FLAG}" )
  set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SSI_LD_FLAG}" )
  
  find_library (MINGW_PTHREAD "pthread" HINTS "/usr/lib" )
  # x11 dependencies
  find_library (X11 "X11" HINTS "/usr/X11R6/lib")
  find_library (Xi "Xi" HINTS "/usr/X11R6/lib")

ELSE(MINGW) # visual studio has its own ways for winsockets/threading eg #pragma
  SET(MINGW_WSOCKET "")
  SET(MINGW_WSOCKET2 "")
  SET(MINGW_WMM "")
   SET(MINGW_PTHREAD "")
ENDIF(MINGW)


set(SRC_FILES ${COMMON_SRC_FILES} )

# add main target
add_library(ssimouse SHARED ${SRC_FILES})

# link against internal and external librarys
IF(UNIX)
target_link_libraries(ssimouse ssi ${MINGW_WSOCKET} ${MINGW_WSOCKET2} ${MINGW_WMM} ${MINGW_PTHREAD})
ELSE(UNIX)
target_link_libraries(ssimouse ssi ${MINGW_WSOCKET} ${MINGW_WSOCKET2} ${MINGW_WMM} ${MINGW_PTHREAD} ${X11} ${Xi})
ENDIF(UNIX)

#add_executable(myapp main.c)

#rename targets if debug
set_target_properties(ssimouse PROPERTIES DEBUG_POSTFIX "d" PREFIX "")

#install target to ssi install path set in base directory
install(TARGETS ssimouse DESTINATION ${SSI_INSTALL}/${SSI_PLATFORM}/${SSI_COMPILER})
