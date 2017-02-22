// ****************************************************************************************
//
// Fubi User
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler 
// 
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
// 
// ****************************************************************************************
#pragma once

/** \file FubiUser.h 
 * \brief a header file containing the FubiUser class definition
*/ 

#include "FubiPredefinedGestures.h"
#include "FubiMath.h"

#include <map>

class CombinationRecognizer;

/** \addtogroup FUBIUSER FUBI User class
* Contains the FubiUser class that holds informations for tracked users
* 
* @{
*/

/**
* \brief The FubiUser class hold all relevant informations for each tracked user
*/
class FubiUser
{
	// This may not be the best solution to make the private vars only writable by certain classes, but it works
	friend class FubiCore;
	friend class FubiPlayer;
	// Those two access the inScene attribute
	friend class FubiOpenNISensor;
	friend class FubiOpenNI2Sensor;
public:
	/**
	* \brief Operator used for comparing users according to their distance to the sensor
	* in the x-z plane --> get the closest users
	*/
	static bool closerToSensor(const FubiUser* u1, const FubiUser* u2)
	{
		const Fubi::SkeletonJointPosition& pos1 = u1->currentTrackingData()->jointPositions[Fubi::SkeletonJoint::TORSO];
		const Fubi::SkeletonJointPosition& pos2 = u2->currentTrackingData()->jointPositions[Fubi::SkeletonJoint::TORSO];

		if (u1->m_isTracked && pos1.m_confidence > 0.1f)
		{
			if (u2->m_isTracked  && pos2.m_confidence > 0.1f)
			{
				// Compare their distance (int the x,z-plane) to the sensor
				float dist1 = sqrtf(pos1.m_position.z*pos1.m_position.z + pos1.m_position.x*pos1.m_position.x);
				float dist2 = sqrtf(pos2.m_position.z*pos2.m_position.z + pos2.m_position.x*pos2.m_position.x);
				return dist1 < dist2;
			}
			else
			{
				// u1 is "closer" to the sensor (only valid user)
				return true;
			}
		}
		else if (u2->m_isTracked  && pos2.m_confidence > 0.1f)
		{
			return false; // u2 is "closer" to the sensor (only valid user)
		}

		// No valid user -> comparison has no meaning
		// but we compare the id to retain a strict weak ordering
		return u1->m_id < u2->m_id;
	}

	/**
	* \brief Operator used for comparing which user is more left of the sensor
	*/
	static bool moreLeft(const FubiUser* u1, const FubiUser* u2)
	{
		const Fubi::SkeletonJointPosition& pos1 = u1->currentTrackingData()->jointPositions[Fubi::SkeletonJoint::TORSO];
		const Fubi::SkeletonJointPosition& pos2 = u2->currentTrackingData()->jointPositions[Fubi::SkeletonJoint::TORSO];

		if (u1->m_isTracked && pos1.m_confidence > 0.1f)
		{
			if (u2->m_isTracked  && pos2.m_confidence > 0.1f)
			{
				// Compare their x value
				return pos1.m_position.x < pos2.m_position.x;
			}
			else
			{
				// u1 is "more left" to the sensor (only valid user)
				return true;
			}
		}
		else if (u2->m_isTracked  && pos2.m_confidence > 0.1f)
		{
			return false; // u2 is "more left" to the sensor (only valid user)
		}

		// No valid user -> comparison has no meaning
		// but we compare the id to retain a strict weak ordering
		return u1->m_id < u2->m_id;
	}

	/**
	* \brief Whether the user is currently seen in the depth image
	*/
	bool inScene() const { return m_inScene; }


	/**
	* \brief Id of this user
	*/
	unsigned int id() const { return m_id; }

	/**
	* \brief Whether the user is currently tracked
	*/
	bool isTracked() const { return m_isTracked; }

	/**
	* \brief Complete history of tracking data including joint positions and orientations (both local and global ones)
	*/
	const std::deque<Fubi::TrackingData*>* trackingData() const { return &m_trackingData; }

	/**
	* \brief Complete history of filtered tracking data including joint positions and orientations (both local and global ones)
	*/
	const std::deque<Fubi::TrackingData*>* filteredTrackingData() const { return &m_filteredTrackingData; }

