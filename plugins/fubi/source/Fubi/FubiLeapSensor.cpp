// ****************************************************************************************
//
// Fubi Leap Sensor
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
//
// ****************************************************************************************

#include "FubiLeapSensor.h"

#ifdef FUBI_USE_LEAP

#include <LeapMath.h>

#pragma comment(lib, "Leap.lib")


using namespace Fubi;

FubiLeapSensor::FubiLeapSensor() : m_hasNewData(false)
{
	m_type = FingerSensorType::LEAP;
}

FubiLeapSensor::~FubiLeapSensor()
{
}

void FubiLeapSensor::update()
{
	if (m_controller.isConnected())
	{
		int64_t lastID = m_currFrame.id();
		m_currFrame = m_controller.frame();
		m_hasNewData = lastID != m_currFrame.id();
	}
}

unsigned short FubiLeapSensor::getHandIDs(unsigned int* handIDs)
{
	int numHands = 0;
	if (m_currFrame.isValid())
	{
		numHands = m_currFrame.hands().count();
		if (handIDs)
		{
			for (int i = 0; i < numHands; ++i)
			{
				handIDs[i] = m_currFrame.hands()[i].id();
			}
		}
	}
	return numHands;
}

bool FubiLeapSensor::isTracking(int id)
{
	if (m_currFrame.isValid())
	{
		const Leap::Hand& h = m_currFrame.hand(id);
		return h.isValid();
	}
	return false;
}

Fubi::SkeletonJoint::Joint FubiLeapSensor::getHandType(int handID)
{
	if (m_currFrame.isValid())
	{
		const Leap::Hand& h = m_currFrame.hand(handID);
		if (h.isValid())
		{
			if (h.isLeft())
				return SkeletonJoint::LEFT_HAND;
			if (h.isRight())
				return SkeletonJoint::RIGHT_HAND;
		}
	}
	return SkeletonJoint::NUM_JOINTS;
}

void FubiLeapSensor::getFingerTrackingData(int handID, SkeletonHandJoint::Joint joint,
	Fubi::SkeletonJointPosition& position, Fubi::SkeletonJointOrientation& orientation)
{
	bool fingerFound = false;
	if (m_currFrame.isValid())
	{
		const Leap::Hand& h = m_currFrame.hand(handID);
		if (h.isValid())
		{
			Leap::Vector pos;
			float roll, yaw, pitch;

			if (joint == SkeletonHandJoint::PALM)
			{
				fingerFound = true;
				pos = h.palmPosition();
				roll = h.direction().roll();
				yaw = h.direction().yaw();
				pitch = h.direction().pitch();
			}
			else
			{
				int fingerIndex = joint - 1;
				Leap::FingerList fingers = h.fingers();
				if (fingerIndex < fingers.count())
				{
					const Leap::Finger& f = fingers[fingerIndex];
					if (f.isValid())
					{
						fingerFound = true;
						pos = f.tipPosition();
						roll = f.direction().roll();
						yaw = f.direction().yaw();
						pitch = f.direction().pitch();
					}
				}
			}

			if (fingerFound)
			{
				position.m_confidence = h.confidence();
				position.m_position.x = pos.x + m_offsetPos.x;
				position.m_position.y = pos.y + m_offsetPos.y;
				position.m_position.z = pos.z + m_offsetPos.z;

				orientation.m_confidence = h.confidence();
				orientation.m_orientation = Matrix3f::RotMat(pitch, -yaw, -(roll+Math::Pi));
			}
		}
	}

	if (!fingerFound)
	{
		// not found so reset the confidence
		position.m_confidence = 0;
		orientation.m_confidence = 0;
	}
}

int FubiLeapSensor::getFingerCount(int id)
{
	if (m_currFrame.isValid())
	{
		const Leap::Hand& h = m_currFrame.hand(id);
		if (h.isValid())
		{
			// Only extended fingers
			return h.fingers().extended().count();
		}
	}
	return -1;
}

bool FubiLeapSensor::init(Fubi::Vec3f offsetPos)
{
	m_offsetPos  = offsetPos;
	m_controller.setPolicyFlags(Leap::Controller::POLICY_DEFAULT);
	// Actually nothing to do here for a real initialization...
	Fubi_logInfo("FubiLeapSensor: succesfully initialized!\n");
	return true;
}

const unsigned char* FubiLeapSensor::getImageData(unsigned int index)
{
	// Request image retreival if not activated, yet
	if ((m_controller.policyFlags() & Leap::Controller::POLICY_IMAGES) == 0)
		m_controller.setPolicyFlags(Leap::Controller::POLICY_IMAGES);

	Leap::ImageList imgList = (m_currFrame.isValid()) ? m_currFrame.images() : m_controller.images();

	return ((signed)index < imgList.count()) ? imgList[index].data() : 0x0;
}

void FubiLeapSensor::getImageConfig(int& width, int& height, unsigned int& numStreams)
{
	// Request image retreival if not activated, yet
	if ((m_controller.policyFlags() & Leap::Controller::POLICY_IMAGES) == 0)
		m_controller.setPolicyFlags(Leap::Controller::POLICY_IMAGES);

	Leap::ImageList imgList = (m_currFrame.isValid()) ? m_currFrame.images() : m_controller.images();

	numStreams = imgList.count();
	width = numStreams ? imgList[0].width() : -1;
	height = numStreams ? imgList[0].height() : -1;
}
#endif
