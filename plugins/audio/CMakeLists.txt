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

project(openssi-audio)


IF(UNIX)
    IF(ANDROID)

# find_package(OpenSLES)
    ELSE(ANDROID)

    INCLUDE(FindPkgConfig)
    pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)
    add_subdirectory(test)
    add_dependencies(ssiportaudio_test ssi)

    set(ANDROID_SYSTEM_INCLUDE_DIRS "")

    ENDIF(ANDROID)
else(UNIX)
    set(ANDROID_SYSTEM_INCLUDE_DIRS "")
endif(UNIX)



include_directories(include
					../../core/include/ioput/socket
					../../core/include/ioput
					../../core/include
					../
                                        ${PORTAUDIO_INCLUDE_DIRS}
                                        ${ANDROID_SYSTEM_INCLUDE_DIRS}
)

message(STATUS ${ANDROID_SYSTEM_INCLUDE_DIRS})
#message(STATUS "${ANDROID_SYSROOT}/usr/include")

set(COMMON_SRC_FILES

source/AudioMono.cpp
source/AudioActivity.cpp
source/VoiceActivitySender.cpp
source/VoiceActivityVerifier.cpp
source/AudioLpc.cpp
source/AudioConvert.cpp
source/AudioIntensity.cpp
source/ExportMain.cpp
source/PreEmphasis.cpp
source/SNRatio.cpp
source/WavReader.cpp
source/WavWriter.cpp
source/WavProvider.cpp
source/fftsg.c
)

if(MINGW OR UNIX)
set(VOLUME "")
else(MINGW OR UNIX)
set( VOLUME
source/VolumeInXXX.cpp
source/VolumeOutMaster.cpp
source/VolumeOutWave.cpp
)
endif(MINGW OR UNIX)


 IF(WIN32)
 set(P_SRC_FILES
source/Audio.cpp
source/AudioPlayer.cpp

source/AudioCons.cpp
source/AudioIn.cpp
source/AudioMixer.cpp
source/AudioOut.cpp

	)
 ELSEIF(ANDROID)
 set(P_SRC_FILES
source/AudioOpenSL.cpp
source/AudioOpenSLPlayer.cpp
    )
 ELSE(WIN32)
 set(P_SRC_FILES
source/AudioPortaudio.cpp
source/AudioPortaudioPlayer.cpp
        )
 ENDIF(WIN32)



 set(SRC_FILES ${COMMON_SRC_FILES} ${P_SRC_FILES} ${VOLUME})




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
#reuse variable for hackishly linking OpenSLES
  set(MINGW_PTHREAD "-lOpenSLES")
ELSE(ANDROID)
  find_library (MINGW_PTHREAD "pthread" HINTS "/usr/lib" )
  link_directories(${PORTAUDIO_LIBRARY_DIRS})
ENDIF(ANDROID)
ELSE(MINGW)
  SET(MINGW_WSOCKET "")
  SET(MINGW_WSOCKET2 "")
  SET(MINGW_WMM "")
   SET(MINGW_PTHREAD "")
ENDIF(MINGW)

add_library(ssiaudio SHARED ${SRC_FILES} )

IF(UNIX)
target_link_libraries(ssiaudio ssi ${MINGW_PTHREAD} ${PORTAUDIO_LIBRARIES} -ldl)
ELSE(UNIX)
target_link_libraries(ssiaudio ssi ${MINGW_GDIPLUS} ${MINGW_GDI32} ${MINGW_WSOCKET} ${MINGW_WSOCKET2} ${MINGW_PTHREAD} ${MINGW_WMM})
ENDIF(UNIX)

#add_executable(myapp main.c)

#rename targets if debug

  if(ANDROID)

  else(ANDROID)
set_target_properties(ssiaudio PROPERTIES DEBUG_POSTFIX "d" PREFIX "")
  endif(ANDROID)

#install target to ssi install path set in base directory
install(TARGETS ssiaudio DESTINATION ${SSI_INSTALL}/${SSI_PLATFORM}/${SSI_COMPILER})
