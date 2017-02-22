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
#include "DTWRecognizer.h"

using namespace Fubi;

DTWRecognizer::DTWRecognizer(const std::vector<Fubi::SkeletonJoint::Joint>& joints, const std::vector<Fubi::SkeletonJoint::Joint>& relJoints,
	const std::deque<FubiUser::TrackingData>& trainingData, float maxDistance, Fubi::DistanceMeasure::Measure distanceMeasure /*= Fubi::DistanceMeasure::Euclidean*/,
	float maxRotation /*= 45.0f*/, bool aspectInvariant /*= false*/, unsigned int ignoreAxes /*= Fubi::CoordinateAxis::NONE*/, bool useOrientations /*=false*/, const Fubi::BodyMeasurementDistance* bodyMeasures /*= 0x0*/,
	Fubi::BodyMeasurement::Measurement measuringUnit /*= Fubi::BodyMeasurement::NUM_MEASUREMENTS*/,
	float minConfidence /*= -1.0f*/, bool useLocalTransformations /*= false*/, bool useFilteredData /*= false*/)
	: m_maxDist(maxDistance), m_distanceMeasure(distanceMeasure), m_maxRotation(degToRad(maxRotation)), m_aspectInvariant(aspectInvariant),
	m_ignoreAxes(ignoreAxes), m_useOrientations(useOrientations),	m_measuringUnit(measuringUnit), m_useLocalTransformations(useLocalTransformations),
	IGestureRecognizer(false, minConfidence, useFilteredData)
{
	if (joints.size() > 0 && trainingData.size() > 0)
	{
		// Find the data
		if (m_useOrientations)
		{
			for (unsigned int j = 0; j < joints.size(); ++j)
			{
				DTWData dtwData;
				dtwData.m_joint = joints[j];
				for (auto iter = trainingData.begin(); iter != trainingData.end(); ++iter)
				{				
					const SkeletonJointOrientation* jointOrient = m_useLocalTransformations
						? &(iter->localJointOrientations[dtwData.m_joint])
						: &(iter->jointOrientations[dtwData.m_joint]);
					// We normalize the rotations later, so we don't care if they are in degrees or radians
					Vec3f rot = jointOrient->m_orientation.getRot(false);
					// Reduce axes
					dtwData.m_data.push_back(reduceAxes(rot));
				}
				m_trainingData.push_back(dtwData);
			}
		}
		else
		{
			const BodyMeasurementDistance* measure = (m_measuringUnit == BodyMeasurement::NUM_MEASUREMENTS)
				? 0x0 : &(bodyMeasures[m_measuringUnit]);
			for (unsigned int j = 0; j < joints.size(); ++j)
			{
				DTWData dtwData;
				dtwData.m_joint = joints[j];
				dtwData.m_relJoint = (relJoints.size() > j) ? relJoints[j] : SkeletonJoint::NUM_JOINTS;
				for (auto iter = trainingData.begin(); iter != trainingData.end(); ++iter)
				{
					const SkeletonJointPosition* jointPos = m_useLocalTransformations
						? &(iter->localJointPositions[dtwData.m_joint])
						: &(iter->jointPositions[dtwData.m_joint]);
					const SkeletonJointPosition* relJointPos = 0x0;
					if (dtwData.m_relJoint != SkeletonJoint::NUM_JOINTS)
					{
						relJointPos = m_useLocalTransformations
							? &(iter->localJointPositions[dtwData.m_relJoint])
							: &(iter->jointPositions[dtwData.m_relJoint]);
					}
					// Calculate the final coordinate values to be used for training
					Vec3f point = jointPos->m_position;
					if (relJointPos)
						point -= relJointPos->m_position;
					if (measure && measure->m_dist > Math::Epsilon)
						point /= measure->m_dist;
					// Always reduce the axes before the normalizations
					dtwData.m_data.push_back(reduceAxes(point));
				}
				m_trainingData.push_back(dtwData);
			}
		}
		normalizeTrainingData();
	}
}
DTWRecognizer::DTWRecognizer(const std::vector<Fubi::SkeletonHandJoint::Joint>& joints, const std::vector<Fubi::SkeletonHandJoint::Joint>& relJoints,
	const std::deque<FubiHand::FingerTrackingData>& trainingData, float maxDistance, Fubi::DistanceMeasure::Measure distanceMeasure /*= Fubi::DistanceMeasure::Euclidean*/,
	float maxRotation /*= 45.0f*/, bool aspectInvariant /*= false*/, unsigned int ignoreAxes /*= Fubi::CoordinateAxis::NONE*/, bool useOrientations /*=false*/,
	float minConfidence /*= -1.0f*/, bool useLocalTransformations /*= false*/, bool useFilteredData /*= false*/)
	: m_maxDist(maxDistance), m_distanceMeasure(distanceMeasure), m_maxRotation(degToRad(maxRotation)), m_aspectInvariant(aspectInvariant),
	m_ignoreAxes(ignoreAxes), m_useOrientations(useOrientations), m_measuringUnit(Fubi::BodyMeasurement::NUM_MEASUREMENTS), m_useLocalTransformations(useLocalTransformations),
	IGestureRecognizer(false, minConfidence, useFilteredData)
{
	if (joints.size() > 0 && trainingData.size() > 0)
	{
		// Find the data
		if (m_useOrientations)
		{
			for (unsigned int j = 0; j < joints.size(); ++j)
			{
				DTWData dtwData;
				// Convert the hand joint to a skeleton joint, so that it can be used in the same manner
				dtwData.m_joint = (const SkeletonJoint::Joint)joints[j];
				for (auto iter = trainingData.begin(); iter != trainingData.end(); ++iter)
				{
					const SkeletonJointOrientation* jointOrient = m_useLocalTransformations
						? &(iter->localJointOrientations[dtwData.m_joint])
						: &(iter->jointOrientations[dtwData.m_joint]);
					// We normalize the rotations later, so we don't care if they are in degrees or radians
					Vec3f rot = jointOrient->m_orientation.getRot(false);
					// Reduce axes
					dtwData.m_data.push_back(reduceAxes(rot));
				}
				m_trainingData.push_back(dtwData);
			}
		}
		else
		{
			for (unsigned int j = 0; j < joints.size(); ++j)
			{
				DTWData dtwData;
				// Convert the hand joints to skeleton joints, so that they can be used in the same manner
				dtwData.m_joint = (const SkeletonJoint::Joint)joints[j];
				dtwData.m_relJoint = (relJoints.size() > j && relJoints[j] != SkeletonHandJoint::NUM_JOINTS) ? (const SkeletonJoint::Joint)relJoints[j] : SkeletonJoint::NUM_JOINTS;
				for (auto iter = trainingData.begin(); iter != trainingData.end(); ++iter)
				{
					const SkeletonJointPosition* jointPos = m_useLocalTransformations
						? &(iter->localJointPositions[dtwData.m_joint])
						: &(iter->jointPositions[dtwData.m_joint]);
					const SkeletonJointPosition* relJointPos = 0x0;
					if (dtwData.m_relJoint != SkeletonJoint::NUM_JOINTS)
					{
						relJointPos = m_useLocalTransformations
							? &(iter->localJointPositions[dtwData.m_relJoint])
							: &(iter->jointPositions[dtwData.m_relJoint]);
					}
					// Calculate the final coordinate values to be used for training
					Vec3f point = jointPos->m_position;
					if (relJointPos)
						point -= relJointPos->m_position;
					// Always reduce the axes before the normalizations
					dtwData.m_data.push_back(reduceAxes(point));
				}
				m_trainingData.push_back(dtwData);
			}
		}
		normalizeTrainingData();
	}
}

