// ****************************************************************************************
//
// Joint Orientation Recognizers
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler 
// 
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
// 
// ****************************************************************************************
#include "JointOrientationRecognizer.h"

using namespace Fubi;

JointOrientationRecognizer::JointOrientationRecognizer(Fubi::SkeletonJoint::Joint joint, const Fubi::Vec3f& minValues /*= Fubi::Vec3f(-180.0f,-180.0f,-180.0f)*/, 
		const Fubi::Vec3f& maxValues /*= Fubi::Vec3f(180.0f, 180.0f, 180.0f)*/, bool useLocalOrientation /*= true*/, float minConfidence /*= -1.0f*/,
	bool useFilteredData /*= false*/)
	: m_joint(joint), m_minValues(minValues), m_maxValues(maxValues), m_useLocalOrientations(useLocalOrientation), m_useOrientation(false),
	IGestureRecognizer(false, minConfidence, useFilteredData)
{
	m_jointUsableForHandTracking = m_joint < SkeletonHandJoint::NUM_JOINTS;
	normalizeRotationVec(m_minValues);
	normalizeRotationVec(m_maxValues);

	m_xSegmentFlipped = m_minValues.x > m_maxValues.x;
	m_ySegmentFlipped = m_minValues.y > m_maxValues.y;
	m_zSegmentFlipped = m_minValues.z > m_maxValues.z;
}

JointOrientationRecognizer::JointOrientationRecognizer(Fubi::SkeletonJoint::Joint joint, const Fubi::Vec3f& orientation, float maxAngleDiff /*= 45.0f*/, 
		bool useLocalOrientation /*= true*/, float minConfidence /*= -1.0f*/, bool useFilteredData /*= false*/)
		: m_joint(joint), m_maxAngleDiff(maxAngleDiff), m_useLocalOrientations(useLocalOrientation), m_useOrientation(true), 
	IGestureRecognizer(false, minConfidence, useFilteredData)
{
	m_jointUsableForHandTracking = m_joint < SkeletonHandJoint::NUM_JOINTS;
	Vec3f rot(orientation);
	degToRad(rot);
	// Inverse and transposed rotation matrices are equal, but this is faster to calculate
	m_invertedRotMat = Matrix3f::RotMat(rot).transposed();
}

Fubi::RecognitionResult::Result JointOrientationRecognizer::recognize(const Fubi::SkeletonJointOrientation* joint, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	bool recognized = false;

	if (joint->m_confidence >= m_minConfidence)
	{
		Matrix3f rotMat = joint->m_orientation;
		if (m_useOrientation)
		{
			Vec3f rotDiff = (m_invertedRotMat*rotMat).getRot();
			float angleDiff = rotDiff.length();
			recognized = angleDiff <= m_maxAngleDiff;
			if (!recognized && correctionHint)
			{
				correctionHint->m_joint = m_joint;
				correctionHint->m_isAngle = true;
				correctionHint->m_changeType = RecognitionCorrectionHint::DIRECTION;
				correctionHint->m_dirX = -rotDiff.x;
				correctionHint->m_dirY = -rotDiff.y;
				correctionHint->m_dirZ = -rotDiff.z;
				correctionHint->m_recTarget = m_targetSkeletonType;
			}
		}
		else
		{
			Vec3f orient = rotMat.getRot();
			// Note the special case when a min value is larger than the max value
			// In this case the -180/+180 rotation is between min and max
			// The || operator works as the min values and max values are always normalized to [-180;180]
			bool xInRange = !m_xSegmentFlipped
				? (orient.x >= m_minValues.x && orient.x <= m_maxValues.x)
				: (orient.x >= m_minValues.x || orient.x <= m_maxValues.x);
			bool yInRange = !m_ySegmentFlipped
				? (orient.y >= m_minValues.y && orient.y <= m_maxValues.y)
				: (orient.y >= m_minValues.y || orient.y <= m_maxValues.y);
			bool zInRange = !m_zSegmentFlipped
				? (orient.z >= m_minValues.z && orient.z <= m_maxValues.z)
				: (orient.z >= m_minValues.z || orient.z <= m_maxValues.z);
			recognized = xInRange && yInRange && zInRange;

			if (!recognized && correctionHint)
			{
				correctionHint->m_joint = m_joint;
				correctionHint->m_isAngle = true;
				correctionHint->m_changeType = RecognitionCorrectionHint::POSE;
				correctionHint->m_measuringUnit = BodyMeasurement::NUM_MEASUREMENTS;
				correctionHint->m_changeDirection = RecognitionCorrectionHint::DIFFERENT;
				correctionHint->m_recTarget = m_targetSkeletonType;
				
				// Try to find the optimal turning direction
				
				if ((m_xSegmentFlipped && orient.x < m_minValues.x && orient.x > m_maxValues.x)
					|| orient.x < m_minValues.x || orient.x > m_maxValues.x)
				{
					float toMin = m_minValues.x - orient.x;
					normalizeRotation(toMin);
					float toMax = m_maxValues.x - orient.x;
					normalizeRotation(toMax);
					if (fabsf(toMin) < fabsf(toMax))
						correctionHint->m_dirX = toMin;
					else
						correctionHint->m_dirX = toMax;
				}
				
				if ((m_ySegmentFlipped && orient.y < m_minValues.y && orient.y > m_maxValues.y)
					|| orient.y < m_minValues.y || orient.y > m_maxValues.y)
				{
					float toMin = m_minValues.y - orient.y;
					normalizeRotation(toMin);
					float toMax = m_maxValues.y - orient.y;
					normalizeRotation(toMax);
					if (fabsf(toMin) < fabsf(toMax))
						correctionHint->m_dirY = toMin;
					else
						correctionHint->m_dirY = toMax;
				}

				if ((m_zSegmentFlipped && orient.z < m_minValues.z && orient.z > m_maxValues.z)
					|| orient.z < m_minValues.z || orient.z > m_maxValues.z)
				{
					float toMin = m_minValues.z - orient.z;
					normalizeRotation(toMin);
					float toMax = m_maxValues.z - orient.z;
					normalizeRotation(toMax);
					if (fabsf(toMin) < fabsf(toMax))
						correctionHint->m_dirZ = toMin;
					else
						correctionHint->m_dirZ = toMax;
				}
			}
		}
	}
	else
		return Fubi::RecognitionResult::TRACKING_ERROR;

	return recognized ? Fubi::RecognitionResult::RECOGNIZED : Fubi::RecognitionResult::NOT_RECOGNIZED;
}

Fubi::RecognitionResult::Result JointOrientationRecognizer::recognizeOn(const FubiUser* user, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	const Fubi::TrackingData* data = user->currentTrackingData();
	if (m_useFilteredData)
		data = user->currentFilteredTrackingData();

	if (m_useLocalOrientations)
		return recognize(&(data->localJointOrientations[m_joint]), correctionHint);	
	return recognize(&(data->jointOrientations[m_joint]), correctionHint);
}

Fubi::RecognitionResult::Result JointOrientationRecognizer::recognizeOn(const FubiHand* hand, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	const Fubi::TrackingData* data = hand->currentTrackingData();
	if (m_useFilteredData)
		data = hand->currentFilteredTrackingData();

	if (m_useLocalOrientations)
		return recognize(&(data->localJointOrientations[m_joint]), correctionHint);	
	return recognize(&(data->jointOrientations[m_joint]), correctionHint);
}