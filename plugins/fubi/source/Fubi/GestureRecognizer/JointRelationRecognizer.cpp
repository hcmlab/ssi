// ****************************************************************************************
//
// Joint Relation Recognizers
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler 
// 
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
// 
// ****************************************************************************************
#include "JointRelationRecognizer.h"

using namespace Fubi;

JointRelationRecognizer::JointRelationRecognizer(Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJoint::Joint relJoint, 
	const Fubi::Vec3f& minValues /*= Fubi::DefaultMinVec*/, 
	const Fubi::Vec3f& maxValues /*= Fubi::DefaultMaxVec*/, 
	float minDistance /*= 0*/, 
	float maxDistance /*= Fubi::Math::MaxFloat*/,
	Fubi::SkeletonJoint::Joint midJoint /*= Fubi::SkeletonJoint::NUM_JOINTS*/,
	const Fubi::Vec3f& midJointMinValues /*= Fubi::DefaultMinVec*/, 
	const Fubi::Vec3f& midJointMaxValues /*= Fubi::DefaultMaxVec*/,
	float midJointMinDistance /*= 0*/, 
	float midJointMaxDistance /*= Fubi::Math::MaxFloat*/,
	bool useLocalPositions /*=false*/,
	float minConfidence /*= -1.0f*/,
	Fubi::BodyMeasurement::Measurement measuringUnit /*= Fubi::BodyMeasurement::NUM_MEASUREMENTS*/,
	bool useFilteredData /*= false*/)
	: m_joint(joint), m_relJoint(relJoint),
	  m_minValues(minValues), m_maxValues(maxValues), m_minDistance(minDistance), m_maxDistance(maxDistance), 
	  m_midJoint(midJoint), m_midJointMinValues(midJointMinValues), m_midJointMaxValues(midJointMaxValues),
	  m_midJointMinDistance(midJointMinDistance), m_midJointMaxDistance(midJointMaxDistance),
	  m_useLocalPositions(useLocalPositions),
	  m_measuringUnit(measuringUnit),
	  IGestureRecognizer(false, minConfidence, useFilteredData)
{
	// It is of course a bit hacky to use the skeleton joint enum for the hand joint values, but it saves a lot of work...
	m_jointsUsableForHandTracking = m_joint < SkeletonHandJoint::NUM_JOINTS && (m_relJoint < SkeletonHandJoint::NUM_JOINTS || m_relJoint == SkeletonJoint::NUM_JOINTS) && (m_midJoint < SkeletonHandJoint::NUM_JOINTS || m_midJoint == SkeletonJoint::NUM_JOINTS);
}

Fubi::RecognitionResult::Result JointRelationRecognizer::recognize(const SkeletonJointPosition* joint, const SkeletonJointPosition* relJoint, 
	const Fubi::SkeletonJointPosition* midJoint, const Fubi::BodyMeasurementDistance* measure /*=0x0*/, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	bool recognized = false;
	if (joint->m_confidence >= m_minConfidence)
	{
		bool vecValid = false;
		Vec3f vector(Fubi::Math::NO_INIT);
		
		if (relJoint)
		{
			vecValid = relJoint->m_confidence >= m_minConfidence;
			if(vecValid)
			{
				vector = joint->m_position - relJoint->m_position;
			}
		}
		else
		{
			vector = joint->m_position;
			vecValid = true;
		}
			
		if (vecValid && measure != 0x0)
		{
			if (measure->m_confidence >= m_minConfidence && measure->m_dist > Math::Epsilon)
				vector /= measure->m_dist;
			else
				vecValid = false;
		}

		if (vecValid)
		{			

			float distance = vector.length();		

			Vec3f posInRange = inRange(vector, m_minValues, m_maxValues);
			float distInRange = inRange(distance, m_minDistance, m_maxDistance);
			recognized = posInRange == NullVec && distInRange == 0;

			if (!recognized && correctionHint)
			{
				correctionHint->m_changeType = RecognitionCorrectionHint::POSE;
				correctionHint->m_joint = m_joint;
				correctionHint->m_isAngle = false;
				correctionHint->m_changeDirection = RecognitionCorrectionHint::DIFFERENT;
				correctionHint->m_dirX = -posInRange.x;
				correctionHint->m_dirY = -posInRange.y;
				correctionHint->m_dirZ = -posInRange.z;
				correctionHint->m_dist = -distInRange;
				correctionHint->m_measuringUnit = m_measuringUnit;
				correctionHint->m_recTarget = m_targetSkeletonType;
			}
			else if (recognized && relJoint && midJoint)
			{
				if (midJoint->m_confidence < m_minConfidence)
					return Fubi::RecognitionResult::TRACKING_ERROR;

				Vec3f vector2(Math::NO_INIT);

				if (measure != 0x0 && measure->m_confidence >= m_minConfidence && measure->m_dist > Math::Epsilon)
					vector2 = (midJoint->m_position - closestPointFromPointToRay(midJoint->m_position, relJoint->m_position, vector*measure->m_dist, true)) / measure->m_dist;
				else
					vector2 = midJoint->m_position - closestPointFromPointToRay(midJoint->m_position, relJoint->m_position, vector, true);

				float distance = vector2.length();

				Vec3f posInRange = inRange(vector2, m_midJointMinValues, m_midJointMaxValues);
				distInRange = inRange(distance, m_midJointMinDistance, m_midJointMaxDistance);
				recognized = posInRange == NullVec && distInRange == 0;

				if (!recognized && correctionHint)
				{
					correctionHint->m_changeType = RecognitionCorrectionHint::POSE;
					correctionHint->m_joint = m_midJoint;
					correctionHint->m_isAngle = false;
					correctionHint->m_changeDirection = RecognitionCorrectionHint::DIFFERENT;
					correctionHint->m_dirX = -posInRange.x;
					correctionHint->m_dirY = -posInRange.y;
					correctionHint->m_dirZ = -posInRange.z;
					correctionHint->m_dist = -distInRange;
					correctionHint->m_measuringUnit = m_measuringUnit;
					correctionHint->m_recTarget = m_targetSkeletonType;
					
				}
			}
		}
		else
			return Fubi::RecognitionResult::TRACKING_ERROR;
	}
	else
		return Fubi::RecognitionResult::TRACKING_ERROR;

	return recognized ? Fubi::RecognitionResult::RECOGNIZED : Fubi::RecognitionResult::NOT_RECOGNIZED;
}

