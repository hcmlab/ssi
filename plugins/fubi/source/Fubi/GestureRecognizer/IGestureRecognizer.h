// ****************************************************************************************
//
// Fubi Gesture Recognizer
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler 
// 
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
// 
// ****************************************************************************************

#pragma once

#include "../FubiUser.h"
#include "../FubiHand.h"


class IGestureRecognizer
{
public:
	// Constructor with default values
	IGestureRecognizer() : m_ignoreOnTrackingError(false), m_minConfidence(0.51f), m_useFilteredData(false), m_targetSkeletonType(Fubi::RecognizerTarget::INVALID) {}
	// Fully specified constructor
	IGestureRecognizer(bool ignoreOnTrackingError, float minconfidence, bool useFilteredData)
		: m_ignoreOnTrackingError(ignoreOnTrackingError), m_useFilteredData(useFilteredData), m_targetSkeletonType(Fubi::RecognizerTarget::INVALID)
	{ m_minConfidence = (minconfidence >= 0) ? minconfidence : 0.51f; }
	// Virtual destructor for correct sub class deletion
	virtual ~IGestureRecognizer() {}

	// Method signatures for testing the recognizer on a specfic user/hand, need to be overwritten by sub classes to apply the recognition
	virtual Fubi::RecognitionResult::Result recognizeOn(const FubiUser* user, Fubi::RecognitionCorrectionHint* correctionHint = 0x0)
	{ return Fubi::RecognitionResult::NOT_RECOGNIZED; }
	virtual Fubi::RecognitionResult::Result recognizeWithHistory(const FubiUser* user, const Fubi::TrackingData* initialData, const Fubi::TrackingData* initialFilteredData, Fubi::RecognitionCorrectionHint* correctionHint = 0x0)
	{ return Fubi::RecognitionResult::NOT_RECOGNIZED; }
	virtual Fubi::RecognitionResult::Result recognizeOn(const FubiHand* hand, Fubi::RecognitionCorrectionHint* correctionHint = 0x0)
	{ return Fubi::RecognitionResult::NOT_RECOGNIZED; }	
	virtual Fubi::RecognitionResult::Result recognizeWithHistory(const FubiHand* hand, const Fubi::TrackingData* initialData, const Fubi::TrackingData* initialFilteredData, Fubi::RecognitionCorrectionHint* correctionHint = 0x0)
	{ return Fubi::RecognitionResult::NOT_RECOGNIZED; }

	// To be overwritten when using tracking history (tracking data at the entry and exit of the states within a combination recognizer)
	virtual bool useHistory() { return false; }

	// Clone method for subclasses that directly returns a base class pointer, used when instantiating a recognizer for one user
	virtual IGestureRecognizer* clone() = 0;

	// General options for all types of recognizers
	bool m_ignoreOnTrackingError;
	float m_minConfidence;
	bool m_useFilteredData;
	// Determines whether this recognizer targets user or finger tracking data or both and which hand if applicable
	Fubi::RecognizerTarget::Target m_targetSkeletonType;
};