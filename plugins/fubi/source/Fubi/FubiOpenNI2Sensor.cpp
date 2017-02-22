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

#include "FubiOpenNI2Sensor.h"

#ifdef FUBI_USE_OPENNI2

#pragma comment(lib, "openNI2.lib")
#pragma comment(lib, "NiTE2.lib")


#include "FubiUser.h"
#include "Fubi.h"


using namespace Fubi;
using namespace std;

// Static helper functions
static inline Vec3f niteJointToVec3f(const nite::SkeletonJoint& joint)
{
	return Vec3f(joint.getPosition().x, joint.getPosition().y, joint.getPosition().z);
}

static nite::JointType JointToNiteJoint(const SkeletonJoint::Joint j)
{
	switch (j)
	{
	case SkeletonJoint::HEAD:
		return nite::JOINT_HEAD;
	case SkeletonJoint::NECK:
		return nite::JOINT_NECK;
	case SkeletonJoint::TORSO:
		return nite::JOINT_TORSO;
	case SkeletonJoint::LEFT_SHOULDER:
		return nite::JOINT_LEFT_SHOULDER;
	case SkeletonJoint::LEFT_ELBOW:
		return nite::JOINT_LEFT_ELBOW;
	case SkeletonJoint::LEFT_WRIST:
		return nite::JOINT_LEFT_HAND;
	case SkeletonJoint::LEFT_HAND:
		return nite::JOINT_LEFT_HAND;
	case SkeletonJoint::RIGHT_SHOULDER:
		return nite::JOINT_RIGHT_SHOULDER;
	case SkeletonJoint::RIGHT_ELBOW:
		return nite::JOINT_RIGHT_ELBOW;
	case SkeletonJoint::RIGHT_WRIST:
		return nite::JOINT_RIGHT_HAND;
	case SkeletonJoint::RIGHT_HAND:
		return nite::JOINT_RIGHT_HAND;
	case SkeletonJoint::LEFT_HIP:
		return nite::JOINT_LEFT_HIP;
	case SkeletonJoint::LEFT_KNEE:
		return nite::JOINT_LEFT_KNEE;
	case SkeletonJoint::LEFT_ANKLE:
		return nite::JOINT_LEFT_FOOT;
	case SkeletonJoint::LEFT_FOOT:
		return nite::JOINT_LEFT_FOOT;
	case SkeletonJoint::RIGHT_HIP:
		return nite::JOINT_RIGHT_HIP;
	case SkeletonJoint::RIGHT_KNEE:
		return nite::JOINT_RIGHT_KNEE;
	case SkeletonJoint::RIGHT_ANKLE:
		return nite::JOINT_RIGHT_FOOT;
	case SkeletonJoint::RIGHT_FOOT:
		return nite::JOINT_RIGHT_FOOT;
	}
	return nite::JOINT_HEAD;
}

FubiOpenNI2Sensor::FubiOpenNI2Sensor() : m_hasNewTrackingData(false)
{
	m_options.m_type = SensorType::OPENNI2;
	memset(m_skeletonStates, nite::SKELETON_NONE, Fubi::MaxUsers*sizeof(nite::SkeletonState));
}

