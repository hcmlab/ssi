// ****************************************************************************************
//
// Fubi User
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler 
// 
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
// 
// ****************************************************************************************

#include "FubiUser.h"

#include "FubiISensor.h"
#include "FubiMath.h"
#include "FubiCore.h"
#include "FubiImageProcessing.h"
#include "FubiRecognizerFactory.h"
#include "GestureRecognizer/CombinationRecognizer.h"

using namespace Fubi;

FubiUser::FubiUser() : m_inScene(false), m_id(0), m_isTracked(false), m_maxTrackingHistoryLength(2),
	m_lastBodyMeasurementUpdate(0), m_bodyMeasurementUpdateIntervall(0.5), m_assignedLeftHandId(0), m_assignedRightHandId(0)
{
	// Init for filtered velocity used for filtering
	memset(m_lastFilteredVelocity, 0, sizeof(Fubi::Vec3f)*Fubi::SkeletonJoint::NUM_JOINTS);

	// Init the combination recognizers
	for (unsigned int i = 0; i < Combinations::NUM_COMBINATIONS; ++i)
	{
		m_combinationRecognizers[i] = Fubi::createCombinationRecognizer(this, (Combinations::Combination) i);
	}
}


FubiUser::~FubiUser()
{
	for (unsigned int i = 0; i < Combinations::NUM_COMBINATIONS; ++i)
	{
		delete m_combinationRecognizers[i];
		m_combinationRecognizers[i] = 0x0;
	}

	clearUserDefinedCombinationRecognizers();

	for (auto iter = m_trackingData.begin(); iter != m_trackingData.end(); ++iter)
		delete *iter;
	for (auto iter = m_filteredTrackingData.begin(); iter != m_filteredTrackingData.end(); ++iter)
		delete *iter;
}

void FubiUser::clearUserDefinedCombinationRecognizers()
{
	auto end = m_userDefinedCombinationRecognizers.end();
	for (auto iter = m_userDefinedCombinationRecognizers.begin(); iter != end; ++iter)
	{
		delete iter->second;
	}
	m_userDefinedCombinationRecognizers.clear();
}

void FubiUser::enableCombinationRecognition(Fubi::Combinations::Combination postureID, bool enable)
{
	if (postureID < Fubi::Combinations::NUM_COMBINATIONS && m_combinationRecognizers[postureID])
	{
		if (enable)
			m_combinationRecognizers[postureID]->start();
		else
			m_combinationRecognizers[postureID]->stop();
	}
}

void FubiUser::enableCombinationRecognition(const CombinationRecognizer* recognizerTemplate, bool enable)
{
	if (recognizerTemplate)
	{
		std::map<std::string, CombinationRecognizer*>::iterator rec = m_userDefinedCombinationRecognizers.find(recognizerTemplate->getName());
		if (rec != m_userDefinedCombinationRecognizers.end())
		{
			if (enable)
				rec->second->start();
			else
			{
				rec->second->stop();
			}
		}
		else if (enable)
		{
			CombinationRecognizer* clonedRec = recognizerTemplate->clone();
			clonedRec->setUser(this);
			m_userDefinedCombinationRecognizers[recognizerTemplate->getName()] = clonedRec;
			clonedRec->start();
		}
	}
}

void FubiUser::enableFingerTrackingWithConvexityDefectMethod(bool enable)
{
	m_leftFingerCountData.useConvexityDefectMethod = m_rightFingerCountData.useConvexityDefectMethod = enable;
}

void FubiUser::enableFingerTracking(bool enable, bool leftHand /*= false*/)
{
	FingerCountData& data = leftHand ? m_leftFingerCountData : m_rightFingerCountData;
	bool wasEnabled = data.trackingEnabled;
	data.trackingEnabled = enable;
	if (enable && !wasEnabled)
	{
		// Reset for tracking start
		data.fingerCounts.clear();
		// Immediately update the finger detection for the first time
		updateFingerCount(leftHand, Fubi::currentTime());
	}

}

