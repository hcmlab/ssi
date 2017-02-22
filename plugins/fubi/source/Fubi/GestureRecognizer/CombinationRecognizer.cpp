// ****************************************************************************************
//
// Fubi Combination Recognizer
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler 
// 
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
// 
// ****************************************************************************************

#include "CombinationRecognizer.h"

#include "../Fubi.h"
#include "../FubiConfig.h"

#include <algorithm>

using namespace Fubi;

const std::vector<IGestureRecognizer*> RecognitionState::s_emptyRecVec = std::vector<IGestureRecognizer*>();
const std::map<std::string, std::string> RecognitionState::s_emptyMetaInfo = std::map<std::string, std::string>();

CombinationRecognizer::CombinationRecognizer(const FubiUser* user, Combinations::Combination gestureID)
	: m_currentState(-1), m_stateStart(0), m_minDurationPassed(false), m_user(user), m_hand(0x0), m_running(false), m_gestureID(gestureID),
	m_interruptionStart(0), m_interrupted(false), m_recognition(RecognitionResult::NOT_RECOGNIZED), m_waitUntilLastStateRecognizersStop(false), 
	m_targetSkeletonType(RecognizerTarget::INVALID)
{
	m_name = getCombinationName(gestureID);
}

CombinationRecognizer::CombinationRecognizer(const std::string& recognizerName)	// Only for creating a template recognizer, will not work until m_user is set
	: m_currentState(-1), m_stateStart(0), m_minDurationPassed(false), m_user(0x0), m_hand(0x0), m_running(false), 
	m_gestureID(Combinations::NUM_COMBINATIONS), m_interruptionStart(0), m_interrupted(false), m_recognition(RecognitionResult::NOT_RECOGNIZED), 
	m_name(recognizerName), m_waitUntilLastStateRecognizersStop(false), m_targetSkeletonType(RecognizerTarget::INVALID)
{
}

CombinationRecognizer::CombinationRecognizer(const CombinationRecognizer& other)
	: m_currentState(-1), m_stateStart(0), m_minDurationPassed(false), m_user(other.m_user), m_hand(other.m_hand), m_running(false), m_gestureID(other.m_gestureID),
	m_interruptionStart(0), m_interrupted(false), m_recognition(RecognitionResult::NOT_RECOGNIZED), m_name(other.m_name), m_waitUntilLastStateRecognizersStop(other.m_waitUntilLastStateRecognizersStop), 
	m_targetSkeletonType(other.m_targetSkeletonType)
{
	for (std::vector<RecognitionState>::const_iterator iter = other.m_RecognitionStates.begin(); iter != other.m_RecognitionStates.end(); ++iter)
	{
		std::vector<IGestureRecognizer*> recognizers;
		for (RecognitionState::GestureIter iter2 = iter->m_gestures.begin(); iter2 != iter->m_gestures.end(); ++iter2)
		{
			recognizers.push_back((*iter2)->clone());
		}
		std::vector<IGestureRecognizer*> notRecognizers;
		for (RecognitionState::GestureIter iter3 = iter->m_notGestures.begin(); iter3 != iter->m_notGestures.end(); ++iter3)
		{
			notRecognizers.push_back((*iter3)->clone());
		}
		std::vector<IGestureRecognizer*> alternativeRecognizers;
		for (RecognitionState::GestureIter iter4 = iter->m_alternativeGestures.begin(); iter4 != iter->m_alternativeGestures.end(); ++iter4)
		{
			alternativeRecognizers.push_back((*iter4)->clone());
		}
		std::vector<IGestureRecognizer*> alternativeNotRecognizers;
		for (RecognitionState::GestureIter iter5 = iter->m_alternativeNotGestures.begin(); iter5 != iter->m_alternativeNotGestures.end(); ++iter5)
		{
			alternativeNotRecognizers.push_back((*iter5)->clone());
		}
		m_RecognitionStates.push_back(RecognitionState(recognizers, notRecognizers, iter->m_minDuration, iter->m_maxDuration, iter->m_timeForTransition, iter->m_maxInterruptionTime, iter->m_noInterrruptionBeforeMinDuration, alternativeRecognizers, alternativeNotRecognizers, iter->m_restartOnFail, iter->m_MetaInfo));
	}
}

