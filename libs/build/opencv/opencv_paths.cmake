
function( opencvPaths OPENCV_PATH OPENCV_PATH_SHARED OPENCV_LIB_DEBUG OPENCV_LIB OPENCV_SHARED_DEBUG OPENCV_SHARED) 

	# add static debug libs
	if(WIN32)			
		set(${OPENCV_LIB_DEBUG}
							
								${OPENCV_PATH}/opencv_calib3d300d.lib
								${OPENCV_PATH}/opencv_core300d.lib
								${OPENCV_PATH}/opencv_features2d300d.lib
								${OPENCV_PATH}/opencv_flann300d.lib
								${OPENCV_PATH}/opencv_hal300d.lib
								${OPENCV_PATH}/opencv_highgui300d.lib
								${OPENCV_PATH}/opencv_imgcodecs300d.lib
								${OPENCV_PATH}/opencv_imgproc300d.lib
								${OPENCV_PATH}/opencv_ml300d.lib
								${OPENCV_PATH}/opencv_objdetect300d.lib
								${OPENCV_PATH}/opencv_photo300d.lib
								${OPENCV_PATH}/opencv_shape300d.lib
								${OPENCV_PATH}/opencv_stitching300d.lib
								${OPENCV_PATH}/opencv_superres300d.lib
								${OPENCV_PATH}/opencv_ts300d.lib
								${OPENCV_PATH}/opencv_video300d.lib
								${OPENCV_PATH}/opencv_videoio300d.lib
								${OPENCV_PATH}/opencv_videostab300d.lib
															
						PARENT_SCOPE)
	endif(WIN32)
	
	# add static release libs
	if(WIN32)
		set(${OPENCV_LIB}
							
								${OPENCV_PATH}/opencv_calib3d300.lib
								${OPENCV_PATH}/opencv_core300.lib
								${OPENCV_PATH}/opencv_features2d300.lib
								${OPENCV_PATH}/opencv_flann300.lib
								${OPENCV_PATH}/opencv_hal300.lib
								${OPENCV_PATH}/opencv_highgui300.lib
								${OPENCV_PATH}/opencv_imgcodecs300.lib
								${OPENCV_PATH}/opencv_imgproc300.lib
								${OPENCV_PATH}/opencv_ml300.lib
								${OPENCV_PATH}/opencv_objdetect300.lib
								${OPENCV_PATH}/opencv_photo300.lib
								${OPENCV_PATH}/opencv_shape300.lib
								${OPENCV_PATH}/opencv_stitching300.lib
								${OPENCV_PATH}/opencv_superres300.lib
								${OPENCV_PATH}/opencv_ts300.lib
								${OPENCV_PATH}/opencv_video300.lib
								${OPENCV_PATH}/opencv_videoio300.lib
								${OPENCV_PATH}/opencv_videostab300.lib
							
								PARENT_SCOPE)
	endif(WIN32)

	# add debug dlls
	if(WIN32)			
		set(${OPENCV_SHARED_DEBUG}
							
								${OPENCV_PATH_SHARED}/opencv_calib3d300d.lib
								${OPENCV_PATH_SHARED}/opencv_core300d.lib
								${OPENCV_PATH_SHARED}/opencv_features2d300d.lib
								${OPENCV_PATH_SHARED}/opencv_flann300d.lib
								${OPENCV_PATH_SHARED}/opencv_hal300d.lib
								${OPENCV_PATH_SHARED}/opencv_highgui300d.lib
								${OPENCV_PATH_SHARED}/opencv_imgcodecs300d.lib
								${OPENCV_PATH_SHARED}/opencv_imgproc300d.lib
								${OPENCV_PATH_SHARED}/opencv_ml300d.lib
								${OPENCV_PATH_SHARED}/opencv_objdetect300d.lib
								${OPENCV_PATH_SHARED}/opencv_photo300d.lib
								${OPENCV_PATH_SHARED}/opencv_shape300d.lib
								${OPENCV_PATH_SHARED}/opencv_stitching300d.lib
								${OPENCV_PATH_SHARED}/opencv_superres300d.lib
								${OPENCV_PATH_SHARED}/opencv_ts300d.lib
								${OPENCV_PATH_SHARED}/opencv_video300d.lib
								${OPENCV_PATH_SHARED}/opencv_videoio300d.lib
								${OPENCV_PATH_SHARED}/opencv_videostab300d.lib
							
								PARENT_SCOPE)
	endif(WIN32)
	
	# add release dlls
	if(WIN32)
		set(${OPENCV_SHARED}
							
								${OPENCV_PATH_SHARED}/opencv_calib3d300.lib
								${OPENCV_PATH_SHARED}/opencv_core300.lib
								${OPENCV_PATH_SHARED}/opencv_features2d300.lib
								${OPENCV_PATH_SHARED}/opencv_flann300.lib
								${OPENCV_PATH_SHARED}/opencv_hal300.lib
								${OPENCV_PATH_SHARED}/opencv_highgui300.lib
								${OPENCV_PATH_SHARED}/opencv_imgcodecs300.lib
								${OPENCV_PATH_SHARED}/opencv_imgproc300.lib
								${OPENCV_PATH_SHARED}/opencv_ml300.lib
								${OPENCV_PATH_SHARED}/opencv_objdetect300.lib
								${OPENCV_PATH_SHARED}/opencv_photo300.lib
								${OPENCV_PATH_SHARED}/opencv_shape300.lib
								${OPENCV_PATH_SHARED}/opencv_stitching300.lib
								${OPENCV_PATH_SHARED}/opencv_superres300.lib
								${OPENCV_PATH_SHARED}/opencv_ts300.lib
								${OPENCV_PATH_SHARED}/opencv_video300.lib
								${OPENCV_PATH_SHARED}/opencv_videoio300.lib
								${OPENCV_PATH_SHARED}/opencv_videostab300.lib
							
								PARENT_SCOPE)
	endif(WIN32)



endfunction(opencvPaths)
