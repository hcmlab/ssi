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
#include "RightHandCloseToArmRecognizer.h"

using namespace Fubi;

Fubi::RecognitionResult::Result RightHandCloseToArmRecognizer::recognizeOn(const FubiUser* user, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	const Fubi::TrackingData* data = user->currentTrackingData();
	if (m_useFilteredData)
		data = user->currentFilteredTrackingData();
	const SkeletonJointPosition& rightHand = data->jointPositions[SkeletonJoint::RIGHT_HAND];
	const SkeletonJointPosition& leftHand = data->jointPositions[SkeletonJoint::LEFT_HAND];
	const SkeletonJointPosition& leftShoulder = data->jointPositions[SkeletonJoint::LEFT_SHOULDER];
	const SkeletonJointPosition& leftElbow = data->jointPositions[SkeletonJoint::LEFT_ELBOW];
	Fubi::RecognitionResult::Result result = Fubi::RecognitionResult::NOT_RECOGNIZED;

	if (rightHand.m_confidence >= m_minConfidence && leftElbow.m_confidence >= m_minConfidence)
	{
		const Vec3f& elbow = leftElbow.m_position;
		const Vec3f& hand = rightHand.m_position;

		// Hand close to elbow
		if ((elbow-hand).length() < 100.0f)
			result = Fubi::RecognitionResult::RECOGNIZED;
		else if (leftHand.m_confidence >= m_minConfidence)
		{
			Vec3f dir = leftHand.m_position-elbow;

			float distanceToArm = distancePointToRay(hand, elbow, dir, true);

			// Hand close to lower arm
			if (distanceToArm >= 0.0f && distanceToArm < 100.0f)
				result = Fubi::RecognitionResult::RECOGNIZED;
		}
		else
			result = Fubi::RecognitionResult::TRACKING_ERROR;

		if (result != Fubi::RecognitionResult::RECOGNIZED)
		{
			if (leftShoulder.m_confidence >= m_minConfidence)
			{
				Vec3f dir = leftShoulder.m_position-elbow;

				float distanceToArm = distancePointToRay(hand, elbow, dir, true);

				// Hand close to upper arm
				if (distanceToArm >= 0.0f && distanceToArm < 100.0f)
					result = Fubi::RecognitionResult::RECOGNIZED;
				else
					result = Fubi::RecognitionResult::NOT_RECOGNIZED;
			}
			else
				result = Fubi::RecognitionResult::TRACKING_ERROR;
		}
	}
	else
		result = Fubi::RecognitionResult::TRACKING_ERROR;
	return result;
}