// ****************************************************************************************
//
// Fubi Config
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
//
// ****************************************************************************************

#pragma once

/** @file FubiConfig.h
 * \brief a head file containing defines for configuring the Fubi build
 *
 * Defines:
 * - FUBI_USE_OPENNI1 - define to use OpenNI version 1.x
 * - FUBI_USE_OPENNI2 - define to use OpenNI version 2.x
 * - FUBI_USE_KINECT_SDK - define to use the Kinect SDK (default)
 * - FUBI_KINECT_SDK_MAX_NUM_FACES_TRACKED - define the number of users for the kinect sdk1 face tracking to be used
 * - FUBI_USE_KINECT_SDK_2 - define to use the Kinect SDK 2
 * - FUBI_USE_LEAP - define to use Leap sensor
 * - FUBI_USE_OPENCV - define to use OpenCV
 * - FUBI_OPENCV_VERSION - define a special version of OpenCV (given as a three digit number) to link against,
 *   else it will be found out of the opencv header files
 * - FUBI_COMBINATIONREC_DEBUG_LOGGING - define to print more information about the progress of the combination recognizers to the console
 * - FUBI_TEMPLATEREC_DEBUG_DRAWING - define to let template recognizers plot their training and input data using opencv
 * - FUBI_LOG_LEVEL - Define log level for debug or release configuration
 *   (=FUBI_LOG_VERBOSE, FUBI_LOG_ERR_WRN_INFO, FUBI_LOG_ERR_WRN, FUBI_LOG_ERR_INFO,FUBI_LOG_ERR, FUBI_LOG_SILENT)
 *
*/

//<-----------------------------------------------------------------
// Essential config. Fubi might not work if there is something wrong here.

// Uncomment to use OpenNI version 1.x
//#define FUBI_USE_OPENNI1

// Uncomment to use OpenNI version 2.x
//#define FUBI_USE_OPENNI2

// Uncomment to use the Kinect SDK (default)
//#define FUBI_USE_KINECT_SDK

// Define the number of users for the kinect sdk1 face tracking to be used
#define FUBI_KINECT_SDK_MAX_NUM_FACES_TRACKED 2

// Uncomment to use the Kinect SDK 2
//#define FUBI_USE_KINECT_SDK_2

// Check if Kinect SDK 2 is possible at all (do not edit!)
#if defined FUBI_USE_KINECT_SDK_2 && _MSC_VER < 1700
// If the next line is active, you cannot use the Kinect SDK 2 sensor as your visual studio version is too old
#undef FUBI_USE_KINECT_SDK_2
#endif

// Uncomment to use Leap sensor
//#define FUBI_USE_LEAP

// Uncomment to use OpenCV
// Note: without OpenCV active, you won't get debug information (e.g. the skeletons) rendered onto the depth image and finger count recognizers won't work!
#define FUBI_USE_OPENCV
// Uncomment to link against a special version of OpenCV (given as number consisting of digits only)
// Else the version will be found out from the opencv headers
// Note: not all OpenCV versions might work, the version here is the latest tested version
//#define FUBI_OPENCV_VERSION 300

//----------------------------------------------------------------->


//<-----------------------------------------------------------------
// Optional configs, for getting more debug infos, etc.

// Uncomment to print more information about the progress of the combination recognizers to the console
//#define FUBI_COMBINATIONREC_DEBUG_LOGGING

// Uncomment to let template recognizers plot their training and input data using opencv
//#define FUBI_TEMPLATEREC_DEBUG_DRAWING


// Log level options (Do not modify!)
// 0=all messages 1=errors, warnings, and infos 2= errors and warnings 3=errors and infos 4=errors only 5=silent
#define FUBI_LOG_VERBOSE 0
#define FUBI_LOG_ERR_WRN_INFO 1
#define FUBI_LOG_ERR_WRN 2
#define FUBI_LOG_ERR_INFO 3
#define FUBI_LOG_ERR 4
#define FUBI_LOG_SILENT 5

// Define log level (edit for debug or release configuration)
#ifdef _DEBUG
#define FUBI_LOG_LEVEL FUBI_LOG_VERBOSE
#else
#define FUBI_LOG_LEVEL FUBI_LOG_ERR_WRN_INFO
#endif

//----------------------------------------------------------------->