Fubi::RecognitionResult::Result JointRelationRecognizer::recognizeOn(const FubiUser* user, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	const Fubi::TrackingData* data = user->currentTrackingData();
	if (m_useFilteredData)
		data = user->currentFilteredTrackingData();

	const SkeletonJointPosition* joint = &(data->jointPositions[m_joint]);
	if (m_useLocalPositions)
		joint = &(data->localJointPositions[m_joint]);
	const SkeletonJointPosition* relJoint = 0x0;
	if (m_relJoint != Fubi::SkeletonJoint::NUM_JOINTS)
	{
		relJoint = &(data->jointPositions[m_relJoint]);
		if (m_useLocalPositions)
			relJoint = &(data->localJointPositions[m_relJoint]);
	}
	const SkeletonJointPosition* midJoint = 0x0;
	if (m_midJoint != Fubi::SkeletonJoint::NUM_JOINTS)
	{
		midJoint = &(data->jointPositions[m_midJoint]);
		if (m_useLocalPositions)
			midJoint = &(data->localJointPositions[m_midJoint]);
	}
	const BodyMeasurementDistance* measure = 0x0;
	if (m_measuringUnit != BodyMeasurement::NUM_MEASUREMENTS)
		measure = &(user->bodyMeasurements()[m_measuringUnit]);

	return recognize(joint, relJoint, midJoint, measure, correctionHint);
}

Fubi::RecognitionResult::Result JointRelationRecognizer::recognizeOn(const FubiHand* hand, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	if (m_jointsUsableForHandTracking)
	{
		const Fubi::TrackingData* data = hand->currentTrackingData();
		if (m_useFilteredData)
			data = hand->currentFilteredTrackingData();

		const SkeletonJointPosition* joint = &(data->jointPositions[m_joint]);
		if (m_useLocalPositions)
			joint = &(data->localJointPositions[m_joint]);
		const SkeletonJointPosition* relJoint = 0x0;
		if (m_relJoint != Fubi::SkeletonJoint::NUM_JOINTS)
		{
			relJoint = &(data->jointPositions[m_relJoint]);
			if (m_useLocalPositions)
				relJoint = &(data->localJointPositions[m_relJoint]);
		}
		const SkeletonJointPosition* midJoint = 0x0;
		if (m_midJoint != Fubi::SkeletonJoint::NUM_JOINTS)
		{
			midJoint = &(data->jointPositions[m_midJoint]);
			if (m_useLocalPositions)
				midJoint = &(data->localJointPositions[m_midJoint]);
		}
		return recognize(joint, relJoint, midJoint, 0x0, correctionHint);
	}
	return Fubi::RecognitionResult::NOT_RECOGNIZED;
}