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

#include "FubiCore.h"

// Defines for enabling/disabling OpenNI and OpenCV dependencies
#include "FubiConfig.h"

// Gesture recognition
#include "FubiRecognizerFactory.h"
#include "GestureRecognizer/TemplateRecognizer.h"

// Image processing
#include "FubiImageProcessing.h"

#ifdef FUBI_USE_OPENNI2
// OpenNI v2.x integration
#include "FubiOpenNI2Sensor.h"
#endif
#ifdef FUBI_USE_OPENNI1
// OpenNI v1.x integration
#include "FubiOpenNISensor.h"
#endif
#ifdef FUBI_USE_KINECT_SDK
// Kinect SDK integration
#include "FubiKinectSDKSensor.h"
#endif
#ifdef FUBI_USE_KINECT_SDK_2
// Kinect SDK 2 integration
#include "FubiKinectSDK2Sensor.h"
#endif

#ifdef FUBI_USE_LEAP
// Leap sensor integration
#include "FubiLeapSensor.h"
#endif

// Xml parsing
#include "FubiXMLParser.h"

// Recording and Playback
#include "FubiRecorder.h"
#include "FubiPlayer.h"

// Sorting and more
#include <algorithm>

using namespace Fubi;
using namespace std;

const std::string FubiCore::s_emtpyString;

FubiCore* FubiCore::s_instance = 0x0;

FubiCore::~FubiCore()
{
	for (unsigned int i = 0; i < Postures::NUM_POSTURES; ++i)
	{
		delete m_postureRecognizers[i];
		m_postureRecognizers[i] = 0x0;
	}
	clearUserDefinedRecognizers();

	for (unsigned int i = 0; i < MaxUsers; ++i)
	{
		delete m_users[i];
		m_users[i] = 0x0;
	}
	m_numUsers = 0;

	for (unsigned int i = 0; i < MaxHands; ++i)
	{
		delete m_hands[i];
		m_hands[i] = 0x0;
	}
	m_numHands = 0;

	delete m_sensor; m_sensor = 0x0;
	delete m_fingerSensor; m_fingerSensor = 0x0;
	delete m_recorder; m_recorder = 0x0;
	delete m_player; m_player = 0x0;
}

FubiCore::FubiCore() : m_numUsers(0), m_numHands(0), m_sensor(0x0), m_fingerSensor(0x0)
{
	m_player = new FubiPlayer();
	m_recorder = new FubiRecorder();

	for (unsigned int i = 0; i < MaxUsers; ++i)
	{
		m_users[i] = new FubiUser();
		for (unsigned int p = 0; p < Postures::NUM_POSTURES; ++p)
		{
			m_currentPostureRecognitions[i][p] = false;
		}
		for (unsigned int p = 0; p < Combinations::NUM_COMBINATIONS; ++p)
		{
			m_currentPredefinedCombinationRecognitions[i][p] = false;
		}
	}

	for (unsigned int i = 0; i < MaxHands; ++i)
	{
		m_hands[i] = new FubiHand();
	}

	for (unsigned int i = 0; i < Combinations::NUM_COMBINATIONS+1; ++i)
	{
		m_autoStartCombinationRecognizers[i] = false;
	}

	// Init posture recognizers
	for (unsigned int i = 0; i < Postures::NUM_POSTURES; ++i)
	{
		m_postureRecognizers[i] = createPostureRecognizer((Postures::Posture)i);
	}
}

bool FubiCore::initFromXml(const char* xmlPath, Fubi::SkeletonTrackingProfile::Profile profile /*= Fubi::SkeletonTrackingProfile::ALL*/,
	bool mirrorStream /*= true*/, bool registerStreams /*=true*/)
{
	delete m_sensor;
	m_sensor = 0x0;

	for (unsigned int i = 0; i < MaxUsers; ++i)
	{
		m_users[i]->reset();
	}
	m_numUsers = 0;
	m_userIDToUsers.clear();

	if (xmlPath != 0)
	{
#ifdef FUBI_USE_OPENNI1
		m_sensor = new FubiOpenNISensor();
		if (!m_sensor->initFromXml(xmlPath, profile, mirrorStream, registerStreams))
		{
			delete m_sensor;
			m_sensor = 0x0;
			return false;
		}
#else
		Fubi_logErr("Tried to init OpenNI 1.x via XML, but no activated sensor type supports xml init.\n -Did you forget to uncomment the FUBI_USE_OPENNI1 define in the FubiConfig.h?\n");
		return false;
#endif
	}
	return true;
}

bool FubiCore::initSensorWithOptions(const Fubi::SensorOptions& options)
{
	delete m_sensor;
	m_sensor = 0x0;
	bool success = false;

	for (unsigned int i = 0; i < MaxUsers; ++i)
	{
		m_users[i]->reset();
	}
	m_numUsers = 0;
	m_userIDToUsers.clear();

	if (options.m_type == SensorType::OPENNI2)
	{
#ifdef FUBI_USE_OPENNI2
		m_sensor = new FubiOpenNI2Sensor();
		success = m_sensor->initWithOptions(options);
#else
		Fubi_logErr("Openni 2.x sensor is not activated\n -Did you forget to uncomment the FUBI_USE_OPENNI2 define in the FubiConfig.h?\n");
#endif
	}
	else if (options.m_type == SensorType::OPENNI1)
	{
#ifdef FUBI_USE_OPENNI1
		m_sensor = new FubiOpenNISensor();
		success = m_sensor->initWithOptions(options);
#else
		Fubi_logErr("Openni 1.x sensor is not activated\n -Did you forget to uncomment the FUBI_USE_OPENNI1 define in the FubiConfig.h?\n");
#endif
	}
	else if (options.m_type == SensorType::KINECTSDK)
	{
#ifdef FUBI_USE_KINECT_SDK
		m_sensor = new FubiKinectSDKSensor();
		success = m_sensor->initWithOptions(options);
#else
		Fubi_logErr("Kinect SDK sensor is not activated\n -Did you forget to uncomment the USE_KINECTSDK define in the FubiConfig.h?\n");
#endif
	}
	else if (options.m_type == SensorType::KINECTSDK2)
	{
#ifdef FUBI_USE_KINECT_SDK_2
		m_sensor = new FubiKinectSDK2Sensor();
		success = m_sensor->initWithOptions(options);
#else
		Fubi_logErr("Kinect SDK 2 sensor is not activated\n -Did you forget to uncomment the USE_KINECT_SDK_2 define in the FubiConfig.h?\n");
#endif
	}
	else if (options.m_type == SensorType::NONE)
	{
		Fubi_logInfo("FubiCore: Current sensor deactivated, now in non-tracking mode!\n");
		success = true;
	}

	if (!success)
	{
		delete m_sensor;
		m_sensor = 0x0;
	}

	return success;
}

void FubiCore::updateSensor()
{
	bool userChanged = false;
	if (m_sensor)
	{
		m_sensor->update();

		// Get the current number and ids of users, adapt the useridTouser map
		// init new users and let them do the update for tracking info
		userChanged = updateUsers();
	}

	bool wasPlaying = m_player->isPlaying();
	m_player->update();

	if (m_fingerSensor)
	{
		// Update sensor data
		m_fingerSensor->update();

		// Get all tracked hands, adapt the handIdToUser map, init new hands
		if (updateHands() || userChanged)
		{
			// hands or users changed so update the assignment
			updateHandToUserAssignment();
		}
	}

	if (m_recorder->isRecording())
	{
		FubiUser* user = getUser(m_recorder->getUserID());
		if (user)
			m_recorder->recordNextSkeletonFrame(*user, user->m_leftFingerCountData, user->m_rightFingerCountData);
		FubiHand* hand = getHand(m_recorder->getHandID());
		if (hand)
			m_recorder->recordNextSkeletonFrame(*hand);
	}

	// Check all recognizer if callback registered
	if (m_recognitionStartCallback || m_recognitionEndCallback)
	{
		// Normal hands/users
		for (unsigned short i = 0; i < m_numUsers; ++i)
			checkRecognizers(i, false);
		for (unsigned short i = 0; i < m_numHands; ++i)
			checkRecognizers(i, true);
		// Playback hands/users
		if (m_player->isPlaying())
		{
			if (getUser(PlaybackUserID))
				checkRecognizers(MaxUsers, false);
			if (getHand(PlaybackHandID))
				checkRecognizers(MaxHands, true);
		}
		else if (wasPlaying)
		{
			// Clear playback users on playback stop
			clearRecognitions(MaxUsers, false);
			clearRecognitions(MaxHands, true);
		}
	}
}

