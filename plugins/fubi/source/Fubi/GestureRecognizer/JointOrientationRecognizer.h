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

#pragma once

#include "IGestureRecognizer.h"

class JointOrientationRecognizer : public IGestureRecognizer
{
public:
	// orientation values in degrees
	JointOrientationRecognizer(Fubi::SkeletonJoint::Joint joint, const Fubi::Vec3f& minValues = Fubi::Vec3f(-180.0f,-180.0f,-180.0f), 
		const Fubi::Vec3f& maxValues = Fubi::Vec3f(180.0f, 180.0f, 180.0f), bool useLocalOrientation = true, float minConfidence = -1.0f,
		bool useFilteredData = false);
	JointOrientationRecognizer(Fubi::SkeletonJoint::Joint joint, const Fubi::Vec3f& orientation, float maxAngleDiff = 45.0f, bool useLocalOrientation = true, float minConfidence = -1.0f,
		bool useFilteredData = false);

	virtual ~JointOrientationRecognizer() {}

	virtual Fubi::RecognitionResult::Result recognizeOn(const FubiUser* user, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);
	virtual Fubi::RecognitionResult::Result recognizeOn(const FubiHand* hand, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);

	virtual IGestureRecognizer* clone() { return new JointOrientationRecognizer(*this); }

private:
	Fubi::RecognitionResult::Result recognize(const Fubi::SkeletonJointOrientation* joint, Fubi::RecognitionCorrectionHint* correctionHint);

	Fubi::SkeletonJoint::Joint m_joint;
	bool m_jointUsableForHandTracking;
	Fubi::Vec3f m_minValues, m_maxValues;
	bool m_xSegmentFlipped, m_ySegmentFlipped, m_zSegmentFlipped;
	bool m_useLocalOrientations;

	Fubi::Matrix3f	m_invertedRotMat;
	bool			m_useOrientation;
	float			m_maxAngleDiff;
};