CombinationRecognizer::~CombinationRecognizer()
{
	stop();

	for (std::vector<RecognitionState>::iterator iter = m_RecognitionStates.begin(); iter != m_RecognitionStates.end(); ++iter)
	{
		for (RecognitionState::GestureIter iter2 = iter->m_gestures.begin(); iter2 != iter->m_gestures.end(); ++iter2)
		{
			delete (*iter2);
		}
		iter->m_gestures.clear();

		for (RecognitionState::GestureIter iter3 = iter->m_notGestures.begin(); iter3 != iter->m_notGestures.end(); ++iter3)
		{
			delete (*iter3);
		}
		iter->m_alternativeNotGestures.clear();
		for (RecognitionState::GestureIter iter4 = iter->m_alternativeGestures.begin(); iter4 != iter->m_alternativeGestures.end(); ++iter4)
		{
			delete (*iter4);
		}
		iter->m_alternativeGestures.clear();

		for (RecognitionState::GestureIter iter5 = iter->m_alternativeNotGestures.begin(); iter5 != iter->m_alternativeNotGestures.end(); ++iter5)
		{
			delete (*iter5);
		}
		iter->m_alternativeNotGestures.clear();
	}
}

void CombinationRecognizer::start()
{
	if ((m_user || m_hand) && !m_running && m_RecognitionStates.size() > 0)
	{
		m_running = true;
		m_currentState = -1;
		m_minDurationPassed = true;
		m_interrupted = false;
		m_recognition = RecognitionResult::NOT_RECOGNIZED;
		m_trackingStates.clear();
		m_filteredTrackingStates.clear();
	}
}

void CombinationRecognizer::stop()
{
	m_running = false;
	m_currentState = -1;
	m_minDurationPassed = false;
	m_interrupted = false;
}

Fubi::RecognitionResult::Result CombinationRecognizer::getRecognitionProgress(std::vector<Fubi::TrackingData>* trackingStates,
	bool restart, bool returnFilteredData, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	RecognitionResult::Result recognized = m_recognition;

	if (recognized == RecognitionResult::RECOGNIZED)
	{
		if (trackingStates != 0x0)
		{
			if (returnFilteredData)
				trackingStates->insert(trackingStates->begin(), m_filteredTrackingStates.begin(), m_filteredTrackingStates.end());
			else
				trackingStates->insert(trackingStates->begin(), m_trackingStates.begin(), m_trackingStates.end());
		}

		if (restart)
			this->restart();
		else
			m_recognition = RecognitionResult::NOT_RECOGNIZED;
	}
	else if (isWaitingForLastStateFinish())	// Special case: already recognized, but waiting for the last state to finish because of set flag
	{
		if (trackingStates != 0x0)
		{
			if (returnFilteredData)
				trackingStates->insert(trackingStates->begin(), m_filteredTrackingStates.begin(), m_filteredTrackingStates.end());
			else
				trackingStates->insert(trackingStates->begin(), m_trackingStates.begin(), m_trackingStates.end());
		}
		return Fubi::RecognitionResult::WAITING_FOR_LAST_STATE_TO_FINISH;
	}

	if (recognized == RecognitionResult::NOT_RECOGNIZED && correctionHint)
	{
		*correctionHint = m_currentCorrectionHint;
	}

	return recognized;
}

