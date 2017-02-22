// ****************************************************************************************
//
// Fubi Hand
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler 
// 
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
// 
// ****************************************************************************************

#include "FubiHand.h"

#include "FubiIFingerSensor.h"
#include "FubiCore.h"
#include "FubiRecognizerFactory.h"
#include "GestureRecognizer/CombinationRecognizer.h"

using namespace Fubi;

FubiHand::FubiHand() : m_id(0), m_isTracked(false), m_maxTrackingHistoryLength(2)
{
	// And additional init for filtered data
	memset(m_lastFilteredVelocity, 0, sizeof(Fubi::Vec3f)*Fubi::SkeletonHandJoint::NUM_JOINTS);
}


FubiHand::~FubiHand()
{
	clearCombinationRecognizers();

	for (auto iter = m_trackingData.begin(); iter != m_trackingData.end(); ++iter)
		delete *iter;
	for (auto iter = m_filteredTrackingData.begin(); iter != m_filteredTrackingData.end(); ++iter)
		delete *iter;
}

void FubiHand::clearCombinationRecognizers()
{
	auto end = m_combinationRecognizers.end();
	for (auto iter = m_combinationRecognizers.begin(); iter != end; ++iter)
	{
		delete iter->second;
	}
	m_combinationRecognizers.clear();
}

void FubiHand::enableCombinationRecognition(const CombinationRecognizer* recognizerTemplate, bool enable)
{
	if (recognizerTemplate)
	{
		std::map<std::string, CombinationRecognizer*>::iterator rec = m_combinationRecognizers.find(recognizerTemplate->getName());
		if (rec != m_combinationRecognizers.end())
		{
			if (enable)
				rec->second->start();
			else
			{
				rec->second->stop();
			}
		}
		else if (enable && recognizerTemplate->getRecognizerTarget() != RecognizerTarget::BODY && recognizerTemplate->getRecognizerTarget() != RecognizerTarget::BODY_AND_HANDS)
		{
			CombinationRecognizer* clonedRec = recognizerTemplate->clone();
			clonedRec->setHand(this);
			m_combinationRecognizers[recognizerTemplate->getName()] = clonedRec;
			clonedRec->start();
		}
	}
}

void FubiHand::updateFingerTrackingData(FubiIFingerSensor* sensor)
{
	if (sensor)
	{
		// First update tracking state
		m_isTracked = sensor->isTracking(m_id);

		if (sensor->hasNewTrackingData())
		{
			// First add a new tracking data point at the end and remove the oldest one
			addNewTrackingData(Fubi::currentTime());

			// The other joints are only valid if the user is tracked
			if (m_isTracked)
			{
				FingerTrackingData* data = (FingerTrackingData*) m_trackingData.back();

				// Get finger count
				data->fingerCount = sensor->getFingerCount(m_id);

				// Get all joint positions/orientations for that hand
				for (unsigned int j=0; j < SkeletonHandJoint::NUM_JOINTS; ++j)
				{
					sensor->getFingerTrackingData(m_id, (SkeletonHandJoint::Joint) j, data->jointPositions[j], data->jointOrientations[j]);
				}

				// Update hand Type
				m_handType = sensor->getHandType(m_id);

				// Update filtered data and combinations
				update();
			}
		}
	}
}

