// ****************************************************************************************
//
// Fubi FubiCore
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
//
// ****************************************************************************************

#pragma once

// General includes
#include "Fubi.h"
#include "FubiISensor.h"
#include "FubiIFingerSensor.h"

// Recognizer interfaces
#include "GestureRecognizer/CombinationRecognizer.h"

#include <functional>

class FubiCore
{
public:
	// Singleton init only if not yet done
	static bool init(const char* xmlPath, Fubi::SkeletonTrackingProfile::Profile profile = Fubi::SkeletonTrackingProfile::ALL,
		float filterMinCutOffFrequency = 1.0f, float filterVelocityCutOffFrequency = 1.0f, 
		float filterCutOffSlope = 0.007f, float bodyMeasureFilterFac = 0.1f,
		bool mirrorStream = true, bool registerStreams =true)
	{
		bool success = true;
		if (s_instance == 0x0)
		{
			s_instance = new FubiCore();
			if (xmlPath == 0x0)
			{
				Fubi_logInfo("FubiCore: Initialized in non-tracking mode!\n");
			}
			else
			{
				// init with xml file
				success = s_instance->initFromXml(xmlPath, profile, mirrorStream, registerStreams);
			}

			if (!success)
			{
				Fubi_logErr("Failed to inialize the sensor via XML!\n");
				delete s_instance;
				s_instance = 0x0;
			}
			else
			{
				Fubi_logInfo("FubiCore: Succesfully initialized the sensor via XML.\n");
				// Set filter options if succesful
				s_instance->setFilterOptions(filterMinCutOffFrequency, filterVelocityCutOffFrequency, 
					filterCutOffSlope, bodyMeasureFilterFac);
			}
		}
		else
		{
			Fubi_logWrn("Fubi already initalized. New init will be ignored!\n");
		}
		return success;
	}
	static bool init(const Fubi::SensorOptions& sensorOptions, const Fubi::FilterOptions& filterOptions)
	{
		bool success = true;
		if (s_instance == 0x0)
		{
			s_instance = new FubiCore();

			// init with options
			success = s_instance->initSensorWithOptions(sensorOptions);

			if (!success)
			{
				Fubi_logErr("Failed to inialize the sensor with the given options!\n");
				delete s_instance;
				s_instance = 0x0;
			}
			else
			{
				Fubi_logInfo("FubiCore: Succesfully initialized the sensor with the given options.\n");
				// Set filter options if succesful
				s_instance->setFilterOptions(filterOptions);
			}
		}
		else
		{
			Fubi_logWrn("Fubi already initalized. New init will be ignored!\n");
		}
		return success;
	}

	// Singleton getter (maybe null if not initialized!)
	static FubiCore* getInstance()
	{
		return s_instance;
	}

	// Release the singleton
	static void release()
	{
		delete s_instance;
		s_instance = 0x0;
	}

	// init an additional finger sensor
	bool initFingerSensor(Fubi::FingerSensorType::Type type, const Fubi::Vec3f& offsetPos);

	void updateSensor();

	// Get the floor plane
	Fubi::Plane getFloor();

	// Get current users as an array
	unsigned short getCurrentUsers(FubiUser*** userContainer);

	void getDepthResolution(int& width, int& height);
	void getRgbResolution(int& width, int& height);
	void getIRResolution(int& width, int& height);