bool FubiOpenNI2Sensor::initWithOptions(const Fubi::SensorOptions& options)
{
	// Init sensor streams and user tracking
	openni::Status rc = openni::OpenNI::initialize();
	if (rc != openni::STATUS_OK)
	{
		Fubi_logErr("InitWithOptions failed to initialize OpenNI\n %s\n", openni::OpenNI::getExtendedError());
		return false;
	}

	const char* deviceUri = openni::ANY_DEVICE;
	rc = m_device.open(deviceUri);
	if (rc != openni::STATUS_OK)
	{
		Fubi_logErr("InitWithOptions Failed to open device\n %s\n", openni::OpenNI::getExtendedError());
		return false;
	}

	if (options.m_depthOptions.isValid())
	{
		rc = m_depth.create(m_device, openni::SENSOR_DEPTH);
		if (rc != openni::STATUS_OK)
		{
			Fubi_logErr("InitWithOptions Unable to open depth stream:\n %s\n", openni::OpenNI::getExtendedError());
			return false;
		}
		else
		{
			// Set video mode
			openni::VideoMode mode;
			mode.setFps(options.m_depthOptions.m_fps);
			mode.setResolution(options.m_depthOptions.m_width, options.m_depthOptions.m_height);
			mode.setPixelFormat(openni::PIXEL_FORMAT_DEPTH_1_MM);
			m_depth.setVideoMode(mode);
			m_depth.setMirroringEnabled(options.m_mirrorStreams);
			rc = m_depth.start();
			if (rc != openni::STATUS_OK)
			{
				Fubi_logErr("InitWithOptions Unable to start depth stream:\n %s\n", openni::OpenNI::getExtendedError());
				m_depth.destroy();
				return false;
			}
		}
	}

	if (options.m_rgbOptions.isValid())
	{
		rc = m_color.create(m_device, openni::SENSOR_COLOR);
		if (rc == openni::STATUS_OK)
		{
			openni::VideoMode mode;
			mode.setFps(options.m_rgbOptions.m_fps);
			mode.setResolution(options.m_rgbOptions.m_width, options.m_rgbOptions.m_height);
			mode.setPixelFormat(openni::PIXEL_FORMAT_RGB888);
			m_color.setVideoMode(mode);
			m_color.setMirroringEnabled(options.m_mirrorStreams);
			rc = m_color.start();
			if (rc != openni::STATUS_OK)
			{
				Fubi_logErr("InitWithOptions Couldn't start color stream:\n %s\n", openni::OpenNI::getExtendedError());
				m_color.destroy();
			}
			else if (options.m_registerStreams)
			{
				m_options.m_registerStreams = true;
				m_device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
			}
		}
		else
		{
			Fubi_logErr("InitWithOptions Couldn't find color stream:\n %s\n", openni::OpenNI::getExtendedError());
		}
	}

	if (options.m_irOptions.isValid())
	{
		rc = m_ir.create(m_device, openni::SENSOR_IR);
		if (rc == openni::STATUS_OK)
		{
			openni::VideoMode mode;
			mode.setFps(options.m_irOptions.m_fps);
			mode.setResolution(options.m_irOptions.m_width, options.m_irOptions.m_height);
			mode.setPixelFormat(openni::PIXEL_FORMAT_GRAY16);
			m_ir.setVideoMode(mode);
			m_ir.setMirroringEnabled(options.m_mirrorStreams);
			rc = m_ir.start();
			if (rc != openni::STATUS_OK)
			{
				Fubi_logWrn("InitWithOptions Couldn't start ir stream:\n %s\n", openni::OpenNI::getExtendedError());
				m_ir.destroy();
			}
		}
		else
		{
			Fubi_logWrn("InitWithOptions Couldn't find ir stream:\n %s\n", openni::OpenNI::getExtendedError());
		}
	}

	if (options.m_trackingProfile != SkeletonTrackingProfile::NONE)
	{
		nite::NiTE::initialize();

		if (m_userTracker.create(&m_device) != nite::STATUS_OK)
		{
			return false;
		}
		m_options.m_trackingProfile = SkeletonTrackingProfile::ALL;  // Currently only all or none supported
	}
	else
		m_options.m_trackingProfile = SkeletonTrackingProfile::NONE;

	// Disable global smoothing
	m_userTracker.setSkeletonSmoothingFactor(0);

	// Update the stream options
	updateOptions();
	// And the rest of the options (basically copied from the passed options, but tracking profile already has been set)
	m_options.m_mirrorStreams = options.m_mirrorStreams; // Currently only a global mirror

	Fubi_logInfo("FubiOpenNI2Sensor: succesfully initialized with options!\n");
	return true;
}


FubiOpenNI2Sensor::~FubiOpenNI2Sensor()
{
	// We have to call most of the relase/destroy functions manually to achieve the correct order
	// Shutdown NiTE
	m_currentTrackerFrame.release();
	m_userTracker.destroy();
	nite::NiTE::shutdown();

	// And OpenNI
	m_color.stop();
	m_ir.stop();
	m_depth.stop();
	m_depth.destroy();
	m_color.destroy();
	m_ir.destroy();
	m_device.close();
	openni::OpenNI::shutdown();
}