void FubiCore::checkRecognizers(unsigned int targetIndex, bool isHand)
{
	unsigned int targetID;
	if (!isHand && targetIndex == MaxUsers)
	{
		// Check playback user
		targetID = PlaybackUserID;
	}
	else if (isHand && targetIndex == m_numHands)
	{
		// Check playback hand
		targetID = PlaybackHandID;
	}
	else // Check normal user
		targetID = isHand ? m_hands[targetIndex]->id() : m_users[targetIndex]->id();

	// Predefined gestures (not available for hands)
	if (!isHand)
	{
		for (unsigned int i = 0; i < Postures::NUM_POSTURES; ++i)
		{
			Postures::Posture p = (Postures::Posture) i;
			const char* postureName = getPostureName(p);
			if (recognizeGestureOn(p, targetID) == RecognitionResult::RECOGNIZED)
			{
				if (!m_currentPostureRecognitions[targetIndex][i])
				{
					m_currentPostureRecognitions[targetIndex][i] = true;
					if (m_recognitionStartCallback)
						m_recognitionStartCallback(postureName, targetID, isHand, RecognizerType::PREDEFINED_GESTURE);
				}
			}
			else if (m_currentPostureRecognitions[targetIndex][i])
			{
				m_currentPostureRecognitions[targetIndex][i] = false;
				if (m_recognitionEndCallback)
					m_recognitionEndCallback(postureName, targetID, isHand, RecognizerType::PREDEFINED_GESTURE);
			}
		}
	}

	// User defined gestures
	unsigned int numRecs = getNumUserDefinedRecognizers();
	// Resize recognition buffers
	if (isHand)
		m_currentHandGestureRecognitions[targetIndex].resize(numRecs, false);
	else
		m_currentGestureRecognitions[targetIndex].resize(numRecs, false);
	for (unsigned int i = 0; i < numRecs; ++i)
	{
		const std::string& gestureName = getUserDefinedRecognizerName(i);
		RecognitionResult::Result result = isHand
			? recognizeGestureOnHand(gestureName, targetID)
			: recognizeGestureOn(i, targetID);
		const auto& storedRecognition = isHand
			? (m_currentHandGestureRecognitions[targetIndex][i])
			: (m_currentGestureRecognitions[targetIndex][i]);

		if (result == RecognitionResult::RECOGNIZED)
		{
			if (!storedRecognition)
			{
			    if (isHand)
                    m_currentHandGestureRecognitions[targetIndex][i] = true;
                else
                    m_currentGestureRecognitions[targetIndex][i] = true;
				if (m_recognitionStartCallback)
					m_recognitionStartCallback(gestureName.c_str(), targetID, isHand, RecognizerType::USERDEFINED_GESTURE);
			}
		}
		else if (storedRecognition)
		{
			if (isHand)
                m_currentHandGestureRecognitions[targetIndex][i] = false;
            else
                m_currentGestureRecognitions[targetIndex][i] = false;
			if (m_recognitionEndCallback)
				m_recognitionEndCallback(gestureName.c_str(), targetID, isHand, RecognizerType::USERDEFINED_GESTURE);
		}
	}

	// Predefined combinations (not available for hands)
	if (!isHand)
	{
		for (unsigned int i = 0; i < Combinations::NUM_COMBINATIONS; ++i)
		{
			Combinations::Combination c = (Combinations::Combination) i;
			RecognitionResult::Result result = getCombinationRecognitionProgressOn(c, targetID);
			if (result == RecognitionResult::RECOGNIZED)
			{
				if (m_recognitionStartCallback && !m_currentPredefinedCombinationRecognitions[targetIndex][i])
					m_recognitionStartCallback(getCombinationName(c), targetID, isHand, RecognizerType::PREDEFINED_COMBINATION);
				if (m_recognitionEndCallback)
					m_recognitionEndCallback(getCombinationName(c), targetID, isHand, RecognizerType::PREDEFINED_COMBINATION);
				m_currentPredefinedCombinationRecognitions[targetIndex][i] = false;
			}
			if (result == RecognitionResult::WAITING_FOR_LAST_STATE_TO_FINISH)
			{
				if (!m_currentPredefinedCombinationRecognitions[targetIndex][i])
				{
					m_currentPredefinedCombinationRecognitions[targetIndex][i] = true;
					if (m_recognitionStartCallback)
						m_recognitionStartCallback(getCombinationName(c), targetID, isHand, RecognizerType::PREDEFINED_COMBINATION);
				}
			}
			else
				m_currentPredefinedCombinationRecognitions[targetIndex][i] = false;
		}
	}

	// User defined combinations
	unsigned int numCombs = getNumUserDefinedCombinationRecognizers();
	// Resize recognition buffers
	if (isHand)
		m_currentHandCombinationRecognitions[targetIndex].resize(numCombs, false);
	else
		m_currentUserCombinationRecognitions[targetIndex].resize(numCombs, false);
	for (unsigned int i = 0; i < numCombs; ++i)
	{
		const std::string& gestureName = getUserDefinedCombinationRecognizerName(i);
		RecognitionResult::Result result = isHand
			? getCombinationRecognitionProgressOnHand(gestureName.c_str(), targetID)
			: getCombinationRecognitionProgressOn(gestureName.c_str(), targetID);
		const auto& storedRecognition = isHand
			? m_currentHandCombinationRecognitions[targetIndex][i]
			: m_currentUserCombinationRecognitions[targetIndex][i];
		if (result == RecognitionResult::RECOGNIZED)
		{
			if (m_recognitionStartCallback && !storedRecognition)
				m_recognitionStartCallback(gestureName.c_str(), targetID, isHand, RecognizerType::USERDEFINED_COMBINATION);
			if (m_recognitionEndCallback)
				m_recognitionEndCallback(gestureName.c_str(), targetID, isHand, RecognizerType::USERDEFINED_COMBINATION);

			if (isHand)
                m_currentHandCombinationRecognitions[targetIndex][i] = false;
			else
                m_currentUserCombinationRecognitions[targetIndex][i] = false;
		}
		if (result == RecognitionResult::WAITING_FOR_LAST_STATE_TO_FINISH)
		{
			if (!storedRecognition)
			{
				if (isHand)
                m_currentHandCombinationRecognitions[targetIndex][i] = true;
			else
                m_currentUserCombinationRecognitions[targetIndex][i] = true;
				if (m_recognitionStartCallback)
					m_recognitionStartCallback(gestureName.c_str(), targetID, isHand, RecognizerType::USERDEFINED_COMBINATION);
			}
		}
		else
		{
		    if (isHand)
                m_currentHandCombinationRecognitions[targetIndex][i] = false;
			else
                m_currentUserCombinationRecognitions[targetIndex][i] = false;
		}
	}
}

void FubiCore::clearRecognitions(unsigned int targetIndex, bool isHand)
{
	unsigned int targetID;
	if (!isHand && targetIndex == MaxUsers)
	{
		// Check playback user
		targetID = PlaybackUserID;
	}
	else if (isHand && targetIndex == MaxHands)
	{
		// Check playback hand
		targetID = PlaybackHandID;
	}
	else // Normal user/hand
		targetID = isHand ? m_hands[targetIndex]->id() : m_users[targetIndex]->id();

	// Predefined gestures (not available for hands)
	if (!isHand)
	{
		for (unsigned int i = 0; i < Postures::NUM_POSTURES; ++i)
		{
			if (m_currentPostureRecognitions[targetIndex][i])
			{
				m_currentPostureRecognitions[targetIndex][i] = false;
				if (m_recognitionEndCallback)
					m_recognitionEndCallback(getPostureName((Postures::Posture) i), targetID, isHand, RecognizerType::PREDEFINED_GESTURE);
			}
		}
	}

	// User defined gestures
	size_t numRecs = isHand ?	m_currentHandGestureRecognitions[targetIndex].size()
		: m_currentGestureRecognitions[targetIndex].size();
	for (size_t i = 0; i < numRecs; ++i)
	{
		const auto& storedRecognition = isHand
			? m_currentHandGestureRecognitions[targetIndex][i]
			: m_currentGestureRecognitions[targetIndex][i];
		if (storedRecognition)
		{
			if (isHand)
                m_currentHandGestureRecognitions[targetIndex][i] = false;
			else
                m_currentGestureRecognitions[targetIndex][i] = false;
			if (m_recognitionEndCallback)
				m_recognitionEndCallback(getUserDefinedRecognizerName((unsigned int)i).c_str(), targetID, isHand, RecognizerType::USERDEFINED_GESTURE);
		}
	}

	// Predefined combinations (not available for hands)
	if (!isHand)
	{
		for (unsigned int i = 0; i < Combinations::NUM_COMBINATIONS; ++i)
		{
			if (m_currentPredefinedCombinationRecognitions[targetIndex][i])
			{
				m_currentPredefinedCombinationRecognitions[targetIndex][i] = false;
				if (m_recognitionEndCallback)
					m_recognitionEndCallback(getCombinationName((Combinations::Combination) i), targetID, isHand, RecognizerType::PREDEFINED_COMBINATION);
			}
		}
	}

	// User defined combinations
	size_t numCombs = isHand ? m_currentHandCombinationRecognitions[targetIndex].size()
		: m_currentUserCombinationRecognitions[targetIndex].size();
	for (size_t i = 0; i < numCombs; ++i)
	{
		const auto& storedRecognition = isHand
			? m_currentHandCombinationRecognitions[targetIndex][i]
			: m_currentUserCombinationRecognitions[targetIndex][i];
		if (storedRecognition)
		{
			if (isHand)
                m_currentHandCombinationRecognitions[targetIndex][i] = false;
			else
                m_currentUserCombinationRecognitions[targetIndex][i] = false;
			if (m_recognitionEndCallback)
				m_recognitionEndCallback(getUserDefinedCombinationRecognizerName((unsigned int)i).c_str(), targetID, isHand, RecognizerType::USERDEFINED_COMBINATION);
		}
	}
}

