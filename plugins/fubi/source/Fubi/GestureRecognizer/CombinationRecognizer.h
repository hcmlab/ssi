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

#pragma once

#include "../FubiPredefinedGestures.h"
#include "IGestureRecognizer.h"

// A state of the combination recognizer
struct RecognitionState
{
	RecognitionState(const std::vector<IGestureRecognizer*>& gestureRecognizers, const std::vector<IGestureRecognizer*>& notRecognizers,
		double minDuration, double maxDuration, double timeForTransition, double maxInterruption, bool noInterrruptionBeforeMinDuration,
		const std::vector<IGestureRecognizer*>& alternativeGestureRecognizers, const std::vector<IGestureRecognizer*>& alternativeNotRecognizers,
		bool restartOnFail, const std::map<std::string, std::string>& metaInfo)
		: m_gestures(gestureRecognizers), m_notGestures(notRecognizers),
			m_minDuration(minDuration), m_maxDuration(maxDuration),
			m_timeForTransition(timeForTransition), m_maxInterruptionTime(maxInterruption), m_noInterrruptionBeforeMinDuration(noInterrruptionBeforeMinDuration),
			m_alternativeGestures(alternativeGestureRecognizers), m_alternativeNotGestures(alternativeNotRecognizers),
			m_restartOnFail(restartOnFail), m_MetaInfo(metaInfo)
	{
		if (m_maxInterruptionTime < 0)
			m_maxInterruptionTime = Fubi::clamp<double>(m_timeForTransition, 0, 0.1);
	}

	static const std::vector<IGestureRecognizer*> s_emptyRecVec;
	typedef std::vector<IGestureRecognizer*>::const_iterator GestureIter;
	std::vector<IGestureRecognizer*> m_gestures;
	std::vector<IGestureRecognizer*> m_notGestures;
	std::vector<IGestureRecognizer*> m_alternativeGestures;
	std::vector<IGestureRecognizer*> m_alternativeNotGestures;
	double m_minDuration;
	double m_maxDuration;
	double m_timeForTransition;
	double m_maxInterruptionTime;
	bool m_noInterrruptionBeforeMinDuration;
	bool m_restartOnFail;
	std::map<std::string, std::string> m_MetaInfo;
	static const std::map<std::string, std::string> s_emptyMetaInfo;
};

class CombinationRecognizer
{
public:
	CombinationRecognizer(const FubiUser* user, Fubi::Combinations::Combination gestureID);
	CombinationRecognizer(const std::string& recognizerName);
	CombinationRecognizer(const CombinationRecognizer& other);

	~CombinationRecognizer();

	// updates the current combination state
	void update();

	// Starts and stops the recognition
	void start();
	void stop();
	void restart() { stop(); start(); }

	// True if a combination start is detected (First state recognized, but last one not yet)
	bool isInProgress() {return m_currentState > -1; }
	// True if the recognizer is currently trying to detect the combination (Last state not yet recognized)
	bool isRunning()	{ return m_running; }
	// True if recognizer is currently trying to detect the combination or has already suceeded (running or recognized, but not stopped)
	bool isActive()		{ return m_running || m_recognition == Fubi::RecognitionResult::RECOGNIZED; }
	// True if min duration passed and 
	bool isWaitingForTransition() { return m_running && m_minDurationPassed && m_RecognitionStates.size() > 0; }
	// True if already recognized, but still waiting for the last state to finish
	bool isWaitingForLastStateFinish()		{ return m_waitUntilLastStateRecognizersStop && isWaitingForTransition() && (m_currentState == m_RecognitionStates.size()-1); }
	// True if current state recognizers are currently interrupted, but not yet aborted
	bool isInterrupted() { return m_interrupted; }
	// @param userStates: vector were the user skeletonData and timestamps are stored for each transition during a recognition that has been successful
	// @param restart: if true, the recognition will automatically restart if it is successful
	// returns true if a combination is completed 
	Fubi::RecognitionResult::Result getRecognitionProgress(std::vector<Fubi::TrackingData>* trackingStates, bool restart, bool returnFilteredData, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);

	// Add a state for this combination (used for creating the recognizer)
	void addState(const std::vector<IGestureRecognizer*>& gestureRecognizers, const std::vector<IGestureRecognizer*>& notRecognizers = RecognitionState::s_emptyRecVec,
		double minDuration = 0, double maxDuration = -1, double timeForTransition = 1, double maxInterruption = -1, bool noInterrruptionBeforeMinDuration = false,
		const std::vector<IGestureRecognizer*>& alternativeGestureRecognizers = RecognitionState::s_emptyRecVec, const std::vector<IGestureRecognizer*>& alternativeNotRecognizers = RecognitionState::s_emptyRecVec,
		bool restartOnFail = true, const std::map<std::string, std::string>& metaInfo = RecognitionState::s_emptyMetaInfo);

	// Assign a user to this recognizer (from whom the recognizer retrieves the tracking data for the recognition)
	void setUser(const FubiUser* user);
	// Or alternatively a hand
	void setHand(const FubiHand* hand);

	CombinationRecognizer* clone() const;

	std::string getName() const;

	unsigned int getNumStates() const { return (unsigned int) m_RecognitionStates.size(); }

	int getCurrentState() const { return m_currentState; }

	void setWaitUntilLastStateRecognizersStop(bool enable) { m_waitUntilLastStateRecognizersStop = enable; }

	Fubi::RecognizerTarget::Target getRecognizerTarget() const { return m_targetSkeletonType; }

	const std::string& getStateMetaInfo(unsigned int stateIndex, const std::string& propertyName);

protected:
	// Check all recognizers of the given state and return a summarized result
	Fubi::RecognitionResult::Result areAllGesturesRecognized(int stateIndex, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);
	// Called by the above; checks one recognizer
	Fubi::RecognitionResult::Result checkRecognizer(IGestureRecognizer* rec, int stateIndex, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);

	void checkRecognizerTargets(const std::vector<IGestureRecognizer*>& gestureRecognizers);

	void saveCurrentTrackingState();

	void logProgress(const std::string& msg);

	bool m_running;

	// All recognition states ordered in time
	std::vector<RecognitionState>	m_RecognitionStates;
	// The current state index
	int							m_currentState;
	// When the current state started or the transition started
	double						m_stateStart, m_interruptionStart;
	// If min duration has been reached
	bool						m_minDurationPassed;
	// If the gestures of the current state are temporarly not recognized
	bool						m_interrupted;
	// If the recognition was successful
	Fubi::RecognitionResult::Result		m_recognition;
	// User this recognizer is attachded to
	const FubiUser*					m_user;
	// Alternatively the hand
	const FubiHand*					m_hand;
	// gesture id of this recognizer (only predefined ones)
	Fubi::Combinations::Combination	m_gestureID;
	// Name of the recognizer (all recognizers)
	std::string					m_name;

	bool						m_waitUntilLastStateRecognizersStop;

	Fubi::RecognitionCorrectionHint m_currentCorrectionHint;

	std::vector<Fubi::TrackingData> m_trackingStates;
	std::vector<Fubi::TrackingData> m_filteredTrackingStates;

	// For which sensor this is combination can be used
	Fubi::RecognizerTarget::Target	m_targetSkeletonType;
};