void FubiOpenNI2Sensor::update()
{
	// Get new stream data
	bool gotNewDepthFrame = false;
	int changedIndex;
	openni::VideoStream* stream;
	if (m_depth.isValid())
	{
		stream = &m_depth;
		openni::Status rc = openni::OpenNI::waitForAnyStream(&stream, 1, &changedIndex, openni::TIMEOUT_NONE);
		if (rc == openni::STATUS_OK && changedIndex == 0)
		{
			// New stream data available
			m_depth.readFrame(&m_depthFrame);
			gotNewDepthFrame = true;
		}
	}
	if (m_color.isValid())
	{
		stream = &m_color;
		openni::Status rc = openni::OpenNI::waitForAnyStream(&stream, 1, &changedIndex, openni::TIMEOUT_NONE);
		if (rc == openni::STATUS_OK && changedIndex == 0)
		{
			// New stream data available
			m_color.readFrame(&m_colorFrame);
		}
	}
	if (m_ir.isValid())
	{
		stream = &m_ir;
		openni::Status rc = openni::OpenNI::waitForAnyStream(&stream, 1, &changedIndex, openni::TIMEOUT_NONE);
		if (rc == openni::STATUS_OK && changedIndex == 0)
		{
			// New stream data available
			m_ir.readFrame(&m_irFrame);
		}
	}

	
	// And tracking
	m_hasNewTrackingData = false;
	if (gotNewDepthFrame && m_userTracker.isValid())
	{
		int oldIndex = m_currentTrackerFrame.isValid() ? m_currentTrackerFrame.getFrameIndex() : -1;
		nite::Status rc = m_userTracker.readFrame(&m_currentTrackerFrame);
		if (rc != nite::STATUS_OK)
		{
			Fubi_logWrn("UserTracker readFrame failed\n");
			return;
		}

		if (m_currentTrackerFrame.isValid() && oldIndex != m_currentTrackerFrame.getFrameIndex())
		{
			m_hasNewTrackingData = true;

			const nite::Array<nite::UserData>& users = m_currentTrackerFrame.getUsers();
			for (int i = 0; i < users.getSize(); ++i)
			{
				const nite::UserData& user = users[i];
				unsigned int id = user.getId();

				if (user.isNew())
				{
					// New user found
					Fubi_logInfo("FubiOpenNI2Sensor: New User %d\n", id);
					m_userTracker.startSkeletonTracking(id);
				}

				FubiUser* fUser = Fubi::getUser(id);
				if (fUser)
				{
					bool nowVisible = user.isVisible();
					if (nowVisible && !fUser->inScene())
						Fubi_logInfo("FubiOpenNI2Sensor: User reentered %d\n", id);
					else if (!nowVisible && fUser->inScene())
						Fubi_logInfo("FubiOpenNI2Sensor: User exit %d\n", id);
					fUser->m_inScene = nowVisible;

					if (user.isLost())
					{
						Fubi_logInfo("FubiOpenNI2Sensor: Lost user %d\n", id);
					}
					else if (user.isNew())
					{
						m_userTracker.startSkeletonTracking(user.getId());
					}
					else
					{
						if(m_skeletonStates[id] != user.getSkeleton().getState())
						{
							switch(m_skeletonStates[id] = user.getSkeleton().getState())
							{
							case nite::SKELETON_NONE:
								Fubi_logInfo("FubiOpenNI2Sensor: Stopped tracking user %d\n", id);
								break;
							case nite::SKELETON_CALIBRATING:
								Fubi_logInfo("FubiOpenNI2Sensor: Calibrating user %d\n", id);
								break;
							case nite::SKELETON_TRACKED:
								Fubi_logInfo("FubiOpenNI2Sensor: Tracking user %d\n", id);
								break;
							case nite::SKELETON_CALIBRATION_ERROR_NOT_IN_POSE:
							case nite::SKELETON_CALIBRATION_ERROR_HANDS:
							case nite::SKELETON_CALIBRATION_ERROR_LEGS:
							case nite::SKELETON_CALIBRATION_ERROR_HEAD:
							case nite::SKELETON_CALIBRATION_ERROR_TORSO:
								Fubi_logInfo("FubiOpenNI2Sensor: Calibration Failed for user %d\n", id);
								break;
							}
						}				
					}
				}
			}
		}
	}
}

