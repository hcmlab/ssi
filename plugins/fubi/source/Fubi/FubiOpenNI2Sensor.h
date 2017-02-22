// ****************************************************************************************
//
// Fubi OpenNI sensor
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler 
// 
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
// 
// ****************************************************************************************
#pragma once

#include "FubiConfig.h"

#ifdef FUBI_USE_OPENNI2
#include "FubiISensor.h"

#include "NiTE.h"

// The FubiOpenNISensor class is responsible for all OpenNI stuff
class FubiOpenNI2Sensor : public FubiISensor
{
public:
	FubiOpenNI2Sensor();
	virtual ~FubiOpenNI2Sensor();


	// Init with options for streams and tracking
	virtual bool initWithOptions(const Fubi::SensorOptions& options);

	// Update should be called once per frame for the sensor to update its streams and tracking data
	virtual void update();

	// Get the ids of all currently valid users: Ids will be stored in userIDs (if not 0x0), returns the number of valid users
	virtual unsigned short getUserIDs(unsigned int* userIDs);

	// Check if the sensor has new tracking data available
	virtual bool hasNewTrackingData();

	// Check if that user with the given id is tracked by the sensor
	virtual bool isTracking(unsigned int id);

	// Get the current joint position and orientation of one user
	virtual void getSkeletonJointData(unsigned int id, Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJointPosition& position, Fubi::SkeletonJointOrientation& orientation);

	// Get Stream data
	virtual const unsigned short* getDepthData();
	virtual const unsigned char* getRgbData();
	virtual const unsigned short* getIrData();
	virtual const unsigned short* getUserLabelData(Fubi::CoordinateType::Type coordType);

	// Get the floor plane
	virtual Fubi::Plane getFloor();

	// Resets the tracking of a users
	virtual void resetTracking(unsigned int id);
	
	// Convert coordinates between real world, depth, color, or IR image
	// inputType should be != outputType
	virtual Fubi::Vec3f convertCoordinates(const Fubi::Vec3f& inputCoords, Fubi::CoordinateType::Type inputType, Fubi::CoordinateType::Type outputType);

private:
	// set the options according to the current OpenNI config
	void updateOptions();


	// OpenNi and NiTE handles
	openni::Device		m_device;
	nite::UserTracker	m_userTracker;

	// Current frames
	nite::UserTrackerFrameRef	m_currentTrackerFrame;
	openni::VideoFrameRef		m_depthFrame;
	openni::VideoFrameRef		m_colorFrame;
	openni::VideoFrameRef		m_irFrame;

	// Current user tracking status
	nite::SkeletonState m_skeletonStates[Fubi::MaxUsers];

	// The openni streams
	openni::VideoStream			m_depth, m_color, m_ir;

	// Whether last update was able to get a new tracking data frame
	bool m_hasNewTrackingData;

};

#endif