void DTWRecognizer::normalizeTrainingData()
{
	// Apply normalizations simliar to the 1$ Recognizer
	// (but no rotation normalization, as we want to keep rotation differences)
	// TODO: DTW should not need resampling, but maybe it can help to increase performance or even accuracy
	for (auto iter = m_trainingData.begin(); iter != m_trainingData.end(); ++iter)
	{
		if (!m_useOrientations)
		{
			// Translate centroid to origin
			translate(iter->m_data, -centroid(iter->m_data));
			// Calculate indicative angle (=angle of first point); Before Scaling!
			iter->m_indicativeOrient = calculateIndicativeOrient(iter->m_data);
		}
				// Scale data to a unit cube (only normalizaton done for orientations as well),
		// but keep aspect ratio by fitting the largest side to length one if aspect invariance is deactivated
		scale(iter->m_data, Vec3f(1.0f, 1.0f, 1.0f), !m_aspectInvariant, !m_aspectInvariant);
	}
	unsigned int dataSize = m_trainingData.front().m_data.size();
	m_maxWarpingDistance = dataSize / 2;
	// Resize dtw distance buffer to it's maximum required size and fill it with MaxFloats
	m_dtwDistanceBuffer.resize(dataSize, std::vector<float>(dataSize + m_maxWarpingDistance, Math::MaxFloat));
}

