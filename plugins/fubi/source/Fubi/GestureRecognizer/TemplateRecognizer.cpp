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
#include "TemplateRecognizer.h"
#include "../FubiGMR.h"

#include "../FubiImageProcessing.h"
#include "../FubiConfig.h"

using namespace Fubi;

TemplateRecognizer::TemplateRecognizer(float maxDistance, Fubi::DistanceMeasure::Measure distanceMeasure, float maxRotation, bool aspectInvariant,
	unsigned int ignoreAxes, bool useOrientations, Fubi::ResamplingTechnique::Technique resamplingTechnique, int resampleSize,
	const Fubi::BodyMeasurementDistance* bodyMeasures, Fubi::BodyMeasurement::Measurement measuringUnit,
	float minConfidence, bool useLocalTransformations, bool useFilteredData)
	: m_maxDist(maxDistance), m_distanceMeasure(distanceMeasure), m_maxRotation(degToRad(maxRotation)), m_aspectInvariant(aspectInvariant),
	m_ignoreAxes(ignoreAxes), m_numIgnoredAxes(calcNumIgnoredAxesAndMinAabbSize()), m_useOrientations(useOrientations), m_useDTW(false), 
	m_resamplingTechnique(resamplingTechnique), m_resampleSize(resampleSize), m_useGMR(false),
	m_numGMRStates(0), m_measuringUnit(measuringUnit), m_useLocalTransformations(useLocalTransformations),
	m_maxSampleSize(0), m_requiredSampleSize(Math::MaxInt32), m_useGSS(false), m_maxWarpingFac(0.5f),
	IGestureRecognizer(false, minConfidence, useFilteredData)
{
	// Constructor for usage in sub classes that do their own  training
}

