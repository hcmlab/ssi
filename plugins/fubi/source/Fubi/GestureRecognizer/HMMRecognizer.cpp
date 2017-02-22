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
#include "HMMRecognizer.h"

using namespace Fubi;

HMMRecognizer::HMMRecognizer(const std::vector<Fubi::SkeletonJoint::Joint>& joints, const std::vector<Fubi::SkeletonJoint::Joint>& relJoints,
	const std::vector<std::deque<Fubi::TrackingData>>& trainingData, float maxDistance, Fubi::DistanceMeasure::Measure distanceMeasure /*= Fubi::DistanceMeasure::Euclidean*/,
	float maxRotation /*= 45.0f*/, bool aspectInvariant /*= false*/, unsigned int ignoreAxes /*= Fubi::CoordinateAxis::NONE*/, bool useOrientations /*=false*/,
	Fubi::ResamplingTechnique::Technique resamplingTechnique /*= Fubi::ResamplingTechnique::None*/, int resampleSize /*= -1*/,
	const Fubi::BodyMeasurementDistance* bodyMeasures /*= 0x0*/, Fubi::BodyMeasurement::Measurement measuringUnit /*= Fubi::BodyMeasurement::NUM_MEASUREMENTS*/,
	float minConfidence /*= -1.0f*/, bool useLocalTransformations /*= false*/, bool useFilteredData /*= true*/)
	: TemplateRecognizer(maxDistance, distanceMeasure, maxRotation, aspectInvariant, ignoreAxes, useOrientations, resamplingTechnique, resampleSize,
		bodyMeasures, measuringUnit, minConfidence, useLocalTransformations, useFilteredData)
{
	// Constructor for user tracking data
	if (joints.size() > 0 && trainingData.size() > 0)
	{
		// If a body measure is given, all data is divided by that length
		const BodyMeasurementDistance* measure = (m_measuringUnit == BodyMeasurement::NUM_MEASUREMENTS)
			? 0x0 : &(bodyMeasures[m_measuringUnit]);

		// It can be the case that multiple joints are to be used of the training data
		for (unsigned int j = 0; j < joints.size(); ++j)
		{
			// Get the joints and replace with NUM_JOINTS if empty
			m_joints.push_back(joints[j]);
			m_relJoints.push_back((relJoints.size() > j) ? relJoints[j] : SkeletonJoint::NUM_JOINTS);

			// Find and convert the data
			std::vector<std::vector<Vec3f>> convertedData;
			extractTrainingData(trainingData, m_joints.back(), m_relJoints.back(), measure, convertedData);

			// TODO: Now use convertedData to train the HMM, maybe first gather the data of the different joints
		}
	}
}
HMMRecognizer::HMMRecognizer(const std::vector<Fubi::SkeletonHandJoint::Joint>& joints, const std::vector<Fubi::SkeletonHandJoint::Joint>& relJoints,
	const std::vector<std::deque<Fubi::TrackingData>>& trainingData, float maxDistance, Fubi::DistanceMeasure::Measure distanceMeasure /*= Fubi::DistanceMeasure::Euclidean*/,
	float maxRotation /*= 45.0f*/, bool aspectInvariant /*= false*/, unsigned int ignoreAxes /*= Fubi::CoordinateAxis::NONE*/, bool useOrientations /*=false*/,
	Fubi::ResamplingTechnique::Technique resamplingTechnique /*= Fubi::ResamplingTechnique::None*/, int resampleSize /*= -1*/,
	float minConfidence /*= -1.0f*/, bool useLocalTransformations /*= false*/, bool useFilteredData /*= true*/)
	: TemplateRecognizer(maxDistance, distanceMeasure, maxRotation, aspectInvariant, ignoreAxes, useOrientations, resamplingTechnique, resampleSize,
	0x0, BodyMeasurement::NUM_MEASUREMENTS, minConfidence, useLocalTransformations, useFilteredData)
{
	//Constructor for finger tracking data

	if (joints.size() > 0 && trainingData.size() > 0)
	{
		for (unsigned int j = 0; j < joints.size(); ++j)
		{
			// Get the joints, convert from SkeletonHandJoint to SkeletonJoint, and replace with NUM_JOINTS if empty
			m_joints.push_back((joints[j] != SkeletonHandJoint::NUM_JOINTS) ? (SkeletonJoint::Joint) joints[j] : SkeletonJoint::NUM_JOINTS);
			m_relJoints.push_back((relJoints.size() > j && relJoints[j] != SkeletonHandJoint::NUM_JOINTS) ? (SkeletonJoint::Joint) relJoints[j] : SkeletonJoint::NUM_JOINTS);

			// Find and convert the data
			std::vector<std::vector<Vec3f>> convertedData;
			extractTrainingData(trainingData, m_joints.back(), m_relJoints.back(), 0x0, convertedData);

			// TODO: Now use convertedData to train the HMM, maybe first gather the data of the different joints
		}
	}
}