Vec3f DTWRecognizer::calculateIndicativeOrient(const std::vector<Fubi::Vec3f>& data)
{
	// x and y rotation are calculated out of the first point
	Vec3f orient = data.front().toRotation();

	// TODO: The furthest point might already be too different even for slight variations of certain shapes
	//// for the z rotation, we take the point furthest away from the axis spanned up by the first point
	//float maxDistSq = 0;
	//Vec3f origin(0, 0, 0);
	//Vec3f furthestDir;
	//auto end = data.end();
	//for (auto iter = data.begin()+1; iter != end; ++iter)
	//{
	//	Vec3f intersect = closestPointFromPointToRay(*iter, origin, data.front());
	//	float distSq = (*iter - intersect).length2();
	//	if (distSq > maxDistSq)
	//	{
	//		maxDistSq = distSq;
	//		furthestDir = *iter - intersect;
	//	}
	//}
	//// Now calculate the angle between this direction and the rotated up vector
	//Vec3f rotatedUpVec = Matrix3f::RotMat(orient) * Vec3f(0, 1.0f, 0);
	//orient.z = acosf(furthestDir.normalized().dot(rotatedUpVec));

	return orient;
}

Fubi::Vec3f& DTWRecognizer::reduceAxes(Fubi::Vec3f& vec)
{
	if (m_ignoreAxes & CoordinateAxis::X)
		vec.x = 0;
	if (m_ignoreAxes & CoordinateAxis::Y)
		vec.y = 0;
	if (m_ignoreAxes & CoordinateAxis::Z)
		vec.z = 0;
	return vec;
}

bool DTWRecognizer::hasTrackingErrors(Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJoint::Joint relJoint, const std::deque<FubiUser::TrackingData*>* data)
{
	const FubiUser::TrackingData* lastFrame = data->back();
	const FubiUser::TrackingData* firstFrame = data->front();
	// Now check confidences for first and last frame only
	if (lastFrame->jointPositions[joint].m_confidence < m_minConfidence ||
		(relJoint != SkeletonJoint::NUM_JOINTS && lastFrame->jointPositions[relJoint].m_confidence < m_minConfidence) ||
		firstFrame->jointPositions[joint].m_confidence < m_minConfidence ||
		(relJoint != SkeletonJoint::NUM_JOINTS && firstFrame->jointPositions[relJoint].m_confidence < m_minConfidence))
	{
		return true;
	}
	return false;
}
bool DTWRecognizer::hasTrackingErrors(Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJoint::Joint relJoint, const std::deque<FubiHand::FingerTrackingData*>* data)
{
	const FubiHand::FingerTrackingData* lastFrame = data->back();
	const FubiHand::FingerTrackingData* firstFrame = data->front();
	// Now check confidences for first and last frame only
	if (lastFrame->jointPositions[joint].m_confidence < m_minConfidence ||
		(relJoint != SkeletonJoint::NUM_JOINTS && lastFrame->jointPositions[relJoint].m_confidence < m_minConfidence) ||
		firstFrame->jointPositions[joint].m_confidence < m_minConfidence ||
		(relJoint != SkeletonJoint::NUM_JOINTS && firstFrame->jointPositions[relJoint].m_confidence < m_minConfidence))
	{
		return true;
	}
	return false;
}

