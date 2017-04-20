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

project(openssi-model-tools)






if(ANDROID)




else(ANDROID)


add_subdirectory(xmlTrain)
add_dependencies(ssixmltrain ssimodel ssi )

#add_subdirectory(Train)
#add_dependencies(ssimlpxmltrain ssimodel ssi )

add_subdirectory(anno2sample)
add_dependencies(ssianno2sample ssimodel ssi )

add_subdirectory(elan2ssiAnno)
add_dependencies(elan2ssiAnno ssimodel ssi )


endif(ANDROID)