Plane FubiOpenNI2Sensor::getFloor()
{
	if (m_currentTrackerFrame.isValid())
	{
		const nite::Plane& plane = m_currentTrackerFrame.getFloor();
		Vec3f normal(plane.normal.x, plane.normal.y, plane.normal.z);
		Vec3f point(plane.point.x, plane.point.y, plane.point.z);
		float lambda = normal.dot(point);
		return Plane(normal.x, normal.y, normal.z, lambda);
	}
	return Plane();
}


bool FubiOpenNI2Sensor::hasNewTrackingData()
{
	return m_hasNewTrackingData;
}

bool FubiOpenNI2Sensor::isTracking(unsigned int id)
{
	if (m_currentTrackerFrame.isValid())
	{
		const nite::UserData* user = m_currentTrackerFrame.getUserById(id);
		if (user)
			return (user->getSkeleton().getState() == nite::SKELETON_TRACKED);
	}
	return false;
}

void FubiOpenNI2Sensor::getSkeletonJointData(unsigned int id, Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJointPosition& position, Fubi::SkeletonJointOrientation& orientation)
{
	if (m_currentTrackerFrame.isValid())
	{
		const nite::UserData* userData = m_currentTrackerFrame.getUserById(id);
		if (userData)
		{
			FubiUser* user = Fubi::getUser(id);
			if (user)
			{
				if (user->isTracked())
				{
					// Standard case user is tracked
					// Get the current tracking data
					
					if (joint == SkeletonJoint::WAIST) // special case currently not supported by OpenNI but approximated as center between hips
					{
						// Position is middle between the hips
						const nite::SkeletonJoint& jointData1 = userData->getSkeleton().getJoint(nite::JOINT_LEFT_HIP);
						const nite::SkeletonJoint& jointData2 = userData->getSkeleton().getJoint(nite::JOINT_RIGHT_HIP);
						position.m_confidence = minf(jointData1.getPositionConfidence(), jointData2.getPositionConfidence());
						position.m_position = niteJointToVec3f(jointData2) + (niteJointToVec3f(jointData1) - niteJointToVec3f(jointData2))*0.5f;

						// Rotation is the same as torso rotation
						const nite::SkeletonJoint& jointData3 = userData->getSkeleton().getJoint(nite::JOINT_TORSO);
						const nite::Quaternion& rotQuat = jointData3.getOrientation();
						orientation = SkeletonJointOrientation(Quaternion(rotQuat.x, rotQuat.y, rotQuat.z, rotQuat.w), jointData3.getOrientationConfidence());
					}
					else if (joint >= SkeletonJoint::FACE_NOSE && joint <= SkeletonJoint::FACE_CHIN) // Another special case: face coordinates
					{
						// approximated out of the head position
						const nite::SkeletonJoint& headData = userData->getSkeleton().getJoint(nite::JOINT_HEAD);
						// but relative to the torso transformation
						const nite::SkeletonJoint& torsoData = userData->getSkeleton().getJoint(nite::JOINT_TORSO);
						const nite::Quaternion& torsoQuat = torsoData.getOrientation();
						Matrix3f torsoOrient(Quaternion(torsoQuat.x, torsoQuat.y, torsoQuat.z, torsoQuat.w));
						Vec3f torsoPos = niteJointToVec3f(torsoData);
						Vec3f localPos(Math::NO_INIT);
						Fubi::calculateLocalPosition(niteJointToVec3f(headData), torsoPos, torsoOrient, localPos);
						// translate to face joint from local head position
						switch (joint)
						{
						case SkeletonJoint::FACE_NOSE:
							localPos.y -= 25.0f;
							localPos.z -= 80.0f;
							break;
						case SkeletonJoint::FACE_FOREHEAD:
							localPos.y += 100.0f;
							localPos.z -= 40.0f;
							break;
						case SkeletonJoint::FACE_CHIN:
							localPos.y -= 120.0f;
							localPos.z -= 60.0f;
							break;
						case SkeletonJoint::FACE_LEFT_EAR:
							localPos.x -= 80;
							localPos.y -= 20.0f;
							break;
						case SkeletonJoint::FACE_RIGHT_EAR:
							localPos.x += 80;
							localPos.y -= 20.0f;
							break;
						}
						// Transform back to global positions
						Fubi::calculateGlobalPosition(localPos, torsoPos, torsoOrient, position.m_position);
						position.m_confidence = minf(torsoData.getPositionConfidence(), minf(torsoData.getOrientationConfidence(), headData.getPositionConfidence()));;
						// Orientation is taken from the head
						const nite::Quaternion& rotQuat = headData.getOrientation();
						orientation.m_orientation = Matrix3f(Quaternion(rotQuat.x, rotQuat.y, rotQuat.z, rotQuat.w));
						orientation.m_confidence = headData.getOrientationConfidence();
					}
					else
					{
						nite::JointType njoint = JointToNiteJoint(joint);
						// Standard case, just get the data
						const nite::SkeletonJoint& jointData = userData->getSkeleton().getJoint(njoint);
						const nite::Quaternion& rotQuat = jointData.getOrientation();
						position.m_position = niteJointToVec3f(jointData);
						position.m_confidence = jointData.getPositionConfidence();
						orientation.m_orientation = Matrix3f(Quaternion(rotQuat.x, rotQuat.y, rotQuat.z, rotQuat.w));
						orientation.m_confidence = jointData.getOrientationConfidence();
					}
				}
				else
				{
					// Not tracked return the center of mass instead
					// that should be independent of tracking state
					const nite::Point3f& point = userData->getCenterOfMass();
					position.m_confidence = 0.5f; // leave confidence at 0.5 as this is not really the wanted position
					position.m_position.x = point.x;
					position.m_position.y = point.y;
					position.m_position.z = point.z;
				}
			}
		}
	}
}