Fubi::RecognitionResult::Result CombinationRecognizer::checkRecognizer(IGestureRecognizer* rec, int stateIndex, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	Fubi::RecognitionResult::Result result = Fubi::RecognitionResult::TRACKING_ERROR;
	if (m_user)
	{
		if (rec->useHistory())
		{
			Fubi::TrackingData *histData = 0x0, *histFilteredData = 0x0;
			if (stateIndex == m_currentState && m_trackingStates.size() > 0 && m_filteredTrackingStates.size() > 0)
			{
				// We need the data when the current state has started (before min duration has been reached)
				if (m_minDurationPassed && m_trackingStates.size() > 1 && m_filteredTrackingStates.size() > 1)
				{
					histData = &(*(m_trackingStates.end()-2));
					histFilteredData = &(*(m_filteredTrackingStates.end()-2));
				}
				else
				{
					histData = &(m_trackingStates.back());
					histFilteredData = &(m_filteredTrackingStates.back());
				}
			}
			if (rec->m_targetSkeletonType == RecognizerTarget::LEFT_HAND && m_user->m_assignedLeftHandId != 0)
				result = rec->recognizeWithHistory(getHand(m_user->m_assignedLeftHandId), histData, histFilteredData, correctionHint);
			else if (rec->m_targetSkeletonType == RecognizerTarget::RIGHT_HAND && m_user->m_assignedRightHandId != 0)
				result = rec->recognizeWithHistory(getHand(m_user->m_assignedRightHandId), histData, histFilteredData, correctionHint);
			else if (rec->m_targetSkeletonType == RecognizerTarget::BOTH_HANDS)
			{
				RecognitionResult::Result res1 = RecognitionResult::TRACKING_ERROR, res2 = RecognitionResult::TRACKING_ERROR;
				if (m_user->m_assignedLeftHandId != 0)
					res1 = rec->recognizeWithHistory(getHand(m_user->m_assignedLeftHandId), histData, histFilteredData, correctionHint);
				if (m_user->m_assignedRightHandId != 0)
					res2 = rec->recognizeWithHistory(getHand(m_user->m_assignedRightHandId), histData, histFilteredData, correctionHint);
				if (res1 != RecognitionResult::TRACKING_ERROR && res2 != RecognitionResult::TRACKING_ERROR)
					result = std::max(res1, res2);
				else if (res1 != RecognitionResult::TRACKING_ERROR)
					result = res1;
				else if (res2 != RecognitionResult::TRACKING_ERROR)
					result = res2;
				else
					result = rec->recognizeWithHistory(m_user, histData, histFilteredData, correctionHint);
			}
			else
				result = rec->recognizeWithHistory(m_user, histData, histFilteredData, correctionHint);
		}
		else
		{
			if (rec->m_targetSkeletonType == RecognizerTarget::LEFT_HAND && m_user->m_assignedLeftHandId != 0)
				result = rec->recognizeOn(getHand(m_user->m_assignedLeftHandId), correctionHint);
			else if (rec->m_targetSkeletonType == RecognizerTarget::RIGHT_HAND && m_user->m_assignedRightHandId != 0)
				result = rec->recognizeOn(getHand(m_user->m_assignedRightHandId), correctionHint);
			else if (rec->m_targetSkeletonType == RecognizerTarget::BOTH_HANDS)
			{
				RecognitionResult::Result res1 = RecognitionResult::TRACKING_ERROR, res2 = RecognitionResult::TRACKING_ERROR;
				if (m_user->m_assignedLeftHandId != 0)
					res1 = rec->recognizeOn(getHand(m_user->m_assignedLeftHandId), correctionHint);
				if (m_user->m_assignedRightHandId != 0)
					res2 = rec->recognizeOn(getHand(m_user->m_assignedRightHandId), correctionHint);
				if (res1 != RecognitionResult::TRACKING_ERROR && res2 != RecognitionResult::TRACKING_ERROR)
					result = std::max(res1, res2);
				else if (res1 != RecognitionResult::TRACKING_ERROR)
					result = res1;
				else if (res2 != RecognitionResult::TRACKING_ERROR)
					result = res2;
				else
					result = rec->recognizeOn(m_user, correctionHint);
			}
			else
				result = rec->recognizeOn(m_user, correctionHint);			
		}
	}
	else if (m_hand)
	{
		if (rec->useHistory())
		{
			Fubi::TrackingData *histData = 0x0, *histFilteredData = 0x0;
			if (stateIndex == m_currentState && m_trackingStates.size() > 0 && m_filteredTrackingStates.size() > 0)
			{
				// We need the data when the current state has started (before min duration has been reached)
				if (m_minDurationPassed && m_trackingStates.size() > 1 && m_filteredTrackingStates.size() > 1)
				{
					histData = &(*(m_trackingStates.end()-2));
					histFilteredData = &(*(m_filteredTrackingStates.end()-2));
				}
				else
				{
					histData = &(m_trackingStates.back());
					histFilteredData = &(m_filteredTrackingStates.back());
				}
			}
			result = rec->recognizeWithHistory(m_hand, histData, histFilteredData, correctionHint);
		}
		else
			result = rec->recognizeOn(m_hand, correctionHint);
	}

	return result;
}

