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

project(openssi-plugins)


option(OPENSSI_BUILD_MOUSE "Build mouse plugin" OFF)
option(OPENSSI_BUILD_BEACON_SCANNER "Build beacon scanner plugin" OFF)
option(OPENSSI_BUILD_FFMPEG "Build ffmpeg plugin" OFF)
option(OPENSSI_BUILD_WEBSOCKET "Build websocket plugin" OFF)
option(OPENSSI_BUILD_MQTT "Build mqtt plugin" OFF)
option(OPENSSI_BUILD_STIMULI "Build stimuli plugin" OFF)
option(OPENSSI_BUILD_OPENSMILE "Build opensmile plugin" OFF)
option(OPENSSI_BUILD_EMOVOICE "Build emovoice plugin" OFF)
option(OPENSSI_BUILD_OPENFACE "Build OpenFace plugin" OFF)
option(OPENSSI_BUILD_EVENTSERIALIZER "Build eventserializer plugin" OFF)
option(OPENSSI_BUILD_SIGNAL "Build signal plugin" ON)
option(OPENSSI_BUILD_BIOSIG "Build biosig plugin" OFF)
option(OPENSSI_BUILD_MODEL "Build model plugin" ON)
option(OPENSSI_BUILD_AUDIO "Build audio plugin" OFF)
option(OPENSSI_BUILD_XMPP "Build xmpp plugin" OFF)
option(OPENSSI_BUILD_PYTHON "Build python plugin" OFF)
option(OPENSSI_BUILD_SSJSENSOR "Build ssjSensor plugin" OFF)
option(OPENSSI_BUILD_SHORE "Build shore plugin" OFF)
option(OPENSSI_BUILD_FUSION "Build fusion plugin" OFF)
option(OPENSSI_BUILD_CONTINUOUSSYNC "Build coninuous sync plugin" OFF)
option(OPENSSI_BUILD_LIBLINEAR "Build liblinear plugin" OFF)
option(OPENSSI_BUILD_LIBLINEARINCREMENTAL "Build liblinearincremental plugin" OFF)
option(OPENSSI_BUILD_SENSINGTEX "Build sensingtex plugin" OFF)
option(OPENSSI_BUILD_PRESSUREMATRIX "Build pressurematrix plugin" OFF)
option(OPENSSI_BUILD_GSRRESPONSE "Build Gsr Response plugin" OFF)
option(OPENSSI_BUILD_GTEC "Build GTEC plugin" OFF)
option(OPENSSI_BUILD_ACC_FEATURES "Build ACC_FEATURES plugin" OFF)
option(OPENSSI_BUILD_STREAMDM "Build StreamDM plugin" OFF)

include_directories()

add_subdirectory(event)
add_dependencies(ssievent ssi )

#add_subdirectory(dialog)
#add_dependencies(ssidialog ssi )

add_subdirectory(frame)
add_dependencies(ssiframe ssi )

add_subdirectory(ioput)
add_dependencies(ssiioput ssi )

if(ANDROID)


	add_subdirectory(androidSensors)
	add_dependencies(ssiandroidsensors ssiframe ssi )

	add_subdirectory(androidJavaSensors)
	add_dependencies(ssiandroidjavasensors ssiframe ssi )

	add_subdirectory(ssjbridge)
	add_dependencies(ssissjbridge ssi )

else(ANDROID)

	IF(WIN32)
		add_subdirectory(graphic)
	endif(WIN32)
	IF(SSI_GUI STREQUAL "SDL")
		add_subdirectory(graphic)
		add_dependencies(ssigraphic ssi )
	endif(SSI_GUI STREQUAL "SDL")

	if(OPENSSI_BUILD_MOUSE)
		add_subdirectory(openface)
		add_dependencies(ssiopenface ssi )
	endif(OPENSSI_BUILD_MOUSE)
	
	if(OPENSSI_BUILD_OPENFACE)
		add_subdirectory(mouse)
		add_dependencies(ssimouse ssi )
	endif(OPENSSI_BUILD_OPENFACE)

	if(OPENSSI_BUILD_BEACON_SCANNER)
		add_subdirectory(beaconScanner)
		add_dependencies(ssibeaconscanner ssi serial)
	endif(OPENSSI_BUILD_BEACON_SCANNER)

	if(OPENSSI_BUILD_FFMPEG)
		add_subdirectory(ffmpeg)
		add_dependencies(ssiffmpeg ssi )
	endif(OPENSSI_BUILD_FFMPEG)

	if(OPENSSI_BUILD_PYTHON)
		add_subdirectory(python)
		add_dependencies(ssipython ssi )
	endif(OPENSSI_BUILD_PYTHON)

	if(OPENSSI_BUILD_SENSINGTEX)
	    add_subdirectory(sensingtex)
	    add_dependencies(ssisensingtex ssi serial)
	endif(OPENSSI_BUILD_SENSINGTEX)

	if(OPENSSI_BUILD_PRESSUREMATRIX)
	    add_subdirectory(pressurematrix)
	    add_dependencies(ssipressurematrix ssi serial)
	endif(OPENSSI_BUILD_PRESSUREMATRIX)

