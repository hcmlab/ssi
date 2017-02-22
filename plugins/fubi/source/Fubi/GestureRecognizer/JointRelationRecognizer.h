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

#pragma once

#include "IGestureRecognizer.h"

class JointRelationRecognizer : public IGestureRecognizer
{
public:
	// +-MaxFloat are the default values, as they represent no restriction
	JointRelationRecognizer(Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJoint::Joint relJoint,
		const Fubi::Vec3f& minValues = Fubi::DefaultMinVec, 
		const Fubi::Vec3f& maxValues = Fubi::DefaultMaxVec,
		float minDistance = 0, 
		float maxDistance = Fubi::Math::MaxFloat,
		Fubi::SkeletonJoint::Joint midJoint = Fubi::SkeletonJoint::NUM_JOINTS,
		const Fubi::Vec3f& midJointMinValues = Fubi::DefaultMinVec, 
		const Fubi::Vec3f& midJointMaxValues = Fubi::DefaultMaxVec,
		float midJointMinDistance = 0, 
		float midJointMaxDistance = Fubi::Math::MaxFloat,
		bool useLocalPositions = false,
		float minConfidence = -1.0f,
		Fubi::BodyMeasurement::Measurement measuringUnit = Fubi::BodyMeasurement::NUM_MEASUREMENTS,
		bool useFilteredData = false);

	virtual ~JointRelationRecognizer() {}

	virtual Fubi::RecognitionResult::Result recognizeOn(const FubiUser* user, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);
	virtual Fubi::RecognitionResult::Result recognizeOn(const FubiHand* hand, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);

	virtual IGestureRecognizer* clone() { return new JointRelationRecognizer(*this); }

private:
	Fubi::RecognitionResult::Result recognize(const Fubi::SkeletonJointPosition* joint, const Fubi::SkeletonJointPosition* relJoint,
		const Fubi::SkeletonJointPosition* midJoint, const Fubi::BodyMeasurementDistance* measure = 0x0, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);

	Fubi::SkeletonJoint::Joint m_joint;
	Fubi::SkeletonJoint::Joint m_relJoint;
	bool m_jointsUsableForHandTracking;
	Fubi::Vec3f m_minValues, m_maxValues;
	float m_minDistance, m_maxDistance;
	Fubi::SkeletonJoint::Joint m_midJoint;
	Fubi::Vec3f m_midJointMinValues, m_midJointMaxValues;
	float m_midJointMinDistance, m_midJointMaxDistance;
	bool m_useLocalPositions;
	Fubi::BodyMeasurement::Measurement m_measuringUnit;
};