Fubi::RecognitionResult::Result CombinationRecognizer::areAllGesturesRecognized(int stateIndex, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	if (stateIndex < 0 || stateIndex >= (signed) m_RecognitionStates.size())
		return Fubi::RecognitionResult::NOT_RECOGNIZED;

	const RecognitionState& state = m_RecognitionStates[stateIndex];
	bool atLeastOneRecognized = false;
	bool atLeastOneFailed = false;
	bool atLeastOneHistoryRecWaiting = false;
	bool onlyTrackingErrors = true;

	for (RecognitionState::GestureIter iter = state.m_gestures.begin(); iter != state.m_gestures.end(); ++iter)
	{
		Fubi::RecognitionResult::Result res = checkRecognizer(*iter, stateIndex, correctionHint);
		if (res != RecognitionResult::TRACKING_ERROR)
			onlyTrackingErrors = false;
		if (res == Fubi::RecognitionResult::NOT_RECOGNIZED 
			|| (res == Fubi::RecognitionResult::TRACKING_ERROR && !(*iter)->m_ignoreOnTrackingError))
		{ 
			atLeastOneRecognized = false;
			atLeastOneFailed = true;
			atLeastOneHistoryRecWaiting = false;
			break;
		}
		else if (res == Fubi::RecognitionResult::RECOGNIZED)
		{
			atLeastOneRecognized = true;
		}
		// Special case: history rec not finished
		else if (res == Fubi::RecognitionResult::WAITING_FOR_LAST_STATE_TO_FINISH)
		{
			atLeastOneRecognized = true;
			atLeastOneHistoryRecWaiting = true;
		}
	}
	// Only check not recognizers if not already one recognizer failed
	if (!atLeastOneFailed)
	{
		for (RecognitionState::GestureIter iter = state.m_notGestures.begin(); iter != state.m_notGestures.end(); ++iter)
		{
			// Not recognizers are "recognized" if the recognition fails,
			// and fail if the recognition is successful or cannot be performed because of tracking errors
			Fubi::RecognitionResult::Result res = checkRecognizer(*iter, stateIndex);
			if (res != RecognitionResult::TRACKING_ERROR)
				onlyTrackingErrors = false;
			if (res == Fubi::RecognitionResult::RECOGNIZED 
				|| (res == Fubi::RecognitionResult::TRACKING_ERROR && !(*iter)->m_ignoreOnTrackingError))
			{ 
				atLeastOneRecognized = false;
				atLeastOneHistoryRecWaiting = false;
				break;
			}
			else if (res == Fubi::RecognitionResult::NOT_RECOGNIZED)
				atLeastOneRecognized = true;
			// Special case: history rec not finished -> same as not recognized in this case
			else if (res == Fubi::RecognitionResult::WAITING_FOR_LAST_STATE_TO_FINISH)
			{
				atLeastOneRecognized = true;
			}
		}
	}

	if (!atLeastOneRecognized || atLeastOneHistoryRecWaiting)
	{
		bool atLeastOneAlternativeRecognized = false;
		bool atLeastOneAlternativeHistoryRecWaiting = false;
		atLeastOneFailed = false;
		// No recognition finished so try alternative recognizers
		for (RecognitionState::GestureIter iter = state.m_alternativeGestures.begin(); iter != state.m_alternativeGestures.end(); ++iter)
		{
			Fubi::RecognitionResult::Result res = checkRecognizer(*iter, stateIndex, correctionHint);
			if (res != RecognitionResult::TRACKING_ERROR)
				onlyTrackingErrors = false;
			if (res == Fubi::RecognitionResult::NOT_RECOGNIZED 
				|| (res == Fubi::RecognitionResult::TRACKING_ERROR && !(*iter)->m_ignoreOnTrackingError))
			{ 
				atLeastOneAlternativeRecognized = false;
				atLeastOneFailed = true;
				atLeastOneAlternativeHistoryRecWaiting = false;
				break;
			}
			else if (res == Fubi::RecognitionResult::RECOGNIZED)
				atLeastOneAlternativeRecognized = true;
			// Special case: history rec not finished
			else if (res == Fubi::RecognitionResult::WAITING_FOR_LAST_STATE_TO_FINISH)
			{
				atLeastOneAlternativeRecognized = true;
				atLeastOneAlternativeHistoryRecWaiting = true;
			}
		}
		// Only check not recognizers if not already one recognizer failed
		if (!atLeastOneFailed)
		{
			for (RecognitionState::GestureIter iter = state.m_alternativeNotGestures.begin(); iter != state.m_alternativeNotGestures.end(); ++iter)
			{
				// Not recognizers are "recognized" if the recognition fails,
				// and fail if the recognition is successful or cannot be performed because of tracking errors
				Fubi::RecognitionResult::Result res = checkRecognizer(*iter, stateIndex, correctionHint);
				if (res != RecognitionResult::TRACKING_ERROR)
					onlyTrackingErrors = false;
				if (res == Fubi::RecognitionResult::RECOGNIZED 
					|| (res == Fubi::RecognitionResult::TRACKING_ERROR && !(*iter)->m_ignoreOnTrackingError))
				{ 
					atLeastOneAlternativeRecognized = false;
					atLeastOneAlternativeHistoryRecWaiting = false;
					break;
				}
				else if (res == Fubi::RecognitionResult::NOT_RECOGNIZED)
					atLeastOneAlternativeRecognized = true;
				// Special case: history rec not finished -> same as not recognized in this case
				else if (res == Fubi::RecognitionResult::WAITING_FOR_LAST_STATE_TO_FINISH)
				{
					atLeastOneAlternativeRecognized = true;
				}
			}
		}

		 if (!atLeastOneRecognized)
		 {
			 // Default recognizers failed completely, so we can take the alternative results directly
			 atLeastOneRecognized = atLeastOneAlternativeRecognized;
			 atLeastOneHistoryRecWaiting = atLeastOneAlternativeHistoryRecWaiting;
		 }
		 else if (atLeastOneHistoryRecWaiting && atLeastOneAlternativeRecognized && !atLeastOneAlternativeHistoryRecWaiting)
		 {
			 // Default recognizers have one waiting rec, so  we only take the alternative if it is not waiting
			 atLeastOneHistoryRecWaiting = false;
			 atLeastOneRecognized = true;
		 }
	}

	if (!atLeastOneRecognized)
	{
		// Rec failed completely
		if (onlyTrackingErrors)
		{
			return RecognitionResult::TRACKING_ERROR;
		}
		else
		{
			if (correctionHint)
				correctionHint->m_failedState = stateIndex;
			return RecognitionResult::NOT_RECOGNIZED;
		}
	}
	if (atLeastOneHistoryRecWaiting)
		return Fubi::RecognitionResult::WAITING_FOR_LAST_STATE_TO_FINISH;	// We are only waiting for history recs to finish
	return Fubi::RecognitionResult::RECOGNIZED;	// Recognition succesful
}

