// ****************************************************************************************
//
// Fubi API
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
//
// ****************************************************************************************

#pragma once
#ifndef FUBI_H
#define FUBI_H

/** \file Fubi.h
* \brief The Fubi C++ API
*/

//#if defined (WIN32) || defined (_WINDOWS)
//#	 ifdef FUBI_EXPORTS
//#       define FUBI_API extern "C" __declspec( dllexport )
//#	 else
//#       define FUBI_API extern "C" __declspec( dllimport )
//#    endif
//#else
//#	 define FUBI_API extern "C"
//#endif
//
//#if defined (WIN32) || defined (_WINDOWS)
//#	 ifdef FUBI_EXPORTS
//#       define FUBI_CPP_API  __declspec( dllexport )
//#	 else
//#       define FUBI_CPP_API __declspec( dllimport )
//#    endif
//#else
//#	 define FUBI_CPP_API
//#endif
// SSI directly includes fubi in the build instead of creating a dll
#define FUBI_API
#define FUBI_CPP_API

#include "FubiUser.h"
#include "FubiHand.h"

#include <functional>

/**
* \mainpage Fubi - Full Body Interaction Framework
*
* \section intro_sec Introduction
*
* Full Body Interaction Framework (FUBI) is a framework for recognizing full body gestures and postures in real time from the data of a depth sensor compatible with OpenNI (1.x or 2.x) or the Kinect SDK.
* It further supports finger tracking using a Leap motion sensor or injecting formated tracking data of an own sensor
*/


/**
* \namespace Fubi
*
* \brief The Fubi namespace provides all methods to control the Full Body Interaction framework (FUBI).
*
*/
namespace Fubi
{
	/** \addtogroup FUBICPP FUBI C++ API
	* All the C++ API functions
	*
	* @{
	*/

	/**
	* \brief Initializes Fubi with OpenN 1.x using the given xml file and sets the skeleton profile.
	*        If no xml file is given, Fubi will be intialized without OpenNI tracking enabled --> methods that need an openni context won't work.
	*
	* @param openniXmlconfig name of the xml file for OpenNI 1.x initialization inlcuding all needed productions nodes
	(should be placed in the working directory, i.e. "bin" folder)
	if config == 0x0, then OpenNI won't be initialized and Fubi stays in non-tracking mode
	* @param profile set the openNI skeleton profile
	* @param filterMinCutOffFrequency, filterVelocityCutOffFrequency, filterCutOffSlope, bodyMeasureFilterFac options for filtering the tracking data if wanted
	* @param mirrorStreams whether the stream should be mirrored or not
	* @param registerStreams whether the depth stream should be registered to the color stream
	* @return true if succesfully initialized or already initialized before,
	false means bad xml file or other serious problem with OpenNI initialization
	*/
	FUBI_API bool initFromXML(const char* openniXmlconfig = 0x0, Fubi::SkeletonTrackingProfile::Profile profile = Fubi::SkeletonTrackingProfile::ALL,
		float filterMinCutOffFrequency = 1.0f, float filterVelocityCutOffFrequency = 1.0f, 
		float filterCutOffSlope = 0.007f, float bodyMeasureFilterFac = 0.1f,
		bool mirrorStreams = true, bool registerStreams =true);


	/**
	* \brief Initializes Fubi with an options file for the sensor init
	*
	* @param sensorOptions configuration for the sensor
	* @param filterOptions filter options for additionally filtering of the tracking data
	* @return true if succesfully initialized or already initialized before,
	false means problem with sensor init
	*/
	FUBI_CPP_API bool init(const SensorOptions& sensorOptions, const FilterOptions& filterOptions = FilterOptions());

	/**
	* \brief Initializes Fubi with specific options for the sensor init
	*
	* @param depthWidth, depthHeight, depthFPS, rgbWidth, rgbHeight, rgbFPS,
	irWidth, irHeight, irFPS, sensorType, profile, filterMinCutOffFrequency, filterVelocityCutOffFrequency, 
	filterCutOffSlope, bodyMeasureFilterFac, mirrorStream, registerStreams 
	configuration for the sensor as in the SensorOptions and FilterOptions struct
	* @return true if succesfully initialized or already initialized before,
	false means problem with sensor init
	*/
	FUBI_API bool init(int depthWidth, int depthHeight, int depthFPS = 30,
		int rgbWidth = 640, int rgbHeight = 480, int rgbFPS = 30,
		int irWidth = -1, int irHeight = -1, int irFPS = -1,
		Fubi::SensorType::Type sensorType = Fubi::SensorType::OPENNI2,
		Fubi::SkeletonTrackingProfile::Profile profile = Fubi::SkeletonTrackingProfile::ALL,
		float filterMinCutOffFrequency = 1.0f, float filterVelocityCutOffFrequency = 1.0f,
		float filterCutOffSlope = 0.007f, float bodyMeasureFilterFac = 0.1f,
		bool mirrorStream = true, bool registerStreams = true);

	/**
	* \brief Allows you to switch between different sensor types during runtime
	*		  Note that this will also reinitialize most parts of Fubi
	*
	* @param options options for initializing the new sensor
	* @return true if the sensor has been succesfully initialized
	*/
	FUBI_CPP_API bool switchSensor(const SensorOptions& options);

	/**
	* \brief Allows you to switch between different sensor types during runtime
	*		  Note that this will also reinitialize most parts of Fubi
	*
	* @param sensorType the sensor type to switch to
	* @param sensorType, depthWidth, depthHeight, depthFPS, rgbWidth, rgbHeight, rgbFPS,
	irWidth, irHeight, irFPS, profile, mirrorStream, registerStreams
	configuration for the sensor as in the SensorOptions struct
	* @return true if the sensor has been succesfully initialized
	*/
	FUBI_API bool switchSensor(Fubi::SensorType::Type sensorType, int depthWidth, int depthHeight, int depthFPS = 30,
		int rgbWidth = 640, int rgbHeight = 480, int rgbFPS = 30,
		int irWidth = -1, int irHeight = -1, int irFPS = -1,
		Fubi::SkeletonTrackingProfile::Profile profile = Fubi::SkeletonTrackingProfile::ALL,
		bool mirrorStream = true, bool registerStreams =true);


	/**
	* \brief Get the currently available sensor types (defined in FubiConfig.h before compilation)
	*
	* @return an int composed of the currently available sensor types (see SensorType enum for the meaning)
	*/
	FUBI_API int getAvailableSensorTypes();

	/**
	* \brief Get the type of the currently active sensor
	*
	* @return the current sensor type
	*/
	FUBI_API Fubi::SensorType::Type getCurrentSensorType();


	/**
	* \brief Shuts down Fubi and its sensors, releasing all allocated memory
	*
	*/
	FUBI_API void release();

	/**
	* \brief Returns true if Fubi has been already initialized
	*
	*/
	FUBI_API bool isInitialized();


	/**
	* \brief Updates the current sensor to get the next frame of depth, rgb/ir, and tracking data.
	*        Also searches for users in the scene to start tracking them
	*
	*/
	FUBI_API void updateSensor();