TemplateRecognizer::TemplateRecognizer(const std::vector<Fubi::SkeletonJoint::Joint>& joints, const std::vector<Fubi::SkeletonJoint::Joint>& relJoints,
	const std::vector<std::deque<Fubi::TrackingData>>& trainingData, float maxDistance, Fubi::DistanceMeasure::Measure distanceMeasure /*= Fubi::DistanceMeasure::Euclidean*/,
	float maxRotation /*= 45.0f*/, bool aspectInvariant /*= false*/, unsigned int ignoreAxes /*= Fubi::CoordinateAxis::NONE*/, bool useOrientations /*=false*/,
	bool useDTW /*= true*/, float maxWarpingFactor /*= 0.5f*/, Fubi::ResamplingTechnique::Technique resamplingTechnique /*= Fubi::ResamplingTechnique::None*/, int resampleSize /*= -1*/,
	bool searchBestInputLength /*= false*/, bool useGMR /*= true*/, unsigned int numGMRStates /*= 5*/,
	const Fubi::BodyMeasurementDistance* bodyMeasures /*= 0x0*/, Fubi::BodyMeasurement::Measurement measuringUnit /*= Fubi::BodyMeasurement::NUM_MEASUREMENTS*/,
	float minConfidence /*= -1.0f*/, bool useLocalTransformations /*= false*/, bool useFilteredData /*= true*/)
	: m_maxDist(maxDistance), m_distanceMeasure(distanceMeasure), m_maxRotation(degToRad(maxRotation)), m_aspectInvariant(aspectInvariant),
	m_ignoreAxes(ignoreAxes), m_numIgnoredAxes(calcNumIgnoredAxesAndMinAabbSize()), m_useOrientations(useOrientations), m_useDTW(useDTW),
	m_resamplingTechnique(resamplingTechnique), m_resampleSize(resampleSize), m_useGMR(useGMR),
	m_numGMRStates(numGMRStates), m_measuringUnit(measuringUnit), m_useLocalTransformations(useLocalTransformations),
	m_maxSampleSize(0), m_requiredSampleSize(Math::MaxInt32), m_useGSS(searchBestInputLength), m_maxWarpingFac(maxWarpingFactor),
	IGestureRecognizer(false, minConfidence, useFilteredData)
{
	if (joints.size() > 0 && trainingData.size() > 0)
	{
		const BodyMeasurementDistance* measure = (m_measuringUnit == BodyMeasurement::NUM_MEASUREMENTS)
			? 0x0 : &(bodyMeasures[m_measuringUnit]);

		for (unsigned int j = 0; j < joints.size(); ++j)
		{
			TemplateData templateData;
			templateData.m_joint = joints[j];
			templateData.m_relJoint = (relJoints.size() > j) ? relJoints[j] : SkeletonJoint::NUM_JOINTS;

			// Find and convert the data
			std::vector<std::vector<Vec3f>> convertedData;
			std::vector<Vec3f> indicativeOrients;
			extractTrainingData(trainingData, templateData.m_joint, templateData.m_relJoint, measure, convertedData, &indicativeOrients);

			// Calculate mean and covariance matrices for the coordinate points and the indicative orients
			// Also calculates the polyline indices
			calculateMeanAndCovs(templateData, convertedData, indicativeOrients);

			// Now the data is ready
			m_trainingData.push_back(templateData);
		}
		// Resize dtw distance buffer to it's maximum required size and fill it with MaxFloats
		m_dtwDistanceBuffer.resize(m_maxSampleSize, std::vector<float>(ftoi_t(m_maxSampleSize*(1.0f + m_maxWarpingFac)), Math::MaxFloat));
	}
}
TemplateRecognizer::TemplateRecognizer(const std::vector<Fubi::SkeletonHandJoint::Joint>& joints, const std::vector<Fubi::SkeletonHandJoint::Joint>& relJoints,
	const std::vector<std::deque<Fubi::TrackingData>>& trainingData, float maxDistance, Fubi::DistanceMeasure::Measure distanceMeasure /*= Fubi::DistanceMeasure::Euclidean*/,
	float maxRotation /*= 45.0f*/, bool aspectInvariant /*= false*/, unsigned int ignoreAxes /*= Fubi::CoordinateAxis::NONE*/, bool useOrientations /*=false*/,
	bool useDTW /*= true*/, float maxWarpingFactor /*= 0.5f*/, Fubi::ResamplingTechnique::Technique resamplingTechnique /*= Fubi::ResamplingTechnique::None*/, int resampleSize /*= -1*/,
	bool searchBestInputLength /*= false*/, bool useGMR /*= true*/, unsigned int numGMRStates /*= 5*/,
	float minConfidence /*= -1.0f*/, bool useLocalTransformations /*= false*/, bool useFilteredData /*= true*/)
	: m_maxDist(maxDistance), m_distanceMeasure(distanceMeasure), m_maxRotation(degToRad(maxRotation)), m_aspectInvariant(aspectInvariant),
	m_ignoreAxes(ignoreAxes), m_numIgnoredAxes(calcNumIgnoredAxesAndMinAabbSize()), m_useOrientations(useOrientations), m_useDTW(useDTW),
	m_resamplingTechnique(resamplingTechnique), m_resampleSize(resampleSize), m_useGMR(useGMR),
	m_numGMRStates(numGMRStates), m_measuringUnit(Fubi::BodyMeasurement::NUM_MEASUREMENTS), m_useLocalTransformations(useLocalTransformations),
	m_maxSampleSize(0), m_requiredSampleSize(Math::MaxInt32), m_useGSS(searchBestInputLength), m_maxWarpingFac(maxWarpingFactor),
	IGestureRecognizer(false, minConfidence, useFilteredData)
{
	if (joints.size() > 0 && trainingData.size() > 0)
	{
		for (unsigned int j = 0; j < joints.size(); ++j)
		{
			TemplateData templateData;
			templateData.m_joint = (joints[j] != SkeletonHandJoint::NUM_JOINTS) ? (SkeletonJoint::Joint) joints[j] : SkeletonJoint::NUM_JOINTS;
			templateData.m_relJoint = (relJoints.size() > j && relJoints[j] != SkeletonHandJoint::NUM_JOINTS) ? (SkeletonJoint::Joint) relJoints[j] : SkeletonJoint::NUM_JOINTS;

			// Find and convert the data
			std::vector<std::vector<Vec3f>> convertedData;
			std::vector<Vec3f> indicativeOrients;
			extractTrainingData(trainingData, templateData.m_joint, templateData.m_relJoint, 0x0, convertedData, &indicativeOrients);

			// Calculate mean and covariance matrices for the coordinate points and the indicative orients
			// Also calculates the polyline indices
			calculateMeanAndCovs(templateData, convertedData, indicativeOrients);
			// Now the data is ready
			m_trainingData.push_back(templateData);
		}
		// Resize dtw distance buffer to it's maximum required size and fill it with MaxFloats
		m_dtwDistanceBuffer.resize(m_maxSampleSize, std::vector<float>(ftoi_t(m_maxSampleSize*(1.0f + m_maxWarpingFac)), Math::MaxFloat));
	}
}

unsigned int TemplateRecognizer::calcNumIgnoredAxesAndMinAabbSize()
{
	m_numIgnoredAxes = 0;
	if (m_ignoreAxes & CoordinateAxis::X)
		++m_numIgnoredAxes;
	if (m_ignoreAxes & CoordinateAxis::Y)
		++m_numIgnoredAxes;
	if (m_ignoreAxes & CoordinateAxis::Z)
		++m_numIgnoredAxes;

	m_minAabbSize = powf((m_useOrientations ? 0.05f : 50.0f), (3.0f - m_numIgnoredAxes));

	return m_numIgnoredAxes;
}

