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

project(emovoice)

include_directories(include ../../../core/include )



set(COMMON_SRC_FILES

source/ev___program.c
source/ev_acf.c
source/ev_basics.c
source/ev_channel.c
source/ev_class.c
source/ev_csp.c
source/ev_dct.c
source/ev_delay.c
source/ev_deriv.c
source/ev_dft.c
source/ev_durationFeatures.c
source/ev_dwt.c
source/ev_efeatures.c
source/ev_eigen.c
source/ev_energyFeatures.c
source/ev_extract.c
source/ev_extract_v1.c
source/ev_fex.c
source/ev_fextract.c
source/ev_fft.c
source/ev_filterbank.c
source/ev_fselect.c
source/ev_fw_masking.c
source/ev_hashq.c
source/ev_histogram.c
source/ev_hnr.c
source/ev_io.c
source/ev_lpc.c
source/ev_matrix.c
source/ev_mel.c
source/ev_memory.c
source/ev_messages.c
source/ev_mfcc.c
source/ev_mfcc_1_3.c
source/ev_mfcc_1_4.c
source/ev_mfcc_1_6.c
source/ev_mfccFeatures.c
source/ev_naive_bayes.c
source/ev_name.c
source/ev_pitch.c
source/ev_pitchFeatures.c
source/ev_preemph.c
source/ev_prob.c
source/ev_queue.c
source/ev_score.c
source/ev_sdft.c
source/ev_segment.c
source/ev_select.c
source/ev_serien.c
source/ev_shift.c
source/ev_spectralCoG.c
source/ev_spectralFeatures.c
source/ev_spectrum.c
source/ev_sum.c
source/ev_svm.cpp
source/ev_svm_wrapper.c
source/ev_tirol.c
source/ev_vad.c
source/ev_vector.c
source/ev_voiceQualityFeatures.c
source/ev_voicingFeatures.c
source/ev_window.c


)
 #set_source_files_properties(
 #PROPERTIES LANGUAGE CXX )

set(SRC_FILES ${COMMON_SRC_FILES} )

#find_library(OPENSSI_CORE_LIBRARY NAMES openssi-core HINTS ../../cmake-core/corelib/Debug/ ../../cmake-core/corelib/Release/)

IF(UNIX)
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ggdb  -fPIC" )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -ggdb  -fPIC" )
ENDIF(UNIX)

add_library(emovoice STATIC ${SRC_FILES} )

#add_executable(myapp main.c)

IF(UNIX)
target_link_libraries(emovoice ssi ssiml -ldl )
ELSE(UNIX)
target_link_libraries(emovoice ssi ssiml )
ENDIF(UNIX)
#rename targets if debug
set_target_properties(emovoice PROPERTIES DEBUG_POSTFIX "d" PREFIX "")

#install target to ssi install path set in base directory
install(TARGETS emovoice DESTINATION ${SSI_INSTALL}/${SSI_PLATFORM}/${SSI_COMPILER})