void FubiUser::addFingerCount(int count, bool leftHand, double currentTime)
{
	FingerCountData& data = leftHand ? m_leftFingerCountData : m_rightFingerCountData;
	if (count > -1)
	{
		// Valid detection, add it to the cue
		data.fingerCounts.push_back(count);
		data.timeStamps.push_back(currentTime);
		// Remove old data if too many
		if (data.timeStamps.size() > data.maxHistoryLength)
		{
			data.fingerCounts.pop_front();
			data.timeStamps.pop_front();
		}
	}
	// Additionally remove too old data according to their time stamp
	while (data.timeStamps.size() > 0
		&& currentTime-data.timeStamps.back() > data.maxHistoryLength*data.trackingIntervall)
	{
		data.fingerCounts.pop_front();
		data.timeStamps.pop_front();
	}
}

int FubiUser::getFingerCount(bool leftHand /*= false*/, bool getMedianOfLastFrames /*= true*/, 
	unsigned int medianWindowSize /*= 10*/, bool useOldConvexityDefectMethod /*= false*/)
{
	int fingerCount = -1;
	FingerCountData& data = leftHand ? m_leftFingerCountData : m_rightFingerCountData;

	if (getMedianOfLastFrames)
	{
		// Automatically adapt the window size if too small
		if (data.maxHistoryLength < medianWindowSize)
			data.maxHistoryLength = medianWindowSize;
		// Activate finger tracking if not already happened
		enableFingerTracking(true, leftHand);
		// Now calculate the median using the last tracked values
		fingerCount = heapMedian(data.fingerCounts, medianWindowSize);

		/*if (data.fingerCounts.size() > 0)
		{
			printf("FingerCounts: %d", data.fingerCounts[0]);
			for (unsigned int i = 1; i < data.fingerCounts.size(); ++i)
				printf(", %d", data.fingerCounts[i]);
			printf("\n");
		}*/
	}
	else if (FubiCore::getInstance() && FubiCore::getInstance()->getSensor())
	{
		FubiISensor* sensor = FubiCore::getInstance()->getSensor();
		// No median calculations wanted
		// First check if last finger count calculation is already up to date
		if (data.timeStamps.size() > 0
			&& (!sensor->getStreamOptions(ImageType::Depth).isValid() || currentTime() - data.timeStamps.back() < (1.0f / sensor->getStreamOptions(ImageType::Depth).m_fps)))
			fingerCount = data.fingerCounts.back();
		else
			// Calculate the finger count instantly
			fingerCount = FubiImageProcessing::applyFingerCount(FubiCore::getInstance()->getSensor(), m_id, leftHand, 
				useOldConvexityDefectMethod, &data.fingerCountImage);
	}

	return fingerCount;
}

