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

cmake_minimum_required(VERSION 2.8)

project(ssi)

#add_subdirectory(test)
add_subdirectory(tools)

IF(SSI_GUI STREQUAL "SDL")
	IF(UNIX)
		INCLUDE(FindPkgConfig)
                pkg_check_modules(SDL2TTF REQUIRED SDL2_ttf)
		pkg_check_modules(SDL2 REQUIRED sdl2)
	endif(UNIX)
endif(SSI_GUI STREQUAL "SDL")

include_directories(include include/ioput/socket include/ioput ../libs/build/ ../plugins/ ${CAIRO_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS})

set(COMMON_SRC_FILES

source/buffer/Buffer.cpp
source/buffer/TimeBuffer.cpp
source/event/IESelect.cpp
source/event/EventList.cpp

source/struct/Queue.cpp
source/struct/BinTree.cpp

source/ioput/file/FileMemAscii.cpp
source/ioput/file/FileMem.cpp
source/ioput/jpg/jpgd.cpp
source/ioput/jpg/jpge.cpp
source/ioput/lz4/lz4.c
source/ioput/lz4/lz4frame.c
source/ioput/lz4/lz4hc.c
source/ioput/lz4/xxhash.c
source/ioput/socket/SocketImage.cpp
#source/model/Annotation.cpp
#source/model/ModelTools.cpp
source/r250.c
source/SSI_Global.cpp
source/thread/ClockThread.cpp
source/thread/Condition.cpp
source/thread/Event.cpp
source/thread/Lock.cpp
source/thread/Mutex.cpp
source/thread/RunAsThread.cpp
source/thread/Thread.cpp
source/thread/ThreadPool.cpp
source/thread/Timer.cpp
source/thread/ThreadSafe.cpp
source/signal/SignalTools.cpp
#source/signal/MatrixOps_ux.cpp

source/ioput/example/Example.cpp
source/ioput/example/Exsemble.cpp

source/ioput/file/File.cpp
source/ioput/file/FileAnnotationWriter.cpp
source/ioput/file/FileAnnotationIn.cpp
source/ioput/file/FileAnnotationOut.cpp
source/ioput/file/FileAscii.cpp
source/ioput/file/FileBinary.cpp
source/ioput/file/FileBinaryLZ4.cpp
source/ioput/file/FileCSV.cpp
source/ioput/file/FileEventsIn.cpp
source/ioput/file/FileEventsOut.cpp
source/ioput/file/FilePath.cpp
source/ioput/file/FileProvider.cpp
source/ioput/file/FileSamplesIn.cpp
source/ioput/file/FileSamplesOut.cpp
source/ioput/file/FileStreamIn.cpp
source/ioput/file/FileStreamOut.cpp
source/ioput/file/FileTools.cpp
source/ioput/option/CmdArgOption.cpp
source/ioput/option/CmdArgParser.cpp
source/ioput/option/OptionList.cpp
source/ioput/socket/Socket.cpp
source/ioput/socket/SocketOsc.cpp
source/ioput/socket/SocketOscEventWriter.cpp

source/ioput/socket/ip/IpEndpointName.cpp

source/ioput/socket/osc/OscOutboundPacketStream.cpp
source/ioput/socket/osc/OscPrintReceivedElements.cpp
source/ioput/socket/osc/OscReceivedElements.cpp
source/ioput/socket/osc/OscTypes.cpp
source/ioput/wav/WavTools.cpp
source/ioput/xml/tinystr.cpp
source/ioput/xml/tinyxml.cpp
source/ioput/xml/tinyxmlerror.cpp
source/ioput/xml/tinyxmlparser.cpp

source/ioput/option/OptionList.cpp
source/ioput/option/CmdArgParser.cpp
source/ioput/option/CmdArgOption.cpp

source/base/String.cpp
source/base/StringList.cpp
source/base/Random.cpp

source/event/EventAddress.cpp
#source/model/SampleList.cpp

source/base/Factory.cpp

source/ioput/socket/SocketUdp.cpp
source/graphic/Console.cpp


)

 IF((SSI_GUI STREQUAL "SDL") AND UNIX)
 set(GUI_SRC_FILES
        source/graphic/SDL_Canvas.cpp
        source/graphic/SDL_Window.cpp
        source/graphic/SDL_WindowManager.cpp
        source/graphic/SDL_Painter.cpp
	source/graphic/PaintData.cpp
	source/graphic/GraphicTools.cpp
	source/graphic/Colormap.cpp
        source/graphic/SDL_Monitor.cpp
	)