const unsigned short* FubiOpenNI2Sensor::getDepthData()
{
	if (m_depthFrame.isValid())
	{
		return (const unsigned short*) m_depthFrame.getData();
	}		
	return 0x0;
}

const unsigned char* FubiOpenNI2Sensor::getRgbData()
{
	if (m_colorFrame.isValid())
	{
		return (const unsigned char*) m_colorFrame.getData();
	}
	return 0x0;
}

const unsigned short* FubiOpenNI2Sensor::getIrData()
{
	if (m_irFrame.isValid())
	{
		return (const unsigned short*) m_irFrame.getData();
	}
	return 0x0;
}

const unsigned short* FubiOpenNI2Sensor::getUserLabelData(Fubi::CoordinateType::Type coordType)
{
	if (m_currentTrackerFrame.isValid() && m_currentTrackerFrame.getUsers().getSize() > 0)
	{
		const nite::UserMap& userLabels = m_currentTrackerFrame.getUserMap();
		// TODO: adapt for coordinate type
		return (const unsigned short*)userLabels.getPixels();
	}
	return 0x0;
}

unsigned short FubiOpenNI2Sensor::getUserIDs(unsigned int* userIDs)
{
	int numUsers = 0;
	if (m_currentTrackerFrame.isValid())
	{
		const nite::Array<nite::UserData>& users = m_currentTrackerFrame.getUsers();
		numUsers = users.getSize();
		if (userIDs)
		{
			for (int i = 0; i < numUsers; ++i)
			{
				userIDs[i] = users[i].getId();
			}
		}
	}
	return numUsers;
}