endif(ANDROID)

if(OPENSSI_BUILD_STREAMDM)
	add_subdirectory(streamDM)
	add_dependencies(ssistreamdm ssi )
endif(OPENSSI_BUILD_STREAMDM)
if(OPENSSI_BUILD_WEBSOCKET)
	add_subdirectory(websocket)
	add_dependencies(ssiwebsocket ssi )
endif(OPENSSI_BUILD_WEBSOCKET)

if(OPENSSI_BUILD_MQTT)
	add_subdirectory(mqtt)
	add_dependencies(ssimqtt ssi )
endif(OPENSSI_BUILD_MQTT)


if(OPENSSI_BUILD_STIMULI)
	#add_subdirectory(stimuli)
	#add_dependencies(ssistimuli ssi )
endif(OPENSSI_BUILD_STIMULI)


if(OPENSSI_BUILD_FASTDTW)
	add_subdirectory(fastDTW)
	add_dependencies(ssifastDTW ssi )
endif(OPENSSI_BUILD_FASTDTW)


if(OPENSSI_BUILD_OPENSMILE)
	add_subdirectory(opensmile)
	add_dependencies(ssiopensmile ssi )
endif(OPENSSI_BUILD_OPENSMILE)


if(OPENSSI_BUILD_EMOVOICE)
	add_subdirectory(emovoice)
	add_dependencies(ssiemovoice ssi emovoice)
endif(OPENSSI_BUILD_EMOVOICE)

if(OPENSSI_BUILD_FUSION)
	add_subdirectory(fusion)
        add_dependencies(ssifusion ssi ssifusion)
endif(OPENSSI_BUILD_FUSION)

if(OPENSSI_BUILD_EVENTSERIALIZER)
	
endif(OPENSSI_BUILD_EVENTSERIALIZER)

if(OPENSSI_BUILD_SIGNAL)
	add_subdirectory(signal)
	add_dependencies(ssisignal ssi kissFFT ExpressionParser)
endif(OPENSSI_BUILD_SIGNAL)

if(OPENSSI_BUILD_BIOSIG)
	add_subdirectory(biosig)
	add_dependencies(ssibiosig ssi )
endif(OPENSSI_BUILD_BIOSIG)

if(OPENSSI_BUILD_MODEL)
	add_subdirectory(model)
	add_dependencies(ssimodel ssi AlgLib ssiml)
endif(OPENSSI_BUILD_MODEL)


if(OPENSSI_BUILD_AUDIO)
	add_subdirectory(audio)
	add_dependencies(ssiaudio ssi )
endif(OPENSSI_BUILD_AUDIO)


if(OPENSSI_BUILD_XMPP)
	add_subdirectory(xmpp)
	add_dependencies(ssixmpp ssi )
endif(OPENSSI_BUILD_XMPP)


if(OPENSSI_BUILD_CONTINUOUSSYNC)
        add_subdirectory(continuousSync)
        add_dependencies(ssicontinuoussync ssi )
endif(OPENSSI_BUILD_CONTINUOUSSYNC)

if(OPENSSI_BUILD_LIBLINEAR)
        add_subdirectory(liblinear)
        add_dependencies(ssiliblinear ssi )
endif(OPENSSI_BUILD_LIBLINEAR)

if(OPENSSI_BUILD_LIBLINEARINCREMENTAL)
        add_subdirectory(liblinearincremental)
        add_dependencies(ssiliblinearincremental ssi )
endif(OPENSSI_BUILD_LIBLINEARINCREMENTAL)

if(OPENSSI_BUILD_GSRRESPONSE)
        add_subdirectory(gsr_response)
        add_dependencies(ssigsrResponse ssi )
endif(OPENSSI_BUILD_GSRRESPONSE)

IF(SSI_GUI STREQUAL "SDL")
else(SSI_GUI STREQUAL "SDL")
endif(SSI_GUI STREQUAL "SDL")


if(OPENSSI_BUILD_SSJSENSOR)
	add_subdirectory(ssjSensor)
	add_dependencies(ssissjSensor ssi )
endif(OPENSSI_BUILD_SSJSENSOR)

if(OPENSSI_BUILD_GTEC)
	add_subdirectory(gtec)
	add_dependencies(ssigtec ssi )
endif(OPENSSI_BUILD_GTEC)

IF(WIN32)
if(OPENSSI_BUILD_SHORE)
        add_subdirectory(shore)
        add_dependencies(ssishore ssi )
endif(OPENSSI_BUILD_SHORE)
endif(WIN32)

if(OPENSSI_BUILD_ACC_FEATURES)
    add_subdirectory(accFeatures)
    add_dependencies(ssiaccFeatures ssi)
endif(OPENSSI_BUILD_ACC_FEATURES)

#install(TARGETS openssi-core DESTINATION bin)
