// ****************************************************************************************
//
// Posture Recognizers
// ---------------------------------------------------------
// Copyleft (C) 2010-2012 Felix Kistler 
// 
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
// 
// ****************************************************************************************
#include "LeftHandCloseToArmRecognizer.h"

using namespace Fubi;


Fubi::RecognitionResult::Result LeftHandCloseToArmRecognizer::recognizeOn(const FubiUser* user, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	const Fubi::TrackingData* data = user->currentTrackingData();
	if (m_useFilteredData)
		data = user->currentFilteredTrackingData();
	const SkeletonJointPosition& leftHand = data->jointPositions[SkeletonJoint::LEFT_HAND];
	const SkeletonJointPosition& rightHand = data->jointPositions[SkeletonJoint::RIGHT_HAND];
	const SkeletonJointPosition& rightShoulder = data->jointPositions[SkeletonJoint::RIGHT_SHOULDER];
	const SkeletonJointPosition& rightElbow = data->jointPositions[SkeletonJoint::RIGHT_ELBOW];

	Fubi::RecognitionResult::Result result = Fubi::RecognitionResult::NOT_RECOGNIZED;
	if (leftHand.m_confidence >= m_minConfidence && rightElbow.m_confidence >= m_minConfidence)
	{
		const Vec3f& elbow = rightElbow.m_position;
		const Vec3f& hand = leftHand.m_position;

		// Hand close to elbow
		if ((elbow-hand).length() < 100.0f)
			result = Fubi::RecognitionResult::RECOGNIZED;
		else if (rightHand.m_confidence >= m_minConfidence)
		{
			Vec3f dir = rightHand.m_position-elbow;

			float distanceToArm = distancePointToRay(hand, elbow, dir, true);

			// Hand close to lower arm
			if (distanceToArm >= 0.0f && distanceToArm < 100.0f)
				result = Fubi::RecognitionResult::RECOGNIZED;
		}
		else
			result = Fubi::RecognitionResult::TRACKING_ERROR;

		if (result != Fubi::RecognitionResult::RECOGNIZED)
		{
			if (rightShoulder.m_confidence >= m_minConfidence)
			{
				Vec3f dir = rightShoulder.m_position-elbow;

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