	/**
	* \brief retrieve an image from one of the sensor streams with specific format and optionally enhanced by different
	*        tracking information
	*		  Some render options require an OpenCV installation!
	*
	* @param[out] outputImage pointer to a unsigned char array
	*        Will be filled with wanted image
	*		  Array has to be of correct size, e.g. depth image (640x480 std resolution) with tracking info
	*		  requires 4 channels (RGBA) --> size = 640*480*4 = 1228800
	* @param type can be color, depth, ir image, or blank for only rendering tracking info
	* @param numChannels number channels in the image 1, 3 or 4
	* @param depth the pixel depth of the image, 8 bit (standard), 16 bit (mainly usefull for depth images) or 32 bit float
	* @param renderOptions options for rendering additional informations into the image (e.g. tracking skeleton) or swapping the r and b channel
	* @param jointsToRender defines for which of the joints the trackinginfo (see renderOptions) should be rendererd
	* @param depthModifications options for transforming the depth image to a more visible format
	* @param userId If set to something else than 0 an image will be cut cropped around (the joint of interest of) this user, if 0 the whole image is put out.
	* @param jointOfInterest the joint of the user the image is cropped around and a threshold on the depth values is applied.
	If set to num_joints fubi tries to crop out the whole user.
	* @param moveCroppedToUpperLeft moves the cropped image to the upper left corner
	*/
	FUBI_API bool getImage(unsigned char* outputImage, ImageType::Type type, ImageNumChannels::Channel numChannels, ImageDepth::Depth depth,
		int renderOptions = (RenderOptions::Shapes | RenderOptions::Skeletons | RenderOptions::UserCaptions),
		int jointsToRender = RenderOptions::ALL_JOINTS,
		DepthImageModification::Modification depthModifications = DepthImageModification::UseHistogram,
		unsigned int userId = 0, Fubi::SkeletonJoint::Joint jointOfInterest = Fubi::SkeletonJoint::NUM_JOINTS, bool moveCroppedToUpperLeft = false);

	/**
	* \brief save an image from one of the sensor streams with specific format and optionally enhanced by different
	*        tracking information
	*
	* @param fileName filename where the image should be saved to
	*        can be relative to the working directory (bin folder) or absolute
	*		  the file extension determins the file format (should be jpg)
	* @param jpegQuality qualitiy (= 88) of the jpeg compression if a jpg file is requested, ranges from 0 to 100 (best quality)
	* @param type can be color, depth, or ir image
	* @param numChannels number channels in the image 1, 3 or 4
	* @param depth the pixel depth of the image, 8 bit (standard) or 16 bit (mainly usefull for depth images
	* @param renderOptions options for rendering additional informations into the image (e.g. tracking skeleton) or swapping the r and b channel
	* @param jointsToRender defines for which of the joints the trackinginfo (see renderOptions) should be rendererd
	* @param depthModifications options for transforming the depht image to a more visible format
	* @param userId If set to something else than 0 an image will be cut cropped around (the joint of interest of) this user, if 0 the whole image is put out.
	* @param jointOfInterest the joint of the user the image is cropped around and a threshold on the depth values is applied.
	If set to num_joints fubi tries to crop out the whole user.
	*/
	FUBI_API bool saveImage(const char* fileName, int jpegQuality, ImageType::Type type, ImageNumChannels::Channel numChannels, ImageDepth::Depth depth,
		int renderOptions = (RenderOptions::Shapes | RenderOptions::Skeletons | RenderOptions::UserCaptions),
		int jointsToRender = RenderOptions::ALL_JOINTS,
		DepthImageModification::Modification depthModifications = DepthImageModification::UseHistogram,
		unsigned int userId = 0, Fubi::SkeletonJoint::Joint jointOfInterest = Fubi::SkeletonJoint::NUM_JOINTS);


	/**
	* \brief Tries to recognize a posture in the current frame of tracking data of one user
	*
	* @param postureID enum id of the posture to be found in FubiPredefinedGestures.h
	* @param userID the id of the user to be checked
	* @return RECOGNIZED in case of a succesful detection, TRACKING_ERROR if a needed joint is currently not tracked, NOT_RECOGNIZED else
	*/
	FUBI_API Fubi::RecognitionResult::Result recognizePostureOn(Postures::Posture postureID, unsigned int userID);

	/**
	* \brief Checks a user defined gesture or posture recognizer for its success
	*
	* @param recognizerIndex id of the recognizer return during its creation
	* @param userID the id of the user to be checked
	* @param[out] correctionHint will contain information about why the recognition failed if wanted
	* @return RECOGNIZED in case of a succesful detection, TRACKING_ERROR if a needed joint is currently not tracked, NOT_RECOGNIZED else
	*/
	FUBI_CPP_API Fubi::RecognitionResult::Result recognizeGestureOn(unsigned int recognizerIndex, unsigned int userID, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);

	/**
	* \brief Checks a user defined gesture or posture recognizer for its success
	*
	* @param recognizerName name of the recognizer return during its creation
	* @param userID the id of the user to be checked
	* @param[out] correctionHint will contain information about why the recognition failed if wanted
	* @return RECOGNIZED in case of a succesful detection, TRACKING_ERROR if a needed joint is currently not tracked, NOT_RECOGNIZED else
	*/
	FUBI_API Fubi::RecognitionResult::Result recognizeGestureOn(const char* recognizerName, unsigned int userID, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);

	/**
	* \brief Checks a combination recognizer for its progress
	*
	* @param combinationID  enum id of the combination to be found in FubiPredefinedGestures.h
	* @param userID the id of the user to be checked
	* @param userStates (= 0x0) pointer to a vector of tracking data that represents the tracking information of the user
	*		  during the recognition of each state
	* @param restart (=true) if set to true, the recognizer automatically restarts, so the combination can be recognized again.
	* @param returnFilteredData if true, the user states vector will contain filtered data
	* @param[out] correctionHint on NOT_RECOGNIZED, this struct will contain information about why the recognition failed if wanted
	* @return RECOGNIZED in case of a succesful detection, TRACKING_ERROR if a needed joint is currently not tracked, NOT_RECOGNIZED else
	*/
	FUBI_API Fubi::RecognitionResult::Result getCombinationRecognitionProgressByIDOn(Combinations::Combination combinationID, unsigned int userID,
		std::vector<Fubi::TrackingData>* userStates = 0x0, bool restart = true, bool returnFilteredData = false, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);

	/**
	* \brief Checks a user defined combination recognizer for its progress
	*
	* @param recognizerName name of the combination
	* @param userID the id of the user to be checked
	* @param userStates (= 0x0) pointer to a vector of tracking data that represents the tracking information of the user
	*		  during the recognition of each state
	* @param restart (=true) if set to true, the recognizer automatically restarts, so the combination can be recognized again.
	* @param returnFilteredData if true, the user states vector will contain filtered data
	* @param[out] correctionHint on NOT_RECOGNIZED, this struct will contain information about why the recognition failed if wanted
	* @return RECOGNIZED in case of a succesful detection, TRACKING_ERROR if a needed joint is currently not tracked, NOT_RECOGNIZED else
	*/
	FUBI_API Fubi::RecognitionResult::Result getCombinationRecognitionProgressOn(const char* recognizerName, unsigned int userID,
		std::vector<Fubi::TrackingData>* userStates = 0x0, bool restart = true, bool returnFilteredData = false, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);

	/**
	* \brief Starts or stops the recognition process of a combination for one user
	*
	* @param combinationID enum id of the combination to be found in FubiPredefinedGestures.h or Combinations::NUM_COMBINATIONS for all combinations
	* @param userID the id of the user for whom the recognizers should be modified
	* @param enable if set to true, the recognizer will be started (if not already stared), else it stops
	*/
	FUBI_API void enableCombinationRecognitionByID(Combinations::Combination combinationID, unsigned int userID, bool enable);

	/**
	* \brief Starts or stops the recognition process of a user defined combination for one user
	*
	* @param combinationName name defined for this recognizer
	* @param userID the id of the user for whom the recognizers should be modified
	* @param enable if set to true, the recognizer will be started (if not already stared), else it stops
	*/
	FUBI_API void enableCombinationRecognition(const char* combinationName, unsigned int userID, bool enable);

	/**
	* \brief Automatically starts combination recognition for new users
	*
	* @param enable if set to true, the recognizer will automatically start for new users, else this must be done manually (by using enableCombinationRecognition(..))
	* @param combinationID enum id of the combination to be found in FubiPredefinedGestures.h or Combinations::NUM_COMBINATIONS for all combinations (also the user defined ones!)
	*/
	FUBI_API void setAutoStartCombinationRecognition(bool enable, Combinations::Combination combinationID = Combinations::NUM_COMBINATIONS);