void TemplateRecognizer::extractTrainingData(const std::vector<std::deque<Fubi::TrackingData>>& trainingData, Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJoint::Joint relJoint, const BodyMeasurementDistance* measure,
	std::vector<std::vector<Vec3f >>& convertedData, std::vector<Fubi::Vec3f>* indicativeOrients /*= 0x0*/)
{
	// Calculate mean and min and max size of training data (consider warping for min and max)
	size_t sizeSum = 0;
	for (auto iter = trainingData.begin(); iter != trainingData.end(); ++iter)
	{
		sizeSum += iter->size();
		m_requiredSampleSize = maxi(1, mini(m_requiredSampleSize, ftoi_t(iter->size() * (1.0f - m_maxWarpingFac))));
		m_maxSampleSize = maxi(m_maxSampleSize, ftoi_t(iter->size() * (1.0f + m_maxWarpingFac)));
	}
	int resampleSize = m_resampleSize;
	if (resampleSize < 2)
		resampleSize = (int)(sizeSum / trainingData.size());

	for (auto iter = trainingData.begin(); iter != trainingData.end(); ++iter)
	{
		std::vector<Fubi::Vec3f> rawData, resampledData, *data;
		for (auto iter2 = iter->begin(); iter2 != iter->end(); ++iter2)
		{
			Vec3f dataPoint(Math::NO_INIT);
			if (m_useLocalTransformations) // local transformations are not provided in the recording, so we need to calculate them...
				calculateLocalTransformations(iter2->jointPositions, iter2->jointOrientations, iter2->localJointPositions, iter2->localJointOrientations);
			if (m_useOrientations)
			{
				const SkeletonJointOrientation* jointOrient = m_useLocalTransformations
					? &(iter2->localJointOrientations[joint])
					: &(iter2->jointOrientations[joint]);
				// We normalize the rotations later, so we don't care if they are in degrees or radians
				dataPoint = jointOrient->m_orientation.getRot(false);
			}
			else
			{
				const SkeletonJointPosition* jointPos = m_useLocalTransformations
					? &(iter2->localJointPositions[joint])
					: &(iter2->jointPositions[joint]);
				const SkeletonJointPosition* relJointPos = 0x0;
				if (relJoint != SkeletonJoint::NUM_JOINTS)
				{
					relJointPos = m_useLocalTransformations
						? &(iter2->localJointPositions[relJoint])
						: &(iter2->jointPositions[relJoint]);
				}
				// Calculate the final coordinate values to be used for training
				dataPoint = jointPos->m_position;
				if (relJointPos)
					dataPoint -= relJointPos->m_position;
				if (measure && measure->m_dist > Math::Epsilon)
					dataPoint /= measure->m_dist;
			}
			// Always reduce the axes before the normalizations
			rawData.push_back(reduceAxes(dataPoint));
		}

#ifdef FUBI_TEMPLATEREC_DEBUG_DRAWING
		std::stringstream ss;
		static int num = 0;
		ss << "rawTrainingData" << num++;
		FubiImageProcessing::showPlot(rawData, 0x0, 640, 480, ss.str());
#endif

		// Apply normalizations simliar to the 1$ Recognizer
		// (but no rotation normalization, as we want to keep rotation differences)
		if (rawData.size() == resampleSize && trainingData.size() == 1)
			data = &rawData;
		else
		{
			// Resampling to get an equal size of points for all samples which are well aligned in the best case
			if (m_resamplingTechnique == ResamplingTechnique::HermiteSpline)
				hermiteSplineRescale(rawData, resampleSize, resampledData);
			else //if (m_resamplingTechnique == ResamplingTechnique::EquiDistant || None || PolyLine )
				equiDistResample(rawData, resampleSize, resampledData);
			data = &resampledData;
		}
		if (!m_useOrientations)
		{
			// Translate centroid to origin
			translate(*data, -centroid(*data));
			// Calculate indicative angle (=angle of first point); Before Scaling!
			if (indicativeOrients)
				indicativeOrients->push_back(calculateIndicativeOrient(*data));
		}
		// Scale data to a unit cube (only normalizaton done for orientations as well),
		// but keep aspect ratio by fitting the largest side to length one if aspect invariance is deactivated
		scale(*data, Vec3f(1.0f, 1.0f, 1.0f), 0.0f, !m_aspectInvariant, !m_aspectInvariant);

		convertedData.push_back(*data);
	}
}