void FubiCore::updateHandToUserAssignment()
{
	for (unsigned short i = 0; i < m_numUsers; ++i)
	{
		bool foundLeftHand = false, foundRightHand = false;
		if (m_users[i]->m_isTracked)
		{
			SkeletonJointPosition userHandPos = m_users[i]->currentFilteredTrackingData()->jointPositions[SkeletonJoint::RIGHT_HAND];
			if (userHandPos.m_confidence > 0.25f)
			{
				FubiHand* hand = getClosestHand(userHandPos.m_position, SkeletonJoint::RIGHT_HAND);
				if (hand)
				{
					FubiUser* foundUser = 0x0;
					SkeletonJoint::Joint foundJoint = SkeletonJoint::NUM_JOINTS;
					if (getClosestUserHandJoint(hand->currentFilteredTrackingData()->jointPositions[SkeletonHandJoint::PALM].m_position, &foundUser, foundJoint))
					{
						if (foundUser == m_users[i] && foundJoint == SkeletonJoint::RIGHT_HAND)
						{
							foundUser->m_assignedRightHandId = hand->m_id;
							foundRightHand = true;
						}
					}
				}
			}
			userHandPos = m_users[i]->currentFilteredTrackingData()->jointPositions[SkeletonJoint::LEFT_HAND];
			if (userHandPos.m_confidence > 0.25f)
			{
				FubiHand* hand = getClosestHand(userHandPos.m_position, SkeletonJoint::LEFT_HAND);
				if (hand)
				{
					FubiUser* foundUser = 0x0;
					SkeletonJoint::Joint foundJoint = SkeletonJoint::NUM_JOINTS;
					if (getClosestUserHandJoint(hand->currentFilteredTrackingData()->jointPositions[SkeletonHandJoint::PALM].m_position, &foundUser, foundJoint))
					{
						if (foundUser == m_users[i] && foundJoint == SkeletonJoint::LEFT_HAND)
						{
							foundUser->m_assignedLeftHandId = hand->m_id;
							foundLeftHand = true;
						}
					}
				}
			}
		}
		if (!foundLeftHand)
			m_users[i]->m_assignedLeftHandId = 0;
		if (!foundRightHand)
			m_users[i]->m_assignedRightHandId = 0;
		Fubi_logDbg("Updated user%u's hands: left=%u, right=%u\n", m_users[i]->id(), m_users[i]->m_assignedLeftHandId, m_users[i]->m_assignedRightHandId);
	}
}


Fubi::RecognitionResult::Result FubiCore::recognizeGestureOn(Postures::Posture postureID, unsigned int userID)
{
	if (postureID < Postures::NUM_POSTURES && m_postureRecognizers[postureID])
	{
		FubiUser* user = getUser(userID);
		if (user)
		{
			// Found user
			return m_postureRecognizers[postureID]->recognizeOn(user);
		}
	}
	return Fubi::RecognitionResult::NOT_RECOGNIZED;
}

Fubi::RecognitionResult::Result FubiCore::recognizeGestureOn(unsigned int recognizerIndex, unsigned int userID, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	if (recognizerIndex < m_userDefinedRecognizers.size())
	{
		FubiUser* user = getUser(userID);
		if (user)
		{
			// Found the user
			if (user->isTracked() && user->inScene())
			{
				IGestureRecognizer* rec = m_userDefinedRecognizers[recognizerIndex].second;
				if (rec->m_targetSkeletonType == RecognizerTarget::LEFT_HAND && user->m_assignedLeftHandId != 0)
					return rec->recognizeOn(getHand(user->m_assignedLeftHandId), correctionHint);
				else if (rec->m_targetSkeletonType == RecognizerTarget::RIGHT_HAND && user->m_assignedRightHandId != 0)
					return rec->recognizeOn(getHand(user->m_assignedRightHandId), correctionHint);
				else if (rec->m_targetSkeletonType == RecognizerTarget::BOTH_HANDS)
				{
					RecognitionResult::Result res1 = RecognitionResult::TRACKING_ERROR, res2 = RecognitionResult::TRACKING_ERROR;
					if (user->m_assignedLeftHandId != 0)
						res1 = rec->recognizeOn(getHand(user->m_assignedLeftHandId), correctionHint);
					if (user->m_assignedRightHandId != 0)
						res2 = rec->recognizeOn(getHand(user->m_assignedRightHandId), correctionHint);
					if (res1 != RecognitionResult::TRACKING_ERROR && res2 != RecognitionResult::TRACKING_ERROR)
						return max(res1, res2);
					else if (res1 != RecognitionResult::TRACKING_ERROR)
						return res1;
					else if (res2 != RecognitionResult::TRACKING_ERROR)
						return res2;
				}
				// If everything else failed, we try to take the user
				return rec->recognizeOn(user, correctionHint);
			}
			else
				return Fubi::RecognitionResult::TRACKING_ERROR;
		}
	}
	return Fubi::RecognitionResult::NOT_RECOGNIZED;
}

Fubi::RecognitionResult::Result FubiCore::recognizeGestureOn(const string& name, unsigned int userID, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	int recognizerIndex = getUserDefinedRecognizerIndex(name);
	if (recognizerIndex >= 0)
		return recognizeGestureOn((unsigned) recognizerIndex, userID, correctionHint);
	return Fubi::RecognitionResult::NOT_RECOGNIZED;
}

Fubi::RecognitionResult::Result FubiCore::recognizeGestureOnHand(const string& name, unsigned int handID)
{
	int recognizerIndex = getUserDefinedRecognizerIndex(name);
	if (recognizerIndex >= 0)
	{
		IGestureRecognizer* rec = m_userDefinedRecognizers[recognizerIndex].second;
		if (rec->m_targetSkeletonType != RecognizerTarget::BODY)
		{
			FubiHand* hand = getHand(handID);
			if (hand && hand->m_isTracked)
			{
				// Found the hand
				return rec->recognizeOn(hand);
			}
		}
	}
	return Fubi::RecognitionResult::NOT_RECOGNIZED;
}

int FubiCore::getUserDefinedRecognizerIndex(const std::string& name)
{
	if (name.length() > 0)
	{
		vector<pair<string, IGestureRecognizer*> >::iterator iter = m_userDefinedRecognizers.begin();
		vector<pair<string, IGestureRecognizer*> >::iterator end = m_userDefinedRecognizers.end();
		for (int i = 0; iter != end; ++iter, ++i)
		{
			if (name == iter->first)
				return i;
		}
	}
	return -1;
}

int FubiCore::getHiddenUserDefinedRecognizerIndex(const std::string& name)
{
	if (name.length() > 0)
	{
		vector<pair<string, IGestureRecognizer*> >::iterator iter = m_hiddenUserDefinedRecognizers.begin();
		vector<pair<string, IGestureRecognizer*> >::iterator end = m_hiddenUserDefinedRecognizers.end();
		for (int i = 0; iter != end; ++iter, ++i)
		{
			if (name == iter->first)
				return i;
		}
	}
	return -1;
}

int FubiCore::getUserDefinedCombinationRecognizerIndex(const std::string& name)
{
	if (name.length() > 0)
	{
		vector<pair<string, CombinationRecognizer*> >::iterator iter = m_userDefinedCombinationRecognizers.begin();
		vector<pair<string, CombinationRecognizer*> >::iterator end = m_userDefinedCombinationRecognizers.end();
		for (int i = 0; iter != end; ++iter, ++i)
		{
			if (name == iter->first)
				return i;
		}
	}
	return -1;
}

IGestureRecognizer* FubiCore::cloneRecognizer(const std::string& name)
{
	// First look for "normal" recognizers
	IGestureRecognizer* rec = cloneUserDefinedRecognizer(getUserDefinedRecognizerIndex(name.c_str()));
	if (rec == 0x0) // now for hidden ones
		rec = cloneHiddenUserDefinedRecognizer(getHiddenUserDefinedRecognizerIndex(name.c_str()));
	if (rec == 0x0) // name as well might represent the index of a recognizer
	{
		int index = atoi(name.c_str());
		if (index > 0 || name == "0")
		{
			rec = cloneUserDefinedRecognizer((unsigned)index);
		}
	}
	if (rec == 0x0) // last option: name belongs to a predefined gesture
	{
		Postures::Posture p = getPostureID(name);
		if (p < Postures::NUM_POSTURES)
		{
			rec = createPostureRecognizer(p);
		}
	}
	return rec;
}

CombinationRecognizer* FubiCore::getUserDefinedCombinationRecognizer(unsigned int index)
{
	if (index < m_userDefinedCombinationRecognizers.size())
	{
		return m_userDefinedCombinationRecognizers[index].second;
	}
	return 0x0;
}

CombinationRecognizer* FubiCore::getUserDefinedCombinationRecognizer(const std::string& name)
{
	if (name.length() > 0)
	{
		vector<pair<string, CombinationRecognizer*> >::iterator iter = m_userDefinedCombinationRecognizers.begin();
		vector<pair<string, CombinationRecognizer*> >::iterator end = m_userDefinedCombinationRecognizers.end();
		for (int i = 0; iter != end; ++iter, ++i)
		{
			if (name == iter->first)
				return iter->second;
		}
	}
	return 0x0;
}