void FubiUser::updateTrackingData(FubiISensor* sensor)
{
	if (sensor)
	{
		// First update tracking state
		m_isTracked = sensor->isTracking(m_id);

		if (sensor->hasNewTrackingData())
		{
			// First add a new tracking data point at the end and remove the oldest one
			addNewTrackingData(Fubi::currentTime());

			TrackingData* data = m_trackingData.back();

			if (m_isTracked)
			{
				// Get all joint positions for that user
				for (unsigned int j=0; j < SkeletonJoint::NUM_JOINTS; ++j)
				{
					sensor->getSkeletonJointData(m_id, (SkeletonJoint::Joint) j, data->jointPositions[j], data->jointOrientations[j]);
				}

				// Update filtered and local data, measures, combinations and finger counts
				update(data->timeStamp);
			}
			else
			{
				// Only try to get the torso (should be independent of complete tracking)
				sensor->getSkeletonJointData(m_id, SkeletonJoint::TORSO, data->jointPositions[SkeletonJoint::TORSO], data->jointOrientations[SkeletonJoint::TORSO]);
			}
		}
	}
}
void FubiUser::addNewTrackingData(double timeStamp /*= -1*/)
{
	// Cleanup too old data
	while (m_trackingData.size() > m_maxTrackingHistoryLength)
	{
		delete m_trackingData.front();
		delete m_filteredTrackingData.front();
		m_trackingData.pop_front();
		m_filteredTrackingData.pop_front();
	}

	// Add new data point at the end
	Fubi::TrackingData* data, *filteredData;
	if (m_trackingData.size() < m_maxTrackingHistoryLength)
	{
		// Create new data
		data = new Fubi::UserTrackingData();
		filteredData = new Fubi::UserTrackingData();
	}
	else // m_trackingData.size() == m_maxTrackingHistoryLength
	{
		// Reuse oldest data 
		data = m_trackingData.front();
		filteredData = m_filteredTrackingData.front();
		m_trackingData.pop_front();
		m_filteredTrackingData.pop_front();
	}
	m_trackingData.push_back(data);
	m_filteredTrackingData.push_back(filteredData);

	// Update timestamp
	filteredData->timeStamp = data->timeStamp = (timeStamp >= 0) ? timeStamp : Fubi::currentTime();
}

void FubiUser::addNewTrackingData(float* skeleton, double timeStamp /*= -1*/)
{
	// First add a new tracking data point at the end and remove the oldest one
	addNewTrackingData(timeStamp);

	// Get the reference to them
	TrackingData* data = m_trackingData.back();
	TrackingData* filteredData = m_filteredTrackingData.back();

	// Copy in the new data
	for (int i = 0; i < SkeletonJoint::NUM_JOINTS; ++i)
	{
		int startIndex = i * 8;
		SkeletonJointPosition& pos = data->jointPositions[i];
		pos.m_position.x = skeleton[startIndex];
		pos.m_position.y = skeleton[startIndex + 1];
		pos.m_position.z = skeleton[startIndex + 2];
		pos.m_confidence = skeleton[startIndex + 3];
		float rotX = degToRad(skeleton[startIndex + 4]);
		float rotY = degToRad(skeleton[startIndex + 5]);
		float rotZ = degToRad(skeleton[startIndex + 6]);
		SkeletonJointOrientation& orient = data->jointOrientations[i];
		orient.m_orientation = Matrix3f::RotMat(rotX, rotY, rotZ);
		orient.m_confidence = skeleton[startIndex + 7];
	}

	// Update filtered and local data, measures, combinations and finger counts
	update(data->timeStamp);
}

void FubiUser::addNewTrackingData(const Fubi::SkeletonJointPosition* positions, const Fubi::SkeletonJointOrientation* orientations,
	int leftFingerCount /*= -1*/, int rightFingerCount /*= -1*/, double timeStamp /*= -1*/)
{
	// First add a new tracking data point at the end and remove the oldest one
	addNewTrackingData(timeStamp);

	// Get the reference to them
	TrackingData* data = m_trackingData.back();
	TrackingData* filteredData = m_filteredTrackingData.back();

	// Set new transformations
	for (unsigned int j=0; j < SkeletonJoint::NUM_JOINTS; ++j)
	{
		SkeletonJoint::Joint joint = (SkeletonJoint::Joint) j;
		data->jointPositions[joint] = positions[j];
		data->jointOrientations[joint] = orientations[j];
	}

	// Update finger counts
	addFingerCount(leftFingerCount, true, data->timeStamp);
	addFingerCount(rightFingerCount, false, data->timeStamp);

	// Update filtered and local data, measures, combinations and finger counts
	update(data->timeStamp);
}