RecognitionResult::Result HMMRecognizer::recognizeOn(const FubiUser* user, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	// Recognition for user tracking data
	if (user == 0x0)
		return RecognitionResult::NOT_RECOGNIZED;

	const Fubi::TrackingData* data = user->currentTrackingData();
	if (m_useFilteredData)
		data = user->currentFilteredTrackingData();
	// If a body meaure is provided, all data will be divided by it
	const BodyMeasurementDistance* measure = 0x0;
	if (m_measuringUnit != BodyMeasurement::NUM_MEASUREMENTS)
		measure = &user->bodyMeasurements()[m_measuringUnit];
	return recognizeOn(data, measure, correctionHint);
}

RecognitionResult::Result HMMRecognizer::recognizeOn(const FubiHand* hand, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	// Recognition for finger tracking data
	if (hand == 0x0)
		return RecognitionResult::NOT_RECOGNIZED;

	const Fubi::TrackingData* data = hand->currentTrackingData();
	if (m_useFilteredData)
		data = hand->currentFilteredTrackingData();
	return recognizeOn(data, 0x0, correctionHint);
}

RecognitionResult::Result HMMRecognizer::recognizeOn(const Fubi::TrackingData* data, const BodyMeasurementDistance* measure /*= 0x0*/, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	RecognitionResult::Result result = RecognitionResult::NOT_RECOGNIZED;

	// Extract the data for the required joints
	for (unsigned int j = 0; j < m_joints.size(); ++j)
	{
		SkeletonJoint::Joint joint = m_joints[j];
		SkeletonJoint::Joint relJoint = m_relJoints[j];
		if (data->jointPositions[joint].m_confidence < m_minConfidence || data->jointPositions[relJoint].m_confidence < m_minConfidence)
		{
			result = RecognitionResult::TRACKING_ERROR;
			break;
		}
		else
		{
			// Extract the relevant part of the data
			Vec3f inputVec;
			if (m_useOrientations)
			{
				const SkeletonJointOrientation* jointOrient = m_useLocalTransformations
					? &(data->localJointOrientations[joint])
					: &(data->jointOrientations[joint]);
				// Don't forget to reduce the axes
				inputVec = jointOrient->m_orientation.getRot(false);
				reduceAxes(inputVec);
			}
			else
			{
				inputVec = m_useLocalTransformations
					? data->localJointPositions[joint].m_position
					: data->jointPositions[joint].m_position;
				if (relJoint != SkeletonJoint::NUM_JOINTS)
				{
					if (m_useLocalTransformations)
						inputVec -= data->localJointPositions[relJoint].m_position;
					else
						inputVec -= data->jointPositions[relJoint].m_position;
				}
				if (measure && measure->m_confidence > m_minConfidence && measure->m_dist > Math::Epsilon)
				{
					// Apply measuring unit
					inputVec /= measure->m_dist;
				}
				// Don't forget to reduce the axes
				reduceAxes(inputVec);
			}

			// TODO: Add the inputVec to the HMM for that specific user
		}
	}

	if (result != RecognitionResult::TRACKING_ERROR)
	{
		// TODO: check if gesture is recognized and calculate a distance score
		float dist = Math::MaxFloat;
		if (dist < m_maxDist)
			result = Fubi::RecognitionResult::RECOGNIZED;
		// Always set correction hint to notify about the current distance
		if (correctionHint)
		{
			correctionHint->m_dist = dist;
			correctionHint->m_changeType = RecognitionCorrectionHint::FORM;
			correctionHint->m_changeDirection = RecognitionCorrectionHint::DIFFERENT;
		}
	}
	else
		result = Fubi::RecognitionResult::TRACKING_ERROR;
	return result;
}
