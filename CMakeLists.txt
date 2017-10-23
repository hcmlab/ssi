#-----------------------------------------------------------------------------
# Initialize project.

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	if(NOT DEFINED WITH_IN_SOURCE_BUILD)
		message(FATAL_ERROR
			"CMake generation for openssi is not allowed within the source directory!"
			"/n Remove the CMakeCache.txt file and try again from another folder, e.g.:"
			"/n "
			"/n rm CMakeCache.txt"
			"/n cd .."
			"/n mkdir cmake-make"
			"/n cd cmake-make"
			"/n cmake ../core"
			"/n "
			"/n Alternately define WITH_IN_SOURCE_BUILD to force this option (not recommended!)"
		)
	endif()
endif()


#http://www.cmake.org/Wiki/CMake/Tutorials/How_to_create_a_ProjectConfig.cmake_file
#http://stackoverflow.com/questions/25609692/how-to-add-source-files-in-another-folder
#file(GLOB Dir1_Sources RELATIVE "Dir1" "*.cpp")
#http://www.opentissue.org/mediawiki/index.php/Using_CMake
cmake_minimum_required(VERSION 2.8)

project(openssi)

option(OPENSSI_BUILD_GUI "Build OpenSSI GUI (depends on SDL2)" OFF)


#set install path

SET(SSI_INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/bin_cmake)


SET(SSI_LIBS ${CMAKE_CURRENT_SOURCE_DIR}/libs)
SET(SSI_BASE ${CMAKE_CURRENT_SOURCE_DIR})

#set compiler
#`CMAKE_<LANG>_COMPILER_VERSION`
#  if(MSVC)
#    include(CMakeDetermineVSServicePack)
#    DetermineVSServicePack( my_service_pack )
#    if( my_service_pack )
#      SET(SSI_COMPILER my_service_pack)
#    endif()
#  endif()

if(MSVC10)

	SET(SSI_COMPILER vc100)

endif(MSVC10)

if(MINGW)
	SET(SSI_COMPILER mingw)
	#SET(MINGWLIB "C:/Program Files (x86)/mingw-w64/i686-5.1.0-posix-dwarf-rt_v4-rev0/mingw32/i686-w64-mingw32/lib")
	SET(MINGWLIB "Z:/home/simon/Simon/MMP2015/mobileSSI/windowsPortableEnv/gcc-4.9-win32/lib/")
endif(MINGW)

#set plattform

if(WIN32)
	SET(SSI_PLATFORM Win32)
	if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
		SET(SSI_PLATFORM x64)
	endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

#disable SDL-GUI, _WIN32 #defines get it right anyways
SET(SSI_GUI windows)

elseif(UNIX)
if(ANDROID)
SET(SSI_PLATFORM Android)
#set compiler and linker flags
#for android 4 and later use -fPIE position independent executeable
SET(SSI_CFLAG " -ggdb -std=c++11 -fPIE -DHEADLESS")
SET(SSI_LD_FLAG "-pie")
# none for Android
SET(SSI_GUI none)

else(ANDROID)
SET(SSI_PLATFORM Linux)


if(OPENSSI_BUILD_GUI)
    SET(SSI_GUI SDL)
else(OPENSSI_BUILD_GUI)
    SET(SSI_GUI NONE)
endif(OPENSSI_BUILD_GUI)

#set compiler and linker flags
IF(SSI_GUI STREQUAL "SDL")
    SET(SSI_CFLAG "-std=c++11 -ggdb  -fPIC")
    SET(SSI_LD_FLAG "-lrt")
else(SSI_GUI STREQUAL "SDL")
    SET(SSI_CFLAG "-std=c++11 -ggdb -fPIC -DHEADLESS")
    SET(SSI_LD_FLAG "-lrt")
endif(SSI_GUI STREQUAL "SDL")


endif(ANDROID)
endif(WIN32)



add_subdirectory(core)
add_subdirectory(libs)
add_subdirectory(plugins)

#IF(IS_DIRECTORY programms)
#        add_subdirectory(programms)
#ENDIF(IS_DIRECTORY programms)