void FubiUser::updateCombinationRecognizers()
{
	// Update the posture combination recognizers
	for (unsigned int i=0; i < Fubi::Combinations::NUM_COMBINATIONS; ++i)
	{
		if (m_combinationRecognizers[i])
		{
			if (!m_combinationRecognizers[i]->isActive() && Fubi::getAutoStartCombinationRecognition((Fubi::Combinations::Combination)i))
			{
				// Reactivate combination recognizers that should already be active
				m_combinationRecognizers[i]->start();
			}
			m_combinationRecognizers[i]->update();
		}
	}
	std::map<std::string, CombinationRecognizer*>::iterator iter;
	std::map<std::string, CombinationRecognizer*>::iterator end = m_userDefinedCombinationRecognizers.end();
	for (iter = m_userDefinedCombinationRecognizers.begin(); iter != end; ++iter)
	{
		if (iter->second)
		{
			if (!iter->second->isActive() && Fubi::getAutoStartCombinationRecognition())
			{
				// Reactivate combination recognizers that should already be active
				iter->second->start();
			}
			iter->second->update();
		}
	}
}

void FubiUser::calculateFilteredTransformations(const Fubi::FilterOptions& filterOptions)
{
	// Apply filter to current global positions and orientations
	const TrackingData* data =  currentTrackingData();

	if (m_trackingData.size() > 1 && data->timeStamp > lastTrackingData()->timeStamp )
	{
		// Passed time since last update
		const float timeStep = float(data->timeStamp - lastTrackingData()->timeStamp);

		// Global alpha values for the velocity
		const float velAlpha = oneEuroAlpha(timeStep, filterOptions.m_velocityCutOffFrequency);
		const float invVelAlpha = 1.0f-velAlpha;

		const TrackingData* lastFilteredData = lastFilteredTrackingData();
		TrackingData* filteredData = m_filteredTrackingData.back();

		for (unsigned int i=0; i < SkeletonJoint::NUM_JOINTS; ++i)
		{
			// Calculate velocity as position difference..
			Vec3f vel = data->jointPositions[i].m_position - lastFilteredData->jointPositions[i].m_position;
			// ..divided by time difference
			vel /= timeStep;
			// Now low-pass filter the velocity
			vel = vel*velAlpha + m_lastFilteredVelocity[i]*invVelAlpha;
			// And save it
			m_lastFilteredVelocity[i] = vel;
			// Calculate the alpha values for the position filtering out of the filtered velocity
			Vec3f posAlpha(Math::NO_INIT);
			posAlpha.x = oneEuroAlpha(timeStep, filterOptions.m_minCutOffFrequency + filterOptions.m_cutOffSlope * abs(vel.x));
			posAlpha.y = oneEuroAlpha(timeStep, filterOptions.m_minCutOffFrequency + filterOptions.m_cutOffSlope * abs(vel.y));
			posAlpha.z = oneEuroAlpha(timeStep, filterOptions.m_minCutOffFrequency + filterOptions.m_cutOffSlope * abs(vel.z));
			Vec3f invPosAlpha(1.0f-posAlpha.x, 1.0f-posAlpha.y, 1.0f-posAlpha.z);
			// Now we can finally apply the filter
			filteredData->jointPositions[i].m_position = (data->jointPositions[i].m_position*posAlpha) + (lastFilteredData->jointPositions[i].m_position*invPosAlpha);
			// Calculate confidence according to the average alpha value
			float avgAlpha = (posAlpha.x+posAlpha.y+posAlpha.z)/3.0f;
			filteredData->jointPositions[i].m_confidence = avgAlpha*data->jointPositions[i].m_confidence + (1.0f - avgAlpha)*lastFilteredData->jointPositions[i].m_confidence;

			// For the orientations, we directly use the min cut off frequency without any velocity dependency
			filteredData->jointOrientations[i].m_orientation = (data->jointOrientations[i].m_orientation*filterOptions.m_minCutOffFrequency)
				+ (lastFilteredData->jointOrientations[i].m_orientation*(1.0f - filterOptions.m_minCutOffFrequency));
			// Calculate confidence according to the alpha value
			filteredData->jointOrientations[i].m_confidence = filterOptions.m_minCutOffFrequency*data->jointOrientations[i].m_confidence 
				+ (1.0f - filterOptions.m_minCutOffFrequency)*lastFilteredData->jointOrientations[i].m_confidence;
		}
	}
	else
	{
		// Unable to calculate new filtered values
		TrackingData* filteredData = m_filteredTrackingData.back();
		const TrackingData* dataToCopy = 0x0;
		if (m_trackingData.size() <= 1 ||data->timeStamp < lastTrackingData()->timeStamp)
		{
			// Take the raw values if we only have a single data, yet or if we are moving back in time
			dataToCopy = data;
		}
		else
		{
			// Else use the last filtered ones (probably we are pausing)
			dataToCopy = lastFilteredTrackingData();
		}
		for (unsigned int i = 0; i < SkeletonJoint::NUM_JOINTS; ++i)
		{
			filteredData->jointPositions[i] = dataToCopy->jointPositions[i];
			filteredData->jointOrientations[i] = dataToCopy->jointOrientations[i];
		}
	}
}

