// ****************************************************************************************
//
// Fubi Angular Movement Recognizer
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler 
// 
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
// 
// ****************************************************************************************

#include "AngularMovementRecognizer.h"


using namespace Fubi;

AngularMovementRecognizer::AngularMovementRecognizer(Fubi::SkeletonJoint::Joint joint, const Fubi::Vec3f& minAngularVelocity /*= Fubi::DefaultMinVec*/,
		const Fubi::Vec3f& maxAngularVelocity /*= Fubi::DefaultMaxVec*/,
		bool useLocalOrients /*= true*/, float minConfidence /*= -1.0f*/, bool useFilteredData /*= false*/)
		: m_joint(joint), m_minAngularVelocity(minAngularVelocity), m_maxAngularVelocity(maxAngularVelocity), m_useLocalOrients(useLocalOrients),
	  IGestureRecognizer(false, minConfidence, useFilteredData)
{
	m_jointUsableForFingerTracking = m_joint < SkeletonHandJoint::NUM_JOINTS;
}


Fubi::RecognitionResult::Result AngularMovementRecognizer::recognizeOn(const FubiUser* user, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	Fubi::RecognitionResult::Result result = Fubi::RecognitionResult::NOT_RECOGNIZED;
	
	if (user != 0x0)
	{
		// Get joint positions
		const Fubi::TrackingData* data = user->currentTrackingData();
		const Fubi::TrackingData* lastData = user->lastTrackingData();
		if (m_useFilteredData)
		{
			data = user->currentFilteredTrackingData();
			lastData = user->lastFilteredTrackingData();
		}

		float movTime;
		Vec3f angularMovement(Math::NO_INIT);
		if (calcAngularMovement(lastData, data, angularMovement, movTime))
		{
			Vec3f angularVel = angularMovement / movTime;
			// Check if velocity per axis is in between the boundaries
			Vec3f diff = inRange(angularVel, m_minAngularVelocity, m_maxAngularVelocity);
			if (diff == NullVec)
				result = RecognitionResult::RECOGNIZED;
			else if (correctionHint)
			{
				// Wrong speed
				correctionHint->m_joint = m_joint;
				correctionHint->m_isAngle = true;
				correctionHint->m_changeType = RecognitionCorrectionHint::SPEED;
				correctionHint->m_changeDirection = RecognitionCorrectionHint::DIFFERENT;				
				correctionHint->m_dirX = -diff.x;
				correctionHint->m_dirY = -diff.y;
				correctionHint->m_dirZ = -diff.z;
				correctionHint->m_recTarget = m_targetSkeletonType;
			}
		}
		else
			result = Fubi::RecognitionResult::TRACKING_ERROR;
	}

	return result;
}

bool AngularMovementRecognizer::calcAngularMovement(const Fubi::TrackingData* start, const Fubi::TrackingData* end, Vec3f& angularMovement, float& movementTime)
{
	const SkeletonJointOrientation* endJoint = &(end->jointOrientations[m_joint]);
	const SkeletonJointOrientation* startJoint = &(start->jointOrientations[m_joint]);
	if (m_useLocalOrients)
	{
		endJoint = &(end->localJointOrientations[m_joint]);
		startJoint = &(start->localJointOrientations[m_joint]);
	}

	// Check confidence
	if (endJoint->m_confidence >= m_minConfidence && startJoint->m_confidence >= m_minConfidence)
	{
		// Calculate orientation of current and prev frame		
		Vec3f vector = endJoint->m_orientation.getRot();
		Vec3f prevVector = startJoint->m_orientation.getRot();

		// Get the difference between both vectors
		angularMovement = vector - prevVector;

		// Calc time difference (do never return zero here!)
		movementTime = clamp(float(end->timeStamp - start->timeStamp), Math::Epsilon, Math::MaxFloat);
		return true;
	}
	return false;
}

Fubi::RecognitionResult::Result AngularMovementRecognizer::recognizeOn(const FubiHand* hand, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	Fubi::RecognitionResult::Result result = Fubi::RecognitionResult::NOT_RECOGNIZED;
	
	if (m_jointUsableForFingerTracking)
	{
		// Get joint positions
		const Fubi::TrackingData* data = hand->currentTrackingData();
		const Fubi::TrackingData* lastData = hand->lastTrackingData();
		if (m_useFilteredData)
		{
			data = hand->currentFilteredTrackingData();
			lastData = hand->lastFilteredTrackingData();
		}

		float movTime;
		Vec3f angularMovement(Math::NO_INIT);
		if (calcAngularMovement(lastData, data, angularMovement, movTime))
		{
			Vec3f angularVel = angularMovement / movTime;
			// Check if velocity per axis is in between the boundaries
			Vec3f diff = inRange(angularVel, m_minAngularVelocity, m_maxAngularVelocity);
			if (diff == NullVec)
				result = RecognitionResult::RECOGNIZED;
			else if (correctionHint)
			{
				// Wrong speed
				correctionHint->m_joint = m_joint;
				correctionHint->m_isAngle = true;
				correctionHint->m_changeType = RecognitionCorrectionHint::SPEED;
				correctionHint->m_changeDirection = RecognitionCorrectionHint::DIFFERENT;
				correctionHint->m_dirX = -diff.x;
				correctionHint->m_dirY = -diff.y;
				correctionHint->m_dirZ = -diff.z;
				correctionHint->m_recTarget = m_targetSkeletonType;
			}
		}
		else
			result = Fubi::RecognitionResult::TRACKING_ERROR;
	}

	return result;
}