void TemplateRecognizer::calculateMeanAndCovs(TemplateData& data, const std::vector<std::vector<Fubi::Vec3f>>& convertedData, const std::vector<Fubi::Vec3f>& indicativeOrients)
{
	if (convertedData.size() == 1)
	{
		// Directly use the single element
		data.m_data = convertedData[0];
		if (!m_useOrientations)
			data.m_indicativeOrient = indicativeOrients[0];
	}
	else
	{
		// Calculate the mean indicative orientation
		if (!m_useOrientations)
			data.m_indicativeOrient = calculateMean(indicativeOrients);

		// Get dimensions
		float numSamples = (float)convertedData.size();
		size_t size = convertedData[0].size();

		// Resize data and covs
		data.m_data.resize(size);
		float nulls[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		Matrix3f nullMatrix(nulls);
		data.m_inverseCovs.resize(size, nullMatrix);

		// Calculate the mean of the data and its covariance matrix for each element of the converted data
		if (m_useGMR)
		{
			// Use GMR to do this
			FubiGMR g(convertedData, maxi(1, mini((unsigned int)(size/2), m_numGMRStates)));

			// Apply EM to get the GMM
			g.calculateGMM(MaxGMMEMIterations);

			// Apply GMR to get new general mean and covariance values
			g.calculateGMR(data.m_data, data.m_inverseCovs);
		}
		else
		{
			// Calculate them by hand for the raw data
			for (unsigned int i = 0; i < size; ++i) // iterate over all data points
			{
				// Calculate mean between different samples
				for (auto iter = convertedData.begin(); iter != convertedData.end(); ++iter)
				{
					Vec3f value = iter->at(i);
					data.m_data[i] += value;
				}
				data.m_data[i] /= numSamples;

				// Calculate covariances
				for (auto iter = convertedData.begin(); iter != convertedData.end(); ++iter)
				{
					Vec3f value = iter->at(i);
					for (int j = 0; j < 3; ++j)
					{
						for (int k = 0; k < 3; ++k)
						{
							float cov = (value[j] - data.m_data[i][j]) * (value[k] - data.m_data[i][k]);
							data.m_inverseCovs[i].c[j][k] += cov / numSamples;
						}
					}
				}
				// Don't forget to invert the matrix
				// But first fix zero values in the diagonal (usually caused by m_ignoreAxes)
				for (int j = 0; j < 3; ++j)
				{
					if (fabsf(data.m_inverseCovs[i].c[j][j]) < Math::Epsilon)
						data.m_inverseCovs[i].c[j][j] = Math::Epsilon;
				}
				data.m_inverseCovs[i] = data.m_inverseCovs[i].inverted();
			}
		}
	}

	if (m_resamplingTechnique == ResamplingTechnique::PolyLine)
	{
		// We can only calculate the polyline indices now
		polyLineReduction(data.m_data, data.m_polyLineIndices);
	}

#ifdef FUBI_TEMPLATEREC_DEBUG_DRAWING
	if (m_resamplingTechnique == ResamplingTechnique::PolyLine)
	{
		std::vector<Vec3f> tempVec;
		std::vector<Matrix3f> tempMat;
		int covSize = (int)data.m_inverseCovs.size();
		for (auto iter = data.m_polyLineIndices.begin(); iter != data.m_polyLineIndices.end(); ++iter)
		{
			tempVec.push_back(data.m_data[*iter]);
			if (covSize > *iter)
				tempMat.push_back(data.m_inverseCovs[*iter]);
		}
		std::stringstream ss;
		static int num = 0;
		ss << "normalizedTrainingData" << num++;
		FubiImageProcessing::showPlot(tempVec, &tempMat, 640, 480, ss.str());
	}
	else
		FubiImageProcessing::showPlot(data.m_data, &data.m_inverseCovs, 640, 480, "normalizedTrainingData");
#endif
}

Vec3f TemplateRecognizer::calculateIndicativeOrient(const std::vector<Fubi::Vec3f>& data)
{
	if (m_numIgnoredAxes > 1)
		return NullVec;

	const Vec3f& firstPoint = data.front();
	Vec3f orient;
	if (m_numIgnoredAxes == 1)
	{
		// Only calculate angle around the ignored axes
		float a, b;
		float* angle;
		if (m_ignoreAxes & CoordinateAxis::X)
		{
			a = firstPoint.z;
			b = firstPoint.y;
			angle = &orient.x;
		}
		else if (m_ignoreAxes & CoordinateAxis::Y)
		{
			a = -firstPoint.z;
			b = -firstPoint.x;
			angle = &orient.y;
		}
		else if (m_ignoreAxes & CoordinateAxis::Z)
		{
			a = -firstPoint.x;
			b = -firstPoint.y;
			angle = &orient.z;
		}
		// Calc angle from (1,0) to (a,b)
		if (a != 0 || b != 0)
			*angle = atan2f(b, a);
	}
	else
	{
		// Calc angle around x and y
		orient = firstPoint.toRotation();

		// TODO: The furthest point might already be too different even for slight variations of certain shapes
		// Is it necessary to have the third angle?
		// Further, the below calculation does not seem to be correct...
		//
		// For the z rotation, we take the point of the first half points furthest away from the axis spanned up by the first point
		//float maxDistSq = 0;
		//Vec3f origin(0, 0, 0);
		//Vec3f furthestDir;
		//auto end = data.begin() + (data.size() / 2);
		//for (auto iter = data.begin()+1; iter != end; ++iter)
		//{
		//	Vec3f intersect = closestPointFromPointToRay(*iter, origin, firstPoint);
		//	float distSq = (*iter - intersect).length2();
		//	if (distSq > maxDistSq)
		//	{
		//		maxDistSq = distSq;
		//		furthestDir = *iter - intersect;
		//	}
		//}
		//// Now calculate the angle between this direction and the rotated x axis vector
		//Vec3f rotatedX1Vec = Matrix3f::RotMat(orient) * Vec3f(1.0f, 0, 0);
		//furthestDir.normalize(); // Normalize for easier calculations
		//float DirDotUp = furthestDir.dot(rotatedX1Vec);
		//Vec3f DirCrossUp = furthestDir.cross(rotatedX1Vec);
		//orient.z = atan2f(DirCrossUp.length(), DirDotUp);
		//if (DirCrossUp.dot(firstPoint) < 0)
		//	orient.z *= -1.0f;
	}

	return orient;
}

Fubi::Vec3f& TemplateRecognizer::reduceAxes(Fubi::Vec3f& vec)
{
	if (m_ignoreAxes & CoordinateAxis::X)
		vec.x = 0;
	if (m_ignoreAxes & CoordinateAxis::Y)
		vec.y = 0;
	if (m_ignoreAxes & CoordinateAxis::Z)
		vec.z = 0;
	return vec;
}

bool TemplateRecognizer::hasTrackingErrors(Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJoint::Joint relJoint, const std::deque<Fubi::TrackingData*>* data)
{
	const Fubi::TrackingData* lastFrame = data->back();
	const Fubi::TrackingData* firstFrame = data->front();
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

RecognitionResult::Result TemplateRecognizer::recognizeOn(const FubiUser* user, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	Fubi::RecognitionResult::Result result = Fubi::RecognitionResult::NOT_RECOGNIZED;

	if (user != 0x0)
	{
		const std::deque<Fubi::TrackingData*>* data = user->trackingData();
		if (m_useFilteredData)
			data = user->filteredTrackingData();

		// Check if data frame is already long enough
		if (m_trainingData.size() > 0 && m_maxSampleSize > 0 && data->size() >= (size_t)m_requiredSampleSize)
		{
			const BodyMeasurementDistance* measure = 0x0;
			if (m_measuringUnit != BodyMeasurement::NUM_MEASUREMENTS)
				measure = &user->bodyMeasurements()[m_measuringUnit];

			float distanceSum = 0;
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
					int extractSize;
					if (m_useGSS)
						extractSize = mini(ftoi_t(iter->m_data.size()*(1.0f + m_maxWarpingFac)), (int)data->size());
					else // Don't extract more if in non-gss mode
						extractSize = mini((int)iter->m_data.size(), (int)data->size());
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
							Vec3f rot = jointOrient->m_orientation.getRot(false);
							m_convertedData.push_back(reduceAxes(rot));
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
					float jointsDist = m_useGSS
						? calculateDistanceAtBestDataLength(*iter, m_convertedData, correctionHint)
						: calculateDistance(*iter, &m_convertedData, correctionHint);
					if (jointsDist < Math::MaxFloat - distanceSum)
						distanceSum += jointsDist;
					else
					{
						distanceSum = Math::MaxFloat;
						// Set correction hint for joint that failed
						if (correctionHint)
							correctionHint->m_joint = iter->m_joint;
						break;
					}
				}
			}
			if (result != RecognitionResult::TRACKING_ERROR)
			{
				if (distanceSum >= 0)
				{
					float dist = (distanceSum < Math::MaxFloat) ? (distanceSum / m_trainingData.size()) : Math::MaxFloat;
					if (dist < m_maxDist)
						result = Fubi::RecognitionResult::RECOGNIZED;
					// Always set correction hint to notify about the current distance
					if (correctionHint)
					{
						correctionHint->m_dist = dist;
						correctionHint->m_changeType = RecognitionCorrectionHint::FORM;
						correctionHint->m_changeDirection = RecognitionCorrectionHint::DIFFERENT;
						correctionHint->m_recTarget = m_targetSkeletonType;
					}
				}
			}
		}
		else
			result = Fubi::RecognitionResult::TRACKING_ERROR;
	}
	return result;
}


