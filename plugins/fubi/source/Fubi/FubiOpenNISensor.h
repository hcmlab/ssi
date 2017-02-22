// ****************************************************************************************
//
// Fubi OpenNI V1.x sensor
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
//
// ****************************************************************************************
#pragma once

#include "FubiConfig.h"

#ifdef FUBI_USE_OPENNI1

#include "FubiISensor.h"

#include <XnCppWrapper.h>

// The FubiOpenNISensor class is responsible for all OpenNI stuff
class FubiOpenNISensor : public FubiISensor
{
public:
	FubiOpenNISensor();
	virtual ~FubiOpenNISensor();

	// Init with a sensor specific xml file for options
	virtual bool initFromXml(const char* xmlPath, Fubi::SkeletonTrackingProfile::Profile trackingProfile = Fubi::SkeletonTrackingProfile::ALL,
		bool mirrorStreams = true, bool registerStreams =true);

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
	// Checks for new users and starts the tracking/calibration process
	void applyUserEvents();

	// Create a mock depth generator with given options, returns true if succesfull
	bool createMockDepth(const Fubi::StreamOptions& options);

	// set the options according to the current OpenNI config
	void updateOptions();

	// OpenNi Callback methods
	// Callback: New user was detected
	static void XN_CALLBACK_TYPE CbNewUser(xn::UserGenerator& generator, unsigned int nId, void* pCookie);
	// Callback: An existing user was lost
	static void XN_CALLBACK_TYPE CbLostUser(xn::UserGenerator& generator, unsigned int nId, void* pCookie);
	// Callback: Exit user
	static void XN_CALLBACK_TYPE CbExitUser(xn::UserGenerator& generator, unsigned int nId, void* pCookie);
	// Callback: Reenter user
	static void XN_CALLBACK_TYPE CbReenterUser(xn::UserGenerator& generator, unsigned int nId, void* pCookie);
	// Callback: Detected the calibration pose
	static void XN_CALLBACK_TYPE CbPoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, unsigned int nId, void* pCookie);
	// Callback: No in calibration pose any more
	static void XN_CALLBACK_TYPE CbOutOfPose(xn::PoseDetectionCapability& capability, const XnChar* strPose, unsigned int nId, void* pCookie);
	// Callback: Started calibration
	static void XN_CALLBACK_TYPE CbCalibrationStart(xn::SkeletonCapability& capability, unsigned int nId, void* pCookie);
	// Callback: Finished calibration
	static void XN_CALLBACK_TYPE CbCalibrationComplete(xn::SkeletonCapability& capability, unsigned int user, XnCalibrationStatus calibrationStatus, void* pCookie);

	// New users
	std::vector<unsigned int> m_newUsers;
	std::vector<unsigned int> m_exitUsers;
	std::vector<unsigned int> m_reenteredUsers;
	bool m_applyingUserEvents;

	// Calibration pose name as requested by the openni user generator
	char m_strPose[20];

	// OpenNI context and generators
	xn::Context			m_Context;
	xn::DepthGenerator	m_DepthGenerator;
	xn::ImageGenerator	m_ImageGenerator;
	xn::IRGenerator		m_IRGenerator;
	xn::UserGenerator	m_UserGenerator;
	xn::SceneAnalyzer	m_SceneAnalyzer;

	// Callback handles for tracking events
	XnCallbackHandle m_hPoseDetected, m_hOutOfPose;
	XnCallbackHandle m_hCalibrationStart, m_hCalibrationComplete, m_hExitUser, m_hReenterUser;
	XnCallbackHandle m_hUserCallbacks;
};
#endif