	/**
	* \brief Check if autostart is activated for a combination recognizer
	*
	* @param combinationID enum id of the combination to be found in FubiPredefinedGestures.h orCombinations::NUM_COMBINATIONS for all combinations
	* @return true if the corresponding auto start is activated
	*/
	FUBI_API bool getAutoStartCombinationRecognition(Combinations::Combination combinationID = Combinations::NUM_COMBINATIONS);


	/**
	* \brief Returns the color for a user in the background image
	*
	* @param id OpennNI user id of the user of interest
	* @param[out] r, g, b returns the red, green, and blue components of the color in which the users shape is displayed in the tracking image
	*/
	FUBI_API void getColorForUserID(unsigned int id, float& r, float& g, float& b);


	/**
	* \brief Returns user id from the user index
	*
	* @param index index of the user in the user array
	* @return id of that user or 0 if not found
	*/
	FUBI_API unsigned int getUserID(unsigned int index);


	/**
	* \brief Creates a user defined joint relation recognizer
	*
	* @param joint the joint of interest
	* @param relJoint the joint in which it has to be in a specific relation
	* @param minX, minY, minZ (=-inf, -inf, -inf) the minimal values allowed for the vector relJoint -> joint
	* @param maxX, maxY, maxZ (=inf, inf, inf) the maximal values allowed for the vector relJoint -> joint
	* @param minDistance (= 0) the minimal distance between joint and relJoint
	* @param maxDistance (= inf) the maximal distance between joint and relJoint
	* @param midJoint (=num_joints)
	* @param midJointMinX, midJointMinY, midJointMinZ (=-inf, -inf, -inf) the minimal values allowed for the vector from the line segment (relJoint-joint) -> midJoint
	* @param midJointMaxX, midJointMaxY, midJointMaxZ (=inf, inf, inf) the maximal values allowed for the vector from the line segment (relJoint-joint) -> midJoint
	* @param midJointMinDistance (= 0) the minimal distance allowed from the line segment (relJoint-joint) -> midJoint
	* @param midJointMaxDistance (= inf) the maximal distance allowed from the line segment (relJoint-joint) -> midJoint
	* @param useLocalPositions use positions in the local coordinate system of the user based on the torso transformation
	* @param atIndex (= -1) if an index is given, the corresponding recognizer will be replaced instead of creating a new one
	* @param name (= 0) sets a name for the recognizer (should be unique!)
	* @param minConfidence (=-1) if given this is the mimimum confidence required from tracking for the recognition to be succesful
	* @param measuringUnit the measuring unit for the values (millimeter by default)
	* @param useFilteredData (=false) if true, the recognizer will use the filtered tracking data instead of the raw one
	*
	* @return index of the recognizer needed to call it later
	*/
	FUBI_API unsigned int addJointRelationRecognizer(SkeletonJoint::Joint joint, SkeletonJoint::Joint relJoint,
		float minX = -Fubi::Math::MaxFloat, float minY = -Fubi::Math::MaxFloat, float minZ = -Fubi::Math::MaxFloat,
		float maxX = Fubi::Math::MaxFloat, float maxY = Fubi::Math::MaxFloat, float maxZ = Fubi::Math::MaxFloat,
		float minDistance = 0,
		float maxDistance = Fubi::Math::MaxFloat,
		Fubi::SkeletonJoint::Joint midJoint = Fubi::SkeletonJoint::NUM_JOINTS,
		float midJointMinX = -Fubi::Math::MaxFloat, float midJointMinY = -Fubi::Math::MaxFloat, float midJointMinZ = -Fubi::Math::MaxFloat,
		float midJointMaxX = Fubi::Math::MaxFloat, float midJointMaxY = Fubi::Math::MaxFloat, float midJointMaxZ = Fubi::Math::MaxFloat,
		float midJointMinDistance = 0,
		float midJointMaxDistance = Fubi::Math::MaxFloat,
		bool useLocalPositions = false,
		int atIndex = -1,
		const char* name = 0,
		float minConfidence = -1.0f,
		Fubi::BodyMeasurement::Measurement measuringUnit = Fubi::BodyMeasurement::NUM_MEASUREMENTS,
		bool useFilteredData = false);

	/**
	* \brief Creates a user defined joint orientation recognizer
	*
	* @param joint the joint of interest
	* @param minX, minY, minZ (=-180, -180, -180) the minimal degrees allowed for the joint orientation
	* @param maxX, maxY, maxZ (=180, 180, 180) the maximal degrees allowed for the joint orientation
	* @param useLocalOrientations if true, uses a local orienation in which the parent orientation has been substracted
	* @param atIndex (= -1) if an index is given, the corresponding recognizer will be replaced instead of creating a new one
	* @param name (= 0) sets a name for the recognizer (should be unique!)
	* @param minConfidence (=-1) if given this is the mimimum confidence required from tracking for the recognition to be succesful
	* @param useFilteredData (=false) if true, the recognizer will use the filtered tracking data instead of the raw one
	*
	* @return index of the recognizer needed to call it later
	*/
	FUBI_API unsigned int addJointOrientationRecognizer(Fubi::SkeletonJoint::Joint joint,
		float minX = -180.0f, float minY = -180.0f, float minZ = -180.0f,
		float maxX = 180.0f, float maxY = 180.0f, float maxZ = 180.0f,
		bool useLocalOrientations = true,
		int atIndex = -1,
		const char* name = 0,
		float minConfidence =-1,
		bool useFilteredData = false);

	/**
	* \brief Creates a user defined joint orientation recognizer
	*
	* @param joint the joint of interest
	* @param orientX, orientY, orientZ indicate the wanted joint orientation
	* @param maxAngleDifference (=45°) the maximum angle difference that is allowed between the requested orientation and the actual orientation
	* @param useLocalOrientations if true, uses a local orienation in which the parent orientation has been substracted
	* @param atIndex (= -1) if an index is given, the corresponding recognizer will be replaced instead of creating a new one
	* @param name (= 0) sets a name for the recognizer (should be unique!)
	* @param minConfidence (=-1) if given this is the mimimum confidence required from tracking for the recognition to be succesful
	* @param useFilteredData (=false) if true, the recognizer will use the filtered tracking data instead of the raw one
	*
	* @return index of the recognizer needed to call it later
	*/
	FUBI_API unsigned int addJointOrientationRecognizerFromOrient(Fubi::SkeletonJoint::Joint joint,
		float orientX, float orientY, float orientZ,
		float maxAngleDifference = 45.0f,
		bool useLocalOrientations = true,
		int atIndex = -1,
		const char* name = 0,
		float minConfidence =-1,
		bool useFilteredData = false);

	/**
	* \brief Creates a user defined finger count recognizer
	*
	* @param handJoint the hand joint of interest
	* @param minFingers the minimum number of fingers the user should show up
	* @param maxFingers the maximum number of fingers the user should show up
	* @param atIndex (= -1) if an index is given, the corresponding recognizer will be replaced instead of creating a new one
	* @param name (= 0) sets a name for the recognizer (should be unique!)
	* @param minConfidence (=-1) if given this is the mimimum confidence required from tracking for the recognition to be succesful
	* @param useMedianCalculation (=false) if true, the median for the finger count will be calculated over several frames instead of always taking the current detection
	* @param medianWindowSize (=10) defines the window size for calculating the median (=number of frames)
	* @param useFilteredData (=false) if true, the recognizer will use the filtered tracking data instead of the raw one
	*
	* @return index of the recognizer needed to call it later
	*/
	FUBI_API unsigned int addFingerCountRecognizer(Fubi::SkeletonJoint::Joint handJoint,
		unsigned int minFingers, unsigned int maxFingers,
		int atIndex = -1,
		const char* name = 0,
		float minConfidence =-1,
		bool useMedianCalculation = false,
		unsigned int medianWindowSize = 10,
		bool useFilteredData = false);