RecognitionResult::Result TemplateRecognizer::recognizeOn(const FubiHand* hand, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	Fubi::RecognitionResult::Result result = Fubi::RecognitionResult::NOT_RECOGNIZED;
	if (hand != 0x0)
	{
		const std::deque<Fubi::TrackingData*>* data = hand->trackingData();
		if (m_useFilteredData)
			data = hand->filteredTrackingData();

		// Check if data frame is already long enough
		if (m_trainingData.size() > 0 && m_maxSampleSize > 0 && data->size() >= (size_t)m_requiredSampleSize)
		{
			float distanceSum = 0;
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
					int extractSize;
					if (m_useGSS)
						extractSize = mini(ftoi_t(iter->m_data.size()*(1.0f + m_maxWarpingFac)), (int)data->size());
					else // Don't extract more if in non-gss mode
						extractSize = mini((int)iter->m_data.size(), (int)data->size());
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
							Vec3f rot = jointOrient->m_orientation.getRot(false);
							m_convertedData.push_back(reduceAxes(rot));
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
					float jointsDist = 
						m_useGSS
						? calculateDistanceAtBestDataLength(*iter, m_convertedData, correctionHint)
						: calculateDistance(*iter, &m_convertedData, correctionHint); 
					if (jointsDist < Math::MaxFloat - distanceSum)
						distanceSum += jointsDist;
					else
					{
						distanceSum = Math::MaxFloat;
						// Set correction hint for joint that failed
						if (correctionHint)
							correctionHint->m_joint = iter->m_joint;
						break;
					}
				}
			}
			if (result != RecognitionResult::TRACKING_ERROR)
			{
				if (distanceSum >= 0)
				{
					float dist = (distanceSum < Math::MaxFloat) ? (distanceSum / m_trainingData.size()) : Math::MaxFloat;
					if (dist < m_maxDist)
						result = Fubi::RecognitionResult::RECOGNIZED;
					// Always set correction hint to notify about current distance
					if (correctionHint)
					{
						correctionHint->m_dist = dist;
						correctionHint->m_changeType = RecognitionCorrectionHint::FORM;
						correctionHint->m_changeDirection = RecognitionCorrectionHint::DIFFERENT;
						correctionHint->m_recTarget = m_targetSkeletonType;
					}
				}
			}
		}
		else
			result = Fubi::RecognitionResult::TRACKING_ERROR;
	}
	return result;
}