void FubiCore::enableCombinationRecognition(Combinations::Combination combinationID, unsigned int userID, bool enable)
{
	// Standard case: enable/disable a single recognizer
	if (combinationID < Combinations::NUM_COMBINATIONS)
	{
		FubiUser* user = getUser(userID);
		if (user)
		{
			// Found user
			user->enableCombinationRecognition(combinationID, enable);
		}
	}
	// Special case: enable/disable all recognizers (even the user defined ones!)
	else if (combinationID == Combinations::NUM_COMBINATIONS)
	{
		FubiUser* user = getUser(userID);
		if (user)
		{
			for (unsigned int i = 0; i < Combinations::NUM_COMBINATIONS; ++i)
				user->enableCombinationRecognition((Combinations::Combination)i, enable);

			std::vector<std::pair<std::string, CombinationRecognizer*> >::iterator iter;
			std::vector<std::pair<std::string, CombinationRecognizer*> >::iterator end = m_userDefinedCombinationRecognizers.end();
			for (iter = m_userDefinedCombinationRecognizers.begin(); iter != end; ++iter)
			{
				user->enableCombinationRecognition(iter->second, enable);
			}
		}
	}
}

void FubiCore::enableCombinationRecognition(const std::string& combinationName, unsigned int userID, bool enable)
{
	FubiUser* user = getUser(userID);
	if (user)
	{
		// Found user
		user->enableCombinationRecognition(getUserDefinedCombinationRecognizer(combinationName), enable);
	}
}

void FubiCore::enableCombinationRecognitionHand(const std::string& combinationName, unsigned int handID, bool enable)
{
	FubiHand* hand = getHand(handID);
	if (hand)
	{
		// Found hand
		hand->enableCombinationRecognition(getUserDefinedCombinationRecognizer(combinationName), enable);
	}
}

bool FubiCore::getAutoStartCombinationRecognition(Fubi::Combinations::Combination combinationID /*= Fubi::Combinations::NUM_COMBINATIONS*/)
{
	if (m_autoStartCombinationRecognizers[Fubi::Combinations::NUM_COMBINATIONS])
		return true;
	return m_autoStartCombinationRecognizers[combinationID];
}

void FubiCore::setAutoStartCombinationRecognition(bool enable, Combinations::Combination combinationID /*= Combinations::NUM_COMBINATIONS*/)
{
	if (combinationID < Combinations::NUM_COMBINATIONS)
	{
		m_autoStartCombinationRecognizers[combinationID] = enable;
		if (enable)
		{
			// Enable it for all current users
			for (unsigned int user = 0; user < m_numUsers; ++user)
			{
				m_users[user]->enableCombinationRecognition(combinationID, true);
			}
			// And the FubiPlayer user
			if (m_player->getUser())
				m_player->getUser()->enableCombinationRecognition(combinationID, true);
		}
	}
	else if (combinationID == Combinations::NUM_COMBINATIONS)
	{
		for (unsigned int i = 0; i < Combinations::NUM_COMBINATIONS; ++i)
			setAutoStartCombinationRecognition(enable, (Combinations::Combination)i);

		m_autoStartCombinationRecognizers[Combinations::NUM_COMBINATIONS] = enable;
		if (enable)
		{
			auto end = m_userDefinedCombinationRecognizers.end();
			for (auto iter = m_userDefinedCombinationRecognizers.begin(); iter != end; ++iter)
			{
				// Enable user defined recognizers for all current users
				for (unsigned int user = 0; user < m_numUsers; ++user)
				{
					m_users[user]->enableCombinationRecognition(iter->second, true);
				}
				// And the FubiPlayer user
				if (m_player->getUser())
					m_player->getUser()->enableCombinationRecognition(iter->second, true);
			}
		}
	}
}

FubiUser* FubiCore::getUser(unsigned int userId)
{
	if (userId == 0)
		return 0x0;

	if (userId == PlaybackUserID)
		return m_player->getUser();

	map<unsigned int, FubiUser*>::const_iterator iter = m_userIDToUsers.find(userId);
	if (iter != m_userIDToUsers.end())
	{
		return iter->second;
	}
	return 0x0;
}

FubiHand* FubiCore::getHand(unsigned int handID)
{
	if (handID == 0)
		return 0x0;

	if (handID == PlaybackHandID)
		return m_player->getHand();

	map<unsigned int, FubiHand*>::const_iterator iter = m_handIDToHands.find(handID);
	if (iter != m_handIDToHands.end())
	{
		return iter->second;
	}
	return 0;
}

bool FubiCore::updateHands()
{
	static unsigned int handIDs[MaxHands];
	bool somethingChanged = false;

	if (m_fingerSensor)
	{
		m_numHands = m_fingerSensor->getHandIDs(handIDs);

		// First sort our hand array according to the given id array
		for (unsigned int i = 0; i < m_numHands; ++i)
		{
			unsigned int id = handIDs[i];
			FubiHand* hand = m_hands[i];
			if (hand->m_id != id) // hand at the wrong place or still unknown
			{
				// Try to find the hand with the correct id (can only be later in the array as we already have corrected the ones before)
				// or at least find a free slot to move the hand that is currently here to
				unsigned int oldIndex = -1;
				unsigned int firstFreeIndex = -1;
				for (unsigned int j = i; j < MaxHands; ++j)
				{
					unsigned int tempId = m_hands[j]->m_id;
					if (tempId == id)
					{
						oldIndex = j;
						break;
					}
					if (firstFreeIndex == -1 && tempId == 0)
					{
						firstFreeIndex = j;
					}
				}
				if (oldIndex != -1)
				{
					// Found it, so swap him to here
					std::swap(m_hands[i], m_hands[oldIndex]);
					hand = m_hands[i];
				}
				else
				{
					// Not found so look what we can do with the one currently here...
					if (firstFreeIndex != -1)
					{
						// We have a free slot to which we can move the current hand
						std::swap(m_hands[i], m_hands[firstFreeIndex]);
					}
					else if (getHand(m_hands[i]->m_id) == m_hands[i])
					{
						// old hand still valid, but no other free slot available
						// so we have to drop him
						// Therefore, we remove the old map entry
						m_handIDToHands.erase(m_hands[i]->m_id);
					}
					// We now must have a usable hand slot at the current index
					hand = m_hands[i];
					// but we have to reset his old data
					hand->reset();
					// set correct id
					hand->m_id = id;
					// and set his map entry
					m_handIDToHands[id] = hand;
					// New hand so something changed
					somethingChanged = true;
				}
			}

			// Now the hand has to be in the correct slot and everything should be set correctly

			bool wasTracked = hand->m_isTracked;
			// get the tracking data from the sensor
			hand->updateFingerTrackingData(m_fingerSensor);

			if (!wasTracked && hand->m_isTracked)
			{
				// Hand tracking has started for this one!
				// Autostart combination detection
				for (unsigned int j = 0; j < getNumUserDefinedCombinationRecognizers(); ++j)
				{
					CombinationRecognizer* rec = getUserDefinedCombinationRecognizer(j);
					hand->enableCombinationRecognition(rec, false);
					if (getAutoStartCombinationRecognition(Fubi::Combinations::NUM_COMBINATIONS))
						hand->enableCombinationRecognition(rec, true);
				}
				// Started tracking so something changed
				somethingChanged = true;
			}
		}

		// invalidate all hands after the now corrected ones
		for (unsigned int i = m_numHands; i < MaxHands; ++i)
		{
			// invalid hand -> reset
			if (m_hands[i]->m_id != 0)
			{
				// hand lost
				somethingChanged = true;
				clearRecognitions(i, false);
				m_hands[i]->reset();
			}
		}
	}
	return somethingChanged;
}

bool FubiCore::updateUsers()
{
	static unsigned int userIDs[MaxUsers];
	bool somethingChanged = false;

	if (m_sensor)
	{
		// Get current user ids
		m_numUsers = m_sensor->getUserIDs(userIDs);

		// Only helpful if > m_numUsers
		unsigned int maxModifiedUserIndex = 0;

		// First sort our user array according to the given id array
		for (unsigned int i = 0; i < m_numUsers; ++i)
		{
			unsigned int id = userIDs[i];
			FubiUser* user = m_users[i];
			if (user->id() != id) // user at the wrong place or still unknown
			{
				// Try to find the user with the correct id (can only be later in the array as we already have corrected the ones before)
				// or at least find a free slot (at the current place or again later in the array)
				unsigned int oldIndex = -1;
				unsigned int firstFreeIndex = -1;
				for (unsigned int j = i; j < MaxUsers; ++j)
				{
					unsigned int tempId = m_users[j]->id();
					if (tempId == id)
					{
						oldIndex = j;
						break;
					}
					if (firstFreeIndex == -1 && tempId == 0)
					{
						firstFreeIndex = j;
					}
				}
				if (oldIndex != -1)
				{
					// Found him, so swap him to here
					std::swap(m_users[i], m_users[oldIndex]);
					user = m_users[i];
					if (oldIndex > maxModifiedUserIndex)
						maxModifiedUserIndex = oldIndex;
				}
				else
				{
					// Not found so look what we can do with the one currently here...
					if (firstFreeIndex != -1)
					{
						// We have a free slot to which we can move the current user
						std::swap(m_users[i], m_users[firstFreeIndex]);
						// set his map entry
						m_userIDToUsers[id] = user;
						if (firstFreeIndex > maxModifiedUserIndex)
							maxModifiedUserIndex = firstFreeIndex;
					}
					// We now must have a usable user slot at the current index
					user = m_users[i];
					// but we have to reset his old data
					user->reset();
					// but keep him in scene
					user->m_inScene = true;
					// set correct id
					user->m_id = id;
					// this is a new user so something changed
					somethingChanged = true;
				}
			}

			// Now the user has to be in the correct slot and everything should be set correctly

			bool wasTracked = user->m_isTracked;
			// get the tracking data from the sensor
			user->updateTrackingData(m_sensor);

			if (!wasTracked && user->m_isTracked)
			{
				// User tracking has started for this one!
				// Autostart posture combination detection
				for (unsigned int k = 0; k <Combinations::NUM_COMBINATIONS; ++k)
				{
					user->enableCombinationRecognition((Combinations::Combination)k,  false);
					if (getAutoStartCombinationRecognition((Combinations::Combination)k))
						user->enableCombinationRecognition((Combinations::Combination)k,  true);
				}
				// Special treatment for user definded posture combinations
				for (unsigned int j = 0; j < getNumUserDefinedCombinationRecognizers(); ++j)
				{
					CombinationRecognizer* rec = getUserDefinedCombinationRecognizer(j);
					user->enableCombinationRecognition(rec, false);
					if (getAutoStartCombinationRecognition(Fubi::Combinations::NUM_COMBINATIONS))
						user->enableCombinationRecognition(rec, true);
				}
				// Started tracking so something changed
				somethingChanged = true;
			}
		}

		// invalidate all users after the now corrected ones
		for (unsigned int i = m_numUsers; i <= maxModifiedUserIndex; ++i)
		{
			// invalid user -> reset
			if (m_users[i]->id() != 0)
			{
				// User lost
				somethingChanged = true;
				clearRecognitions(i, false);
				m_users[i]->reset();
			}
		}
	}
	return somethingChanged;
}

