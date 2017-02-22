// ****************************************************************************************
//
// Fubi Finger Sensor Interface
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
//
// ****************************************************************************************
#pragma once

#include "FubiMath.h"

// The Fubi Sensor interface offers depth/rgb/ir image streams and user tracking data
class FubiIFingerSensor
{
public:
	virtual ~FubiIFingerSensor() {}

	// Get the ids of all currently valid hands: Ids will be stored in handIDs (if not 0x0), returns the number of valid hands
	virtual unsigned short getHandIDs(unsigned int* handIDs) = 0;

	// Update should be called once per frame for the sensor to update its streams and tracking data
	virtual void update() = 0;

	// Check if the sensor has new tracking data available
	virtual bool hasNewTrackingData() = 0;

	// Check if that hand with the given id is tracked by the sensor
	virtual bool isTracking(int id) = 0;

	// Get the current joint position and orientation of one user
	virtual void getFingerTrackingData(int handID, Fubi::SkeletonHandJoint::Joint joint, Fubi::SkeletonJointPosition& position, Fubi::SkeletonJointOrientation& orientation) = 0;

	// Gets the number of shown fingers or -1 on error
	virtual int getFingerCount(int id) = 0;

	// Get the hand type (left or right hand or NUM_JOINTS if unknown)
	virtual Fubi::SkeletonJoint::Joint getHandType(int handID) = 0;

	// init function
	virtual bool init(Fubi::Vec3f offsetPos) = 0;

	// access to image data
	virtual const unsigned char* getImageData(unsigned int index) { return 0x0; }

	// image data resolution
	virtual  void getImageConfig(int& width, int& height, unsigned int& numStreams) { width = -1; height = -1; numStreams = 0; }

	// get/set the offset position of the sensor in relation to the main sensor
	const Fubi::Vec3f& getOffsetPosition() const { return m_offsetPos; }
	void setOffsetPosition(const Fubi::Vec3f& offsetPos) { m_offsetPos = offsetPos; }

	// The sensors type
	Fubi::FingerSensorType::Type getType() { return m_type; }

protected:
	Fubi::FingerSensorType::Type m_type;
	Fubi::Vec3f m_offsetPos;

};