void CombinationRecognizer::logProgress(const std::string& msg)
{
#ifdef FUBI_COMBINATIONREC_DEBUG_LOGGING
	if (m_user)
	{
		Fubi_logDbg("User%d Combination %s State %d: %s\n", m_user->id(), m_name.c_str(), m_currentState, msg.c_str());
	}
	else if (m_hand)
	{
		Fubi_logDbg("Hand%d Combination %s State %d: %s\n", m_hand->id(), m_name.c_str(), m_currentState, msg.c_str());
	}
#endif
}

void CombinationRecognizer::update()
{
	if (m_running && m_RecognitionStates.size() > 0)
	{
		if (m_minDurationPassed) // Min duration already passed
		{
			// Check for the next state
			bool movedToNextState = false;
			int nextStateID = m_currentState+1;
			if ((unsigned) nextStateID < m_RecognitionStates.size())
			{	
				RecognitionResult::Result res = areAllGesturesRecognized(nextStateID, &m_currentCorrectionHint);
				if (res == RecognitionResult::RECOGNIZED || res == RecognitionResult::WAITING_FOR_LAST_STATE_TO_FINISH)
				{
					// Next gestures performed so jump to next state
					m_currentState = nextStateID;
					m_minDurationPassed = m_RecognitionStates[nextStateID].m_minDuration < Math::Epsilon && res != RecognitionResult::WAITING_FOR_LAST_STATE_TO_FINISH;
					m_interrupted = false;
					m_stateStart = currentTime();
					saveCurrentTrackingState();
					movedToNextState = true;
					logProgress("State started");
					if (m_minDurationPassed && !m_waitUntilLastStateRecognizersStop && nextStateID == m_RecognitionStates.size()-1)
					{
						// Last state finished --> recognized
						m_recognition = RecognitionResult::RECOGNIZED;
						logProgress("Recognized!");
						// Stop the recognition
						stop();
					}
				}
				else
					m_recognition = res;
			}
			
			if (!movedToNextState && m_currentState > -1)
			{
				if (m_interrupted)
				{
					// Currently interrupted since:
					double interruptionTime = currentTime() - m_interruptionStart;
					bool checkTimeForTransition = m_RecognitionStates[m_currentState].m_timeForTransition >= 0 && m_currentState != m_RecognitionStates.size()-1;
					if (interruptionTime < m_RecognitionStates[m_currentState].m_maxInterruptionTime && areAllGesturesRecognized(m_currentState) == RecognitionResult::RECOGNIZED)
					{
						// Continued current state
						m_interrupted = false;
					}
					// The next state has to be reached during time for transition or the current state has to be rekept before max interruption time
					else if ((checkTimeForTransition && interruptionTime > m_RecognitionStates[m_currentState].m_timeForTransition
								&& interruptionTime > m_RecognitionStates[m_currentState].m_maxInterruptionTime)
							|| (!checkTimeForTransition && interruptionTime > m_RecognitionStates[m_currentState].m_maxInterruptionTime))
					{

						if (m_waitUntilLastStateRecognizersStop && m_currentState == m_RecognitionStates.size()-1)
						{
							// Last state finished --> recognized
							m_recognition = m_recognition = RecognitionResult::RECOGNIZED;
							saveCurrentTrackingState();
							logProgress("Recognized!");
							// Stop the recognition
							stop();
						}
						else
						{
							logProgress("Next gestures too late");

							// Fail
							if (m_RecognitionStates[m_currentState].m_restartOnFail) // so restart
							{
								logProgress("->restarting");
								restart();
							}
							else // Or go back one state
							{
								--m_currentState;
								m_minDurationPassed = true;
								m_interrupted = true;
								m_interruptionStart = currentTime();
								m_stateStart = currentTime() - m_RecognitionStates[m_currentState].m_minDuration;
								saveCurrentTrackingState();
								logProgress("->going back to previous state");
							}
						}
					}
					// If m_timeForTransition < 0 the user has infinite time for performing the next state
				}
				else if (areAllGesturesRecognized(m_currentState) == RecognitionResult::RECOGNIZED)
				{
					double timeInPose = currentTime()- m_stateStart;
					if (m_RecognitionStates[m_currentState].m_maxDuration > 0 && timeInPose > m_RecognitionStates[m_currentState].m_maxDuration)
					{
						if (m_waitUntilLastStateRecognizersStop && m_currentState == m_RecognitionStates.size()-1)
						{
							// Last state finished --> recognized
							saveCurrentTrackingState();
							m_recognition = m_recognition = RecognitionResult::RECOGNIZED;
							logProgress("Recognized!");
							// Stop the recognition
							stop();
						}
						else
						{
							// Next gestures not performed before max duration is reached so restart recognition
							logProgress("Reached max duration");
							// Fail
							if (m_RecognitionStates[m_currentState].m_restartOnFail) // so restart
							{
								logProgress("->restarting");
								restart();
							}
							else // Or go back one state
							{
								--m_currentState;
								m_minDurationPassed = true;
								m_interrupted = true;
								m_interruptionStart = currentTime();
								m_stateStart = currentTime() - m_RecognitionStates[m_currentState].m_minDuration;
								saveCurrentTrackingState();
								logProgress("->going back to previous state");
							}
						}
					}
				}
				else
				{
					// First frame of interruption
					m_interrupted = true;
					m_interruptionStart = currentTime();
				}
			}
		}
		else if (m_currentState > -1)  // Min duration not yet passed
		{
			RecognitionResult::Result res = areAllGesturesRecognized(m_currentState, &m_currentCorrectionHint);
			if (res == RecognitionResult::RECOGNIZED && (currentTime()-m_stateStart) >= m_RecognitionStates[m_currentState].m_minDuration) // Min duration passed
			{
				// Save user state information for the transition
				saveCurrentTrackingState();

				if (!m_waitUntilLastStateRecognizersStop && m_currentState == m_RecognitionStates.size()-1)
				{
					// Last state finished --> recognized
					m_recognition = m_recognition = RecognitionResult::RECOGNIZED;
					logProgress("Recognized!");
					// Stop the recognition
					stop();
				}
				else
				{
					// Min duration passed, check for the next state
					m_minDurationPassed = true;
					m_interrupted = false;
				}
			}
			else // Within the min duration time frame or not recognized anymore
			{
				// Check if the current gestures are still fulfilled
				if (res == RecognitionResult::NOT_RECOGNIZED || res == RecognitionResult::TRACKING_ERROR)
				{
					m_recognition = res;

					if (m_interrupted || m_RecognitionStates[m_currentState].m_noInterrruptionBeforeMinDuration)
					{
						// Interrupted for more than one frame or no interruption allowed at all
						if (m_RecognitionStates[m_currentState].m_noInterrruptionBeforeMinDuration || (currentTime() - m_interruptionStart) > m_RecognitionStates[m_currentState].m_maxInterruptionTime)
						{
							logProgress("State aborted before min duration " + numToString(m_RecognitionStates[m_currentState].m_minDuration));
							// Fail
							if (m_RecognitionStates[m_currentState].m_restartOnFail) // so restart
							{
								logProgress("->restarting");
								restart();
							}
							else // Or go back one state
							{
								--m_currentState;
								m_minDurationPassed = true;
								m_interrupted = true;
								m_interruptionStart = currentTime();
								m_stateStart = currentTime() - m_RecognitionStates[m_currentState].m_minDuration;
								saveCurrentTrackingState();
								logProgress("->going back to previous state");
							}
						}
					}
					else
					{
						// First frame of interruption
						m_interrupted = true;
						m_interruptionStart = currentTime();
					}
				}
				else
					m_interrupted = false;
			}
		}
	}
}