	/**
	* \brief Most current tracking data including joint positions and orientations (both local and global ones)
	*/
	const Fubi::TrackingData* currentTrackingData() const { return (m_trackingData.size() > 0) ? m_trackingData.back() : &Fubi::EmptyUserTrackingData; }
	
	/**
	* \brief Last frame's tracking data including joint positions and orientations (both local and global ones)
	*/
	const Fubi::TrackingData* lastTrackingData() const { return (m_trackingData.size() > 1) ? *(m_trackingData.end() - 2) : &Fubi::EmptyUserTrackingData; }

	/**
	* \brief Most current filtered tracking data including joint positions and orientations (both local and global ones)
	*/
	const Fubi::TrackingData* currentFilteredTrackingData() const { return (m_filteredTrackingData.size() > 0) ? m_filteredTrackingData.back() : &Fubi::EmptyUserTrackingData; }

	/**
	* \brief Last frame's filtered tracking data including joint positions and orientations (both local and global ones)
	*/
	const Fubi::TrackingData* lastFilteredTrackingData() const { return (m_filteredTrackingData.size() > 1) ? *(m_filteredTrackingData.end() - 2) : &Fubi::EmptyUserTrackingData; }

	/**
	* \brief The user's body measurements
	*/
	const Fubi::BodyMeasurementDistance* bodyMeasurements() const { return m_bodyMeasurements; }

	/**
	* \brief Get the debug image of the last finger count detection
	* Note: For internal use only!
	*/
	const Fubi::FingerCountImageData* getFingerCountImageData(bool left = false)
	{
		return left ? &m_leftFingerCountData.fingerCountImage : &m_rightFingerCountData.fingerCountImage;
	}

	/**
	* \brief Assigned hand from finger sensor for left hand, and right hand
	*/
	unsigned int m_assignedLeftHandId, m_assignedRightHandId;

private:
	/**
	* \brief Constructor
	*  Note: For internal use only!
	*/
	FubiUser();
	/**
	* \brief Destructor
	*  Note: For internal use only!
	*/
	~FubiUser();

	/**
	* \brief Enables/disables a posture combination recognizer of this user
	* Note: For internal use only, please use Fubi::enableCombinationRecognition() instead!
	*/
	void enableCombinationRecognition(Fubi::Combinations::Combination postureID, bool enable);
	void enableCombinationRecognition(const CombinationRecognizer* recognizerTemplate, bool enable);

	/**
	* \brief Enable/disables the tracking of the shown number of fingers for each hand
	* Note: For internal use only, please use Fubi::enableFingerTracking() instead!
	*/
	void enableFingerTracking(bool enable, bool leftHand = false);

	/**
	* \brief Enable/disables usage of the old convexity defect method for finger tracking
	* Note: For internal use only, please use Fubi::enableFingerTracking:() instead!
	*/
	void enableFingerTrackingWithConvexityDefectMethod(bool enable);

	
	/**
	* \brief Gets the finger count optionally calculated by the median of the last medianWindowSize calculations
	* Note: For internal use only, please use Fubi::getFingerCount() instead!
	*/
	int getFingerCount(bool leftHand = false, bool getMedianOfLastFrames = true, unsigned int medianWindowSize = 10, bool useOldConvexityDefectMethod = false);

	/**
	* \brief Stops and removes all user defined recognizers
	* Note: For internal use only, please use Fubi::clearUserDefinedRecognizers() instead!
	*/
	void clearUserDefinedCombinationRecognizers();

	/**
	* \brief Calculates filtered and local transformations, body measures, updates the combination recognizers and finger counts
	* Note: For internal use only, please use Fubi::updateSensor() instead!
	*/
	void update(double currentTimestamp);

	/**
	* \brief Update the tracking info from the given sensor
	* Note: For internal use only, please use Fubi::updateSensor() instead!
	*/ 
	void updateTrackingData(class FubiISensor* sensor);

	/**
	* \brief Reset the user to an initial state 
	* Note: For internal use only, please use Fubi::resetTracking() instead!
	*/
	void reset();