	// Add user defined gestures/postures
	unsigned int addRecognizer(IGestureRecognizer* rec, const std::string& name, int atIndex = -1);
	unsigned int addHiddenRecognizer(IGestureRecognizer* rec, const std::string& name, int atIndex = -1);
	unsigned int addJointRelationRecognizer(Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJoint::Joint relJoint,
		const Fubi::Vec3f& minValues = Fubi::DefaultMinVec,
		const Fubi::Vec3f& maxValues = Fubi::DefaultMaxVec,
		float minDistance = 0,
		float maxDistance = Fubi::Math::MaxFloat,
		Fubi::SkeletonJoint::Joint midJoint = Fubi::SkeletonJoint::NUM_JOINTS,
		const Fubi::Vec3f& midJointMinValues = Fubi::DefaultMinVec,
		const Fubi::Vec3f& midJointMaxValues = Fubi::DefaultMaxVec,
		float midJointMinDistance = 0,
		float midJointMaxDistance = Fubi::Math::MaxFloat,
		bool useLocalPositions = false,
		int atIndex = -1,
		const char* name = 0,
		float minConfidence =-1,
		Fubi::BodyMeasurement::Measurement measuringUnit = Fubi::BodyMeasurement::NUM_MEASUREMENTS,
		bool useFilteredData = false);
	unsigned int addJointOrientationRecognizer(Fubi::SkeletonJoint::Joint joint,
		const Fubi::Vec3f& minValues = Fubi::Vec3f(-180.0f, -180.0f, -180.0f), const Fubi::Vec3f& maxValues = Fubi::Vec3f(180.0f, 180.0f, 180.0f),
		bool useLocalOrientations = true,
		int atIndex = -1,
		const char* name = 0,
		float minConfidence =-1,
		bool useFilteredData = false);
	unsigned int addJointOrientationRecognizer(Fubi::SkeletonJoint::Joint joint,
		const Fubi::Vec3f& orientation, float maxAngleDifference,
		bool useLocalOrientations = true,
		int atIndex = -1,
		const char* name = 0,
		float minConfidence =-1,
		bool useFilteredData = false);
	unsigned int addLinearMovementRecognizer(Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJoint::Joint relJoint,
		const Fubi::Vec3f& direction, float minVel, float maxVel = Fubi::Math::MaxFloat,
		bool useLocalPositions = false,
		int atIndex = -1,
		const char* name = 0,
		float minConfidence =-1,
		float maxAngleDiff = 45.0f,
		bool useOnlyCorrectDirectionComponent = true,
		bool useFilteredData = false);
	unsigned int addLinearMovementRecognizer(Fubi::SkeletonJoint::Joint joint,	const Fubi::Vec3f& direction, float minVel, float maxVel = Fubi::Math::MaxFloat,
		bool useLocalPositions = false,
		int atIndex = -1,
		const char* name = 0,
		float minConfidence =-1,
		float maxAngleDiff = 45.0f,
		bool useOnlyCorrectDirectionComponent = true,
		bool useFilteredData = false);
	unsigned int addLinearMovementRecognizer(Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJoint::Joint relJoint,
		const Fubi::Vec3f& direction, float minVel, float maxVel,
		float minLength, float maxLength = Fubi::Math::MaxFloat,
		Fubi::BodyMeasurement::Measurement measuringUnit = Fubi::BodyMeasurement::NUM_MEASUREMENTS,
		bool useLocalPositions = false,
		int atIndex = -1,
		const char* name = 0,
		float minConfidence =-1,
		float maxAngleDiff = 45.0f,
		bool useOnlyCorrectDirectionComponent = true,
		bool useFilteredData = false);
	unsigned int addLinearMovementRecognizer(Fubi::SkeletonJoint::Joint joint,	const Fubi::Vec3f& direction, float minVel, float maxVel,
		float minLength, float maxLength = Fubi::Math::MaxFloat,
		Fubi::BodyMeasurement::Measurement measuringUnit = Fubi::BodyMeasurement::NUM_MEASUREMENTS,
		bool useLocalPositions = false,
		int atIndex = -1,
		const char* name = 0,
		float minConfidence =-1,
		float maxAngleDiff = 45.0f,
		bool useOnlyCorrectDirectionComponent = true,
		bool useFilteredData = false);
	unsigned int addAngularMovementRecognizer(Fubi::SkeletonJoint::Joint joint, const Fubi::Vec3f& minAngularVelocity = Fubi::DefaultMinVec,
		const Fubi::Vec3f& maxAngularVelocity = Fubi::DefaultMaxVec,
		bool useLocalOrients = true,
		int atIndex = -1,
		const char* name = 0,
		float minConfidence = -1.0f,
		bool useFilteredData = false);
	unsigned int addFingerCountRecognizer(Fubi::SkeletonJoint::Joint handJoint,
		unsigned int minFingers, unsigned int maxFingers,
		int atIndex = -1,
		const char* name = 0,
		float minConfidence =-1,
		bool useMedianCalculation = false,
		unsigned int medianWindowSize = 10,
		bool useFilteredData = false);
	unsigned int addTemplateRecognizer(Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJoint::Joint relJoint,
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
		bool useFilteredData = false);
	unsigned int addTemplateRecognizer(const std::string& xmlDefinition, int atIndex = -1);

