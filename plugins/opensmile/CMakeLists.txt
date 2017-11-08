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

project(ssiopensmile)

#add_subdirectory(opensmileTest)
#add_dependencies(ssiopensmile_test ssiopensmile)
#
#add_subdirectory(laughterTest)
#add_dependencies(ssilaughter_test ssiopensmile)

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
			../../core/include/frame
			../
                        ../../libs/build/opencv/include/
			${CAIRO_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS}
			)

set(COMMON_SRC_FILES

source/Deltas.cpp
source/ExportMain.cpp
source/LaughterFeatureExtractor.cpp
source/LaughterPreProcessor.cpp
source/OSEnergy.cpp
source/OSFFTmagphase.cpp
source/OSFunctionals.cpp
source/OSIntensity.cpp
source/OSLpc.cpp
source/OSMelspec.cpp
source/OSMfcc.cpp
source/OSMfccChain.cpp
source/OSPitchBase.cpp
source/OSPitchChain.cpp
source/OSPitchDirection.cpp
source/OSPitchShs.cpp
source/OSPitchSmoother.cpp
source/OSPlp.cpp
source/OSPlpChain.cpp
source/OSPreemphasis.cpp
source/OSSpecScale.cpp
source/OSTransformFFT.cpp
source/OSVad.cpp
source/OSWindow.cpp
source/fftsg.c


)
 IF( SSI_GUI STREQUAL "SDL")
  set(GUI_SRC_FILES

	)
ELSE( SSI_GUI STREQUAL "SDL")
  set(GUI_SRC_FILES )
ENDIF( SSI_GUI STREQUAL "SDL")

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

IF(ANDROID)
  set( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${SSI_LD_FLAG}" )
  set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${SSI_LD_FLAG}" )
  set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SSI_LD_FLAG}" )
  set( MINGW_PTHREAD "")
ELSE(ANDROID)
  find_library (MINGW_PTHREAD "pthread" HINTS "/usr/lib" )

ENDIF(ANDROID)
ELSE(MINGW)
  SET(MINGW_WSOCKET "")
  SET(MINGW_WSOCKET2 "")
  SET(MINGW_WMM "")
   SET(MINGW_PTHREAD "")
ENDIF(MINGW)




add_library(ssiopensmile SHARED ${SRC_FILES} )
IF(UNIX)
target_link_libraries(ssiopensmile ssi ${MINGW_WSOCKET} ${MINGW_WMM} ${MINGW_WSOCKET2} ${MINGW_PTHREAD} -ldl)
ELSE(UNIX)
target_link_libraries(ssiopensmile ssi ${MINGW_WSOCKET} ${MINGW_WSOCKET2}  ${MINGW_WMM} ${MINGW_PTHREAD})
ENDIF(UNIX)
#add_executable(myapp main.c)

#rename targets if debug
  
  if(ANDROID)
  else(ANDROID)
set_target_properties(ssiopensmile PROPERTIES DEBUG_POSTFIX "d" PREFIX "")
  endif(ANDROID)

#install target to ssi install path set in base directory
install(TARGETS ssiopensmile DESTINATION ${SSI_INSTALL}/${SSI_PLATFORM}/${SSI_COMPILER})
