// ****************************************************************************************
//
// HMM Recognizer
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler 
// 
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
// 
// ****************************************************************************************

#pragma once

#include "TemplateRecognizer.h"

class HMMRecognizer : public TemplateRecognizer
{
public:
	// Constructor for user tracking data
	HMMRecognizer(const std::vector<Fubi::SkeletonJoint::Joint>& joints, const std::vector<Fubi::SkeletonJoint::Joint>& relJoints,
		const std::vector<std::deque<Fubi::TrackingData>>& trainingData, float maxDistance, Fubi::DistanceMeasure::Measure distanceMeasure = Fubi::DistanceMeasure::Euclidean,
		float maxRotation = 45.0f, bool aspectInvariant = false, unsigned int ignoreAxes = Fubi::CoordinateAxis::NONE, bool useOrientations = false,
		Fubi::ResamplingTechnique::Technique resamplingTechnique = Fubi::ResamplingTechnique::None, int resampleSize = -1, 
		const Fubi::BodyMeasurementDistance* bodyMeasures = 0x0, Fubi::BodyMeasurement::Measurement measuringUnit = Fubi::BodyMeasurement::NUM_MEASUREMENTS,
		float minConfidence = -1.0f, bool useLocalTransformations = false, bool useFilteredData = true);
	// Constructor for finger tracking data
	HMMRecognizer(const std::vector<Fubi::SkeletonHandJoint::Joint>& joints, const std::vector<Fubi::SkeletonHandJoint::Joint>& relJoints,
		const std::vector<std::deque<Fubi::TrackingData>>& trainingData, float maxDistance, Fubi::DistanceMeasure::Measure distanceMeasure = Fubi::DistanceMeasure::Euclidean,
		float maxRotation = 45.0f, bool aspectInvariant = false, unsigned int ignoreAxes = Fubi::CoordinateAxis::NONE, bool useOrientations = false,
		Fubi::ResamplingTechnique::Technique resamplingTechnique = Fubi::ResamplingTechnique::None, int resampleSize = -1,
		float minConfidence = -1.0f, bool useLocalTransformations = false, bool useFilteredData = true);
	
	virtual ~HMMRecognizer() {}

	// Recognition for user tracking data
	virtual Fubi::RecognitionResult::Result recognizeOn(const FubiUser* user, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);
	// Recognition for finger tracking data
	virtual Fubi::RecognitionResult::Result recognizeOn(const FubiHand* hand, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);

	virtual IGestureRecognizer* clone() { return new HMMRecognizer(*this); }

private:
	Fubi::RecognitionResult::Result recognizeOn(const Fubi::TrackingData* data, const Fubi::BodyMeasurementDistance* measure = 0x0, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);
	// Used joints, assured to have the same size in the constructor
	std::vector<Fubi::SkeletonJoint::Joint> m_joints;
	std::vector<Fubi::SkeletonJoint::Joint> m_relJoints;
};