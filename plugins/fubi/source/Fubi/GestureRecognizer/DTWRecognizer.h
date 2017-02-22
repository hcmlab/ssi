// ****************************************************************************************
//
// DTW Recognizer
// ---------------------------------------------------------
// Copyright (C) 2010-2014 Felix Kistler 
// 
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
// 
// ****************************************************************************************

#pragma once

#include "IGestureRecognizer.h"

class DTWRecognizer : public IGestureRecognizer
{
public:
	DTWRecognizer(const std::vector<Fubi::SkeletonJoint::Joint>& joints, const std::vector<Fubi::SkeletonJoint::Joint>& relJoints,
		const std::deque<FubiUser::TrackingData>& trainingData, float maxDistance, Fubi::DistanceMeasure::Measure distanceMeasure = Fubi::DistanceMeasure::Euclidean,
		float maxRotation = 45.0f, bool aspectInvariant = false, unsigned int ignoreAxes = Fubi::CoordinateAxis::NONE, bool useOrientations = false,
		const Fubi::BodyMeasurementDistance* bodyMeasures = 0x0, Fubi::BodyMeasurement::Measurement measuringUnit = Fubi::BodyMeasurement::NUM_MEASUREMENTS,
		float minConfidence = -1.0f, bool useLocalTransformations = false, bool useFilteredData = false);
	DTWRecognizer(const std::vector<Fubi::SkeletonHandJoint::Joint>& joints, const std::vector<Fubi::SkeletonHandJoint::Joint>& relJoints,
		const std::deque<FubiHand::FingerTrackingData>& trainingData, float maxDistance, Fubi::DistanceMeasure::Measure distanceMeasure = Fubi::DistanceMeasure::Euclidean,
		float maxRotation = 45.0f, bool aspectInvariant = false, unsigned int ignoreAxes = Fubi::CoordinateAxis::NONE, bool useOrientations = false,
		float minConfidence = -1.0f, bool useLocalTransformations = false, bool useFilteredData = false);
	virtual ~DTWRecognizer() {}

	virtual Fubi::RecognitionResult::Result recognizeOn(const FubiUser* user, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);
	virtual Fubi::RecognitionResult::Result recognizeOn(const FubiHand* hand, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);
	virtual IGestureRecognizer* clone() { return new DTWRecognizer(*this); }

private:
	struct DTWData
	{
		DTWData() : m_joint(Fubi::SkeletonJoint::NUM_JOINTS), m_relJoint(Fubi::SkeletonJoint::NUM_JOINTS) {}
		Fubi::SkeletonJoint::Joint m_joint;
		Fubi::SkeletonJoint::Joint m_relJoint;
		std::vector<Fubi::Vec3f> m_data;
		Fubi::Vec3f m_indicativeOrient;
	};

	void normalizeTrainingData();
	float calculateDTW(const std::vector<Fubi::Vec3f>& trainingData, const Fubi::Vec3f& indicativeOrient, std::vector<Fubi::Vec3f>& testData);
	bool hasTrackingErrors(Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJoint::Joint relJoint, const std::deque<FubiUser::TrackingData*>* data);
	bool hasTrackingErrors(Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJoint::Joint relJoint, const std::deque<FubiHand::FingerTrackingData*>* data);
	static Fubi::Vec3f calculateIndicativeOrient(const std::vector<Fubi::Vec3f>& data);
	Fubi::Vec3f& reduceAxes(Fubi::Vec3f& vec);

	// General recognizer options
	bool m_useLocalTransformations;
	bool m_useOrientations;
	Fubi::BodyMeasurement::Measurement m_measuringUnit;
	
	// The training data
	std::vector<DTWData> m_trainingData;

	// DTW specific options
	float m_maxDist;
	unsigned int m_maxWarpingDistance;
	float m_maxRotation;
	bool m_aspectInvariant;
	unsigned int m_ignoreAxes;
	Fubi::DistanceMeasure::Measure m_distanceMeasure;
	
	// Buffers for converting the data and calculating the dtw warping path
	std::vector<Fubi::Vec3f> m_convertedData;
	std::vector<std::vector<float>> m_dtwDistanceBuffer;
};