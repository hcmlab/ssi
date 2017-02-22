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

#include "FubiOpenNISensor.h"

#ifdef FUBI_USE_OPENNI1

#ifdef WIN64
#pragma comment(lib, "openNI64.lib")
#else
#pragma comment(lib, "openNI.lib")
#endif

#include <XnPropNames.h>

#include "FubiUser.h"
#include "Fubi.h"


#if !defined(WIN32) && !defined(_WINDOWS) && !defined(_WIN32)
#   define Sleep sleep
#endif

#define CHECK_RC(nRetVal, what)										\
	if (nRetVal != XN_STATUS_OK)									\
{																\
	Fubi_logErr("%s failed: %s\n", what, xnGetStatusString(nRetVal));\
	return false;												\
}

using namespace Fubi;
using namespace std;

// Static helper functions
static inline Vec3f xnJointToVec3f(const XnSkeletonJointPosition& jointPos)
{
	return Vec3f(jointPos.position.X, jointPos.position.Y, jointPos.position.Z);
}

static XnSkeletonJoint JointToXNSkeletonJoint(const SkeletonJoint::Joint j)
{
	switch (j)
	{
	case SkeletonJoint::HEAD:
		return XN_SKEL_HEAD;
	case SkeletonJoint::NECK:
		return XN_SKEL_NECK;
	case SkeletonJoint::TORSO:
		return XN_SKEL_TORSO;
	case SkeletonJoint::WAIST:
		return XN_SKEL_WAIST;
	case SkeletonJoint::LEFT_SHOULDER:
		return XN_SKEL_LEFT_SHOULDER;
	case SkeletonJoint::LEFT_ELBOW:
		return XN_SKEL_LEFT_ELBOW;
	case SkeletonJoint::LEFT_WRIST:
		return XN_SKEL_LEFT_HAND;
	case SkeletonJoint::LEFT_HAND:
		return XN_SKEL_LEFT_HAND;
	case SkeletonJoint::RIGHT_SHOULDER:
		return XN_SKEL_RIGHT_SHOULDER;
	case SkeletonJoint::RIGHT_ELBOW:
		return XN_SKEL_RIGHT_ELBOW;
	case SkeletonJoint::RIGHT_WRIST:
		return XN_SKEL_RIGHT_HAND;
	case SkeletonJoint::RIGHT_HAND:
		return XN_SKEL_RIGHT_HAND;
	case SkeletonJoint::LEFT_HIP:
		return XN_SKEL_LEFT_HIP;
	case SkeletonJoint::LEFT_KNEE:
		return XN_SKEL_LEFT_KNEE;
	case SkeletonJoint::LEFT_ANKLE:
		return XN_SKEL_LEFT_FOOT;
	case SkeletonJoint::LEFT_FOOT:
		return XN_SKEL_LEFT_FOOT;
	case SkeletonJoint::RIGHT_HIP:
		return XN_SKEL_RIGHT_HIP;
	case SkeletonJoint::RIGHT_KNEE:
		return XN_SKEL_RIGHT_KNEE;
	case SkeletonJoint::RIGHT_ANKLE:
		return XN_SKEL_RIGHT_FOOT;
	case SkeletonJoint::RIGHT_FOOT:
		return XN_SKEL_RIGHT_FOOT;
	}
	return XN_SKEL_HEAD;
}

FubiOpenNISensor::FubiOpenNISensor()
	:  m_applyingUserEvents(false),	m_hUserCallbacks(0x0),	m_hPoseDetected(0x0), m_hOutOfPose(0x0),
	m_hCalibrationStart(0x0), m_hCalibrationComplete(0x0), m_hExitUser(0x0), m_hReenterUser(0x0)
{
	m_strPose[0] = '\0';
	m_options.m_type = SensorType::OPENNI1;
}