	/**
	* \brief Creates a user defined linear movement recognizer
	* A linear gesture has a vector calculated as joint - relative joint,
	* the direction (each component -1 to +1) that will be applied per component on the vector, and a min and max vel in milimeter per second
	*
	* @param joint the joint of interest
	* @param relJoint the joint in which it has to be in a specifc relation
	* @param dirX, dirY, dirZ the direction in which the movement should happen
	* @param minVel the minimal velocity that has to be reached in this direction
	* @param maxVel (= inf) the maximal velocity that is allowed in this direction
	* @param useLocalPositions use positions in the local coordinate system of the user based on the torso transformation
	* @param atIndex (= -1) if an index is given, the corresponding recognizer will be replaced instead of creating a new one
	* @param name name of the recognizer
	* @param maxAngleDifference (=45°) the maximum angle difference that is allowed between the requested direction and the actual movement direction
	* @param useOnlyCorrectDirectionComponent (=true) If true, this only takes the component of the actual movement that is conform
	*				the requested direction, else it always uses the actual movement for speed calculation
	* @param useFilteredData (=false) if true, the recognizer will use the filtered tracking data instead of the raw one
	*
	* @return index of the recognizer needed to call it later
	*/
	FUBI_CPP_API unsigned int addLinearMovementRecognizer(SkeletonJoint::Joint joint, SkeletonJoint::Joint relJoint,
		float dirX, float dirY, float dirZ, float minVel, float maxVel = Fubi::Math::MaxFloat,
		bool useLocalPositions = false,
		int atIndex = -1,
		const char* name = 0,
		float maxAngleDifference = 45.0f,
		bool useOnlyCorrectDirectionComponent = true,
		bool useFilteredData = false);
	FUBI_API unsigned int addLinearMovementRecognizer(SkeletonJoint::Joint joint,
		float dirX, float dirY, float dirZ, float minVel, float maxVel = Fubi::Math::MaxFloat,
		bool useLocalPositions = false,
		int atIndex = -1,
		const char* name = 0,
		float maxAngleDifference = 45.0f,
		bool useOnlyCorrectDirectionComponent = true,
		bool useFilteredData = false);
	/**
	* \brief Creates a user defined linear movement recognizer
	* A linear gesture has a vector calculated as joint - relative joint,
	* the direction (each component -1 to +1) that will be applied per component on the vector, and a min and max vel in milimeter per second
	*
	* @param joint the joint of interest
	* @param relJoint the joint in which it has to be in a specifc relation
	* @param dirX, dirY, dirZ the direction in which the movement should happen
	* @param minVel the minimal velocity that has to be reached in this direction
	* @param maxVel (= inf) the maximal velocity that is allowed in this direction
	* @param minLength the minimal length of path that has to be reached (only works within a combination rec)
	* @param maxLength the maximal length of path that can be reached (only works within a combination rec)
	* @param measuringUnit measuring unit for the path length
	* @param useLocalPositions use positions in the local coordinate system of the user based on the torso transformation
	* @param atIndex (= -1) if an index is given, the corresponding recognizer will be replaced instead of creating a new one
	* @param name name of the recognizer
	* @param maxAngleDifference (=45°) the maximum angle difference that is allowed between the requested direction and the actual movement direction
	* @param useOnlyCorrectDirectionComponent (=true) If true, this only takes the component of the actual movement that is conform
	*				the requested direction, else it always uses the actual movement for speed calculation
	* @param useFilteredData (=false) if true, the recognizer will use the filtered tracking data instead of the raw one
	*
	* @return index of the recognizer needed to call it later
	*/
	FUBI_CPP_API unsigned int addLinearMovementRecognizer(SkeletonJoint::Joint joint, SkeletonJoint::Joint relJoint,
		float dirX, float dirY, float dirZ, float minVel, float maxVel,
		float minLength, float maxLength = Fubi::Math::MaxFloat,
		Fubi::BodyMeasurement::Measurement measuringUnit = Fubi::BodyMeasurement::NUM_MEASUREMENTS,
		bool useLocalPositions = false,
		int atIndex = -1,
		const char* name = 0,
		float maxAngleDifference = 45.0f,
		bool useOnlyCorrectDirectionComponent = true,
		bool useFilteredData = false);
	FUBI_CPP_API unsigned int addLinearMovementRecognizer(SkeletonJoint::Joint joint,
		float dirX, float dirY, float dirZ, float minVel, float maxVel,
		float minLength, float maxLength = Fubi::Math::MaxFloat,
		Fubi::BodyMeasurement::Measurement measuringUnit = Fubi::BodyMeasurement::NUM_MEASUREMENTS,
		bool useLocalPositions = false,
		int atIndex = -1,
		const char* name = 0,
		float maxAngleDifference = 45.0f,
		bool useOnlyCorrectDirectionComponent = true,
		bool useFilteredData = false);

	/**
	* \brief Creates a user defined angular movement recognizer
	*
	* @param joint the joint of interest
	* @param minVelX, minVelY, minVelZ the minimum angular velocity per axis (also defines the rotation direction)
	* @param maxVelX, maxVelY, maxVelZ the maximum angular velocity per axis (also defines the rotation direction)
	* @param useLocalOrients whether local ("substracted" parent orientation = the actual joint orientation, not the orientation in space)
	*		  or global orientations should be used
	* @param atIndex (= -1) if an index is given, the corresponding recognizer will be replaced instead of creating a new one
	* @param name name of the recognizer
	* @param minConfidence (=-1) if given this is the mimimum confidence required from tracking for the recognition to be succesful
	* @param useFilteredData (=false) if true, the recognizer will use the filtered tracking data instead of the raw one
	*
	* @return index of the recognizer needed to call it later
	*/
	FUBI_API unsigned int addAngularMovementRecognizer(Fubi::SkeletonJoint::Joint joint,
		float minVelX = -Fubi::Math::MaxFloat, float minVelY = -Fubi::Math::MaxFloat, float minVelZ = -Fubi::Math::MaxFloat,
		float maxVelX = Fubi::Math::MaxFloat, float maxVelY = Fubi::Math::MaxFloat, float maxVelZ =  Fubi::Math::MaxFloat,
		bool useLocalOrients = true,
		int atIndex = -1, const char* name = 0,
		float minConfidence = -1.0f, bool useFilteredData = false);

	/**
	* \brief Creates a user defined template recognizer
	*
	* @param joint the joint of interest
	* @param relJoint additional joint, the previous one is taken in relation to (NUM_JOINTS for global coordinates)
	* @param trainingDataFile file containing recorded skeleton data to be used as the template for the gesture
	* @param startFrame first frame index used in the trainingDataFile
	* @param endFrame last frame index used in the trainingDataFile
	* @param maxDistance the maximum distance allowed for gestures in comparison to the template
	* @param distanceMeasure (=Euclidean) masure used to calculate the distance to the template
	* @param maxRotation (=45°) how much the gesture is allowed to be rotated to match the template better (determines level of rotation invariance)
	* @param aspectInvariant (=false) if set to true scale normalization will NOT keep the original aspect ratio increasing scale invariance
	*		 in the sense that a square can no longer be distinguished from other rectangles
	* @param ignoreAxes (=NONE) can combine a flag for each axis that it should be ignored. Only active axes are taken for comparing the gestures
	* @param useOrientations (=false) if set to true orientation data instead of positional dat will be compared
	* @param useDTW (=true) if set to true, dynamic time warping (DTW) will be applied to find the optimal warping path to fit the gesture to the template
	* @param maxWarpingFactor (=0.5f) maximum warping for DTW (default: time cannot be stretched more than half of the whole gesture) 
	*        also applied in the search for best input length using GSS (default here: half more data than the training size will be extracted to catch slower gestures)
	* @param resamplingTechnique (=None) if set to other than None, gesture candidate paths will be resampled to the same length first
	* @param resampleSize (=-1) the size the gesture path will be resampled to if -1, the mean size of the training data will be used
	* @param searchBestInputLength (=false) if true, a golden section search is used to find input length to achieve the least distance with
	* @param stochasticModel (=GMR) by default, a gaussian mixture model will be created out of the training samples and regression will be applied to obtain general mean and covariance values,
	*        if set to HMM, hidden markov models will be used for recognition, if set to NONE, only mean an covariance matrix will be calculated
	* @param numGMRStates (=5) defines the number of states which will be used to create the GMM. the training data will be split into numGMRStates parts of equal length
	* @param measuringUnit (=NUM_MEASUREMENTS) measuring unit for positional data
	* @param atIndex (= -1) if an index is given, the corresponding recognizer will be replaced instead of creating a new one
	* @param name (=0) name of the recognizer
	* @param minConfidence (=-1) if given this is the mimimum confidence required from tracking for the recognition to be succesful
	* @param useLocalTransformations (=false) whether local or global transformations should be used
	* @param useFilteredData (=true) if true, the recognizer will use the filtered tracking data instead of the raw one
	*
	* @return index of the recognizer needed to call it later
	*/
	FUBI_API unsigned int addTemplateRecognizer(Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJoint::Joint relJoint,
		const char* trainingDataFile, int startFrame, int endFrame,
		float maxDistance,
		Fubi::DistanceMeasure::Measure distanceMeasure = Fubi::DistanceMeasure::Euclidean,
		float maxRotation = 45.0f,
		bool aspectInvariant = false,
		unsigned int ignoreAxes = Fubi::CoordinateAxis::NONE,
		bool useOrientations = false,
		bool useDTW = true,
		float maxWarpingFactor = 0.5f,
		Fubi::ResamplingTechnique::Technique resamplingTechnique = Fubi::ResamplingTechnique::None,
		int resampleSize = -1,
		bool searchBestInputLength = false,
		Fubi::StochasticModel::Model stochasticModel = Fubi::StochasticModel::GMR,
		unsigned int numGMRStates = 5,
		Fubi::BodyMeasurement::Measurement measuringUnit = Fubi::BodyMeasurement::NUM_MEASUREMENTS,
		int atIndex = -1, const char* name = 0,
		float minConfidence = -1.0f,
		bool useLocalTransformations = false,
		bool useFilteredData = true);