Fubi::Vec3f FubiOpenNI2Sensor::convertCoordinates(const Fubi::Vec3f& inputCoords, Fubi::CoordinateType::Type inputType, Fubi::CoordinateType::Type outputType)
{
		Vec3f ret(Math::NO_INIT);

		if (inputType == CoordinateType::REAL_WORLD && outputType != CoordinateType::REAL_WORLD && m_depth.isValid())
		{
			openni::CoordinateConverter::convertWorldToDepth(m_depth, inputCoords.x, inputCoords.y, inputCoords.z, &ret.x, &ret.y, &ret.z);
			if (outputType != CoordinateType::DEPTH)
				ret = convertCoordinates(ret, CoordinateType::DEPTH, outputType);
		}
		else if (outputType == CoordinateType::REAL_WORLD && inputType == CoordinateType::DEPTH && m_depth.isValid())
			openni::CoordinateConverter::convertDepthToWorld(m_depth, inputCoords.x, inputCoords.y, inputCoords.z, &ret.x, &ret.y, &ret.z);
		else if (inputType == CoordinateType::DEPTH && outputType == CoordinateType::COLOR && m_depth.isValid() && m_color.isValid())
		{
			int outX, outY;
			openni::CoordinateConverter::convertDepthToColor(m_depth, m_color, ftoi_r(inputCoords.x), ftoi_r(inputCoords.y), (openni::DepthPixel) inputCoords.z, &outX, &outY);
			ret.x = (float) outX;
			ret.y = (float) outY;
			ret.z = inputCoords.z;
		}
		else if (inputType == CoordinateType::DEPTH && outputType == CoordinateType::IR && m_depth.isValid() && m_ir.isValid())
		{
			int outX, outY;
			openni::CoordinateConverter::convertDepthToColor(m_depth, m_ir, ftoi_r(inputCoords.x), ftoi_r(inputCoords.y), (openni::DepthPixel) inputCoords.z, &outX, &outY);
			ret.x = (float)outX;
			ret.y = (float)outY;
			ret.z = inputCoords.z;
		}
		else if (inputType == CoordinateType::IR && outputType != CoordinateType::DEPTH)
		{
			Vec3f depthCoords = convertCoordinates(inputCoords, CoordinateType::IR, CoordinateType::DEPTH);
			ret = convertCoordinates(depthCoords, CoordinateType::DEPTH, outputType);
		}
		else if (outputType == CoordinateType::IR && inputType != CoordinateType::DEPTH)
		{
			Vec3f depthCoords = convertCoordinates(inputCoords, inputType, CoordinateType::DEPTH);
			ret = convertCoordinates(depthCoords, CoordinateType::DEPTH, CoordinateType::IR);
		}
		else
			// Conversion IR<>Depth will be handled in base class, dummy conversion for all conversions from the color stream...
			ret = FubiISensor::convertCoordinates(inputCoords, inputType, outputType);

		return ret;
}

void FubiOpenNI2Sensor::resetTracking(unsigned int id)
{
	if (m_userTracker.isValid())
	{
		m_userTracker.stopSkeletonTracking(id);
		m_userTracker.startSkeletonTracking(id);
	}
}

void FubiOpenNI2Sensor::updateOptions()
{
	// First stream options
	if (m_depth.isValid())
	{
		m_options.m_depthOptions.m_width = m_depth.getVideoMode().getResolutionX();
		m_options.m_depthOptions.m_height = m_depth.getVideoMode().getResolutionY();
		m_options.m_depthOptions.m_fps = m_depth.getVideoMode().getFps();
	}
	else
		m_options.m_depthOptions.invalidate();
	if (m_color.isValid())
	{
		m_options.m_rgbOptions.m_width = m_color.getVideoMode().getResolutionX();
		m_options.m_rgbOptions.m_height = m_color.getVideoMode().getResolutionY();
		m_options.m_rgbOptions.m_fps = m_color.getVideoMode().getFps();
	}
	else
		m_options.m_rgbOptions.invalidate();
	if (m_ir.isValid())
	{
		m_options.m_irOptions.m_width = m_ir.getVideoMode().getResolutionX();
		m_options.m_irOptions.m_height = m_ir.getVideoMode().getResolutionY();
		m_options.m_irOptions.m_fps = m_ir.getVideoMode().getFps();
	}
	else
		m_options.m_irOptions.invalidate();
}

#endif