unsigned int FubiCore::addRecognizer(IGestureRecognizer* rec, const string& name, int atIndex /*= -1*/)
{
	if (!name.empty() && atIndex < 0)
		atIndex = getUserDefinedRecognizerIndex(name);

	// Add recognizer
	if (atIndex < 0 || (size_t)atIndex >= m_userDefinedRecognizers.size())
	{
		// As a new one at the end
		atIndex = (int)m_userDefinedRecognizers.size();
		m_userDefinedRecognizers.push_back(pair<string, IGestureRecognizer*>(name, rec));
	}
	else
	{
		// Replacing an old one
		delete m_userDefinedRecognizers[atIndex].second;
		m_userDefinedRecognizers[atIndex].first = name;
		m_userDefinedRecognizers[atIndex].second = rec;
	}
	// Return index
	return atIndex;
}

unsigned int FubiCore::addHiddenRecognizer(IGestureRecognizer* rec, const string& name, int atIndex /*= -1*/)
{
	if (!name.empty() && atIndex < 0)
		atIndex = getHiddenUserDefinedRecognizerIndex(name);

	// Add recognizer
	if (atIndex < 0 || (size_t)atIndex >= m_hiddenUserDefinedRecognizers.size())
	{
		// As a new one at the end
		atIndex = (int)m_hiddenUserDefinedRecognizers.size();
		m_hiddenUserDefinedRecognizers.push_back(pair<string, IGestureRecognizer*>(name, rec));
	}
	else
	{
		// Replacing an old one
		delete m_hiddenUserDefinedRecognizers[atIndex].second;
		m_hiddenUserDefinedRecognizers[atIndex].first = name;
		m_hiddenUserDefinedRecognizers[atIndex].second = rec;
	}
	// Return index
	return atIndex;
}

unsigned int FubiCore::addJointRelationRecognizer(SkeletonJoint::Joint joint, SkeletonJoint::Joint relJoint,
	const Vec3f& minValues /*= Vec3f(-Math::MaxFloat,-Math::MaxFloat, -Math::MaxFloat)*/,
	const Vec3f& maxValues /*= Vec3f(Math::MaxFloat, Math::MaxFloat, Math::MaxFloat)*/,
	float minDistance /*= 0*/,
	float maxDistance /*= Math::MaxFloat*/,
	Fubi::SkeletonJoint::Joint midJoint /*= Fubi::SkeletonJoint::NUM_JOINTS*/,
	const Fubi::Vec3f& midJointMinValues /*= Fubi::DefaultMinVec*/,
	const Fubi::Vec3f& midJointMaxValues /*= Fubi::DefaultMaxVec*/,
	float midJointMinDistance /*= 0*/,
	float midJointMaxDistance /*= Fubi::Math::MaxFloat*/,
	bool useLocalPositions /*= false*/,
	int atIndex /*=  -1*/,
	const char* name /*= 0*/,
	float minConfidence /*=-1*/,
	Fubi::BodyMeasurement::Measurement measuringUnit /*= Fubi::BodyMeasurement::NUM_MEASUREMENTS*/,
		bool useFilteredData /*= false*/)
{
	string sName;
	if (name != 0)
		sName = name;
	return addRecognizer(createPostureRecognizer(joint, relJoint, minValues, maxValues, minDistance, maxDistance,
		midJoint, midJointMinValues, midJointMaxValues, midJointMinDistance, midJointMaxDistance,
		useLocalPositions, minConfidence, measuringUnit, useFilteredData), sName);
}

unsigned int FubiCore::addJointOrientationRecognizer(SkeletonJoint::Joint joint,
		const Fubi::Vec3f& minValues /*= Fubi::Vec3f(-180.0f, -180.0f, -180.0f)*/, const Fubi::Vec3f& maxValues /*= Fubi::Vec3f(180.0f, 180.0f, 180.0f)*/,
		bool useLocalOrientations /*= true*/,
		int atIndex /*= -1*/,
		const char* name /*= 0*/,
		float minConfidence /*=-1*/,
		bool useFilteredData /*= false*/)
{
	string sName;
	if (name != 0)
		sName = name;
	return addRecognizer(createPostureRecognizer(joint, minValues, maxValues, useLocalOrientations, minConfidence, useFilteredData), sName, atIndex);
}

unsigned int FubiCore::addJointOrientationRecognizer(Fubi::SkeletonJoint::Joint joint,
		const Fubi::Vec3f& orientation, float maxAngleDifference,
		bool useLocalOrientations /*= true*/,
		int atIndex /*= -1*/,
		const char* name /*= 0*/,
		float minConfidence /*=-1*/,
		bool useFilteredData /*= false*/)
{
	string sName;
	if (name != 0)
		sName = name;
	return addRecognizer(createPostureRecognizer(joint, orientation, maxAngleDifference, useLocalOrientations, minConfidence, useFilteredData), sName, atIndex);
}

unsigned int FubiCore::addLinearMovementRecognizer(SkeletonJoint::Joint joint,	const Fubi::Vec3f& direction, float minVel, float maxVel /*= Fubi::Math::MaxFloat*/,
		bool useLocalPositions /*= false*/,
		int atIndex /*= -1*/,
		const char* name /*= 0*/,
		float minConfidence /*=-1*/,
		float maxAngleDiff /*= 45.0f*/,
		bool useOnlyCorrectDirectionComponent /*= true*/,
		bool useFilteredData /*= false*/)
{
	string sName;
	if (name != 0)
		sName = name;
	return addRecognizer(createMovementRecognizer(joint, direction, minVel, maxVel, useLocalPositions, minConfidence, maxAngleDiff, useOnlyCorrectDirectionComponent, useFilteredData), sName, atIndex);
}

unsigned int FubiCore::addLinearMovementRecognizer(SkeletonJoint::Joint joint, SkeletonJoint::Joint relJoint,
	const Vec3f& direction, float minVel, float maxVel /*= Fubi::Math::MaxFloat*/,
	bool useLocalPositions /*= false*/,
	int atIndex /*=  -1*/, const char* name /*= 0*/,
	float minConfidence /*=-1*/,
	float maxAngleDiff /*= 45.0f*/,
	bool useOnlyCorrectDirectionComponent /*= true*/,
		bool useFilteredData /*= false*/)
{
	string sName;
	if (name != 0)
		sName = name;
	return addRecognizer(createMovementRecognizer(joint, relJoint, direction, minVel, maxVel, useLocalPositions, minConfidence, maxAngleDiff, useOnlyCorrectDirectionComponent, useFilteredData), sName, atIndex);
}

unsigned int FubiCore::addLinearMovementRecognizer(SkeletonJoint::Joint joint,	const Fubi::Vec3f& direction, float minVel, float maxVel,
	float minLength, float maxLength /*= Fubi::Math::MaxFloat*/,
	Fubi::BodyMeasurement::Measurement measuringUnit /*= Fubi::BodyMeasurement::NUM_MEASUREMENTS*/,
	bool useLocalPositions /*= false*/,
	int atIndex /*= -1*/,
	const char* name /*= 0*/,
	float minConfidence /*=-1*/,
	float maxAngleDiff /*= 45.0f*/,
	bool useOnlyCorrectDirectionComponent /*= true*/,
	bool useFilteredData /*= false*/)
{
	string sName;
	if (name != 0)
		sName = name;
	return addRecognizer(createMovementRecognizer(joint, direction, minVel, maxVel, minLength, maxLength, measuringUnit, useLocalPositions, minConfidence, maxAngleDiff, useOnlyCorrectDirectionComponent, useFilteredData), sName, atIndex);
}