	/**
	* \brief Creates a user defined template recognizer
	*
	* @param xmlDefinition string containing the xml definition of the recognizer
	* @param atIndex (= -1) if an index is given, the corresponding recognizer will be replaced instead of creating a new one
	* @return index of the recognizer needed to call it later
	*/
	FUBI_API unsigned int addTemplateRecognizerFromXML(const char* xmlDefinition, int atIndex = -1);

	/**
	* \brief load a combination recognizer from a string that represents an xml node with the combination definition
	*
	* @param xmlDefinition string containing the xml definition
	* @return true if the combination was loaded succesfully
	*/
	FUBI_API bool addCombinationRecognizer(const char* xmlDefinition);

	/**
	* \brief Loads a recognizer config xml file and adds the configured recognizers
	*
	* @param fileName name of the xml config file
	* @return true if at least one recognizers was loaded from the given xml
	*/
	FUBI_API bool loadRecognizersFromXML(const char* fileName);

	/**
	* \brief Returns current number of user defined recognizers
	*
	* @return number of recognizers, the recognizers also have the indices 0 to numberOfRecs-1
	*/
	FUBI_API unsigned int getNumUserDefinedRecognizers();

	/**
	* \brief Returns the name of a user defined recognizer
	*
	* @param  recognizerIndex index of the recognizer
	* @return returns the recognizer name or an empty string if the user is not found or the name not set
	*/
	FUBI_API const char* getUserDefinedRecognizerName(unsigned int recognizerIndex);

	/**
	* \brief Returns the index of a user defined recognizer
	*
	* @param recognizerName name of the recognizer
	* @return returns the recognizer name or -1 if not found
	*/
	FUBI_API int getUserDefinedRecognizerIndex(const char* recognizerName);

	/**
	* \brief Returns the index of a user defined combination recognizer
	*
	* @param recognizerName name of the recognizer
	* @return returns the recognizer name or -1 if not found
	*/
	FUBI_API int getUserDefinedCombinationRecognizerIndex(const char* recognizerName);

	/**
	* \brief Returns current number of user defined combination recognizers
	*
	* @return number of recognizers, the recognizers also have the indices 0 to numberOfRecs-1
	*/
	FUBI_API unsigned int getNumUserDefinedCombinationRecognizers();

	/**
	* \brief Returns the name of a user defined combination recognizer
	*
	* @param  recognizerIndex index of the recognizer
	* @return returns the recognizer name or an empty string if the user is not found or the name not set
	*/
	FUBI_API const char* getUserDefinedCombinationRecognizerName(unsigned int recognizerIndex);

	/**
	* \brief Returns all current users with their tracking information
	*
	* @param pUserContainer (=0) pointer where a pointer to the current users will be stored at
	*        The maximal size is Fubi::MaxUsers, but the current size can be requested by leaving the Pointer at 0
	* @return the current number of users (= valid users in the container)
	*/
	FUBI_API unsigned short getCurrentUsers(FubiUser*** pUserContainer = 0);

	/**
	* \brief Returns one user with his tracking information
	*
	* @param id FUBI id of the user
	* @return a pointer to the user data
	*/
	FUBI_API FubiUser* getUser(unsigned int id);


	/**
	* \brief Returns the current depth resolution or -1, -1 if failed
	*
	* @param[out] width, height the resolution
	*/
	FUBI_API void getDepthResolution(int& width, int& height);

	/**
	* \brief Returns the current rgb resolution or -1, -1 if failed
	*
	* @param[out] width, height the resolution
	*/
	FUBI_API void getRgbResolution(int& width, int& height);

	/**
	* \brief Returns the current ir resolution or -1, -1 if failed
	*
	* @param[out] width, height the resolution
	*/
	FUBI_API void getIRResolution(int& width, int& height);

	/**
	* \brief Returns the number of shown fingers detected at the hand of one user (REQUIRES OPENCV!)
	*
	* @param userID FUBI id of the user
	* @param leftHand looks at the left instead of the right hand
	* @param getMedianOfLastFrames uses the precalculated median of finger counts of the last frames (still calculates new one if there is no precalculation)
	* @param medianWindowSize defines the number of last frames that is considered for calculating the median
	* @param useOldConvexityDefectMethod if true using old method that calculates the convexity defects
	* @return the number of shown fingers detected, 0 if there are none or there is an error
	*/
	FUBI_API int getFingerCount(unsigned int userID, bool leftHand = false, bool getMedianOfLastFrames = true, unsigned int medianWindowSize = 10, bool useOldConvexityDefectMethod = false);

	/**
	* \brief Extract the number of shown fingers out of the finger tracking data
	*
	* @param trackingData the finger tracking data struct
	* @return the number of shown fingers, 0 if there are none or there is an error
	*/
	FUBI_API int getHandFingerCount(const Fubi::TrackingData* trackingData);

	/**
	* \brief Enables/Disables finger tracking for the hands of one user
	*        If enabled the finger count will be tracked over time and the
	*		  median of these value will be returned in case of a query
	*		  (REQUIRES OPENCV!)
	*
	* @param userID FUBI id of the user
	* @param leftHand enable/disable finger tracking for the left hand
	* @param rightHand enable/disable finger tracking for the right hand
	* @param useConvexityDefectMethod use the old method for extracting the users hand out of the depth image (not recommended)
	*/
	FUBI_API void enableFingerTracking(unsigned int userID, bool leftHand, bool rightHand, bool useConvexityDefectMethod = false);


	/**
	* \brief  Whether the user is currently seen in the depth image
	*
	* @param userID FUBI id of the user
	*/
	FUBI_API bool isUserInScene(unsigned int userID);

	/**
	* \brief Whether the user is currently tracked
	*
	* @param userID FUBI id of the user
	*/
	FUBI_API bool isUserTracked(unsigned int userID);


	/**
	* \brief Get the most current tracking info of the user
	* (including all joint positions and orientations (local and global) and a timestamp)
	*
	* @param userId id of the user
	* @param filteredData if true the returned data will be data smoothed by the filter configured in the sensor
	* @return the user tracking info struct
	*/
	FUBI_API const Fubi::TrackingData* getCurrentTrackingData(unsigned int userId, bool filteredData = false);