void CombinationRecognizer::checkRecognizerTargets(const std::vector<IGestureRecognizer*>& gestureRecognizers)
{
	for (auto iter = gestureRecognizers.cbegin(); iter != gestureRecognizers.cend(); ++iter)
	{
		if ((*iter)->m_targetSkeletonType != m_targetSkeletonType)
		{
			if (m_targetSkeletonType == RecognizerTarget::INVALID)
				m_targetSkeletonType = (*iter)->m_targetSkeletonType;
			else if ((*iter)->m_targetSkeletonType == RecognizerTarget::BODY || (*iter)->m_targetSkeletonType == RecognizerTarget::BODY_AND_HANDS)
				m_targetSkeletonType = RecognizerTarget::BODY_AND_HANDS;
			else if (m_targetSkeletonType == RecognizerTarget::BODY || m_targetSkeletonType == RecognizerTarget::BODY_AND_HANDS)
				m_targetSkeletonType = RecognizerTarget::BODY_AND_HANDS;
			else
				m_targetSkeletonType = RecognizerTarget::BOTH_HANDS;
		}
	}
}

void CombinationRecognizer::addState(const std::vector<IGestureRecognizer*>& gestureRecognizers, const std::vector<IGestureRecognizer*>& notRecognizers /*= s_emptyRecVec*/, double minDuration /*= 0*/,
	double maxDuration /*= -1*/, double timeForTransition /*= 1*/, double maxInterruption /*= -1*/, bool noInterrruptionBeforeMinDuration /*= false*/,
	const std::vector<IGestureRecognizer*>& alternativeGestureRecognizers /*= RecognitionState::s_emptyRecVec*/, const std::vector<IGestureRecognizer*>& alternativeNotRecognizers /*= RecognitionState::s_emptyRecVec*/,
	bool restartOnFail /*= true*/, const std::map<std::string, std::string>& metaInfo /*= RecognitionState::s_emptyMetaInfo*/)
{
	if (!gestureRecognizers.empty() || !notRecognizers.empty())
	{
		checkRecognizerTargets(gestureRecognizers);
		checkRecognizerTargets(notRecognizers);
		checkRecognizerTargets(alternativeGestureRecognizers);
		checkRecognizerTargets(alternativeNotRecognizers);
		
		m_RecognitionStates.push_back(
			RecognitionState(gestureRecognizers, notRecognizers, minDuration, maxDuration, timeForTransition, 
				maxInterruption, noInterrruptionBeforeMinDuration, alternativeGestureRecognizers,
				alternativeNotRecognizers, restartOnFail, metaInfo));
	}
}

