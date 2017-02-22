// ****************************************************************************************
//
// Fubi Linear Movement Recognizer
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler 
// 
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
// 
// ****************************************************************************************

#pragma once

#include "IGestureRecognizer.h"

class LinearMovementRecognizer : public IGestureRecognizer
{
public:
	LinearMovementRecognizer(Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJoint::Joint relJoint, 
		const Fubi::Vec3f& direction, float minVel, float maxVel = Fubi::Math::MaxFloat,
		bool useLocalPos = false, float minConfidence = -1.0f,
		float maxAngleDiff = 45.0f, bool useOnlyCorrectDirectionComponent = true,
		bool useFilteredData = false);
	LinearMovementRecognizer(Fubi::SkeletonJoint::Joint joint, const Fubi::Vec3f& direction,
		float minVel, float maxVel = Fubi::Math::MaxFloat,
		bool useLocalPos = false, float minConfidence = -1.0f,
		float maxAngleDiff = 45.0f, bool useOnlyCorrectDirectionComponent = true,
		bool useFilteredData = false);
	LinearMovementRecognizer(Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJoint::Joint relJoint, 
		const Fubi::Vec3f& direction, float minVel, float maxVel,
		float minLength, float maxLength = Fubi::Math::MaxFloat,
		Fubi::BodyMeasurement::Measurement measuringUnit = Fubi::BodyMeasurement::NUM_MEASUREMENTS,
		bool useLocalPos = false, float minConfidence = -1.0f,
		float maxAngleDiff = 45.0f, bool useOnlyCorrectDirectionComponent = true,
		bool useFilteredData = false);
	LinearMovementRecognizer(Fubi::SkeletonJoint::Joint joint, const Fubi::Vec3f& direction,
		float minVel, float maxVel,
		float minLength, float maxLength = Fubi::Math::MaxFloat,
		Fubi::BodyMeasurement::Measurement measuringUnit = Fubi::BodyMeasurement::NUM_MEASUREMENTS,
		bool useLocalPos = false, float minConfidence = -1.0f,
		float maxAngleDiff = 45.0f, bool useOnlyCorrectDirectionComponent = true,
		bool useFilteredData = false);

	virtual ~LinearMovementRecognizer() {}

	virtual Fubi::RecognitionResult::Result recognizeOn(const FubiUser* user, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);
	virtual Fubi::RecognitionResult::Result recognizeWithHistory(const FubiUser* user, const Fubi::TrackingData* initialData, 
		const Fubi::TrackingData* initialFilteredData, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);

	virtual Fubi::RecognitionResult::Result recognizeOn(const FubiHand* hand, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);
	virtual Fubi::RecognitionResult::Result recognizeWithHistory(const FubiHand* hand, const Fubi::TrackingData* initialData, 
		const Fubi::TrackingData* initialFilteredData, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);

	virtual bool useHistory() { return m_lengthValid; }

	virtual IGestureRecognizer* clone() { return new LinearMovementRecognizer(*this); }

private:
	Fubi::RecognitionResult::Result recognizeOn(const Fubi::TrackingData* data, const Fubi::TrackingData* lastData, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);

	Fubi::RecognitionResult::Result recognizeWithHistory(const Fubi::TrackingData* startData, const Fubi::TrackingData* endData, const Fubi::TrackingData* lastData,
		const Fubi::BodyMeasurementDistance* bodyMeasure = 0x0, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);

	bool calcMovement(const Fubi::TrackingData* start, const Fubi::TrackingData* end, float& movementLength, float& movementTime,
		float& directionAngleDiff, Fubi::Vec3f& movement);

	Fubi::SkeletonJoint::Joint m_joint;
	Fubi::SkeletonJoint::Joint m_relJoint;
	bool m_jointsUsableForFingerTracking;
	Fubi::Vec3f m_direction;
	bool m_directionValid;
	bool m_useOnlyCorrectDirectionComponent;
	float m_minVel;
	float m_maxVel;
	bool m_useRelJoint;
	bool m_useLocalPos;
	float m_maxAngleDiff;
	float m_minLength;
	float m_maxLength;
	Fubi::BodyMeasurement::Measurement m_measuringUnit;
	bool m_lengthValid;
};