void FubiUser::updateFingerCount(bool leftHand, double currentTimestamp)
{
	// Update finger detection
	FingerCountData& data = leftHand ? m_leftFingerCountData : m_rightFingerCountData;
	if (data.trackingEnabled
		&& (data.timeStamps.size() == 0 || (currentTimestamp - data.timeStamps.back()) > data.trackingIntervall))
	{
		addFingerCount(getFingerCount(leftHand, false, 0, m_leftFingerCountData.useConvexityDefectMethod), leftHand, currentTimestamp);
	}
}

void FubiUser::updateBodyMeasurements(double currentTimestamp, const Fubi::FilterOptions& filterOptions)
{
	// Keep update interval
	if (currentTimestamp - m_lastBodyMeasurementUpdate > m_bodyMeasurementUpdateIntervall)
	{
		m_lastBodyMeasurementUpdate = currentTimestamp;
		// Take the already filtered data to avoid outliers
		const TrackingData* data = currentFilteredTrackingData();

		// Select joints
		SkeletonJoint::Joint footToTake = SkeletonJoint::RIGHT_FOOT;
		SkeletonJoint::Joint kneeForFoot = SkeletonJoint::RIGHT_KNEE;
		if (data->jointPositions[SkeletonJoint::LEFT_FOOT].m_confidence > data->jointPositions[SkeletonJoint::RIGHT_FOOT].m_confidence)
		{
			footToTake = SkeletonJoint::LEFT_FOOT;
			kneeForFoot = SkeletonJoint::LEFT_KNEE;
		}
		SkeletonJoint::Joint hipToTake = SkeletonJoint::RIGHT_HIP;
		SkeletonJoint::Joint kneeForHip = SkeletonJoint::RIGHT_KNEE;
		if (data->jointPositions[SkeletonJoint::LEFT_KNEE].m_confidence > data->jointPositions[SkeletonJoint::RIGHT_KNEE].m_confidence)
		{
			hipToTake = SkeletonJoint::LEFT_HIP;
			kneeForHip = SkeletonJoint::LEFT_KNEE;
		}
		SkeletonJoint::Joint handToTake = SkeletonJoint::RIGHT_HAND;
		SkeletonJoint::Joint elbowForHand = SkeletonJoint::RIGHT_ELBOW;
		if (data->jointPositions[SkeletonJoint::LEFT_HAND].m_confidence > data->jointPositions[SkeletonJoint::RIGHT_HAND].m_confidence)
		{
			handToTake = SkeletonJoint::LEFT_HAND;
			elbowForHand = SkeletonJoint::LEFT_ELBOW;
		}
		SkeletonJoint::Joint shoulderToTake = SkeletonJoint::RIGHT_SHOULDER;
		SkeletonJoint::Joint elbowForShoulder = SkeletonJoint::RIGHT_ELBOW;
		if (data->jointPositions[SkeletonJoint::LEFT_ELBOW].m_confidence > data->jointPositions[SkeletonJoint::RIGHT_ELBOW].m_confidence)
		{
			shoulderToTake = SkeletonJoint::LEFT_SHOULDER;
			elbowForShoulder = SkeletonJoint::LEFT_ELBOW;
		}

		// Body height
		//Add the neck-head distance to compensate for the missing upper head part
		SkeletonJointPosition headEnd(data->jointPositions[SkeletonJoint::HEAD]);
		headEnd.m_position = headEnd.m_position + (headEnd.m_position-data->jointPositions[SkeletonJoint::NECK].m_position);
		Fubi::calculateBodyMeasurement(data->jointPositions[footToTake],
			headEnd, m_bodyMeasurements[BodyMeasurement::BODY_HEIGHT], filterOptions.m_bodyMeasureFilterFac);

		// Torso height
		Fubi::calculateBodyMeasurement(data->jointPositions[SkeletonJoint::WAIST],
			data->jointPositions[SkeletonJoint::NECK], m_bodyMeasurements[BodyMeasurement::TORSO_HEIGHT], filterOptions.m_bodyMeasureFilterFac);

		// Shoulder width
		Fubi::calculateBodyMeasurement(data->jointPositions[SkeletonJoint::LEFT_SHOULDER],
			data->jointPositions[SkeletonJoint::RIGHT_SHOULDER], m_bodyMeasurements[BodyMeasurement::SHOULDER_WIDTH], filterOptions.m_bodyMeasureFilterFac);

		// Hip width
		Fubi::calculateBodyMeasurement(data->jointPositions[SkeletonJoint::LEFT_HIP],
			data->jointPositions[SkeletonJoint::RIGHT_HIP], m_bodyMeasurements[BodyMeasurement::HIP_WIDTH], filterOptions.m_bodyMeasureFilterFac);

		// Arm lengths
		Fubi::calculateBodyMeasurement(data->jointPositions[shoulderToTake],
			data->jointPositions[elbowForShoulder], m_bodyMeasurements[BodyMeasurement::UPPER_ARM_LENGTH], filterOptions.m_bodyMeasureFilterFac);
		Fubi::calculateBodyMeasurement(data->jointPositions[handToTake],
			data->jointPositions[elbowForHand], m_bodyMeasurements[BodyMeasurement::LOWER_ARM_LENGTH], filterOptions.m_bodyMeasureFilterFac);
		m_bodyMeasurements[BodyMeasurement::ARM_LENGTH].m_dist = m_bodyMeasurements[BodyMeasurement::LOWER_ARM_LENGTH].m_dist + m_bodyMeasurements[BodyMeasurement::UPPER_ARM_LENGTH].m_dist;
		m_bodyMeasurements[BodyMeasurement::ARM_LENGTH].m_confidence = minf(m_bodyMeasurements[BodyMeasurement::LOWER_ARM_LENGTH].m_confidence, m_bodyMeasurements[BodyMeasurement::UPPER_ARM_LENGTH].m_confidence);

		// Leg lengths
		Fubi::calculateBodyMeasurement(data->jointPositions[hipToTake],
			data->jointPositions[kneeForHip], m_bodyMeasurements[BodyMeasurement::UPPER_LEG_LENGTH], filterOptions.m_bodyMeasureFilterFac);
		Fubi::calculateBodyMeasurement(data->jointPositions[footToTake],
			data->jointPositions[kneeForFoot], m_bodyMeasurements[BodyMeasurement::LOWER_LEG_LENGTH], filterOptions.m_bodyMeasureFilterFac);
		m_bodyMeasurements[BodyMeasurement::LEG_LENGTH].m_dist = m_bodyMeasurements[BodyMeasurement::LOWER_LEG_LENGTH].m_dist + m_bodyMeasurements[BodyMeasurement::UPPER_LEG_LENGTH].m_dist;
		m_bodyMeasurements[BodyMeasurement::LEG_LENGTH].m_confidence = minf(m_bodyMeasurements[BodyMeasurement::LOWER_LEG_LENGTH].m_confidence, m_bodyMeasurements[BodyMeasurement::UPPER_LEG_LENGTH].m_confidence);
	}
}