float TemplateRecognizer::calculateDistanceAtBestDataLength(const Fubi::TemplateData& trainingData, std::vector<Fubi::Vec3f>& testData,
	Fubi::RecognitionCorrectionHint* correctionHint)
{
	// Find the best data length using a golden section search (GSS)
	static const float phi = 0.5f * (sqrtf(5.0f) - 1.0f); // golden section ratio
	static const float invPhi = 1.0f - phi; // inverse
	const int tau = 2; // tau defines the threshold for ending the search

	int theta0 = 0, theta1 = mini(ftoi_t(trainingData.m_data.size()*2.0f*m_maxWarpingFac), (int)testData.size());
	int x0 = ftoi_r(phi*theta0 + invPhi*theta1);
	int x1 = ftoi_r(invPhi*theta0 + phi*theta1);
	std::vector<Fubi::Vec3f> data0(testData.begin() + x0, testData.end());
	std::vector<Fubi::Vec3f> data1(testData.begin() + x1, testData.end());
	// Don't pass the correction hint, we will handle this later...
	float dist0 = calculateDistance(trainingData, &data0, 0x0);
	float dist1 = calculateDistance(trainingData, &data1,  0x0);

	while (abs((int)(theta1-theta0)) > tau)
	{
		if (dist0 < dist1)
		{
			theta1 = x1;
			x1 = x0;
			dist1 = dist0;
			x0 = ftoi_r(phi*theta0 + invPhi*theta1);
			data0.assign(testData.begin() + x0, testData.end());
			dist0 = calculateDistance(trainingData, &data0, 0x0);
		}
		else
		{
			theta0 = x0;
			x0 = x1;
			dist0 = dist1;
			x1 = ftoi_r(invPhi*theta0 + phi*theta1);
			data1.assign(testData.begin() + x1, testData.end());
			dist1 = calculateDistance(trainingData, &data1, 0x0);
		}
	}
	// Now take the min of both values
	float minDist = minf(dist0, dist1);

	// Additionally test if the whole sequence provides a better result
	float baseDist = calculateDistance(trainingData, &testData, correctionHint);
	if (baseDist < minDist)
	{
		minDist = baseDist;
	}
	else if (correctionHint && correctionHint->m_inputData) // Correction hints calculation only needed if not using the whole sequence
	{
		// Copy input data to correction hint
		if (dist0 < dist1)
			correctionHint->m_inputData->assign(testData.begin() + x0, testData.end());
		else
			correctionHint->m_inputData->assign(testData.begin() + x1, testData.end());
		// normalize again as we "forgot" the old data...
		if (correctionHint->m_normalizeInputData)
		{
			if (m_resamplingTechnique != ResamplingTechnique::None)
			{
				std::vector<Fubi::Vec3f> tempData;
				// Apply rescaling on the tempData vector
				if (m_resamplingTechnique == ResamplingTechnique::HermiteSpline)
					Fubi::hermiteSplineRescale(*correctionHint->m_inputData, trainingData.m_data.size(), tempData);
				else //if (m_resamplingTechnique == ResamplingTechnique::EquiDistant)
					Fubi::equiDistResample(*correctionHint->m_inputData, trainingData.m_data.size(), tempData);
				// Copy tempVec back to the correction hint
				*correctionHint->m_inputData = tempData;
			}
			Vec3f indicativeOrientDiff(Math::NO_INIT);
			if (m_useOrientations)
			{
				// Get angular difference of first points
				indicativeOrientDiff = correctionHint->m_inputData->front() - trainingData.m_data.front();
			}
			else
			{
				// Translate centroid to origin
				translate(*correctionHint->m_inputData, -centroid(*correctionHint->m_inputData));
				// Get angular difference of first points
				indicativeOrientDiff = calculateIndicativeOrient(*correctionHint->m_inputData) - trainingData.m_indicativeOrient;
			}
			if (!m_useOrientations)
				rotate(*correctionHint->m_inputData, -indicativeOrientDiff);
			scale(*correctionHint->m_inputData, Vec3f(1.0f, 1.0f, 1.0f), m_minAabbSize, !m_aspectInvariant, !m_aspectInvariant);
			
			if (m_resamplingTechnique == ResamplingTechnique::PolyLine)
			{
				std::vector<int> indices;
				polyLineReduction(*correctionHint->m_inputData, indices);
				std::vector<Fubi::Vec3f> tempData;
				for (auto iter = indices.begin(); iter != indices.end(); ++iter)
					tempData.push_back(correctionHint->m_inputData->at(*iter));
				// Copy tempVec back to the correction hint
				*correctionHint->m_inputData = tempData;
			}
		}
	}

	return minDist;
}

