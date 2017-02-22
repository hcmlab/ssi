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


add_subdirectory(Cluster)
add_dependencies(ssimodel_cluster_test ssimodel ssi )

add_subdirectory(Fsel)
add_dependencies(ssimodel_fsel_test ssimodel ssi )

add_subdirectory(Fred)
add_dependencies(ssimodel_fred_test ssimodel ssi )

add_subdirectory(Model)
add_dependencies(ssimodel_test ssimodel ssi )

add_subdirectory(Online)
add_dependencies(ssimodel_online_test ssimodel ssi )

add_subdirectory(Samples)
add_dependencies(ssimodel_samples_test ssimodel ssi )


endif(ANDROID)