else((SSI_GUI STREQUAL "SDL") AND UNIX)
set(GUI_SRC_FILES
    source/graphic/WindowFallback.cpp
    )
endif((SSI_GUI STREQUAL "SDL") AND UNIX)

if(ANDROID)
        set(TCP_SOCKET_FILES

        )
else(ANDROID)
	set(TCP_SOCKET_FILES
		source/ioput/socket/ip/TcpSocket.cpp
		source/ioput/socket/SocketTcp.cpp

	)
endif(ANDROID)

 IF(WIN32)
 set(P_SRC_FILES
	source/ioput/socket/ip/win32/NetworkingUtils.cpp
	source/ioput/socket/ip/win32/UdpSocket.cpp

	source/graphic/Canvas.cpp
	source/graphic/Monitor.cpp
	source/graphic/Slider.cpp
	source/graphic/Window.cpp
	source/graphic/DialogLibGateway.cpp
	source/graphic/PaintData.cpp
	source/graphic/Colormap.cpp
	source/graphic/GraphicTools.cpp
	source/graphic/Painter.cpp

	source/ioput/pipe/NamedPipe.cpp
	)
 ELSE(WIN32)
 set(P_SRC_FILES

	source/ioput/socket/ip/posix/NetworkingUtils.cpp
	source/ioput/socket/ip/posix/UdpSocket.cpp

	)
 ENDIF(WIN32)

 set_source_files_properties(source/r250.c
                            source/ioput/lz4/lz4.c
                            source/ioput/lz4/lz4frame.c
                            source/ioput/lz4/lz4hc.c
                            source/ioput/lz4/xxhash.c

 PROPERTIES LANGUAGE CXX )

 set(SRC_FILES ${COMMON_SRC_FILES} ${P_SRC_FILES} ${GUI_SRC_FILES} ${TCP_SOCKET_FILES})


IF(MINGW )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ggdb" )
  find_library (MINGW_PTHREAD "pthread" HINTS ${MINGWLIB} )
ELSEIF(UNIX)
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SSI_CFLAG}" )
  set( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${SSI_LD_FLAG}" )
  set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${SSI_LD_FLAG}" )
  set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SSI_LD_FLAG}" )
  if(ANDROID)
  SET(MINGW_PTHREAD "")
  else(ANDROID)
  find_library (MINGW_PTHREAD "pthread" HINTS "/usr/lib" )
  endif(ANDROID)
   IF(SSI_GUI STREQUAL "SDL")
          link_directories(${SDL2TTF_LIBRARY_DIRS})
	  link_directories(${SDL2_LIBRARY_DIRS})
	else(SSI_GUI STREQUAL "SDL")
	endif(SSI_GUI STREQUAL "SDL")
ELSE(MINGW)
  SET(MINGW_PTHREAD "")

ENDIF(MINGW)


add_library(ssi STATIC ${SRC_FILES})
IF(UNIX)
   IF(SSI_GUI STREQUAL "SDL")
        target_link_libraries(ssi ${MINGW_PTHREAD} ${SDL2TTF_LIBRARIES} ${SDL2_LIBRARIES} -ldl)
	else(SSI_GUI STREQUAL "SDL")
	target_link_libraries(ssi ${MINGW_PTHREAD} -ldl)
	endif(SSI_GUI STREQUAL "SDL")

ELSE(UNIX)
target_link_libraries(ssi ${MINGW_PTHREAD})
ENDIF(UNIX)
#rename targets if debug
if(ANDROID)
else(ANDROID)
set_target_properties(ssi PROPERTIES DEBUG_POSTFIX "d" PREFIX "")
endif(ANDROID)
#install target to ssi install path set in base directory
install(TARGETS ssi DESTINATION ${SSI_INSTALL}/${SSI_PLATFORM}/${SSI_COMPILER})