unsigned int FubiCore::addLinearMovementRecognizer(SkeletonJoint::Joint joint, SkeletonJoint::Joint relJoint,
	const Vec3f& direction, float minVel, float maxVel,
	float minLength, float maxLength /*= Fubi::Math::MaxFloat*/,
	Fubi::BodyMeasurement::Measurement measuringUnit /*= Fubi::BodyMeasurement::NUM_MEASUREMENTS*/,
	bool useLocalPositions /*= false*/,
	int atIndex /*=  -1*/, const char* name /*= 0*/,
	float minConfidence /*=-1*/,
	float maxAngleDiff /*= 45.0f*/,
	bool useOnlyCorrectDirectionComponent /*= true*/,
		bool useFilteredData /*= false*/)
{
	string sName;
	if (name != 0)
		sName = name;
	return addRecognizer(createMovementRecognizer(joint, relJoint, direction, minVel, maxVel, minLength, maxLength, measuringUnit, useLocalPositions, minConfidence, maxAngleDiff, useOnlyCorrectDirectionComponent, useFilteredData), sName, atIndex);
}

unsigned int FubiCore::addAngularMovementRecognizer(Fubi::SkeletonJoint::Joint joint, const Fubi::Vec3f& minAngularVelocity /*= Fubi::DefaultMinVec*/,
		const Fubi::Vec3f& maxAngularVelocity /*= Fubi::DefaultMaxVec*/,
		bool useLocalOrients /*= true*/,
		int atIndex /*= -1*/, const char* name /*= 0*/,
		float minConfidence /*= -1.0f*/, bool useFilteredData /*= false*/)
{
	string sName;
	if (name != 0)
		sName = name;
	return addRecognizer(createMovementRecognizer(joint, minAngularVelocity, maxAngularVelocity, useLocalOrients, minConfidence, useFilteredData), sName, atIndex);
}

unsigned int FubiCore::addFingerCountRecognizer(SkeletonJoint::Joint handJoint,
		unsigned int minFingers, unsigned int maxFingers,
		int atIndex /*= -1*/,
		const char* name /*= 0*/,
		float minConfidence /*=-1*/,
		bool useMedianCalculation /*= false*/,
		unsigned int medianWindowSize /*= 10*/,
		bool useFilteredData /*= false*/)
{
	string sName;
	if (name != 0)
		sName = name;
	return addRecognizer(createPostureRecognizer(handJoint, minFingers, maxFingers, minConfidence, useMedianCalculation, medianWindowSize, useFilteredData), sName, atIndex);
}

unsigned int FubiCore::addTemplateRecognizer(Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJoint::Joint relJoint,
	const char* trainingDataFile, int startFrame, int endFrame,
	float maxDistance,
	Fubi::DistanceMeasure::Measure distanceMeasure /*= Fubi::DistanceMeasure::Euclidean*/,
	float maxRotation /*= 45.0f*/,
	bool aspectInvariant /*= false*/,
	unsigned int ignoreAxes /*= Fubi::CoordinateAxis::NONE*/,
	bool useOrientations /*= false*/,
	bool useDTW /*= true*/,
	float maxWarpingFactor /*= 0.5f*/,
	Fubi::ResamplingTechnique::Technique resamplingTechnique /*= Fubi::ResamplingTechnique::None*/,
	int resampleSize /*= -1*/,
	bool searchBestInputLength /*= false*/,
	Fubi::StochasticModel::Model stochasticModel /*= Fubi::StochasticModel::GMR*/,
	unsigned int numGMRStates /*= 5*/,
	Fubi::BodyMeasurement::Measurement measuringUnit /*= Fubi::BodyMeasurement::NUM_MEASUREMENTS*/,
	int atIndex /*= -1*/, const char* name /*= 0*/,
	float minConfidence /*= -1.0f*/,
	bool useLocalTransformations /*= false*/,
	bool useFilteredData /*= false*/)
{
	string sName;
	if (name != 0)
		sName = name;
	// Parse skeleton data file
	std::deque<Fubi::TrackingData> skelData;
	BodyMeasurementDistance measures[BodyMeasurement::NUM_MEASUREMENTS];
	deque<pair<int, int>> fingerCounts;
	int startMaker, endMarker;
	bool isUser;
	if (FubiXMLParser::parseSkeletonFile(skelData, measures, fingerCounts, startMaker, endMarker, isUser, trainingDataFile, startFrame, endFrame))
	{
		// Create recognizer
		IGestureRecognizer* rec;
		if (isUser)
			rec = createGestureRecognizer(std::vector<SkeletonJoint::Joint>(1, joint), std::vector<SkeletonJoint::Joint>(1, relJoint),
			std::vector<std::deque<Fubi::TrackingData>>(1, skelData), maxDistance, distanceMeasure, maxRotation, aspectInvariant, 
				ignoreAxes, useOrientations, useDTW, maxWarpingFactor, resamplingTechnique, resampleSize, searchBestInputLength,
				stochasticModel, numGMRStates, measures, measuringUnit,	minConfidence, useLocalTransformations, useFilteredData);
		else
			rec = createGestureRecognizer(std::vector<SkeletonHandJoint::Joint>(1, (SkeletonHandJoint::Joint)joint), 
			std::vector<SkeletonHandJoint::Joint>(1, (SkeletonHandJoint::Joint)relJoint), 
			std::vector<std::deque<Fubi::TrackingData>>(1, skelData), maxDistance, distanceMeasure, maxRotation, 
			aspectInvariant, ignoreAxes, useOrientations, useDTW, maxWarpingFactor, resamplingTechnique, resampleSize, searchBestInputLength,
			stochasticModel, numGMRStates, minConfidence, useLocalTransformations, useFilteredData);

		// Add the recognizer
		atIndex = addRecognizer(rec, sName, atIndex);

		// Update tracking history length
		updateTrackingHistoryLength((unsigned int)skelData.size());

		// Return index
		return atIndex;
	}
	return -1;
}

unsigned int FubiCore::addTemplateRecognizer(const std::string& xmlDefinition, int atIndex /*= -1*/)
{
	string name;
	// Create recognizer
	unsigned int requiredHistoryLength = 0;
	IGestureRecognizer* rec = FubiXMLParser::parseTemplateRecognizer(xmlDefinition, requiredHistoryLength, name);
	if (rec)
	{
		// Add the recognizer
		atIndex = addRecognizer(rec, name, atIndex);

		// Update tracking history length
		updateTrackingHistoryLength(requiredHistoryLength);

		// Return index
		return atIndex;
	}
	return -1;
}

void FubiCore::updateTrackingHistoryLength(unsigned int requiredTrackingHistoryLength)
{
	for (unsigned int user = 0; user < Fubi::MaxUsers; ++user)
	{
		// Set requiredTrackingHistoryLength for all uers
		if (m_users[user]->m_maxTrackingHistoryLength < requiredTrackingHistoryLength)
			m_users[user]->m_maxTrackingHistoryLength = requiredTrackingHistoryLength;
	}
	for (unsigned int hand = 0; hand < Fubi::MaxHands; ++hand)
	{
		// Set requiredTrackingHistoryLength for all hands
		if (m_hands[hand]->m_maxTrackingHistoryLength < requiredTrackingHistoryLength)
			m_hands[hand]->m_maxTrackingHistoryLength = requiredTrackingHistoryLength;
	}
	// Set requiredTrackingHistoryLength for the FubiPlayer user and hand
	m_player->updateTrackingHistoryLength(requiredTrackingHistoryLength);
}

unsigned short FubiCore::getCurrentUsers(FubiUser*** userContainer)
{
	if (userContainer != 0)
	{
		*userContainer = m_users;
	}
	return m_numUsers;
}

bool FubiCore::addCombinationRecognizer(const std::string& xmlDefinition)
{
	CombinationRecognizer* rec = FubiXMLParser::parseCombinationRecognizer(xmlDefinition);
	if (rec)
	{
		addCombinationRecognizer(rec);
		return true;
	}
	return false;
}

void FubiCore::addCombinationRecognizer(CombinationRecognizer* rec)
{
	if (rec)
	{
		int index = getUserDefinedCombinationRecognizerIndex(rec->getName());
		// Add recognizer
		if (index < 0)
		{
			// As a new one at the end
			m_userDefinedCombinationRecognizers.push_back(pair<string, CombinationRecognizer*>(rec->getName(), rec));
		}
		else
		{
			// Replacing an old one
			delete m_userDefinedCombinationRecognizers[index].second;
			m_userDefinedCombinationRecognizers[index].first = rec->getName();
			m_userDefinedCombinationRecognizers[index].second = rec;
		}
		
		if (getAutoStartCombinationRecognition(Fubi::Combinations::NUM_COMBINATIONS))
		{
			// Enable new combination for all current users
			for (unsigned int user = 0; user < m_numUsers; ++user)
			{
				m_users[user]->enableCombinationRecognition(rec, true);
			}
			// Enable new combination for the FubiPlayer users
			if (m_player->getUser())
			{
				m_player->getUser()->enableCombinationRecognition(rec, true);
			}
		}
	}
}