	// load a combination recognizer from a string that represents an xml node with the combination definition
	bool addCombinationRecognizer(const std::string& xmlDefinition);
	void addCombinationRecognizer(CombinationRecognizer* rec);

	// Stop and remove all user defined recognizers
	void clearUserDefinedRecognizers();

	// Check current progress in gesture/posture recognition
	Fubi::RecognitionResult::Result recognizeGestureOn(Fubi::Postures::Posture postureID, unsigned int userID);
	Fubi::RecognitionResult::Result recognizeGestureOn(unsigned int recognizerIndex, unsigned int userID, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);
	Fubi::RecognitionResult::Result recognizeGestureOn(const std::string& recognizerName, unsigned int userID, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);
	Fubi::RecognitionResult::Result recognizeGestureOnHand(const std::string& recognizerName, unsigned int handID);


	// Enable a posture combination recognition manually
	void enableCombinationRecognition(Fubi::Combinations::Combination combinationID, unsigned int userID, bool enable);
	// Enable a user defined posture combination recognition manually
	void enableCombinationRecognition(const std::string& combinationName, unsigned int userID, bool enable);
	void enableCombinationRecognitionHand(const std::string& combinationName, unsigned int handID, bool enable);
	// Or auto activate all for each new user
	void setAutoStartCombinationRecognition(bool enable, Fubi::Combinations::Combination combinationID = Fubi::Combinations::NUM_COMBINATIONS);
	bool getAutoStartCombinationRecognition(Fubi::Combinations::Combination combinationID = Fubi::Combinations::NUM_COMBINATIONS);

	// Enable finger tracking in depth image
	void enableFingerTracking(unsigned int userID, bool leftHand, bool rightHand, bool useConvexityDefectMethod = false);
	//  Returns the number of shown fingers detected at the hand of one user (REQUIRES OPENCV!)
	int getFingerCount(unsigned int userID, bool leftHand = false,
		bool getMedianOfLastFrames = true, unsigned int medianWindowSize = 10,
		bool useOldConvexityDefectMethod = false);

	// Get number of user defined recognizers
	unsigned int getNumUserDefinedRecognizers() { return (unsigned int)m_userDefinedRecognizers.size(); }
	// Get given name of a recognizer or an empty string in case of failure
	const std::string& getUserDefinedRecognizerName(unsigned int index) { return (index < m_userDefinedRecognizers.size()) ? m_userDefinedRecognizers[index].first : s_emtpyString; }
	// Get index of a recognizer with the given name or -1 in case of failure
	int getUserDefinedRecognizerIndex(const std::string& name);
	// Get index of a recognizer with the given name or -1 in case of failure
	int getHiddenUserDefinedRecognizerIndex(const std::string& name);

	// Find and clone a recognizer with the given name (user defined, hidden, name represents index, or posture)
	IGestureRecognizer* cloneRecognizer(const std::string& name);