RecognitionResult::Result DTWRecognizer::recognizeOn(const FubiUser* user, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	Fubi::RecognitionResult::Result result = Fubi::RecognitionResult::NOT_RECOGNIZED;

	if (user != 0x0)
	{
		const std::deque<FubiUser::TrackingData*>* data = user->trackingData();
		if (m_useFilteredData)
			data = user->filteredTrackingData();

		// Check if data frame is already long enough
		if (m_trainingData.size() > 0 && m_trainingData.front().m_data.size() > 0 && data->size() >= (m_trainingData.front().m_data.size()-m_maxWarpingDistance))
		{
			const BodyMeasurementDistance* measure = 0x0;
			if (m_measuringUnit != BodyMeasurement::NUM_MEASUREMENTS)
				measure = &user->bodyMeasurements()[m_measuringUnit];

			float dtwDistanceSum = 0;
			auto end = m_trainingData.end();
			for (auto iter = m_trainingData.begin(); iter != end; ++iter)
			{
				if (hasTrackingErrors(iter->m_joint, iter->m_relJoint, data))
				{
					result = RecognitionResult::TRACKING_ERROR;
					break;
				}
				else
				{
					// Extract the relevant part of the data
					int extractSize = mini(iter->m_data.size() + m_maxWarpingDistance, data->size());
					m_convertedData.clear();
					m_convertedData.reserve(extractSize);
					auto end = data->end();
					for (auto iter2 = data->begin() + (data->size() - extractSize); iter2 != end; ++iter2)
					{
						if (m_useOrientations)
						{
							const SkeletonJointOrientation* jointOrient = m_useLocalTransformations
								? &((*iter2)->localJointOrientations[iter->m_joint])
								: &((*iter2)->jointOrientations[iter->m_joint]);
							// Don't forget to reduce the axes
							m_convertedData.push_back(reduceAxes(jointOrient->m_orientation.getRot(false)));
						}
						else
						{
							Vec3f pos = m_useLocalTransformations
								? (*iter2)->localJointPositions[iter->m_joint].m_position
								: (*iter2)->jointPositions[iter->m_joint].m_position;
							if (iter->m_relJoint != SkeletonJoint::NUM_JOINTS)
							{
								if (m_useLocalTransformations)
									pos -= (*iter2)->localJointPositions[iter->m_relJoint].m_position;
								else
									pos -= (*iter2)->jointPositions[iter->m_relJoint].m_position;
							}
							if (measure && measure->m_confidence > m_minConfidence && measure->m_dist > Math::Epsilon)
							{
								// Apply measuring unit
								pos /= measure->m_dist;
							}
							// Don't forget to reduce the axes
							m_convertedData.push_back(reduceAxes(pos));
						}
					}
					float jointsDtw = calculateDTW(iter->m_data, iter->m_indicativeOrient, m_convertedData);
					if (jointsDtw < Math::MaxFloat - dtwDistanceSum)
						dtwDistanceSum += jointsDtw;
					else
					{
						dtwDistanceSum = Math::MaxFloat;
						// Set correction hint for joint that failed
						if (correctionHint)
							correctionHint->m_joint = iter->m_joint;
						break;
					}
				}
			}
			if (result != RecognitionResult::TRACKING_ERROR)
			{
				if (dtwDistanceSum >= 0)
				{
					float dtwDist = (dtwDistanceSum < Math::MaxFloat) ? (dtwDistanceSum / m_trainingData.size()) : Math::MaxFloat;
					if (dtwDist < m_maxDist)
						result = Fubi::RecognitionResult::RECOGNIZED;
					// Always set correction hint to notify about dtw distance
					if (correctionHint)
					{
						correctionHint->m_dist = dtwDist;
						correctionHint->m_changeType = RecognitionCorrectionHint::FORM;
						correctionHint->m_changeDirection = RecognitionCorrectionHint::DIFFERENT;
					}
				}
			}
		}
		else
			result = Fubi::RecognitionResult::TRACKING_ERROR;
	}
	return result;
}