	/**
	* \brief Get the recognition progress of a combination recognizer associated to this user
	* Note: For internal use only, please use Fubi::getCombinationRecognitionProgressOn() instead!
	*/
	Fubi::RecognitionResult::Result getRecognitionProgress(Fubi::Combinations::Combination combinationID, std::vector<Fubi::TrackingData>* userStates,
		bool restart, bool returnFilteredData, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);
	Fubi::RecognitionResult::Result getRecognitionProgress(const std::string& recognizerName, std::vector<Fubi::TrackingData>* userStates,
		bool restart, bool returnFilteredData, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);
	/**
	* \brief or the current state of the combination recognizer 
	* Note: For internal use only, please use Fubi::getCurrentCombinationRecognitionState() instead!
	*/
	int getCurrentRecognitionState(const std::string& recognizerName, unsigned int& numStates, bool& isInterrupted, bool& isInTransition);

	/**
	* \brief Removes the oldest tracking data and adds a new data frame at the beginning reusing the old data if possible (-> so the new data is not valid!)
	* Note: For internal use only, please use Fubi::updateTrackingData() instead!
	*/
	void addNewTrackingData(double timeStamp = -1);
	/**
	* \brief Copies in new tracking data out of the provided positions, orientations, fingercounts and time stamp
	* Note: For internal use only, please use Fubi::updateTrackingData() instead!
	*/
	void addNewTrackingData(const Fubi::SkeletonJointPosition* positions, const Fubi::SkeletonJointOrientation* orientations,
		int leftFingerCount = -1, int rightFingerCount = -1 , double timeStamp = -1);
	/**
	* \brief Copies in new tracking data out of the provided positions and orientations in the skeleton float array and the time stamp
	* Note: For internal use only, please use Fubi::updateTrackingData() instead!
	*/
	void addNewTrackingData(float* skeleton, double timeStamp /*= -1*/);


	/**
	* \brief Adds a finger count detection to the deque for later median calculation
	* Note: For internal use only
	*/
	void addFingerCount(int count, bool leftHand, double currentTime);

	/**
	* \brief Apply a filter on the tracking data
	* Note: For internal use only
	*/
	void calculateFilteredTransformations(const Fubi::FilterOptions& filterOptions);

	/**
	* \brief Update the combination recognizers according to the current tracking data
	* Note: For internal use only
	*/
	void updateCombinationRecognizers();

	/**
	* \brief Update the finger count calculation
	* Note: For internal use only
	*/
	void updateFingerCount(bool leftHand, double currentTimestamp);

	/**
	* \brief Update the body measurements out of the currently tracked positions
	* Note: For internal use only
	*/
	void updateBodyMeasurements(double currentTimestamp, const Fubi::FilterOptions& filterOptions);

	/**
	* \brief Whether the user is currently seen in the depth image
	*/
	bool m_inScene;
	

	/**
	* \brief Id of this user
	*/
	unsigned int m_id;

	/**
	* \brief Whether the user is currently tracked
	*/
	bool m_isTracked;

	/**
	* \brief Current and last, filtered and unfiltered tracking data including joint positions and orientations (both local and global ones)
	*/
	std::deque<Fubi::TrackingData*> m_trackingData, m_filteredTrackingData;

	/**
	* \brief How many tracking data points are kept
	*/
	unsigned int m_maxTrackingHistoryLength;

	/**
	* \brief The user's body measurements
	*/
	Fubi::BodyMeasurementDistance m_bodyMeasurements[Fubi::BodyMeasurement::NUM_MEASUREMENTS];

	/**
	* \brief One posture combination recognizer per posture combination
	* Note: For internal use only
	*/
	CombinationRecognizer* m_combinationRecognizers[Fubi::Combinations::NUM_COMBINATIONS];
	/**
	* \brief And all user defined ones that are currently enabled
	* Note: For internal use only
	*/
	std::map<std::string, CombinationRecognizer*> m_userDefinedCombinationRecognizers;


	/**
	* \brief Fingercount data
	*/
	Fubi::FingerCountData m_leftFingerCountData, m_rightFingerCountData;

	/**
	* \brief Timestamp of the last body measurement update and how often it should be updated
	*/
	double m_lastBodyMeasurementUpdate, m_bodyMeasurementUpdateIntervall;

	/**
	* \brief Additional filtering history for the velocities
	*/
	Fubi::Vec3f m_lastFilteredVelocity[Fubi::SkeletonJoint::NUM_JOINTS];
};

/*! @}*/