	// Get number of user defined recognizers
	unsigned int getNumUserDefinedCombinationRecognizers() { return (unsigned int)m_userDefinedCombinationRecognizers.size(); }
	// Get given name of a recognizer or an empty string in case of failure
	const std::string& getUserDefinedCombinationRecognizerName(unsigned int index) { return (index < (unsigned int)m_userDefinedCombinationRecognizers.size()) ? m_userDefinedCombinationRecognizers[index].first : s_emtpyString; }
	// Get index of a recognizer with the given name or -1 in case of failure
	int getUserDefinedCombinationRecognizerIndex(const std::string& name);
	CombinationRecognizer* getUserDefinedCombinationRecognizer(const std::string& name);
	CombinationRecognizer* getUserDefinedCombinationRecognizer(unsigned int index);
	// Get meta info of a combination recognizer state
	const char* getCombinationRecognitionStateMetaInfo(const char* recognizerName, unsigned int stateIndex, const char* propertyName);
	// Current state and progress info of a combination recognizer
	int getCurrentCombinationRecognitionState(const char* recognizerName, unsigned int userID, unsigned int& numStates, bool& isInterrupted, bool& isInTransition);
	// Checks a combination recognizer for its progress
	Fubi::RecognitionResult::Result getCombinationRecognitionProgressOn(Fubi::Combinations::Combination combinationID, unsigned int userID,
		std::vector<Fubi::TrackingData>* userStates = 0x0, bool restart = true, bool returnFilteredData = false, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);
	// Same for user defined combinations
	Fubi::RecognitionResult::Result getCombinationRecognitionProgressOn(const char* recognizerName, unsigned int userID,
		std::vector<Fubi::TrackingData>* userStates = 0x0, bool restart = true, bool returnFilteredData = false, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);

	/**
	 * \brief Get the target skeleton of a user defined combination recognizer
	 *
	 * @param recognizerName name of the combination
	 * @return the target sensor as defined in FubiUtils.h
	 */
	Fubi::RecognizerTarget::Target getCombinationRecognizerTargetSkeleton(const char* recognizerName);

	/**
	 * \brief Get the target skeleton for a recognizer
	 *
	 * @param recognizerName name of the recognizer
	 * @return the target sensor as defined in FubiUtils.h
	 */
	Fubi::RecognizerTarget::Target getRecognizerTargetSkeleton(const char* recognizerName);

	// Get the id (starting with 1) of a user by its index (starting with 0). Returns 0 if not found
	unsigned int getUserID(unsigned int index)
	{
		if (index < m_numUsers)
			return m_users[index]->id();
		return 0;
	}
	unsigned int getNumUsers() { return m_numUsers; }

	// Get user by id
	FubiUser* getUser(unsigned int userId);

	// Get the user standing closest to the sensor
	FubiUser* getClosestUser();
	unsigned int getClosestUserID();
	std::deque<unsigned int> getClosestUserIDs(int maxNumUsers = -1);
	std::deque<FubiUser*> getClosestUsers(int maxNumUsers = -1);

	// Same for hands
	unsigned int getHandID(unsigned int index)
	{
		if (index < m_numHands)
			return m_hands[index]->m_id;
		return 0;
	}
	unsigned int getNumHands() { return m_numHands; }
	FubiHand* getHand(unsigned int handID);
	FubiHand* getClosestHand(Fubi::Vec3f pos = Fubi::NullVec, Fubi::SkeletonJoint::Joint handType = Fubi::SkeletonJoint::NUM_JOINTS, bool useFilteredData = true);
	unsigned int getClosestHandID()
	{
		FubiHand* hand = getClosestHand();
		return (hand) ? hand->id() : 0;
	}
	FubiIFingerSensor* getFingerSensor() { return m_fingerSensor; }


	/**
	* \brief Set the current tracking data of a user
	*
	* @param userId id of the user
	* @param skeleton as a float array with NUM_JOINTS * (position,orientation) with both position and orientation as 4 floats (x,y,z,conf) in milimeters/degrees
	* @param timeStamp in seconds or -1 for self calculation
	*/
	void updateTrackingData(unsigned int userId, float* skeleton, double timeStamp = -1);

