// ****************************************************************************************
//
// Posture Recognizers
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler 
// 
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
// 
// ****************************************************************************************
#include "LeftKneeUpRecognizer.h"

using namespace Fubi;

Fubi::RecognitionResult::Result LeftKneeUpRecognizer::recognizeOn(const FubiUser* user, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	const Fubi::TrackingData* data = user->currentTrackingData();
	if (m_useFilteredData)
		data = user->currentFilteredTrackingData();
	const SkeletonJointPosition& leftKnee = data->jointPositions[SkeletonJoint::LEFT_KNEE];
	const SkeletonJointPosition& leftHip = data->jointPositions[SkeletonJoint::LEFT_HIP];
	if (leftKnee.m_confidence >= m_minConfidence && leftHip.m_confidence >= m_minConfidence)
	{
		if (abs(leftKnee.m_position.y - leftHip.m_position.y) < 250.0f)
			return Fubi::RecognitionResult::RECOGNIZED;
	}
	else
		return Fubi::RecognitionResult::TRACKING_ERROR;
	return Fubi::RecognitionResult::NOT_RECOGNIZED;
}