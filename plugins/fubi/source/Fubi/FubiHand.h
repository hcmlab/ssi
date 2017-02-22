// ****************************************************************************************
//
// Fubi Hand
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler 
// 
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
// 
// ****************************************************************************************
#pragma once

/** \file FubiHand.h 
 * \brief a header file containing the FubiHand class definition
*/ 

#include "FubiUtils.h"
#include "FubiMath.h"

#include <map>


class CombinationRecognizer;

/** \addtogroup FUBIHAND FUBI Hand class
* Contains the FubiHand class that holds informations for tracked hands
* 
* @{
*/

/**
 * \brief The FubiHand class holds all relevant informations for each tracked hand
 */
class FubiHand
{
	friend class FubiCore;
	friend class FubiPlayer;
public:
	/**
	 * \brief Constructor for FubiHand
	 * Note: For internal use only!
	 */
	FubiHand();
	/**
	 * \brief Destructor for FubiHand
	 * Note: For internal use only!
	 */
	~FubiHand();

	/**
	 * \brief Enables/disables a posture combination recognizer of this user
	 * Note: For internal use only, please use Fubi::enableCombinationRecognition() instead!
	 */
	void enableCombinationRecognition(const CombinationRecognizer* recognizerTemplate, bool enable);

	/**
	 * \brief Stops and removes all combination recognizers
	 * Note: For internal use only, please use Fubi::clearUserDefinedRecognizers() instead!
	 */
	void clearCombinationRecognizers();

	/**
	 * \brief Calculates filtered and local transformations and updates the combination recognizers
	 * Note: For internal use only, please use Fubi::updateSensor() instead!
	 */
	void update();

	/**
	 * \brief Update the tracking info from the given sensor
	 * Note: For internal use only, please use Fubi::updateSensor() instead!
	 */
	void updateFingerTrackingData(class FubiIFingerSensor* sensor);

	/**
	 * \brief Reset the hand to an initial state
	 * Note: For internal use only, please use Fubi::resetTracking() instead!
	 */
	void reset();

	/**
	 * \brief Get the recognition progress of a combination recognizer associated to this hand
	 * Note: For internal use only, please use Fubi::getCombinationRecognitionProgressOn() instead!
	 */
	Fubi::RecognitionResult::Result getRecognitionProgress(const std::string& recognizerName, std::vector<Fubi::TrackingData>* trackingStates,
		bool restart, bool returnFilteredData, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);
	/**
	 * \brief Get the  the current state of the combination recognizer
	 * Note: For internal use only, please use Fubi::getCurrentCombinationRecognitionState() instead!
	 */
	int getCurrentRecognitionState(const std::string& recognizerName, unsigned int& numStates, bool& isInterrupted, bool& isInTransition);

	/**
	 * \brief Id of this hand
	 */
	unsigned int id() const { return m_id; }

	/**
	 * \brief Whether the hand is currently tracked
	 */
	bool isTracked() const { return m_isTracked; }

	/**
	* \brief The recognized hand type (left or right) or num_joints if unkown
	*/
	Fubi::SkeletonJoint::Joint getHandType() const { return m_handType;  }

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
	const Fubi::TrackingData* currentTrackingData() const { return (m_trackingData.size() > 0) ? m_trackingData.back() : &Fubi::EmptyFingerTrackingData; }

	/**
	* \brief Last frame's tracking data including joint positions and orientations (both local and global ones)
	*/
	const Fubi::TrackingData* lastTrackingData() const { return (m_trackingData.size() > 1) ? *(m_trackingData.end() - 2) : &Fubi::EmptyFingerTrackingData; }

	/**
	* \brief Most current filtered tracking data including joint positions and orientations (both local and global ones)
	*/
	const Fubi::TrackingData* currentFilteredTrackingData() const { return (m_filteredTrackingData.size() > 0) ? m_filteredTrackingData.back() : &Fubi::EmptyFingerTrackingData; }

	/**
	* \brief Last frame's filtered tracking data including joint positions and orientations (both local and global ones)
	*/
	const Fubi::TrackingData* lastFilteredTrackingData() const { return (m_filteredTrackingData.size() > 1) ? *(m_filteredTrackingData.end() - 2) : &Fubi::EmptyFingerTrackingData; }
	

private:
	/**
	* \brief Removes the oldest tracking data and adds a new data frame at the beginning reusing the old data if possible (-> so the new data is not valid!)
	* Note: For internal use only, please use Fubi::updateTrackingData() instead!
	*/
	void addNewTrackingData(double timeStamp = -1);
	/**
	* \brief Copies in new tracking data out of the provided positions, fingercount, hand type and time stamp
	* Note: For internal use only, please use Fubi::updateTrackingData() instead!
	*/
	void addNewTrackingData(const Fubi::SkeletonJointPosition* positions, const Fubi::SkeletonJointOrientation* orientations, int fingerCount = -1,
		Fubi::SkeletonJoint::Joint handType = Fubi::SkeletonJoint::NUM_JOINTS, double timeStamp = -1);

	/**
	 * \brief Apply a filter on the tracking data
	 */
	void calculateFilteredTransformations();

	/**
	 * \brief Update the combination recognizers according to the current tracking data
	 */
	void updateCombinationRecognizers();

	/**
	 * \brief All enabled hand defined combination recognizers
	 */
	std::map<std::string, CombinationRecognizer*> m_combinationRecognizers;

	/**
	 * \brief Additional filtering history for the velocities
	 */
	Fubi::Vec3f m_lastFilteredVelocity[Fubi::SkeletonHandJoint::NUM_JOINTS];

	/**
	 * \brief Id of this hand
	 */
	unsigned int m_id;

	/**
	 * \brief Whether the hand is currently tracked
	 */
	bool m_isTracked;

	/**
	 * \brief The recognized hand type (left or right) or num_joints if unkown
	 */
	Fubi::SkeletonJoint::Joint m_handType;

	/**
	* \brief Current and last, filtered and unfiltered tracking data including joint positions and orientations (both local and global ones)
	*/
	std::deque<Fubi::TrackingData*> m_trackingData, m_filteredTrackingData;

	/**
	* \brief How many tracking data points are kept
	*/
	unsigned int m_maxTrackingHistoryLength;
};

/*! @}*/