	/**
	* \brief Get the last tracking info of the user (one frame before the current one)
	* (including all joint positions and orientations (local and global) and a timestamp)
	*
	* @param userId id of the user
	* @param filteredData if true the returned data will be data smoothed by the filter configured in the sensor
	* @return the user tracking info struct
	*/
	FUBI_API const Fubi::TrackingData* getLastTrackingData(unsigned int userId, bool filteredData = false);


	/**
	* \brief  Get the skeleton joint position out of the tracking info
	*
	* @param trackingInfo the trackinginfo struct to extract the info from
	* @param joint the considered joint id
	* @param[out] x, y, z where the position of the joint will be copied to
	* @param[out] confidence where the confidence for this position will be copied to
	* @param[out] timeStamp where the timestamp of this tracking info will be copied to (seconds since program start)
	* @param localPosition if set to true, the function will return local position (vector from parent joint)
	*/
	FUBI_API void getSkeletonJointPosition(const Fubi::TrackingData* trackingInfo, SkeletonJoint::Joint joint,
		float& x, float& y, float& z, float& confidence, double& timeStamp, bool localPosition = false);

	/**
	* \brief  Get the skeleton joint orientation out of the tracking info
	*
	* @param trackingInfo the trackinginfo struct to extract the info from
	* @param joint the considered joint id
	* @param[out] mat rotation 3x3 matrix (9 floats)
	* @param[out] confidence the confidence for this position
	* @param[out] timeStamp (seconds since program start)
	* @param localOrientation if set to true, the function will local orientations (cleared of parent orientation) instead of globals
	*/
	FUBI_API void getSkeletonJointOrientation(const Fubi::TrackingData* trackingInfo, SkeletonJoint::Joint joint,
		float* mat, float& confidence, double& timeStamp, bool localOrientation = true);

	/**
	* \brief  Get a users current value of a specific body measurement distance
	*
	* @param userId id of the user
	* @param measure the requested measurement
	* @param[out] dist the actual distance value
	* @param[out] confidence the confidence for this position
	*/
	FUBI_API void getBodyMeasurementDistance(unsigned int userId, Fubi::BodyMeasurement::Measurement measure, float& dist, float& confidence);

	/**
	* \brief  Creates an empty vector of UserTrackinginfo structs
	*
	*/
	FUBI_API std::vector<Fubi::UserTrackingData>* createTrackingDataVector();

	/**
	* \brief  Releases the formerly created vector
	*
	* @param vec the vector that will be released
	*/
	FUBI_API void releaseTrackingDataVector(std::vector<Fubi::UserTrackingData>* vec);

	/**
	* \brief  Returns the size of the vector
	*
	* @param vec the vector that we get the size of
	*/
	FUBI_API unsigned int getTrackingDataVectorSize(std::vector<Fubi::UserTrackingData>* vec);
	/**
	* \brief  Returns one element of the tracking info vector
	*
	* @param vec the vector that we get the element of
	* @param index element index with the tracking data vector
	*/
	FUBI_API Fubi::UserTrackingData* getTrackingData(std::vector<Fubi::UserTrackingData>* vec, unsigned int index);

	/**
	* \brief Returns the FUBI id of the user standing closest to the sensor (x-z plane)
	*/
	FUBI_API unsigned int getClosestUserID();

	/**
	* \brief Returns the ids of all users order by their distance to the sensor (x-z plane)
	* Closest user is at the front, user with largest distance or untracked users at the back
	*
	* @param maxNumUsers if greater than -1, the given number of closest users is additionally ordered from left to right position
	* @return a deque including the user ids
	*/
	FUBI_CPP_API std::deque<unsigned int> getClosestUserIDs(int maxNumUsers = -1);

	/**
	* \brief Returns the ids of all users order by their distance to the sensor (x-z plane)
	* Closest user is at the front, user with largest distance or untracked users at the back
	*
	* @param userIds an array big enough to receive the indicated number of user ids (Fubi::MaxUsers at max)
	* @param maxNumUsers if greater than -1, the given number of closest users is additionally ordered from left to right position
	* @return the actual number of user ids written into the array
	*/
	FUBI_API unsigned int getClosestUserIDs(unsigned int* userIds, int maxNumUsers = -1);

	/**
	* \brief Returns the user standing closest to the sensor (x-z plane)
	*/
	FUBI_API FubiUser* getClosestUser();

	/**
	* \brief Returns all users order by their distance to the sensor (x-z plane)
	* Closest user is at the front, user with largest distance or untracked users at the back
	* If maxNumUsers is given, the given number of closest users is additionally ordered from left to right position
	*/
	FUBI_CPP_API std::deque<FubiUser*> getClosestUsers(int maxNumUsers = -1);

	/**
	* \brief Stops and removes all user defined recognizers
	*/
	FUBI_API void clearUserDefinedRecognizers();

	/**
	* \brief Set the current tracking data of a user
	*
	* @param userId id of the user
	* @param skeleton as a float array with NUM_JOINTS * (position,orientation) with both position and orientation as 4 floats (x,y,z,conf) in milimeters/degrees
	* @param timeStamp in seconds or -1 for self calculation
	*/
	FUBI_API void updateTrackingData(unsigned int userId, float* skeleton, double timeStamp = -1);


	/**
	* \brief Convert coordinates between real world, depth, color, or IR image
	*
	* @param inputCoords the coordiantes to convert
	* @param inputType the type of the inputCoords
	* @param outputType the type to convert the inputCoords to
	* @return the converted coordinates
	*/
	FUBI_CPP_API Fubi::Vec3f convertCoordinates(const Fubi::Vec3f& inputCoords, Fubi::CoordinateType::Type inputType, Fubi::CoordinateType::Type outputType);

	/**
	* \brief Convert coordinates between real world, depth, color, or IR image
	*
	* @param inputCoordsX, inputCoordsY, inputCoordsZ the coordiantes to convert
	* @param[out] outputCoordsX, outputCoordsY, outputCoordsZ vars to store the converted coordinates to
	* @param inputType the type of the inputCoords
	* @param outputType the type to convert the inputCoords to
	*/
	FUBI_API void convertCoordinates(float inputCoordsX, float inputCoordsY, float inputCoordsZ, float& outputCoordsX, float& outputCoordsY, float& outputCoordsZ,
		Fubi::CoordinateType::Type inputType, Fubi::CoordinateType::Type outputType);


	/**
	* \brief resests the tracking of all users
	*/
	FUBI_API void resetTracking();

	/**
	* \brief get time since program start in seconds
	*/
	FUBI_API double getCurrentTime();

	/**
	* \brief set the filtering options for smoothing the skeleton according to the 1€ filter (still possible to get the unfiltered data)
	*
	* @param minCutOffFrequency (=1.0f) the minimum cutoff frequency for low pass filtering (=cut off frequency for a still joint)
	* @param velocityCutOffFrequency (=1.0f) the cutoff frequency for low pass filtering the velocity
	* @param cutOffSlope (=0.007f) how fast a higher velocity will higher the cut off frequency (->apply less smoothing with higher velocities)
	* @param bodyMeasureFilterFac (=0.1f) how fast a body measurement gets updated with new data (->higher value means faster update)
	*/
	FUBI_API void setFilterOptions(float minCutOffFrequency = 1.0f, float velocityCutOffFrequency = 1.0f, 
		float cutOffSlope = 0.007f, float bodyMeasureFilterFac = 0.1f);

	/**
	* \brief get the filtering options for smoothing the skeleton according to the 1€ filter (still possible to get the unfiltered data)
	*
	* @param[out] minCutOffFrequency the minimum cutoff frequency for low pass filtering (=cut off frequency for a still joint)
	* @param[out] velocityCutOffFrequency the cutoff frequency for low pass filtering the velocity
	* @param[out] cutOffSlope how fast a higher velocity will higher the cut off frequency (->apply less smoothing with higher velocities)
	* @param[out] bodyMeasureFilterFac how fast a body measurement gets updated with new data (->higher value means faster update)
	*/
	FUBI_API void getFilterOptions(float& minCutOffFrequency, float& velocityCutOffFrequency, float& cutOffSlope, float& bodyMeasureFilterFac);