	/**
	* \brief Convert coordinates between real world, depth, color, or IR image
	*
	* @param inputCoords the coordiantes to convert
	* @param inputType the type of the inputCoords
	* @param outputType the type to convert the inputCoords to
	* @return the converted coordinates
	*/
	Fubi::Vec3f convertCoordinates(const Fubi::Vec3f& inputCoords, Fubi::CoordinateType::Type inputType, Fubi::CoordinateType::Type outputType);

	// Reset the tracking of all users in the current sensor
	void resetTracking();

	FubiISensor* getSensor() { return m_sensor; }

	// initialize sensor with an options file
	bool initSensorWithOptions(const Fubi::SensorOptions& options);

	// getter/setter for filter options
	void setFilterOptions(const Fubi::FilterOptions& options)
	{
		m_filterOptions = options;
	}
	const Fubi::FilterOptions& getFilterOptions()
	{
		return m_filterOptions;
	}
	void setFilterOptions(float minCutOffFrequency = 1.0f, float velocityCutOffFrequency = 1.0f, float cutOffSlope = 0.007f, float bodyMeasureFilterFac = 0.1f)
	{
		m_filterOptions.m_minCutOffFrequency = minCutOffFrequency;
		m_filterOptions.m_velocityCutOffFrequency = velocityCutOffFrequency;
		m_filterOptions.m_cutOffSlope = cutOffSlope;
		m_filterOptions.m_bodyMeasureFilterFac = bodyMeasureFilterFac;
	}
	void getFilterOptions(float& filterMinCutOffFrequency, float& filterVelocityCutOffFrequency, float& filterCutOffSlope, float& bodyMeasureFilterFac)
	{
		filterMinCutOffFrequency = m_filterOptions.m_minCutOffFrequency;
		filterVelocityCutOffFrequency = m_filterOptions.m_velocityCutOffFrequency;
		filterCutOffSlope = m_filterOptions.m_cutOffSlope;
		bodyMeasureFilterFac = m_filterOptions.m_bodyMeasureFilterFac;
	}

	// get/set the offset position of the finger sensor in relation to the main sensor
	const Fubi::Vec3f& getFingerSensorOffsetPosition() const { return m_fingerSensor ? m_fingerSensor->getOffsetPosition() : Fubi::NullVec; }
	void setFingerSensorOffsetPosition(const Fubi::Vec3f& offsetPos) { if (m_fingerSensor) m_fingerSensor->setOffsetPosition(offsetPos); }

	/**
	* \brief Start recording tracking data to a file
	*
	* @param fileName name of the file to write to (note: if it already exists, all contents will be overwritten!)
	* @param targetID id of the hand or user to be recorded
	* @param isHand (=false) whether to record a hand or user
	* @param useRawData (=true) if true, raw tracking data will be recorded (default)
	* @param useFilteredData (=false) if true, filtered tracking data will be recorded
	*		 Note that if both raw and filtered tracking data are selected, the file name will be enhanced with "_raw" and "_filtered" to distinguish those
	*
	* @return true if opening the file was successful and all paramters are valid
	*/
	bool startRecordingSkeletonData(const char* fileName, unsigned int targetID, bool isHand, bool useRawData, bool useFilteredData);

	/**
	* \brief Check whether a recording process is currently running
	*
	* @param[out] currentFrameID last frame id that has already been feed into Fubi
	* @return whether a recording process is currently running
	*/
	bool isRecordingSkeletonData(int* currentFrameID = 0x0);

	/**
	* \brief Stop previously started recording of tracking data and close the corresponding file
	*/
	void stopRecordingSkeletonData();

	/**
	* \brief Load tracking data from a previously recorded file for later playback
	*
	* @param fileName name of the file to play back
	*
	* @return number of loaded frames, -1 if something went wrong
	*/
	int loadRecordedSkeletonData(const char* fileName);

