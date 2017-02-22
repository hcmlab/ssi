// ****************************************************************************************
//
// Template Recognizer
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler 
// 
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
// 
// ****************************************************************************************

#pragma once

#include "IGestureRecognizer.h"

class TemplateRecognizer : public IGestureRecognizer
{
public:
	// Constructor for user tracking data
	TemplateRecognizer(const std::vector<Fubi::SkeletonJoint::Joint>& joints, const std::vector<Fubi::SkeletonJoint::Joint>& relJoints,
		const std::vector<std::deque<Fubi::TrackingData>>& trainingData, float maxDistance, Fubi::DistanceMeasure::Measure distanceMeasure = Fubi::DistanceMeasure::Euclidean,
		float maxRotation = 45.0f, bool aspectInvariant = false, unsigned int ignoreAxes = Fubi::CoordinateAxis::NONE, bool useOrientations = false,
		bool useDTW = true, float maxWarpingFactor = 0.5f, Fubi::ResamplingTechnique::Technique resamplingTechnique = Fubi::ResamplingTechnique::None, int resampleSize = -1,
		bool searchBestInputLength = false, bool useGMR = true, unsigned int numGMRStates = 5, const Fubi::BodyMeasurementDistance* bodyMeasures = 0x0,
		Fubi::BodyMeasurement::Measurement measuringUnit = Fubi::BodyMeasurement::NUM_MEASUREMENTS,
		float minConfidence = -1.0f, bool useLocalTransformations = false, bool useFilteredData = true);
	// Constructor for finger tracking data
	TemplateRecognizer(const std::vector<Fubi::SkeletonHandJoint::Joint>& joints, const std::vector<Fubi::SkeletonHandJoint::Joint>& relJoints,
		const std::vector<std::deque<Fubi::TrackingData>>& trainingData, float maxDistance, Fubi::DistanceMeasure::Measure distanceMeasure = Fubi::DistanceMeasure::Euclidean,
		float maxRotation = 45.0f, bool aspectInvariant = false, unsigned int ignoreAxes = Fubi::CoordinateAxis::NONE, bool useOrientations = false,
		bool useDTW = true, float maxWarpingFactor = 0.5f, Fubi::ResamplingTechnique::Technique resamplingTechnique = Fubi::ResamplingTechnique::None, int resampleSize = -1,
		bool searchBestInputLength = false, bool useGMR = true, unsigned int numGMRStates = 5, float minConfidence = -1.0f,
		bool useLocalTransformations = false, bool useFilteredData = true);
	virtual ~TemplateRecognizer() {}

	// Recognition for user tracking data
	virtual Fubi::RecognitionResult::Result recognizeOn(const FubiUser* user, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);
	// Recognition for finger tracking data
	virtual Fubi::RecognitionResult::Result recognizeOn(const FubiHand* hand, Fubi::RecognitionCorrectionHint* correctionHint = 0x0);
	virtual IGestureRecognizer* clone() { return new TemplateRecognizer(*this); }

	const std::vector<Fubi::TemplateData>& getTrainingData() { return m_trainingData; }

protected:
	// Protected constructor only for usage in base classes to init the protected members
	TemplateRecognizer(float maxDistance, Fubi::DistanceMeasure::Measure distanceMeasure, float maxRotation, bool aspectInvariant, 
		unsigned int ignoreAxes, bool useOrientations, Fubi::ResamplingTechnique::Technique resamplingTechnique, int resampleSize,
		const Fubi::BodyMeasurementDistance* bodyMeasures, Fubi::BodyMeasurement::Measurement measuringUnit, 
		float minConfidence, bool useLocalTransformations, bool useFilteredData);

	void extractTrainingData(const std::vector<std::deque<Fubi::TrackingData>>& trainingData, 
		Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJoint::Joint relJoint, const Fubi::BodyMeasurementDistance* measure,
		std::vector<std::vector<Fubi::Vec3f>> &convertedData, std::vector<Fubi::Vec3f>* indicativeOrients = 0x0);
	void calculateMeanAndCovs(Fubi::TemplateData& data, const std::vector<std::vector<Fubi::Vec3f>>& convertedData, const std::vector<Fubi::Vec3f>& indicativeOrients);
	float calculateDistanceAtBestDataLength(const Fubi::TemplateData& trainingData, std::vector<Fubi::Vec3f>& testData, Fubi::RecognitionCorrectionHint* correctionHint);
	float calculateDistance(const Fubi::TemplateData& trainingData, std::vector<Fubi::Vec3f>* testData, Fubi::RecognitionCorrectionHint* correctionHint);
	bool hasTrackingErrors(Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJoint::Joint relJoint, const std::deque<Fubi::TrackingData*>* data);
	Fubi::Vec3f calculateIndicativeOrient(const std::vector<Fubi::Vec3f>& data);
	Fubi::Vec3f& reduceAxes(Fubi::Vec3f& vec);

	// General recognizer options
	bool m_useLocalTransformations;
	bool m_useOrientations;
	Fubi::BodyMeasurement::Measurement m_measuringUnit;
	
	// Recognizer specific options
	float m_maxDist;
	float m_maxRotation;
	bool m_aspectInvariant;
	unsigned int m_ignoreAxes, m_numIgnoredAxes;
	Fubi::DistanceMeasure::Measure m_distanceMeasure;
	Fubi::ResamplingTechnique::Technique m_resamplingTechnique;
	int m_resampleSize;

private:
	unsigned int calcNumIgnoredAxesAndMinAabbSize();
	// The converted training data
	std::vector<Fubi::TemplateData> m_trainingData;
	// Buffer for converting (i.e. normalizing) the input data during recognition
	std::vector<Fubi::Vec3f> m_convertedData;

	// GMR options
	bool m_useGMR;
	unsigned int m_numGMRStates;

	// DTW and GSS options
	bool m_useDTW;
	int m_maxSampleSize, m_requiredSampleSize;
	float m_minAabbSize;
	// Buffer for calculating the dtw warping path
	std::vector<std::vector<float>> m_dtwDistanceBuffer;
	float m_maxWarpingFac;
	bool m_useGSS;
};