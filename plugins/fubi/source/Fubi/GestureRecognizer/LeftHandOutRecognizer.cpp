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
#include "LeftHandOutRecognizer.h"

using namespace Fubi;

Fubi::RecognitionResult::Result LeftHandOutRecognizer::recognizeOn(const FubiUser* user, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	const Fubi::TrackingData* data = user->currentTrackingData();
	if (m_useFilteredData)
		data = user->currentFilteredTrackingData();
	const SkeletonJointPosition& leftHand = data->jointPositions[SkeletonJoint::LEFT_HAND];
	const SkeletonJointPosition& leftShoulder = data->jointPositions[SkeletonJoint::LEFT_SHOULDER];
	Fubi::RecognitionResult::Result result = Fubi::RecognitionResult::NOT_RECOGNIZED;

	if (leftHand.m_confidence >= m_minConfidence && leftShoulder.m_confidence >= m_minConfidence)
	{
		if (abs(leftHand.m_position.y-leftShoulder.m_position.y) < 300.0f
			&& leftHand.m_position.x < leftShoulder.m_position.x - 350.0f)
			result = Fubi::RecognitionResult::RECOGNIZED;
	}
	else
		result = Fubi::RecognitionResult::TRACKING_ERROR;
	return result;
}