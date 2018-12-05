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
#http://stackoverflow.com/questions/15175318/cmake-how-to-build-external-projects-and-include-their-targets
#http://www.cmake.org/Wiki/CMake:Install_Commands
cmake_minimum_required(VERSION 2.8)

project(openssi-openface)


if(ANDROID)
else(ANDROID)

endif(ANDROID)

IF(SSI_GUI STREQUAL "SDL")
	IF(UNIX)
		INCLUDE(FindPkgConfig)
		pkg_check_modules(CAIRO REQUIRED cairo)
		pkg_check_modules(SDL2 REQUIRED sdl2)
        endif(UNIX)
#        add_subdirectory(test)
#        add_dependencies(ssiopenface_test ssi)
endif(SSI_GUI STREQUAL "SDL")

find_package( OpenCV REQUIRED )

set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
	IF(UNIX)
		INCLUDE(FindPkgConfig)
		pkg_check_modules(DLIB REQUIRED dlib-1)
        endif(UNIX)


find_package(Boost 1.67.0 COMPONENTS date_time filesystem system REQUIRED)
#find_package(Dlib  REQUIRED)
message(status "** Boost Libraries: ${Boost_LIBRARIES} ${Boost_INCLUDE_DIRS}  ")
include_directories(${OpenCV_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}/boost ${DLIB_INCLUDE_DIRS} ../../libs/build/opencv/include  include build/lib/local/LandmarkDetector/include  build/lib/local/FaceAnalyser/include ../../core/include/ioput/socket ../../core/include/ioput ../../core/include ../ ${CAIRO_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS})
add_subdirectory(build)

set(COMMON_SRC_FILES

source/ExportMain.cpp
source/Openface.cpp
source/OpenfaceHelper.cpp
source/OpenfacePainter.cpp
source/OpenfaceSelector.cpp
source/OpenfaceNormalizer.cpp
source/OpenfaceAdditionalFeat.cpp




)
 IF(SSI_GUI STREQUAL "SDL")
 set(GUI_SRC_FILES

)
ELSE (SSI_GUI STREQUAL "SDL")
 set(GUI_SRC_FILES)
ENDIF (SSI_GUI STREQUAL "SDL")

 IF(WIN32)
 set(P_SRC_FILES

	)
 ELSE(WIN32)
 set(P_SRC_FILES

	)
 ENDIF(WIN32)
 

 
 set(SRC_FILES ${COMMON_SRC_FILES} ${P_SRC_FILES} ${GUI_SRC_FILES})




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
   if (ANDROID)
    SET(MINGW_PTHREAD "")
  else (ANDROID)
  find_library (MINGW_PTHREAD "pthread" HINTS "/usr/lib" )
  endif (ANDROID)
  
ELSE(MINGW)
  SET(MINGW_WSOCKET "")
  SET(MINGW_WSOCKET2 "")
  SET(MINGW_WMM "")
   SET(MINGW_PTHREAD "")
ENDIF(MINGW)

add_library(ssiopenface SHARED ${SRC_FILES} )
target_link_libraries(ssiopenface LandmarkDetector FaceAnalyser ${OpenCV_LIBS} ${Boost_LIBRARIES}  ${DLIB_LIBRARIES} ssi ${MINGW_WSOCKET}  ${MINGW_WSOCKET2} ${MINGW_WMM} ${MINGW_PTHREAD} )
#add_executable(myapp main.c)

#rename targets if debug
  if(ANDROID)

  else(ANDROID)
set_target_properties(ssiopenface PROPERTIES DEBUG_POSTFIX "d" PREFIX "")
  endif(ANDROID)

#install target to ssi install path set in base directory
install(TARGETS ssiopenface DESTINATION ${SSI_INSTALL}/${SSI_PLATFORM}/${SSI_COMPILER})