	/**
	* \brief Set the playback markers of a previously loaded recording
	*
	* @param currentFrame the current playback marker
	* @param startFrame the first frame to be played
	* @param endFrame (=-1) the last frame to be played (-1 for the end of file)
	*/
	void setPlaybackMarkers(int currentFrame, int startFrame = 0, int endFrame = -1);

	/**
	* \brief Get the current playback markers of a previously loaded recording
	*
	* @param[out] currentFrame the current playback marker
	* @param[out] startFrame the first frame to be played
	* @param[out] endFrame the last frame to be played
	*/
	void getPlaybackMarkers(int& currentFrame, int& startFrame, int& endFrame);

	/**
	* \brief Start play back of a previously loaded recording
	*
	* @param loop (=false) if true, playback restarts when the endframe is reached
	*/
	void startPlayingSkeletonData(bool loop = false);

	/**
	* \brief Check whether the play back of a recording is still running
	*
	* @param[out] currentFrameID last frame id that has already been feed into Fubi
	* @param[out] isPaused whether the playback is paused at the current frame
	* @return whether the play back of a recording is still running
	*/
	bool isPlayingSkeletonData(int* currentFrameID = 0x0, bool* isPaused = 0x0);

	/**
	* \brief Stop previously started playing of tracking data
	*/
	void stopPlayingRecordedSkeletonData();

	/**
	* \brief Pause previously started playing of tracking data (keeps the current user in the scene)
	*/
	void pausePlayingRecordedSkeletonData();

	/**
	* \brief Get the playback duration for the currently set start and end markers
	*
	* @return playback duration in seconds
	*/
	double getPlaybackDuration();

	/**
	* \brief Trim the previously loaded playback file to the currently set markers
	*
	* @return true if successful
	*/
	bool trimPlaybackFileToMarkers();

	/**
	* \brief set a call back function which will be called for every gesture recognition that occurs
	*  You can set a null pointer if you don't want to get callbacks any more
	*  The callback function will be called inside of the updateSensor() call
	*  GetCombinationProgress() conflicts with the callbacks and won't work anymore
	*
	* @param startCallback function that should be called at the start of recognition events; paramters: const char* gestureName, usigned int targetID, bool isHand
	* @param endCallback function that should be called at the end of recognition events; paramters: const char* gestureName, usigned int targetID, bool isHand
	*/
	void setRecognitionCallbacks(const std::function<void(const char*, unsigned int, bool, Fubi::RecognizerType::Type)>& startCallback, const std::function<void(const char*, unsigned int, bool, Fubi::RecognizerType::Type)>& endCallback);

	/**
	* \brief Update to new minimum tracking history length for all users/hands
	*/
	void updateTrackingHistoryLength(unsigned int requiredTrackingHistoryLength);

	/**
	* \brief get the (normalized, resampled, ...) training data of a template recognizer
	*/
	const std::vector<Fubi::TemplateData>& getTemplateTrainingData(const char* recognizerName);

	/**
	* \brief get the (normalized, resampled, ...) training data of a template recognizer
	*/
	const std::vector<Fubi::TemplateData>& getTemplateTrainingData(unsigned int recognizerIndex);

private:
	// private constructor/destructor as it is a singeleton
	FubiCore();
	~FubiCore();

	// initialize with an xml file
	bool initFromXml(const char* xmlPath, Fubi::SkeletonTrackingProfile::Profile profile = Fubi::SkeletonTrackingProfile::ALL,
		bool mirrorStream = true, bool registerStreams =true);

	// Update FubiUser -> OpenNI/KinectSDK ID mapping and tracking data
	// @return whether something has changed
	bool updateUsers();

	// Update hand ids and tracking data
	// @return whether something has changed (hand new, lost or started tracking)
	bool updateHands();