void FubiUser::reset()
{
	m_isTracked = false;
	m_inScene = false;
	for (auto iter = m_trackingData.begin(); iter != m_trackingData.end(); ++iter)
		delete *iter;
	for (auto iter = m_filteredTrackingData.begin(); iter != m_filteredTrackingData.end(); ++iter)
		delete *iter;
	m_trackingData.clear();
	m_filteredTrackingData.clear();
	m_id = 0;
	m_leftFingerCountData.trackingEnabled = false;
	m_rightFingerCountData.trackingEnabled = false;
	m_lastBodyMeasurementUpdate = 0;
	m_assignedLeftHandId = 0;
	m_assignedRightHandId = 0;
}

Fubi::RecognitionResult::Result FubiUser::getRecognitionProgress(Combinations::Combination combinationID, std::vector<Fubi::TrackingData>* userStates,
	bool restart, bool returnFilteredData, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	if(m_combinationRecognizers[combinationID])
		return m_combinationRecognizers[combinationID]->getRecognitionProgress(userStates, restart, returnFilteredData, correctionHint);
	return Fubi::RecognitionResult::NOT_RECOGNIZED;
}

Fubi::RecognitionResult::Result FubiUser::getRecognitionProgress(const std::string& recognizerName, std::vector<Fubi::TrackingData>* userStates,
	bool restart, bool returnFilteredData, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	std::map<string, CombinationRecognizer*>::iterator rec = m_userDefinedCombinationRecognizers.find(recognizerName);
	if (rec != m_userDefinedCombinationRecognizers.end() && rec->second)
		return rec->second->getRecognitionProgress(userStates, restart, returnFilteredData, correctionHint);
	return Fubi::RecognitionResult::NOT_RECOGNIZED;
}