	/**
	* \brief Checks a user defined combination recognizer for its current state
	*
	* @param recognizerName name of the combination
	* @param userID the user id of the user to be checked
	* @param[out] numStates the full number of states of this recognizer
	* @param[out] isInterrupted whether the recognizers of the current state are temporarly interrupted
	* @param[out] isInTransition if the state has already passed its min duration and would be ready to transit to the next state
	* @return number of current state (0..numStates-1), if < 0 -> error: -1 if first state not yet started, -2 user not found, -3 recognizer not found
	*/
	FUBI_API int getCurrentCombinationRecognitionState(const char* recognizerName, unsigned int userID, unsigned int& numStates, bool& isInterrupted, bool& isInTransition);

	/**
	* \brief Get meta information of a state of one recognizers
	*
	* @param recognizerName name of the combination
	* @param stateIndex the state index to get the meta info from
	* @param propertyName the name of the property to get
	* @return the value of the requested meta info property as a string, or 0x0 on error
	*/
	FUBI_API const char* getCombinationRecognitionStateMetaInfo(const char* recognizerName, unsigned int stateIndex, const char* propertyName);

	/**
	* \brief initalizes a finger sensor such as the leap motion for tracking fingers
	*
	* @param type the sensor type (see FingerSensorType definition)
	* @param offsetPosX, offsetPosY, offsetPosZ position of the finger sensor in relation to a second sensor (e.g. the Kinect) to align the coordinate systems
	* @return true if successful initialized
	*/
	FUBI_API bool initFingerSensor(Fubi::FingerSensorType::Type type, float offsetPosX = 0, float offsetPosY = -600.0f, float offsetPosZ = 200.0f);

	/**
	* \brief Get the currently available finger sensor types (defined in FubiConfig.h before compilation)
	*
	* @return an int composed of the currently available sensor types (see FingerSensorType enum for the meaning)
	*/
	FUBI_API int getAvailableFingerSensorTypes();

	/**
	* \brief Get the type of the currently active sensor
	*
	* @return the current sensor type
	*/
	FUBI_API Fubi::FingerSensorType::Type getCurrentFingerSensorType();

	/**
	* \brief Returns the number of currently tracked hands
	*
	* @return the current number of hands
	*/
	FUBI_API unsigned short getNumHands();

	/**
	* \brief Returns the hand id from its index
	*
	* @param index index of the hand in the hand array
	* @return hand id of that hand or 0 if not found
	*/
	FUBI_API unsigned int getHandID(unsigned int index);

	/**
	* \brief Returns the hand closest to the sensor (= finger sensor - offset pos)
	*
	* @return pointer to hand or 0 if none found
	*/
	FUBI_API FubiHand* getClosestHand();

	/**
	* \brief Returns the hand id of the closest hand
	*
	* @return hand id of that hand or 0 if none found
	*/
	FUBI_API unsigned int getClosestHandID();

	/**
	* \brief Returns the hand from its id
	*
	* @param id of the requested hand
	* @return pointer to hand or 0 if not found
	*/
	FUBI_API FubiHand* getHand(unsigned int id);

	/**
	* \brief Get the most current tracking data of a hand
	* (including all joint positions and orientations (local and global) and a timestamp)
	*
	* @param handId id of the hand
	* @param filteredData if true the returned data will be data smoothed by the filter configured in the sensor
	* @return the tracking data struct
	*/
	FUBI_API const Fubi::TrackingData* getCurrentFingerTrackingData(unsigned int handId, bool filteredData = false);

	/**
	* \brief Get the last tracking data of a hand (one frame before the current one)
	* (including all joint positions and orientations (local and global) and a timestamp)
	*
	* @param handId id of the hand
	* @param filteredData if true the returned data will be data smoothed by the filter configured in the sensor
	* @return the tracking data struct
	*/
	FUBI_API const Fubi::TrackingData* getLastFingerTrackingData(unsigned int handId, bool filteredData = false);

	/**
	* \brief  Get the skeleton joint position out of the tracking info
	*
	* @param trackingData the tracking data struct to extract the info from
	* @param joint the considered joint id
	* @param[out] x, y, z where the position of the joint will be copied to
	* @param[out] confidence where the confidence for this position will be copied to
	* @param[out] timeStamp where the timestamp of this tracking info will be copied to (seconds since program start)
	* @param localPosition if set to true, the function will return local position (vector from parent joint)
	*/
	FUBI_API void getSkeletonHandJointPosition(const Fubi::TrackingData* trackingData, SkeletonHandJoint::Joint joint,
		float& x, float& y, float& z, float& confidence, double& timeStamp, bool localPosition = false);

	/**
	* \brief  Get the skeleton joint orientation out of the tracking info
	*
	* @param trackingData the tracking data struct to extract the info from
	* @param joint the considered joint id
	* @param[out] mat rotation 3x3 matrix (9 floats)
	* @param[out] confidence the confidence for this position
	* @param[out] timeStamp (seconds since program start)
	* @param localOrientation if set to true, the function will local orientations (cleared of parent orientation) instead of globals
	*/
	FUBI_API void getSkeletonHandJointOrientation(const Fubi::TrackingData* trackingData, SkeletonHandJoint::Joint joint,
		float* mat, float& confidence, double& timeStamp, bool localOrientation = true);

	/**
	* \brief Checks a user defined gesture or posture recognizer for its success
	*
	* @param recognizerName name of the recognizer return during its creation
	* @param handId of the hand to be checked
	* @return RECOGNIZED in case of a succesful detection, TRACKING_ERROR if a needed joint is currently not tracked, NOT_RECOGNIZED else
	*/
	FUBI_API Fubi::RecognitionResult::Result recognizeGestureOnHand(const char* recognizerName, unsigned int handId);

	/**
	* \brief Checks a user defined combination recognizer for its progress
	*
	* @param recognizerName name of the combination
	* @param handId of the hand to be checked
	* @param[out] handStates (= 0x0) pointer to a vector of tracking data that represents the tracking information of the user
	*		  during the recognition of each state
	* @param restart (=true) if set to true, the recognizer automatically restarts, so the combination can be recognized again.
	* @param returnFilteredData if true, the user states vector will contain filtered data
	* @param[out] correctionHint on NOT_RECOGNIZED, this struct will contain information about why the recognition failed if wanted
	* @return RECOGNIZED in case of a succesful detection, TRACKING_ERROR if a needed joint is currently not tracked, NOT_RECOGNIZED else
	*/
	FUBI_API Fubi::RecognitionResult::Result getCombinationRecognitionProgressOnHand(const char* recognizerName, unsigned int handId,
		std::vector<Fubi::TrackingData>* handStates = 0x0, bool restart = true, bool returnFilteredData = false, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);

	/**
	* \brief Starts or stops the recognition process of a user defined combination for one hand
	*
	* @param combinationName name defined for this recognizer
	* @param handId of the hand for which the recognizers should be modified
	* @param enable if set to true, the recognizer will be started (if not already stared), else it stops
	*/
	FUBI_API void enableCombinationRecognitionHand(const char* combinationName, unsigned int handId, bool enable);

	/**
	* \brief Checks a user defined combination recognizer for its current state
	*
	* @param recognizerName name of the combination
	* @param handID of the hand to be checked
	* @param[out] numStates the full number of states of this recognizer
	* @param[out] isInterrupted whether the recognizers of the current state are temporarly interrupted
	* @param[out] isInTransition if the state has already passed its min duration and would be ready to transit to the next state
	* @return number of current state (0..numStates-1), if < 0 -> error: -1 if first state not yet started, -2 user not found, -3 recognizer not found
	*/
	FUBI_API int getCurrentCombinationRecognitionStateForHand(const char* recognizerName, unsigned int handID, unsigned int& numStates, bool& isInterrupted, bool& isInTransition);