float TemplateRecognizer::calculateDistance(const Fubi::TemplateData& trainingData, std::vector<Fubi::Vec3f>* testData,
	Fubi::RecognitionCorrectionHint* correctionHint)
{
	if (testData->size() == 0)
		return Math::MaxFloat;

	float dist = Math::MaxFloat;
	float resampleCostFactor = 1.0f;
	std::vector<Fubi::Vec3f> resampledData; // Used if resampling is activated

#ifdef FUBI_TEMPLATEREC_DEBUG_DRAWING
	static unsigned int drawCounter = 0;
	bool drawPoints = drawCounter++ % 10 == 0;
	if (drawPoints)
		FubiImageProcessing::showPlot(*testData, 0x0, 640, 480, "rawInput");
#endif

	if (correctionHint && correctionHint->m_inputData && !correctionHint->m_normalizeInputData)
		// Copy input data to correction hint before normalizations
		*correctionHint->m_inputData = *testData;

	// Now apply normalizations simliar to the 1$ Recognizer
	if (m_resamplingTechnique != ResamplingTechnique::None && m_resamplingTechnique != ResamplingTechnique::PolyLine)
	{
		// Apply rescaling on the tempData vector
		if (m_resamplingTechnique == ResamplingTechnique::HermiteSpline)
			Fubi::hermiteSplineRescale(*testData, trainingData.m_data.size(), resampledData);
		else //if (m_resamplingTechnique == ResamplingTechnique::EquiDistant)
			Fubi::equiDistResample(*testData, trainingData.m_data.size(), resampledData);
		// Change testData pointer to tempData
		testData = &resampledData;

//#ifdef FUBI_TEMPLATEREC_DEBUG_DRAWING
//		if (drawPoints)
//			FubiImageProcessing::showPlot(*testData, 640, 480, "resampledInput");
//#endif
	}

	Vec3f indicativeOrientDiff(Math::NO_INIT);
	if (m_useOrientations)
	{
		// Get angular difference of first points
		indicativeOrientDiff = testData->front() - trainingData.m_data.front();
	}
	else
	{
		// Translate centroid to origin
		translate(*testData, -centroid(*testData));
		// Get angular difference of first points
		indicativeOrientDiff = calculateIndicativeOrient(*testData) - trainingData.m_indicativeOrient;
	}
	// Angle should not be more different than the maximum rotation
	if (indicativeOrientDiff.length() < m_maxRotation)
	{
		if (!m_useOrientations)
		{
			// Now remove rotation to keep a little rotation invariance
			rotate(*testData, -indicativeOrientDiff);
		}
		// Scale to unit cube
		// but keep aspect ratio by fitting the largest side to length one
		if (scale(*testData, Vec3f(1.0f, 1.0f, 1.0f), m_minAabbSize, !m_aspectInvariant, !m_aspectInvariant))
		{
			const std::vector<Fubi::Vec3f>* trainingDataP = &trainingData.m_data;
			const std::vector<Fubi::Matrix3f>* trainingCovsP = &trainingData.m_inverseCovs;

			// Special case for poly line resampling which is applied at the end of the other normalization steps
			std::vector<Fubi::Vec3f> polyLineTrainingData, polyLineTestData;
			std::vector<Fubi::Matrix3f> polyLineCovs;
			if (m_resamplingTechnique == ResamplingTechnique::PolyLine)
			{			
				// Polyline indices for both data
				std::vector<int> trainingIndices = trainingData.m_polyLineIndices;
				std::vector<int> testIndices;
				polyLineReduction(*testData, testIndices);
				// Now apply alignment
				unsigned int addedPoints = 0, matchedPoints = 2;
				polyLineAlign(*testData, trainingData.m_data, testIndices, trainingIndices, addedPoints, matchedPoints);
				
				resampleCostFactor = 1.0f + (float)addedPoints / (float)(addedPoints + matchedPoints);

				for (auto iter = trainingIndices.begin(); iter != trainingIndices.end(); ++iter)
				{
					int index = *iter;
					polyLineTrainingData.push_back(trainingData.m_data[index]);
					if (index < (signed)trainingData.m_inverseCovs.size())
						polyLineCovs.push_back(trainingData.m_inverseCovs[index]);
				}
				for (auto iter = testIndices.begin(); iter != testIndices.end(); ++iter)
					polyLineTestData.push_back(testData->at(*iter));
				trainingDataP = &polyLineTrainingData;
				trainingCovsP = &polyLineCovs;
				testData = &polyLineTestData;
			}

#ifdef FUBI_TEMPLATEREC_DEBUG_DRAWING
			if (drawPoints)
			{
				FubiImageProcessing::showPlot(*testData, 0x0, 640, 480, "normalizedInput");
			}
#endif

			if (correctionHint && correctionHint->m_inputData && correctionHint->m_normalizeInputData)
			{
				// Copy input data to correction hint after normalizations
				*correctionHint->m_inputData = (m_resamplingTechnique == ResamplingTechnique::PolyLine) ? polyLineTestData : *testData;
			}

			if (m_useDTW)
			{
				// Perform DTW
				// TODO: applying DTW reversely might be better, but this conflicts with the other normalization steps and the indicative angle
				dist = Fubi::calculateDTW(*trainingDataP, *testData, m_distanceMeasure, ftoi_t(m_maxWarpingFac*trainingDataP->size()), false, &m_dtwDistanceBuffer, trainingCovsP);
				// Normalize by gesture length
				dist /= trainingDataP->size();
			}
			else
				// Use unwarped pair-wise distance similar to the 1$ recognizer (already includes normalization)
				dist = Fubi::calculateMeanDistance(*trainingDataP, *testData, m_distanceMeasure, trainingCovsP);
		}
	}
	else if (correctionHint && correctionHint->m_inputData && correctionHint->m_normalizeInputData)
	{
		// Copy input data to correction hint after normalizations
		if (!m_useOrientations)
			rotate(*testData, -indicativeOrientDiff);
		scale(*testData, Vec3f(1.0f, 1.0f, 1.0f), m_minAabbSize, !m_aspectInvariant, !m_aspectInvariant);
		if (m_resamplingTechnique == ResamplingTechnique::PolyLine)
		{
			std::vector<int> indices;
			polyLineReduction(*testData, indices);
			for (auto iter = indices.begin(); iter != indices.end(); ++iter)
				correctionHint->m_inputData->push_back(testData->at(*iter));
		}
		else
			*correctionHint->m_inputData = *testData;
	}
	return dist * resampleCostFactor;
}