CombinationRecognizer* CombinationRecognizer::clone() const
{
	return new CombinationRecognizer(*this);
}

void CombinationRecognizer::setUser(const FubiUser* user)
{
	// First stop everything, because the old data won't be usefull with a new user
	stop();
	// Now set the new user
	m_user = user;
}

void CombinationRecognizer::setHand(const FubiHand* hand)
{
	// First stop everything, because the old data won't be usefull with a new user
	stop();
	// Now set the new user
	m_hand = hand;

	if (m_targetSkeletonType == RecognizerTarget::BODY || m_targetSkeletonType == RecognizerTarget::BODY_AND_HANDS)
		Fubi_logWrn("Combination recognizer assigned to hand, but at least one included recognizer targets the body!\n");
}

std::string CombinationRecognizer::getName() const
{
	return m_name;
}

void CombinationRecognizer::saveCurrentTrackingState()
{
	if (m_user)
	{
		m_trackingStates.push_back(*(m_user->currentTrackingData()));
		m_filteredTrackingStates.push_back(*(m_user->currentFilteredTrackingData()));
	}
	else if (m_hand)
	{
		m_trackingStates.push_back(*(m_hand->currentTrackingData()));
		m_filteredTrackingStates.push_back(*(m_hand->currentFilteredTrackingData()));
	}
}

const std::string& CombinationRecognizer::getStateMetaInfo(unsigned int stateIndex, const std::string& propertyName)
{
	const static std::string emptyString = std::string();

	if (stateIndex < m_RecognitionStates.size())
	{
		auto iter = m_RecognitionStates[stateIndex].m_MetaInfo.find(propertyName);
		if (iter != m_RecognitionStates[stateIndex].m_MetaInfo.end())
			return iter->second;
	}

	return emptyString;
}