RecognitionResult::Result DTWRecognizer::recognizeOn(const FubiHand* hand, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	Fubi::RecognitionResult::Result result = Fubi::RecognitionResult::NOT_RECOGNIZED;
	if (hand != 0x0)
	{
		const std::deque<FubiHand::FingerTrackingData*>* data = hand->trackingData();
		if (m_useFilteredData)
			data = hand->filteredTrackingData();

		// Check if data frame is already long enough
		if (m_trainingData.size() > 0 && m_trainingData.front().m_data.size() > 0 && data->size() >= (m_trainingData.front().m_data.size() - m_maxWarpingDistance))
		{
			float dtwDistanceSum = 0;
			auto end = m_trainingData.end();
			for (auto iter = m_trainingData.begin(); iter != end; ++iter)
			{
				if (hasTrackingErrors(iter->m_joint, iter->m_relJoint, data))
				{
					result = RecognitionResult::TRACKING_ERROR;
					break;
				}
				else
				{
					// Extract the relevant part of the data
					int extractSize = mini(iter->m_data.size() + m_maxWarpingDistance, data->size());
					m_convertedData.clear();
					m_convertedData.reserve(extractSize);
					auto end = data->end();
					for (auto iter2 = data->begin() + (data->size() - extractSize); iter2 != end; ++iter2)
					{
						if (m_useOrientations)
						{
							const SkeletonJointOrientation* jointOrient = m_useLocalTransformations
								? &((*iter2)->localJointOrientations[iter->m_joint])
								: &((*iter2)->jointOrientations[iter->m_joint]);
							// Don't forget to reduce the axes
							m_convertedData.push_back(reduceAxes(jointOrient->m_orientation.getRot(false)));
						}
						else
						{
							Vec3f pos = m_useLocalTransformations
								? (*iter2)->localJointPositions[iter->m_joint].m_position
								: (*iter2)->jointPositions[iter->m_joint].m_position;
							if (iter->m_relJoint != SkeletonJoint::NUM_JOINTS)
							{
								if (m_useLocalTransformations)
									pos -= (*iter2)->localJointPositions[iter->m_relJoint].m_position;
								else
									pos -= (*iter2)->jointPositions[iter->m_relJoint].m_position;
							}
							// Don't forget to reduce the axes
							m_convertedData.push_back(reduceAxes(pos));
						}
					}
					float jointsDtw = calculateDTW(iter->m_data, iter->m_indicativeOrient, m_convertedData);
					if (jointsDtw < Math::MaxFloat - dtwDistanceSum)
						dtwDistanceSum += jointsDtw;
					else
					{
						dtwDistanceSum = Math::MaxFloat;
						// Set correction hint for joint that failed
						if (correctionHint)
							correctionHint->m_joint = iter->m_joint;
						break;
					}
				}
			}
			if (result != RecognitionResult::TRACKING_ERROR)
			{
				if (dtwDistanceSum >= 0)
				{
					float dtwDist = (dtwDistanceSum < Math::MaxFloat) ? (dtwDistanceSum / m_trainingData.size()) : Math::MaxFloat;
					if (dtwDist < m_maxDist)
						result = Fubi::RecognitionResult::RECOGNIZED;
					// Always set correction hint to notify about dtw distance
					if (correctionHint)
					{
						correctionHint->m_dist = dtwDist;
						correctionHint->m_changeType = RecognitionCorrectionHint::FORM;
						correctionHint->m_changeDirection = RecognitionCorrectionHint::DIFFERENT;
					}
				}
			}
		}
		else
			result = Fubi::RecognitionResult::TRACKING_ERROR;
	}
	return result;
}

float DTWRecognizer::calculateDTW(const std::vector<Fubi::Vec3f>& trainingData, const Fubi::Vec3f& indicativeOrient, std::vector<Fubi::Vec3f>& testData)
{
	float dtwDist = Math::MaxFloat;

	// Now apply normalizations simliar to the 1$ Recognizer
	// TODO: DTW should not need resampling, but maybe it can help to increase performance or even accuracy
	Vec3f indicativeOrientDiff(Math::NO_INIT);
	if (m_useOrientations)
	{
		// Get angular difference of first points
		indicativeOrientDiff = testData.front() - trainingData.front();
	}
	else
	{
		// Translate centroid to origin
		translate(testData, -centroid(testData));
		// Get angular difference of first points
		indicativeOrientDiff = calculateIndicativeOrient(testData) - indicativeOrient;
	}
	// Angle should not be more different than the maximum rotation
	if (indicativeOrientDiff.length() < m_maxRotation)
	{
		if (!m_useOrientations)
		{
			// Now remove rotation to keep a little rotation invariance
			rotate(testData, -indicativeOrientDiff);
		}
		// Scale to unit cube
		// but keep aspect ratio by fitting the largest side to length one
		scale(testData, Vec3f(1.0f, 1.0f, 1.0f), !m_aspectInvariant, !m_aspectInvariant);

		// Perform DTW
		// TODO: applying DTW reversely might be better, but this conflicts with the other normalization steps and the indicative angle
		dtwDist = Fubi::calculateDTW(trainingData, testData, m_distanceMeasure, m_maxWarpingDistance, false, &m_dtwDistanceBuffer);
		// Normalize by gesture length
		dtwDist /= trainingData.size();
	}
	return dtwDist;
}