	// Try to assign tracked hands to users (user new, lost or started tracking)
	void updateHandToUserAssignment();

	// get closest user hand joint to the given point
	bool getClosestUserHandJoint(Fubi::Vec3f pos, FubiUser** user, Fubi::SkeletonJoint::Joint& joint, bool useFilteredData = true);

	// Function called each frame for all tracked users/hands if a recognition callback is registered
	void checkRecognizers(unsigned int targetIndex, bool isHand);

	// Function called when a user/hand is lost to report recognition ends if applicable
	void clearRecognitions(unsigned int targetIndex, bool isHand);

	// Get the actual recognizer
	IGestureRecognizer* cloneUserDefinedRecognizer(unsigned int index) { return (index < m_userDefinedRecognizers.size()) ? m_userDefinedRecognizers[index].second->clone() : 0x0; }
	// Get the actual hidden recognizer
	IGestureRecognizer* cloneHiddenUserDefinedRecognizer(unsigned int index) { return (index < m_hiddenUserDefinedRecognizers.size()) ? m_hiddenUserDefinedRecognizers[index].second->clone() : 0x0; }

	// The singleton instance of the tracker
	static FubiCore* s_instance;

	const static std::string s_emtpyString;

	// Number of current users
	unsigned short m_numUsers;
	// All users
	FubiUser* m_users[Fubi::MaxUsers];
	// Mapping of user ids to users
	std::map<unsigned int, FubiUser*> m_userIDToUsers;

	// Number of current hands
	unsigned short m_numHands;
	// All Hands
	FubiHand* m_hands[Fubi::MaxHands];
	// Mapping of user ids to users
	std::map<unsigned int, FubiHand*> m_handIDToHands;

	// One posture recognizer per posture
	IGestureRecognizer* m_postureRecognizers[Fubi::Postures::NUM_POSTURES];

	// User defined recognizers (joint relations and linear gestures) stored with name
	std::vector<std::pair<std::string, IGestureRecognizer*> > m_userDefinedRecognizers;
	// Hidden user defined recognizers (joint relations and linear gestures) stored with name,
	// can only be used in Combinations, but not directly
	std::vector<std::pair<std::string, IGestureRecognizer*> > m_hiddenUserDefinedRecognizers;
	// User defined Combination recognizers (templates to apply for each user)
	std::vector<std::pair<std::string, CombinationRecognizer*> > m_userDefinedCombinationRecognizers;

	// The Combination recognizers that should start automatically when a new user is detected
	bool m_autoStartCombinationRecognizers[Fubi::Combinations::NUM_COMBINATIONS+1];

	// The sensor for getting stream and tracking data
	FubiISensor* m_sensor;

	// Additional sensor for getting finger tracking data
	FubiIFingerSensor* m_fingerSensor;

	// The filter options
	Fubi::FilterOptions m_filterOptions;

	// For recording skeleton data
	class FubiRecorder* m_recorder;

	// And for playing it again
	class FubiPlayer* m_player;

	// Registered callbacks for gesture recognitions
	std::function<void(const char*, unsigned int, bool, Fubi::RecognizerType::Type)> m_recognitionStartCallback, m_recognitionEndCallback;

	// Stored recognitions for the callback
	bool m_currentPostureRecognitions[Fubi::MaxUsers + 1][Fubi::Postures::NUM_POSTURES];
	std::vector<bool> m_currentGestureRecognitions[Fubi::MaxUsers+1];
	std::vector<bool> m_currentHandGestureRecognitions[Fubi::MaxHands + 1];
	bool m_currentPredefinedCombinationRecognitions[Fubi::MaxUsers + 1][Fubi::Postures::NUM_POSTURES];
	std::vector<bool> m_currentUserCombinationRecognitions[Fubi::MaxUsers + 1];
	std::vector<bool> m_currentHandCombinationRecognitions[Fubi::MaxHands + 1];
};