void FubiCore::getDepthResolution(int& width, int& height)
{
	if (m_sensor)
	{
		width = m_sensor->getStreamOptions(ImageType::Depth).m_width;
		height = m_sensor->getStreamOptions(ImageType::Depth).m_height;
	}
	else
	{
		width = -1;
		height = -1;
	}
}
void FubiCore::getRgbResolution(int& width, int& height)
{
	if (m_sensor)
	{
		width = m_sensor->getStreamOptions(ImageType::Color).m_width;
		height = m_sensor->getStreamOptions(ImageType::Color).m_height;
	}
	else
	{
		width = -1;
		height = -1;
	}
}
void FubiCore::getIRResolution(int& width, int& height)
{
	if (m_sensor)
	{
		width = m_sensor->getStreamOptions(ImageType::IR).m_width;
		height = m_sensor->getStreamOptions(ImageType::IR).m_height;
	}
	else
	{
		width = -1;
		height = -1;
	}
}

unsigned int FubiCore::getClosestUserID()
{
	FubiUser* user = getClosestUser();
	if (user)
		return user->id();
	return 0;
}

FubiUser* FubiCore::getClosestUser()
{
	std::deque<FubiUser*> closestUsers = getClosestUsers();
	if (!closestUsers.empty())
	{
		// Take the closest tracked user for posture rec
		return closestUsers.front();
	}
	return 0x0;
}

std::deque<unsigned int> FubiCore::getClosestUserIDs(int maxNumUsers /*= -1*/)
{
	std::deque<unsigned int> closestUserIDs;
	// Get closest users
	std::deque<FubiUser*> closestUsers = getClosestUsers(maxNumUsers);
	// Copy their ids
	std::deque<FubiUser*>::iterator iter;
	std::deque<FubiUser*>::iterator end = closestUsers.end();
	for (iter = closestUsers.begin(); iter != end; ++iter)
	{
		// Take the closest tracked user for posture rec
		closestUserIDs.push_back((*iter)->id());
	}
	return closestUserIDs;
}

std::deque<FubiUser*> FubiCore::getClosestUsers(int maxNumUsers /*= -1*/)
{
	// Copy array into vector
	std::deque<FubiUser*> closestUsers;
	if (maxNumUsers != 0)
	{
		closestUsers.insert(closestUsers.begin(), m_users, m_users + m_numUsers);

		// Sort vector with special operator according to their distance in the x-z plane
		std::sort(closestUsers.begin(), closestUsers.end(), FubiUser::closerToSensor);

		if (maxNumUsers > 0)
		{
			// Now remove users with largest distance to meet the max user criteria
			while(closestUsers.size() > (unsigned)maxNumUsers)
				closestUsers.pop_back();
			// And sort the rest additionally from left to right
			std::sort(closestUsers.begin(), closestUsers.end(), FubiUser::moreLeft);
		}
	}
	return closestUsers;
}

FubiHand* FubiCore::getClosestHand(Fubi::Vec3f pos /*= Fubi::NullVec*/, Fubi::SkeletonJoint::Joint handType /*= Fubi::SkeletonJoint::NUM_JOINTS*/, bool useFilteredData /*= true*/)
{
	FubiHand* hand = 0x0;
	float minDist2 = Math::MaxFloat;
	for (unsigned short i = 0; i < m_numHands; ++i)
	{
		if (m_hands[i]->isTracked())
		{
			// Skip hands with definetly wrong type
			if (handType != SkeletonJoint::NUM_JOINTS && handType != m_hands[i]->getHandType())
				continue;
			SkeletonJointPosition handPos;
			if (useFilteredData)
				handPos = m_hands[i]->currentFilteredTrackingData()->jointPositions[SkeletonHandJoint::PALM];
			else
				handPos = m_hands[i]->currentTrackingData()->jointPositions[SkeletonHandJoint::PALM];
			//if (handPos.m_confidence > 0.25f)
			{
				float dist2 = (pos-handPos.m_position).length2();
				if (dist2 < minDist2)
				{
					minDist2 = dist2;
					hand = m_hands[i];
				}
			}
		}
	}
	return hand;
}

bool FubiCore::getClosestUserHandJoint(Fubi::Vec3f pos, FubiUser** user, Fubi::SkeletonJoint::Joint& joint, bool useFilteredData /*= true*/)
{
	float minDist2 = Math::MaxFloat;
	for (unsigned short i = 0; i < m_numUsers; ++i)
	{
		if (m_users[i]->m_isTracked)
		{
			SkeletonJointPosition userPos;
			if (useFilteredData)
				userPos = m_users[i]->currentFilteredTrackingData()->jointPositions[SkeletonJoint::RIGHT_HAND];
			else
				userPos = m_users[i]->currentTrackingData()->jointPositions[SkeletonJoint::RIGHT_HAND];
			if (userPos.m_confidence > 0.25f)
			{
				float dist2 = (pos-userPos.m_position).length2();
				if (dist2 < minDist2)
				{
					minDist2 = dist2;
					*user = m_users[i];
					joint = SkeletonJoint::RIGHT_HAND;
				}
			}
			if (useFilteredData)
				userPos = m_users[i]->currentFilteredTrackingData()->jointPositions[SkeletonJoint::LEFT_HAND];
			else
				userPos = m_users[i]->currentTrackingData()->jointPositions[SkeletonJoint::LEFT_HAND];
			if (userPos.m_confidence > 0.25f)
			{
				float dist2 = (pos-userPos.m_position).length2();
				if (dist2 < minDist2)
				{
					minDist2 = dist2;
					*user = m_users[i];
					joint = SkeletonJoint::LEFT_HAND;
				}
			}
		}
	}
	return minDist2 < Math::MaxFloat;
}


void FubiCore::clearUserDefinedRecognizers()
{
	for (unsigned int i = 0; i < Fubi::MaxUsers; ++i)
	{
		m_users[i]->clearUserDefinedCombinationRecognizers();
	}
	updateTrackingHistoryLength(2);

	vector<pair<string, CombinationRecognizer*> >::iterator iter1;
	vector<pair<string, CombinationRecognizer*> >::iterator end1 = m_userDefinedCombinationRecognizers.end();
	for (iter1 = m_userDefinedCombinationRecognizers.begin(); iter1 != end1; ++iter1)
	{
		delete iter1->second;
	}
	m_userDefinedCombinationRecognizers.clear();

	vector<pair<string, IGestureRecognizer*> >::iterator iter;
	vector<pair<string, IGestureRecognizer*> >::iterator end = m_userDefinedRecognizers.end();
	for (iter = m_userDefinedRecognizers.begin(); iter != end; ++iter)
	{
		delete iter->second;
	}
	m_userDefinedRecognizers.clear();

	vector<pair<string, IGestureRecognizer*> >::iterator iter2;
	vector<pair<string, IGestureRecognizer*> >::iterator end2 = m_hiddenUserDefinedRecognizers.end();
	for (iter2 = m_hiddenUserDefinedRecognizers.begin(); iter2 != end2; ++iter2)
	{
		delete iter2->second;
	}
	m_hiddenUserDefinedRecognizers.clear();
}

void FubiCore::updateTrackingData(unsigned int userId, float* skeleton, double timeStamp /*= -1*/)
{
	// First check if this is a new user or if we have to create one
	map<unsigned int, FubiUser*>::iterator iter = m_userIDToUsers.find(userId);
	int index = -1;
	FubiUser* user = 0x0;
	if (iter == m_userIDToUsers.end())
	{
		// new User, new entry
		index = m_numUsers;
		user = m_userIDToUsers[userId] = m_users[index];
		user->m_id = userId;
		m_numUsers++;

		// Init the user info
		user->m_inScene = true;
		user->m_isTracked = true;

		// Autostart posture combination detection
		for (unsigned int i = 0; i < Combinations::NUM_COMBINATIONS; ++i)
		{
			user->enableCombinationRecognition((Combinations::Combination)i, getAutoStartCombinationRecognition((Combinations::Combination)i));
		}
		// Special treatment for user defined posture combinations
		if (getAutoStartCombinationRecognition(Fubi::Combinations::NUM_COMBINATIONS))
		{
			for (unsigned int j = 0; j < getNumUserDefinedCombinationRecognizers(); ++j)
			{
				user->enableCombinationRecognition(getUserDefinedCombinationRecognizer(j), true);
			}
		}
	}
	else
		user = iter->second;


	// Now set the new tracking info for the user and let him do the rest of the updates
	user->addNewTrackingData(skeleton, timeStamp);
}


Fubi::Vec3f FubiCore::convertCoordinates(const Fubi::Vec3f& inputCoords, Fubi::CoordinateType::Type inputType, Fubi::CoordinateType::Type outputType)
{
	// Some default values
	const static double realWorldXtoZ = tan(1.0144686707507438/2.0) * 2.0;
	const static double realWorldYtoZ = tan(0.78980943449644714/2.0) * 2.0;
	const static float coeffX = (float) (640.0 / realWorldXtoZ);
	const static float coeffY = (float) (480.0 / realWorldYtoZ);
	const static int nHalfXres = 320;
	const static int nHalfYres = 240;

	Vec3f ret(Math::NO_INIT);

	if (inputType == outputType)
	{
		// Special case: no conversion just copy
		ret = inputCoords;
	}
	else if (m_sensor)
	{
		// Default case: the sensor does the conversion
		ret = m_sensor->convertCoordinates(inputCoords, inputType, outputType);
	}
	// Some special cases to provide a default conversion if no sensor is present
	else if (inputType == CoordinateType::REAL_WORLD)
	{
		// Conversion from real world to default (depth) resolution
		ret.x = coeffX * inputCoords.x / clamp(abs(inputCoords.z), 1.0f, Math::MaxFloat) + nHalfXres;
		ret.y = nHalfYres - coeffY * inputCoords.y / clamp(abs(inputCoords.z), 1.0f, Math::MaxFloat);
		ret.z = inputCoords.z;
	}
	else if (outputType == CoordinateType::REAL_WORLD)
	{
		// Conversion from default (depth) resolution to real world
		ret.x = (inputCoords.x - nHalfXres) * abs(inputCoords.z) / coeffX;
		ret.y = (inputCoords.y - nHalfYres) * abs(inputCoords.z) / -coeffY;
		ret.z = inputCoords.z;
	}
	else
	{
		// Conversion between input stream default to copy
		ret = inputCoords;
	}

	return ret;
}