bool FubiOpenNISensor::initWithOptions(const Fubi::SensorOptions& options)
{
	XnStatus nRetVal = XN_STATUS_OK;
	xn::EnumerationErrors errors;
	XnMapOutputMode xnOutputMode;

	nRetVal = m_Context.Init();
	CHECK_RC(nRetVal, "FubiOpenNISensor:InitWithOptions");

	nRetVal = m_Context.SetGlobalMirror(m_options.m_mirrorStreams);
	CHECK_RC(nRetVal, "FubiOpenNISensor:InitWithOptions");

	m_options.m_registerStreams = false;

	if (options.m_depthOptions.isValid())
	{
		nRetVal = m_Context.CreateAnyProductionTree(XN_NODE_TYPE_DEPTH, 0x0, m_DepthGenerator, &errors);
		if (nRetVal != XN_STATUS_OK)
		{
			// Fallback: using only a mock generator with some default values (this won't help much in a real app)
			Fubi_logWrn("InitWithOptions Unable to create depth generator. Using a mock generator!!\n");
			m_options.m_depthOptions.m_fps = 30;
			m_options.m_depthOptions.m_width = 640;
			m_options.m_depthOptions.m_height = 480;
			if (!createMockDepth(m_options.m_depthOptions))
			{
				Fubi_logErr("InitWithOptions Error creating mock depth generator!\n");
				return false;
			}
		}
		xnOutputMode.nFPS = (unsigned) options.m_depthOptions.m_fps;
		xnOutputMode.nXRes = (unsigned) options.m_depthOptions.m_width;
		xnOutputMode.nYRes = (unsigned) options.m_depthOptions.m_height;
		nRetVal = m_DepthGenerator.SetMapOutputMode(xnOutputMode);
		CHECK_RC(nRetVal, "Set Mode");
	}


	nRetVal = m_Context.CreateAnyProductionTree(XN_NODE_TYPE_USER, 0x0, m_UserGenerator, &errors);
	if (nRetVal != XN_STATUS_OK)
	{
		nRetVal = m_UserGenerator.Create(m_Context, 0x0, &errors);
		CHECK_RC(nRetVal, "Find user generator");
	}

	if (options.m_rgbOptions.isValid())
	{
		nRetVal = m_Context.CreateAnyProductionTree(XN_NODE_TYPE_IMAGE, 0x0, m_ImageGenerator, &errors);
		if (nRetVal == XN_STATUS_OK)
		{
			Fubi_logInfo("FubiOpenNISensor: Found image generator\n");
			if (options.m_registerStreams)
			{
				m_DepthGenerator.GetAlternativeViewPointCap().SetViewPoint(m_ImageGenerator);
				m_options.m_registerStreams = true;
			}

			xnOutputMode.nFPS = (unsigned) options.m_rgbOptions.m_fps;
			xnOutputMode.nXRes = (unsigned) options.m_rgbOptions.m_width;
			xnOutputMode.nYRes = (unsigned) options.m_rgbOptions.m_height;
			nRetVal = m_ImageGenerator.SetMapOutputMode(xnOutputMode);
			CHECK_RC(nRetVal, "Set Mode");
		}
	}

	if ((!options.m_rgbOptions.isValid() || !m_ImageGenerator.IsValid()) && options.m_irOptions.isValid())
	{
		nRetVal = m_Context.CreateAnyProductionTree(XN_NODE_TYPE_IR, 0x0, m_IRGenerator, &errors);
		if (nRetVal == XN_STATUS_OK)
		{
			Fubi_logInfo("FubiOpenNISensor: Found ir generator\n");

			xnOutputMode.nFPS = (unsigned) options.m_irOptions.m_fps;
			xnOutputMode.nXRes = (unsigned) options.m_irOptions.m_width;
			xnOutputMode.nYRes = (unsigned) options.m_irOptions.m_height;
			nRetVal = m_IRGenerator.SetMapOutputMode(xnOutputMode);
			CHECK_RC(nRetVal, "Set Mode");
		}
	}

	m_Context.CreateAnyProductionTree(XN_NODE_TYPE_SCENE, 0x0, m_SceneAnalyzer, &errors);
	if (nRetVal != XN_STATUS_OK)
	{
		nRetVal = m_SceneAnalyzer.Create(m_Context, 0x0, &errors);
		CHECK_RC(nRetVal, "Find scene analyser");
	}

	if (!m_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
	{
		Fubi_logWrn("Supplied user generator doesn't support skeleton\n");
		return false;
	}
	m_UserGenerator.RegisterUserCallbacks(FubiOpenNISensor::CbNewUser, FubiOpenNISensor::CbLostUser, this, m_hUserCallbacks);
	m_UserGenerator.GetSkeletonCap().RegisterToCalibrationComplete(FubiOpenNISensor::CbCalibrationComplete, this, m_hCalibrationComplete);
	m_UserGenerator.GetSkeletonCap().RegisterToCalibrationStart(FubiOpenNISensor::CbCalibrationStart, this, m_hCalibrationStart);
	m_UserGenerator.RegisterToUserExit(FubiOpenNISensor::CbExitUser, this, m_hExitUser);
	m_UserGenerator.RegisterToUserReEnter(FubiOpenNISensor::CbReenterUser, this, m_hReenterUser);

	if (m_UserGenerator.GetSkeletonCap().NeedPoseForCalibration())
	{
		if (!m_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
		{
			Fubi_logWrn("Pose required, but not supported\n");
			return false;
		}
		m_UserGenerator.GetPoseDetectionCap().RegisterToPoseDetected(FubiOpenNISensor::CbPoseDetected, this, m_hPoseDetected);
		m_UserGenerator.GetPoseDetectionCap().RegisterToOutOfPose(FubiOpenNISensor::CbOutOfPose, this, m_hOutOfPose);
		m_UserGenerator.GetSkeletonCap().GetCalibrationPose(m_strPose);
	}

	m_UserGenerator.GetSkeletonCap().SetSkeletonProfile((XnSkeletonProfile) options.m_trackingProfile);

	// Default is no smoothing (because you better do it yourself adapted to the task you have)
	m_UserGenerator.GetSkeletonCap().SetSmoothing(0);

	nRetVal = m_Context.StartGeneratingAll();
	CHECK_RC(nRetVal, "StartGenerating");

	// Update the options
	updateOptions();
	// No getter for profile in OpenNI
	m_options.m_trackingProfile = options.m_trackingProfile;

	if (nRetVal != XN_STATUS_OK && errors.Begin() != errors.End())
	{
		XnChar strError[1024];
		errors.ToString(strError, 1024);
		Fubi_logErr("%s\n", strError);
	}

	if (nRetVal == XN_STATUS_OK)
		Fubi_logInfo("FubiOpenNISensor: succesfully initialized with options!\n");

	return nRetVal == XN_STATUS_OK;
}

bool FubiOpenNISensor::initFromXml(const char* xmlPath, Fubi::SkeletonTrackingProfile::Profile trackingProfile /*= ALL*/,
	bool mirrorStreams /*= true*/, bool registerStreams /*= true*/)
{
	XnStatus nRetVal = XN_STATUS_OK;

	m_options.m_registerStreams = false;

	xn::ScriptNode scriptNode;
	xn::EnumerationErrors errors;
	nRetVal = m_Context.InitFromXmlFile(xmlPath, scriptNode, &errors);
	if (nRetVal == XN_STATUS_NO_NODE_PRESENT)
	{
		XnChar strError[1024];
		errors.ToString(strError, 1024);
		Fubi_logErr("%s\n", strError);
		return false;
	}
	CHECK_RC(nRetVal, "FubiOpenNISensor:InitFromXml");

	nRetVal = m_Context.SetGlobalMirror(mirrorStreams);
	CHECK_RC(nRetVal, "FubiOpenNISensor:InitFromXml");

	nRetVal = m_Context.FindExistingNode(XN_NODE_TYPE_DEPTH, m_DepthGenerator);
	if (nRetVal != XN_STATUS_OK)
	{
		// Fallback: using only a mock generator with some default values (this won't help much in a real app)
		Fubi_logWrn("No depth generator found. Using a mock generator!!\n");
		m_options.m_depthOptions.m_fps = 30;
		m_options.m_depthOptions.m_width = 640;
		m_options.m_depthOptions.m_height = 480;
		if (!createMockDepth(m_options.m_depthOptions))
		{
			Fubi_logErr("InitFromXml Error creating mock depth generator!\n");
			return false;
		}
	}

	nRetVal = m_Context.FindExistingNode(XN_NODE_TYPE_USER, m_UserGenerator);
	if (nRetVal != XN_STATUS_OK)
	{
		nRetVal = m_UserGenerator.Create(m_Context);
		CHECK_RC(nRetVal, "Find user generator");
	}

	nRetVal = m_Context.FindExistingNode(XN_NODE_TYPE_IMAGE, m_ImageGenerator);
	if (nRetVal == XN_STATUS_OK)
	{
		Fubi_logInfo("FubiOpenNISensor: Found image generator\n");
		if (registerStreams)
		{
			m_DepthGenerator.GetAlternativeViewPointCap().SetViewPoint(m_ImageGenerator);
			m_options.m_registerStreams = true;
		}
	}
	else
	{
		nRetVal = m_Context.FindExistingNode(XN_NODE_TYPE_IR, m_IRGenerator);
		if (nRetVal == XN_STATUS_OK)
		{
			Fubi_logInfo("FubiOpenNISensor: Found ir generator\n");
		}
	}

	m_Context.FindExistingNode(XN_NODE_TYPE_SCENE, m_SceneAnalyzer);
	if (nRetVal != XN_STATUS_OK)
	{
		nRetVal = m_SceneAnalyzer.Create(m_Context);
		CHECK_RC(nRetVal, "Find scene analyser");
	}

	if (!m_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
	{
		Fubi_logErr("Supplied user generator doesn't support skeleton\n");
		return false;
	}
	m_UserGenerator.RegisterUserCallbacks(FubiOpenNISensor::CbNewUser, FubiOpenNISensor::CbLostUser, this, m_hUserCallbacks);
	m_UserGenerator.GetSkeletonCap().RegisterToCalibrationComplete(FubiOpenNISensor::CbCalibrationComplete, this, m_hCalibrationComplete);
	m_UserGenerator.GetSkeletonCap().RegisterToCalibrationStart(FubiOpenNISensor::CbCalibrationStart, this, m_hCalibrationStart);
	m_UserGenerator.RegisterToUserExit(FubiOpenNISensor::CbExitUser, this, m_hExitUser);
	m_UserGenerator.RegisterToUserReEnter(FubiOpenNISensor::CbReenterUser, this, m_hReenterUser);

	if (m_UserGenerator.GetSkeletonCap().NeedPoseForCalibration())
	{
		if (!m_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
		{
			Fubi_logErr("Pose required, but not supported\n");
			return false;
		}
		m_UserGenerator.GetPoseDetectionCap().RegisterToPoseDetected(FubiOpenNISensor::CbPoseDetected, this, m_hPoseDetected);
		m_UserGenerator.GetPoseDetectionCap().RegisterToOutOfPose(FubiOpenNISensor::CbOutOfPose, this, m_hOutOfPose);
		m_UserGenerator.GetSkeletonCap().GetCalibrationPose(m_strPose);
	}

	m_UserGenerator.GetSkeletonCap().SetSkeletonProfile((XnSkeletonProfile) trackingProfile);

	// Default is no smoothing (because you better do it yourself adapted to the task you have)
	m_UserGenerator.GetSkeletonCap().SetSmoothing(0);

	nRetVal = m_Context.StartGeneratingAll();
	CHECK_RC(nRetVal, "StartGenerating");

	// Update the options
	updateOptions();
	// No getter for profile in OpenNI
	m_options.m_trackingProfile = trackingProfile;

	if (nRetVal == XN_STATUS_OK)
		Fubi_logInfo("FubiOpenNISensor: succesfully initialized from XML!\n");
	return nRetVal == XN_STATUS_OK;
}

void FubiOpenNISensor::updateOptions()
{
	// First stream options
	if (m_DepthGenerator.IsValid())
	{
		xn::DepthMetaData dmd;
		m_DepthGenerator.GetMetaData(dmd);
		m_options.m_depthOptions.m_width = (signed)dmd.FullXRes();
		m_options.m_depthOptions.m_height = (signed)dmd.FullYRes();
		m_options.m_depthOptions.m_fps = (signed)dmd.FPS();
	}
	else
		m_options.m_depthOptions.invalidate();
	if (m_ImageGenerator.IsValid())
	{
		xn::ImageMetaData imd;
		m_ImageGenerator.GetMetaData(imd);
		m_options.m_rgbOptions.m_width = (signed)imd.FullXRes();
		m_options.m_rgbOptions.m_height = (signed)imd.FullYRes();
		m_options.m_rgbOptions.m_fps = (signed)imd.FPS();
	}
	else
		m_options.m_rgbOptions.invalidate();
	if (m_IRGenerator.IsValid())
	{
		xn::IRMetaData imd;
		m_IRGenerator.GetMetaData(imd);
		m_options.m_irOptions.m_width = (signed)imd.FullXRes();
		m_options.m_irOptions.m_height = (signed)imd.FullYRes();
		m_options.m_irOptions.m_fps = (signed)imd.FPS();
	}
	else
		m_options.m_irOptions.invalidate();

	// And the mirror
	m_options.m_mirrorStreams = m_Context.GetGlobalMirror() == TRUE;
}

bool FubiOpenNISensor::createMockDepth(const Fubi::StreamOptions& options)
{
	Fubi_logWrn("No depth generator found. Using a mock generator!!\n");
	xn::MockDepthGenerator mockDepth;
	XnStatus nRetVal = mockDepth.Create(m_Context);
	CHECK_RC(nRetVal, "FubiOpenNISensor:Create mock depth");

	// set some defaults
	XnMapOutputMode defaultMode;
	defaultMode.nXRes = options.m_width;
	defaultMode.nYRes = options.m_height;
	defaultMode.nFPS = options.m_fps;
	nRetVal = mockDepth.SetMapOutputMode(defaultMode);
	CHECK_RC(nRetVal, "set default mode");

	// set FOV
	XnFieldOfView fov;
	fov.fHFOV = 1.0225999419141749;
	fov.fVFOV = 0.79661567681716894;
	nRetVal = mockDepth.SetGeneralProperty(XN_PROP_FIELD_OF_VIEW, sizeof(fov), &fov);
	CHECK_RC(nRetVal, "set FOV");

	XnUInt32 nDataSize = defaultMode.nXRes * defaultMode.nYRes * sizeof(XnDepthPixel);
	XnDepthPixel* pData = (XnDepthPixel*)xnOSCallocAligned(nDataSize, 1, XN_DEFAULT_MEM_ALIGN);

	nRetVal = mockDepth.SetData(1, 0, nDataSize, pData);
	CHECK_RC(nRetVal, "set empty depth map");

	m_DepthGenerator = mockDepth;

	return true;
}

FubiOpenNISensor::~FubiOpenNISensor()
{
	m_Context.StopGeneratingAll();

	if (m_hPoseDetected != 0x0)
	{
		m_UserGenerator.GetPoseDetectionCap().UnregisterFromPoseDetected(m_hPoseDetected);
		m_hPoseDetected = 0x0;
	}
	if (m_hOutOfPose != 0x0)
	{
		m_UserGenerator.GetPoseDetectionCap().UnregisterFromOutOfPose(m_hOutOfPose);
		m_hOutOfPose = 0x0;
	}
	if (m_hUserCallbacks != 0x0)
	{
		m_UserGenerator.UnregisterUserCallbacks(m_hUserCallbacks);
		m_hUserCallbacks = 0x0;
	}
	if (m_hCalibrationComplete != 0x0)
	{
		m_UserGenerator.GetSkeletonCap().UnregisterFromCalibrationComplete(m_hCalibrationComplete);
		m_hCalibrationComplete = 0x0;
	}
	if (m_hCalibrationStart != 0x0)
	{
		m_UserGenerator.GetSkeletonCap().UnregisterFromCalibrationStart(m_hCalibrationStart);
		m_hCalibrationStart = 0x0;
	}
	if (m_hExitUser)
	{
		m_UserGenerator.UnregisterFromUserExit(m_hExitUser);
	}
	if (m_hReenterUser)
	{
		m_UserGenerator.UnregisterFromUserReEnter(m_hReenterUser);
	}

	m_Context.Release();
}

void FubiOpenNISensor::update()
{
	// Don't wait for new Fubi data just update to the most recent one
	// For not limiting the framerate
	m_Context.WaitNoneUpdateAll();

	// Apply user events, i.e. new/exit/reenter user
	applyUserEvents();
}

Plane FubiOpenNISensor::getFloor()
{
	XnPlane3D plane;
	m_SceneAnalyzer.GetFloor(plane);
	Vec3f normal(plane.vNormal.X, plane.vNormal.Y, plane.vNormal.Z);
	Vec3f point(plane.ptPoint.X, plane.ptPoint.Y, plane.ptPoint.Z);
	float lambda = normal.dot(point);
	return Plane(normal.x, normal.y, normal.z, lambda);
}

// Callback: New user was detected
void XN_CALLBACK_TYPE FubiOpenNISensor::CbNewUser(xn::UserGenerator& generator, unsigned int nId, void* pCookie)
{
	// New user found
	Fubi_logInfo("FubiOpenNISensor: New User %d\n", nId);

	if (pCookie != 0x0)
	{
		FubiOpenNISensor* sensor = (FubiOpenNISensor*)pCookie;

		while(sensor->m_applyingUserEvents)
		{
			// Wait until users are updated
			Sleep(5);
		}

		sensor->m_newUsers.push_back(nId);
	}
}

void FubiOpenNISensor::applyUserEvents()
{
	m_applyingUserEvents = true;

	// Update exited users
	while (!m_exitUsers.empty())
	{
		unsigned int nId = m_exitUsers.back();
		m_exitUsers.pop_back();

		FubiUser* user = Fubi::getUser(nId);
		if (user)
		{
			user->m_inScene = false;
		}
	}

	// And reentered ones
	while (!m_reenteredUsers.empty())
	{
		unsigned int nId = m_reenteredUsers.back();
		m_reenteredUsers.pop_back();

		FubiUser* user = Fubi::getUser(nId);
		if (user)
		{
			user->m_inScene = true;
		}
	}

	// And start calibration for new ones
	while (!m_newUsers.empty())
	{
		unsigned int nId = m_newUsers.back();
		m_newUsers.pop_back();

		// No calibration was loaded so we have to perform it
		if (m_UserGenerator.GetSkeletonCap().NeedPoseForCalibration())
		{
			m_UserGenerator.GetPoseDetectionCap().StartPoseDetection(m_strPose, nId);
		}
		else
		{
			m_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
		}
	}

	m_applyingUserEvents = false;
}

// Callback: An existing user was lost
void XN_CALLBACK_TYPE FubiOpenNISensor::CbLostUser(xn::UserGenerator& generator, unsigned int nId, void* pCookie)
{
	Fubi_logInfo("FubiOpenNISensor: Lost user %d\n", nId);
}

// Callback: Exit user
void XN_CALLBACK_TYPE FubiOpenNISensor::CbExitUser(xn::UserGenerator& generator, unsigned int nId, void* pCookie)
{
	Fubi_logInfo("FubiOpenNISensor: User exit %d\n", nId);

	if (pCookie != 0x0)
	{
		FubiOpenNISensor* sensor = (FubiOpenNISensor*)pCookie;

		while(sensor->m_applyingUserEvents)
		{
			// Wait until users are updated
			Sleep(5);
		}

		sensor->m_exitUsers.push_back(nId);
	}
}

// Callback: Reenter user
void XN_CALLBACK_TYPE FubiOpenNISensor::CbReenterUser(xn::UserGenerator& generator, unsigned int nId, void* pCookie)
{
	Fubi_logInfo("FubiOpenNISensor: User reentered %d\n", nId);

	if (pCookie != 0x0)
	{
		FubiOpenNISensor* sensor = (FubiOpenNISensor*)pCookie;

		while(sensor->m_applyingUserEvents)
		{
			// Wait until users are updated
			Sleep(10);
		}

		sensor->m_reenteredUsers.push_back(nId);
	}
}

// Callback: Detected a pose
void XN_CALLBACK_TYPE FubiOpenNISensor::CbPoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, unsigned int nId, void* pCookie)
{
	Fubi_logInfo("FubiOpenNISensor: Pose %s detected for user %d\n", strPose, nId);
	if (pCookie != 0x0)
	{
		FubiOpenNISensor* sensor = (FubiOpenNISensor*)pCookie;
		sensor->m_UserGenerator.GetPoseDetectionCap().StopPoseDetection(nId);
		sensor->m_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
	}
}

// Callback: No in calibration pose any more
void XN_CALLBACK_TYPE FubiOpenNISensor::CbOutOfPose(xn::PoseDetectionCapability& capability, const XnChar* strPose, unsigned int nId, void* pCookie)
{
	// User is not in pose any more
}

// Callback: Started calibration
void XN_CALLBACK_TYPE FubiOpenNISensor::CbCalibrationStart(xn::SkeletonCapability& capability, unsigned int nId, void* pCookie)
{
	Fubi_logInfo("FubiOpenNISensor: Calibration started for user %d\n", nId);
}

// Callback: Finished calibration
void XN_CALLBACK_TYPE FubiOpenNISensor::CbCalibrationComplete(xn::SkeletonCapability& capability, unsigned int nId, XnCalibrationStatus calibrationStatus, void* pCookie)
{
	if (pCookie != 0x0)
	{

		FubiOpenNISensor* sensor = (FubiOpenNISensor*)pCookie;

		if (calibrationStatus == XN_CALIBRATION_STATUS_OK)
		{
			// Calibration succeeded
			Fubi_logInfo("FubiOpenNISensor: Calibration complete, start tracking user %d\n", nId);
			sensor->m_UserGenerator.GetSkeletonCap().StartTracking(nId);

			// Nothing more to do here, users will appear as tracked in the next updateTrackingData call
		}
		else
		{
			// Calibration failed
			Fubi_logInfo("FubiOpenNISensor: Calibration failed for user %d\n", nId);
			if (sensor->m_UserGenerator.GetSkeletonCap().NeedPoseForCalibration())
			{
				sensor->m_UserGenerator.GetPoseDetectionCap().StartPoseDetection(sensor->m_strPose, nId);
			}
			else
			{
				sensor->m_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
			}
		}
	}
}

bool FubiOpenNISensor::hasNewTrackingData()
{
	bool isNew =m_UserGenerator.IsDataNew() == TRUE;
	return isNew;
}

bool FubiOpenNISensor::isTracking(unsigned int id)
{
	xn::SkeletonCapability cap = m_UserGenerator.GetSkeletonCap();
	XnPoint3D com;
	m_UserGenerator.GetCoM(id, com);
	return cap.IsTracking(id) != 0 && com.Z > 0;
}

void FubiOpenNISensor::getSkeletonJointData(unsigned int id, Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJointPosition& position, Fubi::SkeletonJointOrientation& orientation)
{
	FubiUser* user = Fubi::getUser(id);
	if (user)
	{
		if (user->isTracked())
		{
			xn::SkeletonCapability cap = m_UserGenerator.GetSkeletonCap();
			// Standard case user is tracked
			// Get the current tracking data
			XnSkeletonJoint xjoint = JointToXNSkeletonJoint(joint);
			if (xjoint == XN_SKEL_WAIST) // special case currently not supported by OpenNI but approximated as center between hips
			{
				// Position is middle between the hips
				XnSkeletonJointPosition pos1, pos2;
				cap.GetSkeletonJointPosition(user->id(), XN_SKEL_LEFT_HIP, pos1);
				cap.GetSkeletonJointPosition(user->id(), XN_SKEL_RIGHT_HIP, pos2);
				position.m_confidence = minf(pos1.fConfidence, pos2.fConfidence);
				position.m_position = xnJointToVec3f(pos2) + (xnJointToVec3f(pos1) - xnJointToVec3f(pos2))*0.5f;

				// Rotation is the same as torso rotation
				XnSkeletonJointOrientation rot;
				cap.GetSkeletonJointOrientation(user->id(), XN_SKEL_TORSO, rot);
				orientation = SkeletonJointOrientation(rot.orientation.elements, rot.fConfidence);
			}
			else
			{
				// Standard case, just get the data
				XnSkeletonJointPosition pos;
				XnSkeletonJointOrientation rot;
				cap.GetSkeletonJointPosition(user->id(), xjoint, pos);
				cap.GetSkeletonJointOrientation(user->id(), xjoint, rot);
				position.m_position = xnJointToVec3f(pos);
				position.m_confidence = pos.fConfidence;
				orientation.m_orientation = Matrix3f(rot.orientation.elements);
				orientation.m_confidence = pos.fConfidence;
			}
		}
		else
		{
			// Not tracked return the center of mass instead
			// that should be independent of tracking state
			XnPoint3D com;
			m_UserGenerator.GetCoM(user->id(), com);
			position.m_confidence = 0.5f; // leave confidence at 0.5 as this is not really the wanted position
			position.m_position.x = com.X;
			position.m_position.y = com.Y;
			position.m_position.z = com.Z;
		}
	}
}

const unsigned short* FubiOpenNISensor::getDepthData()
{
	if (m_DepthGenerator.IsValid() && m_DepthGenerator.IsGenerating())
	{
		xn::DepthMetaData depthMData;
		m_DepthGenerator.GetMetaData(depthMData);
		return depthMData.Data();
	}
	return 0x0;
}

const unsigned char* FubiOpenNISensor::getRgbData()
{
	if (m_ImageGenerator.IsValid() && m_ImageGenerator.IsGenerating())
	{
		xn::ImageMetaData imageMData;
		m_ImageGenerator.GetMetaData(imageMData);
		return imageMData.Data();
	}
	return 0x0;
}

const unsigned short* FubiOpenNISensor::getIrData()
{
	if (m_IRGenerator.IsValid() && m_IRGenerator.IsGenerating())
	{
		xn::IRMetaData irMData;
		m_IRGenerator.GetMetaData(irMData);
		return irMData.Data();
	}
	return 0x0;
}

const unsigned short* FubiOpenNISensor::getUserLabelData(Fubi::CoordinateType::Type coordType)
{
	if (m_UserGenerator.IsValid() && m_UserGenerator.IsGenerating() && m_UserGenerator.GetNumberOfUsers() > 0)
	{
		xn::SceneMetaData sMData;
		m_UserGenerator.GetUserPixels(0, sMData);
		// TODO: adapt for coordinate type
		return sMData.Data();
	}
	return 0x0;
}

unsigned short FubiOpenNISensor::getUserIDs(unsigned int* userIDs)
{
	unsigned short numUsers = 0;
	if (m_UserGenerator.IsValid())
	{
		numUsers = Fubi::MaxUsers;
		m_UserGenerator.GetUsers(userIDs, numUsers);
	}
	return numUsers;
}

Fubi::Vec3f FubiOpenNISensor::convertCoordinates(const Fubi::Vec3f& inputCoords, Fubi::CoordinateType::Type inputType, Fubi::CoordinateType::Type outputType)
{
	if (outputType == CoordinateType::DEPTH && inputType == CoordinateType::REAL_WORLD)
	{
		if (m_DepthGenerator.IsValid())
		{
			XnPoint3D pos;
			pos.X = inputCoords.x;
			pos.Y = inputCoords.y;
			pos.Z = inputCoords.z;
			m_DepthGenerator.ConvertRealWorldToProjective(1, &pos, &pos);
			return Vec3f(pos.X, pos.Y, pos.Z);
		}
	}
	else if (inputType == CoordinateType::DEPTH && outputType == CoordinateType::REAL_WORLD)
	{
		if (m_DepthGenerator.IsValid())
		{
			XnPoint3D pos;
			pos.X = inputCoords.x;
			pos.Y = inputCoords.y;
			pos.Z = inputCoords.z;
			m_DepthGenerator.ConvertProjectiveToRealWorld(1, &pos, &pos);
			return Vec3f(pos.X, pos.Y, pos.Z);
		}
	}
	else if (inputType == CoordinateType::IR && outputType != CoordinateType::DEPTH)
	{
		Vec3f depthCoords = convertCoordinates(inputCoords, CoordinateType::IR, CoordinateType::DEPTH);
		return convertCoordinates(depthCoords, CoordinateType::DEPTH, outputType);
	}
	else if (outputType == CoordinateType::IR && inputType != CoordinateType::DEPTH)
	{
		Vec3f depthCoords = convertCoordinates(inputCoords, inputType, CoordinateType::DEPTH);
		return convertCoordinates(depthCoords, CoordinateType::DEPTH, CoordinateType::IR);
	}

	// Conversion IR<>Depth will be handled in base class, dummy conversion for all conversions with the color stream...
	return FubiISensor::convertCoordinates(inputCoords, inputType, outputType);
}

void FubiOpenNISensor::resetTracking(unsigned int id)
{
	m_UserGenerator.GetSkeletonCap().Reset(id);
	m_newUsers.push_back(id);
}

#endif