int FubiUser::getCurrentRecognitionState(const std::string& recognizerName, unsigned int& numStates, bool& isInterrupted, bool& isInTransition)
{
	std::map<string, CombinationRecognizer*>::iterator rec = m_userDefinedCombinationRecognizers.find(recognizerName);
	if (rec != m_userDefinedCombinationRecognizers.end() && rec->second)
	{
		CombinationRecognizer* r = rec->second;
		numStates = r->getNumStates();
		isInterrupted = r->isInterrupted();
		isInTransition = r->isWaitingForTransition();
		return r->getCurrentState();
	}
	return -3;
}

void FubiUser::update(double currentTimestamp)
{
	// Get the reference to the current data
	TrackingData* data = m_trackingData.back();
	TrackingData* filteredData = m_filteredTrackingData.back();
	// Calculate local transformations out of the global ones
	calculateLocalTransformations(data->jointPositions, data->jointOrientations, data->localJointPositions, data->localJointOrientations);

	FubiCore* core = FubiCore::getInstance();
	if (core)
	{
		const FilterOptions& filterOptions = core->getFilterOptions();
		// Filter the current data
		calculateFilteredTransformations(filterOptions);
		// And calculate the local filtered ones
		calculateLocalTransformations(filteredData->jointPositions, filteredData->jointOrientations, filteredData->localJointPositions, filteredData->localJointOrientations);
		// Update body measurements (uses the local transformations)
		updateBodyMeasurements(currentTimestamp, filterOptions);
	}
						
	// Immediately update the combination recognizers (Only if new joint data is here)
	updateCombinationRecognizers();

	// Check and update finger detection for both hands if necessarry
	updateFingerCount(false, currentTimestamp);
	updateFingerCount(true, currentTimestamp);
}