void FubiCore::resetTracking()
{
	if (m_sensor)
	{
		for (unsigned short i = 0; i < m_numUsers; ++i)
		{
			m_sensor->resetTracking(m_users[i]->id());
		}
	}
}

bool FubiCore::initFingerSensor(FingerSensorType::Type type, const Fubi::Vec3f& offsetPos)
{
	delete m_fingerSensor;
	m_fingerSensor = 0x0;
	bool success = false;

	for (unsigned int i = 0; i < MaxHands; ++i)
	{
		m_hands[i]->m_id = 0;
		m_hands[i]->m_isTracked = false;
	}
	m_numHands = 0;
	m_handIDToHands.clear();

	if (type == FingerSensorType::LEAP)
	{
#ifdef FUBI_USE_LEAP
		m_fingerSensor = new FubiLeapSensor();
		success = m_fingerSensor->init(offsetPos);
#else
		Fubi_logErr("Leap sensor is not activated\n -Did you forget to uncomment the FUBI_USE_LEAP define in the FubiConfig.h?\n");
#endif
	}
	else if (type == FingerSensorType::NONE)
	{
		Fubi_logInfo("FubiCore: Current finger sensor deactivated!\n");
		success = true;
	}

	if (!success)
	{
		delete m_fingerSensor;
		m_fingerSensor = 0x0;
	}

	return success;
}

Fubi::RecognizerTarget::Target FubiCore::getCombinationRecognizerTargetSkeleton(const char* recognizerName)
{
	CombinationRecognizer* rec = getUserDefinedCombinationRecognizer(recognizerName);
	if (rec)
	{
		return rec->getRecognizerTarget();
	}
	return RecognizerTarget::INVALID;
}

Fubi::RecognizerTarget::Target FubiCore::getRecognizerTargetSkeleton(const char* recognizerName)
{
	int recognizerIndex = getUserDefinedRecognizerIndex(recognizerName);
	if (recognizerIndex >= 0)
	{
		return m_userDefinedRecognizers[recognizerIndex].second->m_targetSkeletonType;
	}
	return RecognizerTarget::INVALID;
}

const char* FubiCore::getCombinationRecognitionStateMetaInfo(const char* recognizerName, unsigned int stateIndex, const char* propertyName)
{
	if (propertyName && recognizerName)
	{
		CombinationRecognizer* rec = getUserDefinedCombinationRecognizer(recognizerName);
		if (rec)
		{
			const std::string& value = rec->getStateMetaInfo(stateIndex, propertyName);
			if (value.length() > 0)
				return value.c_str();
		}
	}
	return 0x0;
}


void FubiCore::enableFingerTracking(unsigned int userID, bool leftHand, bool rightHand, bool useConvexityDefectMethod /*= false*/)
{
	FubiUser* user = getUser(userID);
	if (user)
	{
		user->enableFingerTrackingWithConvexityDefectMethod(useConvexityDefectMethod);
		user->enableFingerTracking(leftHand, true);
		user->enableFingerTracking(rightHand, false);
	}
}

int FubiCore::getFingerCount(unsigned int userID, bool leftHand /*= false*/,
	bool getMedianOfLastFrames /*= true*/, unsigned int medianWindowSize /*= 10*/,
	bool useOldConvexityDefectMethod /*= false*/)
{
	FubiUser* user = getUser(userID);
	if (user)
		return user->getFingerCount(leftHand, getMedianOfLastFrames, medianWindowSize, useOldConvexityDefectMethod);
	return -1;
}

int FubiCore::getCurrentCombinationRecognitionState(const char* recognizerName, unsigned int userID, unsigned int& numStates, bool& isInterrupted, bool& isInTransition)
{
	FubiUser* user = getUser(userID);
	if (user)
	{
		// Found the user
		return user->getCurrentRecognitionState(recognizerName, numStates, isInterrupted, isInTransition);
	}
	return -2;
}

Fubi::RecognitionResult::Result FubiCore::getCombinationRecognitionProgressOn(Combinations::Combination combinationID, unsigned int userID,
		std::vector<Fubi::TrackingData>* userStates /*= 0x0*/, bool restart /*= true*/, bool returnFilteredData /*= false*/,
		Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	if (combinationID < Combinations::NUM_COMBINATIONS)
	{
		FubiUser* user = getUser(userID);
		if (user)
		{
			// Found user
			return user->getRecognitionProgress(combinationID, userStates, restart, returnFilteredData, correctionHint);
		}
	}
	return Fubi::RecognitionResult::NOT_RECOGNIZED;
}

Fubi::RecognitionResult::Result FubiCore::getCombinationRecognitionProgressOn(const char* recognizerName, unsigned int userID,
		std::vector<Fubi::TrackingData>* userStates /*= 0x0*/, bool restart /*= true*/, bool returnFilteredData /*= false*/,
		Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	FubiUser* user = getUser(userID);
	if (user)
	{
		// Found the user
		return user->getRecognitionProgress(recognizerName, userStates, restart, returnFilteredData, correctionHint);
	}
	return Fubi::RecognitionResult::NOT_RECOGNIZED;
}

Fubi::Plane FubiCore::getFloor()
{
	if (m_sensor)
		return m_sensor->getFloor();
	return Fubi::Plane();
}

bool FubiCore::startRecordingSkeletonData(const char* fileName, unsigned int targetID, bool isHand, bool useRawData, bool useFilteredData)
{
	if (isHand)
	{
		FubiHand* hand = getHand(targetID);
		if (fileName != 0x0 && hand != 0x0)
			return m_recorder->startRecording(fileName, hand, useRawData, useFilteredData);
	}
	else
	{
		FubiUser* user = getUser(targetID);
		if (fileName != 0x0 && user != 0x0)
			return m_recorder->startRecording(fileName, user, useRawData, useFilteredData);
	}
	return false;
}

bool FubiCore::isRecordingSkeletonData(int* currentFrameID /*= 0x0*/)
{
	return m_recorder->isRecording(currentFrameID);
}

void FubiCore::stopRecordingSkeletonData()
{
	m_recorder->stopRecording();
}

int FubiCore::loadRecordedSkeletonData(const char* fileName)
{
	return m_player->loadRecording(fileName);
}

void FubiCore::setPlaybackMarkers(int currentFrame, int startFrame /*= 0*/, int endFrame /*= -1*/)
{
	m_player->setMarkers(currentFrame, startFrame, endFrame);
}

void FubiCore::getPlaybackMarkers(int& currentFrame, int& startFrame, int& endFrame)
{
	m_player->getMarkers(currentFrame, startFrame, endFrame);
}

void FubiCore::startPlayingSkeletonData(bool loop /*= false*/)
{
	m_player->startPlaying(loop);
}

bool FubiCore::isPlayingSkeletonData(int* currentFrameID /*= 0x0*/, bool* isPaused /*= 0x0*/)
{
	return m_player->isPlaying(currentFrameID, isPaused);
}

void FubiCore::stopPlayingRecordedSkeletonData()
{
	m_player->stopPlaying();
}

void FubiCore::pausePlayingRecordedSkeletonData()
{
	m_player->pausePlaying();
}

double FubiCore::getPlaybackDuration()
{
	return m_player->getPlaybackDuration();
}

bool FubiCore::trimPlaybackFileToMarkers()
{
	return m_player->trimFileToMarkers();
}

void FubiCore::setRecognitionCallbacks(const std::function<void(const char*, unsigned int, bool, Fubi::RecognizerType::Type)>& startCallback, const std::function<void(const char*, unsigned int, bool, Fubi::RecognizerType::Type)>& endCallback)
{
	m_recognitionStartCallback = startCallback;
	m_recognitionEndCallback = endCallback;
}

const std::vector<Fubi::TemplateData>& FubiCore::getTemplateTrainingData(const char* recognizerName)
{
	int recognizerIndex = getUserDefinedRecognizerIndex(recognizerName);
	if (recognizerIndex >= 0)
		return getTemplateTrainingData(recognizerIndex);
	return Fubi::EmptyTemplateDataVec;
}

const std::vector<Fubi::TemplateData>& FubiCore::getTemplateTrainingData(unsigned int recognizerIndex)
{
	if (recognizerIndex < m_userDefinedRecognizers.size())
	{
		TemplateRecognizer* tempRec = dynamic_cast<TemplateRecognizer*>(m_userDefinedRecognizers[recognizerIndex].second);
		if (tempRec)
			return tempRec->getTrainingData();
	}
	return Fubi::EmptyTemplateDataVec;
}