	/**
	* \brief Get the offset position of the current finger sensor to the main sensor
	*
	* @param[out] xOffset, yOffset, zOffset a vector from the main sensor to the finger sensor, (0,0,0) if no sensor present
	*/
	FUBI_API void getFingerSensorOffsetPosition(float& xOffset, float& yOffset, float& zOffset);

	/**
	* \brief Set the offset position of the current finger sensor to the main sensor
	*
	* @param xOffset, yOffset, zOffset the vector from the main sensor to the finger sensor
	*/
	FUBI_API void setFingerSensorOffsetPosition(float xOffset, float yOffset, float zOffset);

	/**
	* \brief Get the target skeleton of a user defined combination recognizer
	*
	* @param recognizerName name of the combination
	* @return the target sensor as defined in FubiUtils.h
	*/
	FUBI_API Fubi::RecognizerTarget::Target getCombinationRecognizerTargetSkeleton(const char* recognizerName);

	/**
	* \brief Get the target skeleton for a recognizer
	*
	* @param recognizerName name of the recognizer
	* @return the target sensor as defined in FubiUtils.h
	*/
	FUBI_API Fubi::RecognizerTarget::Target getRecognizerTargetSkeleton(const char* recognizerName);

	/**
	* \brief Get the floor plane provided by some sensors in the Hesse normal form.
	*		 If not supported by the sensor, the normal will have length 0, else 1.
	* @param[out] normalX, normalY, normalZ the unit normal vector that points from the origin of the coordinate system to the plane
	* @param[out] dist the distance from the origin to the plane
	*/
	FUBI_API void getFloorPlane(float& normalX, float& normalY, float& normalZ, float& dist);

	/**
	* \brief Start recording tracking data to a file
	*
	* @param fileName name of the file to write to (note: if it already exists, all contents will be overwritten!)
	* @param targetID id of the hand or user to be recorded
	* @param isHand (=false) whether to record a hand or user
	* @param useRawData (=true) if true, raw tracking data will be recorded (default)
	* @param useFilteredData (=false) if true, filtered tracking data will be recorded
	*		 Note that if both raw and filtered tracking data are selected, the file name for the filtered data will be enhanced with "_filtered" to distinguish those
	*
	* @return true if opening the file was successful and all paramters are valid
	*/
	FUBI_API bool startRecordingSkeletonData(const char* fileName, unsigned int targetID, bool isHand = false, bool useRawData = true, bool useFilteredData = false);

	/**
	* \brief Check whether a recording process is currently running
	*
	* @param[out] currentFrameID last frame id that has already been feed into Fubi
	* @return whether a recording process is currently running
	*/
	FUBI_API bool isRecordingSkeletonData(int* currentFrameID = 0x0);

	/**
	* \brief Stop previously started recording of tracking data and close the corresponding file
	*/
	FUBI_API void stopRecordingSkeletonData();

	/**
	* \brief Load tracking data from a previously recorded file for later playback
	*
	* @param fileName name of the file to play back
	* @return number of loaded frames, -1 if something went wrong
	*/
	FUBI_API int loadRecordedSkeletonData(const char* fileName);

	/**
	* \brief Set the playback markers of a previously loaded recording
	*
	* @param currentFrame the current playback marker
	* @param startFrame the first frame to be played
	* @param endFrame (=-1) the last frame to be played (-1 for the end of file)
	*/
	FUBI_API void setPlaybackMarkers(int currentFrame, int startFrame = 0, int endFrame = -1);

	/**
	* \brief Get the current playback markers of a previously loaded recording
	*
	* @param[out] currentFrame the current playback marker
	* @param[out] startFrame the first frame to be played
	* @param[out] endFrame the last frame to be played
	*/
	FUBI_API void getPlaybackMarkers(int& currentFrame, int& startFrame, int& endFrame);

	/**
	* \brief Start play back of a previously loaded recording
	*
	* @param loop (=false) if true, playback restarts when the endframe is reached
	*/
	FUBI_API void startPlayingSkeletonData(bool loop = false);

	/**
	* \brief Check whether the play back of a recording is still running
	*
	* @param[out] currentFrameID last frame id that has already been feed into Fubi
	* @param[out] isPaused whether the playback is paused at the current frame
	* @return whether the play back of a recording is still running
	*/
	FUBI_API bool isPlayingSkeletonData(int* currentFrameID = 0x0, bool* isPaused = 0x0);

	/**
	* \brief Stop previously started playing of tracking data
	*/
	FUBI_API void stopPlayingRecordedSkeletonData();

	/**
	* \brief Pause previously started playing of tracking data (keeps the current user in the scene)
	*/
	FUBI_API void pausePlayingRecordedSkeletonData();

	/**
	* \brief Get the playback duration for the currently set start and end markers
	*
	* @return playback duration in seconds
	*/
	FUBI_API double getPlaybackDuration();

	/**
	* \brief Trim the previously loaded playback file to the currently set markers
	*
	* @return true if successful
	*/
	FUBI_API bool trimPlaybackFileToMarkers();


	typedef void(*RecognitionCallback)(const char* gestureName, unsigned int targetID, bool isHand, Fubi::RecognizerType::Type recognizerType);

	/**
	* \brief set a call back function which will be called for every gesture recognition that occurs
	*  You can set a null pointer if you don't want to get callbacks any more
	*  The callback function will be called inside of the updateSensor() call
	*  GetCombinationProgress() conflicts with the callbacks and won't work anymore
	*
	* @param startCallback function that should be called at the start of recognition events; paramters: const char* gestureName, usigned int targetID, bool isHand, RecognizerType::Type recognizerType
	* @param endCallback function that should be called at the end of recognition events; paramters: const char* gestureName, usigned int targetID, bool isHand, :RecognizerType::Type recognizerType
	*/
	FUBI_API void setRecognitionCallbacks(RecognitionCallback startCallback, RecognitionCallback endCallback);

	/**
	* \brief retrieve an image from one of the finger sensor streams with specific format
	*
	* @param[out] outputImage pointer to a unsigned char array
	*        Will be filled with wanted image
	*		  Array has to be of correct size, i.e. width*height*bytesPerPixel*numChannels
	* @param imageIndex index of the image stream, number of streams can be retrieved with getFingerSensorImageConfig
	* @param numChannels number channels in the image 1, 3 or 4
	* @param depth the pixel depth of the image, 8 bit (standard), 16 bit (mainly usefull for depth images) or 32 bit float
	*/
	FUBI_API bool getFingerSensorImage(unsigned char* outputImage, unsigned int imageIndex, Fubi::ImageNumChannels::Channel numChannels, Fubi::ImageDepth::Depth depth);

	/**
	* \brief get the image width, height and number of streams of the current finger sensor
	*        assumes that all streams have the same width and height
	*
	* @param[out] width image width
	* @param[out] height image width
	* @param[out] numStreams number of image streams provided by the current finger sensor
	*/
	FUBI_API void getFingerSensorImageConfig(int& width, int& height, unsigned int& numStreams);

	/**
	* \brief get the (normalized, resampled, ...) training data of a template recognizer
	*/
	FUBI_CPP_API const std::vector<Fubi::TemplateData>& getTemplateTrainingData(const char* recognizerName);

	/**
	* \brief get the (normalized, resampled, ...) training data of a template recognizer
	*/
	FUBI_CPP_API const std::vector<Fubi::TemplateData>& getTemplateTrainingData(unsigned int recognizerIndex);

	/**
	* \brief plot an image out of a vector of points (e.g. the training data of a template recognizer)
	*/
	FUBI_CPP_API void plotImage(const std::vector<Fubi::Vec3f>& dataToPlot, unsigned char* outputImage, unsigned int width, unsigned height, 
		Fubi::ImageNumChannels::Channel numChannels, Fubi::ImageDepth::Depth depth, int lineThickness = 1);
	/*! @}*/
}

#endif
