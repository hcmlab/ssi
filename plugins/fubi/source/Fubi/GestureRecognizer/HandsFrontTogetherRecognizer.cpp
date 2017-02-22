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
#include "HandsFrontTogetherRecognizer.h"

using namespace Fubi;



Fubi::RecognitionResult::Result HandsFrontTogetherRecognizer::recognizeOn(const FubiUser* user, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	bool recognized = false;
	const Fubi::TrackingData* data = user->currentTrackingData();
	if (m_useFilteredData)
		data = user->currentFilteredTrackingData();
	const SkeletonJointPosition& rightHand = data->jointPositions[SkeletonJoint::RIGHT_HAND];
	const SkeletonJointPosition& rightShoulder = data->jointPositions[SkeletonJoint::RIGHT_SHOULDER];
	const SkeletonJointPosition& leftHand = data->jointPositions[SkeletonJoint::LEFT_HAND];
	const SkeletonJointPosition& leftShoulder = data->jointPositions[SkeletonJoint::LEFT_SHOULDER];
	if (rightHand.m_confidence >= m_minConfidence && rightShoulder.m_confidence >= m_minConfidence
		&& leftHand.m_confidence >= m_minConfidence && leftShoulder.m_confidence >= m_minConfidence)
	{
		Vec3f rightArm = rightHand.m_position - rightShoulder.m_position;
		Vec3f leftArm = leftHand.m_position - leftShoulder.m_position;
		float handDist = (leftHand.m_position - rightHand.m_position).length();
		bool littleX =  (rightArm.x < -25 && rightArm.x > -275) && (leftArm.x > 25 && leftArm.x < 275);
		bool moreZ =  rightArm.z < -300 && leftArm.z < -300;
		recognized = littleX && moreZ && handDist < 150;
		/*if (!recognized)
		{
			Fubi_logInfo("HandsFrontFailed: rightArm.x: %.0f, leftArm.x: %.0f, rightArm.z: %.0f, leftArm.z: %.0f\n", rightArm.x, leftArm.x, rightArm.z, leftArm.z);
		}*/
	}
	else
		return Fubi::RecognitionResult::TRACKING_ERROR;
	return recognized ? Fubi::RecognitionResult::RECOGNIZED : Fubi::RecognitionResult::NOT_RECOGNIZED;
}