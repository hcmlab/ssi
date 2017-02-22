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
#pragma once

#include "FubiConfig.h"

#ifdef FUBI_USE_LEAP
#include "FubiIFingerSensor.h"

#include <Leap.h>

#include <map>

// The Fubi Sensor interface offers depth/rgb/ir image streams and user tracking data
class FubiLeapSensor : public FubiIFingerSensor
{
public:
	FubiLeapSensor();
	virtual ~FubiLeapSensor();

	// Get the ids of all currently valid hands: Ids will be stored in handIDs (if not 0x0), returns the number of valid hands
	virtual unsigned short getHandIDs(unsigned int* handIDs);

	// Update should be called once per frame for the sensor to update its streams and tracking data
	virtual void update();

	// Check if the sensor has new tracking data available
	virtual bool hasNewTrackingData() { return m_hasNewData; }

	// Check if that hand with the given id is tracked by the sensor
	virtual bool isTracking(int id);

	// Get the current joint position and orientation of one user
	virtual void getFingerTrackingData(int handID, Fubi::SkeletonHandJoint::Joint joint, Fubi::SkeletonJointPosition& position, Fubi::SkeletonJointOrientation& orientation);

	// Gets the number of shown fingers or -1 on error
	virtual int getFingerCount(int id);

	// Get the hand type (left or right hand or NUM_JOINTS if unknown)
	virtual Fubi::SkeletonJoint::Joint getHandType(int handID);

	// init function
	virtual bool init(Fubi::Vec3f offsetPos);

	// access to image data
	virtual const unsigned char* getImageData(unsigned int index);

	// image data resolution
	virtual  void getImageConfig(int& width, int& height, unsigned int& numStreams);

private:
	Leap::Controller m_controller;

	Leap::Frame m_currFrame;
	bool m_hasNewData;
};

#endif