void FubiHand::addNewTrackingData(double timeStamp /*= -1*/)
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
	TrackingData* data, *filteredData;
	if (m_trackingData.size() < m_maxTrackingHistoryLength)
	{
		// Create new data
		data = new FingerTrackingData();
		filteredData = new FingerTrackingData();
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

void FubiHand::addNewTrackingData(const Fubi::SkeletonJointPosition* positions, const Fubi::SkeletonJointOrientation* orientations,
	int fingerCount /*= -1*/, Fubi::SkeletonJoint::Joint handType /*= Fubi::SkeletonJoint::NUM_JOINTS*/, double timeStamp /*= -1*/)
{
	// First add a new tracking data point at the end and remove the oldest one
	addNewTrackingData(timeStamp);

	// Get the reference to them
	FingerTrackingData* data = (FingerTrackingData*) m_trackingData.back();
	FingerTrackingData* filteredData = (FingerTrackingData*) m_filteredTrackingData.back();

	// Set new transformations
	for (unsigned int j = 0; j < SkeletonHandJoint::NUM_JOINTS; ++j)
	{
		data->jointPositions[j] = positions[j];
		data->jointOrientations[j] = orientations[j];
	}

	data->fingerCount = fingerCount;

	m_handType = handType;

	// Update filtered data and combinations
	update();
}


void FubiHand::updateCombinationRecognizers()
{
	// Update the combination recognizers
	std::map<std::string, CombinationRecognizer*>::iterator iter;
	std::map<std::string, CombinationRecognizer*>::iterator end = m_combinationRecognizers.end();
	for (iter = m_combinationRecognizers.begin(); iter != end; ++iter)
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

void FubiHand::calculateFilteredTransformations()
{
	// Apply filter to current global positions and orientations
	FubiCore* core = FubiCore::getInstance();
	const FingerTrackingData* data = (const FingerTrackingData*) currentTrackingData();
	if (core && data->timeStamp > lastTrackingData()->timeStamp)
	{
		// Get filter config from the core
		float cutOffSlope, minCutOff, velocityCutOffFrequency, bodyMeasureFilterFac;
		core->getFilterOptions(minCutOff, velocityCutOffFrequency, cutOffSlope, bodyMeasureFilterFac);

		// Passed time since last update
		const float timeStep = float(data->timeStamp - lastTrackingData()->timeStamp);

		// Global alpha values for the velocity
		const float velAlpha = oneEuroAlpha(timeStep, velocityCutOffFrequency);
		const float invVelAlpha = 1.0f-velAlpha;

		const FingerTrackingData* lastFilteredData = (const FingerTrackingData*) lastFilteredTrackingData();
		FingerTrackingData* filteredData = (FingerTrackingData*) m_filteredTrackingData.back();

		for (unsigned int i=0; i < SkeletonHandJoint::NUM_JOINTS; ++i)
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
			posAlpha.x = oneEuroAlpha(timeStep, minCutOff + cutOffSlope * abs(vel.x));
			posAlpha.y = oneEuroAlpha(timeStep, minCutOff + cutOffSlope * abs(vel.y));
			posAlpha.z = oneEuroAlpha(timeStep, minCutOff + cutOffSlope * abs(vel.z));
			Vec3f invPosAlpha(1.0f-posAlpha.x, 1.0f-posAlpha.y, 1.0f-posAlpha.z);
			// Now we can finally apply the filter
			filteredData->jointPositions[i].m_position = (data->jointPositions[i].m_position*posAlpha) + (lastFilteredData->jointPositions[i].m_position*invPosAlpha);
			// Calculate confidence according to the average alpha value
			float avgAlpha = (posAlpha.x+posAlpha.y+posAlpha.z)/3.0f;
			filteredData->jointPositions[i].m_confidence = avgAlpha*data->jointPositions[i].m_confidence + (1.0f - avgAlpha)*lastFilteredData->jointPositions[i].m_confidence;

			// For the orientations, we directly use the min cut off frequency without any velocity dependency
			filteredData->jointOrientations[i].m_orientation = (data->jointOrientations[i].m_orientation*minCutOff) + (lastFilteredData->jointOrientations[i].m_orientation*(1.0f - minCutOff));
			// Calculate confidence according to the alpha value
			filteredData->jointOrientations[i].m_confidence = minCutOff*data->jointOrientations[i].m_confidence + (1.0f - minCutOff)*lastFilteredData->jointOrientations[i].m_confidence;

			// Only copy the finger count
			filteredData->fingerCount = data->fingerCount;
		}
	}
	else
	{
		// Unable to calculate new filtered values, so try to reuse the old ones
		TrackingData* filteredData = m_filteredTrackingData.back();
		const TrackingData* lastFilteredData = lastFilteredTrackingData();
		for (unsigned int i = 0; i < SkeletonJoint::NUM_JOINTS; ++i)
		{
			filteredData->jointPositions[i] = lastFilteredData->jointPositions[i];
			filteredData->jointOrientations[i] = lastFilteredData->jointOrientations[i];
		}
	}
}


void FubiHand::reset()
{
	m_isTracked = false;
	m_id = 0;
}

Fubi::RecognitionResult::Result FubiHand::getRecognitionProgress(const std::string& recognizerName, std::vector<TrackingData>* trackingStates,
	bool restart, bool returnFilteredData, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
{
	std::map<string, CombinationRecognizer*>::iterator rec = m_combinationRecognizers.find(recognizerName);
	if (rec != m_combinationRecognizers.end() && rec->second)
		return rec->second->getRecognitionProgress(trackingStates, restart, returnFilteredData, correctionHint);
	return Fubi::RecognitionResult::NOT_RECOGNIZED;
}

int FubiHand::getCurrentRecognitionState(const std::string& recognizerName, unsigned int& numStates, bool& isInterrupted, bool& isInTransition)
{
	std::map<string, CombinationRecognizer*>::iterator rec = m_combinationRecognizers.find(recognizerName);
	if (rec != m_combinationRecognizers.end() && rec->second)
	{
		CombinationRecognizer* r = rec->second;
		numStates = r->getNumStates();
		isInterrupted = r->isInterrupted();
		isInTransition = r->isWaitingForTransition();
		return r->getCurrentState();
	}
	return -3;
}

void FubiHand::update()
{
	FingerTrackingData* data = (FingerTrackingData*) m_trackingData.back();
	FingerTrackingData* filteredData = (FingerTrackingData*) m_filteredTrackingData.back();

	// Calculate local transformations out of the global ones
	calculateLocalHandTransformations(data->jointPositions, data->jointOrientations, data->localJointPositions, data->localJointOrientations);

	// Filter the current data
	calculateFilteredTransformations();

	// And calculate the local filtered ones
	calculateLocalHandTransformations(filteredData->jointPositions, filteredData->jointOrientations, filteredData->localJointPositions, filteredData->localJointOrientations);
					
	// Immediately update the combination recognizers (Only if new joint data is here?)
	updateCombinationRecognizers();
}