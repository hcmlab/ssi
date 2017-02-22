// ****************************************************************************************
//
// Fubi Kinect SDK sensor
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler 
// 
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
// 
// ****************************************************************************************

#include "FubiKinectSDKSensor.h"

#ifdef FUBI_USE_KINECT_SDK

#pragma comment(lib, "FaceTrackLib.lib")
#pragma comment(lib, "Kinect10.lib")


#include "FubiUser.h"
#include "Fubi.h"


using namespace Fubi;
using namespace std;

static NUI_SKELETON_POSITION_INDEX JointToKSDKJoint(const SkeletonJoint::Joint j)
{
	switch (j)
	{
	case SkeletonJoint::HEAD:
		return NUI_SKELETON_POSITION_HEAD;
	case SkeletonJoint::NECK:
		return NUI_SKELETON_POSITION_SHOULDER_CENTER;
	case SkeletonJoint::TORSO:
		return NUI_SKELETON_POSITION_SPINE;
	case SkeletonJoint::LEFT_SHOULDER:
		return NUI_SKELETON_POSITION_SHOULDER_LEFT;
	case SkeletonJoint::LEFT_ELBOW:
		return NUI_SKELETON_POSITION_ELBOW_LEFT;
	case SkeletonJoint::LEFT_WRIST:
		return NUI_SKELETON_POSITION_WRIST_LEFT;
	case SkeletonJoint::LEFT_HAND:
		return NUI_SKELETON_POSITION_HAND_LEFT;
	case SkeletonJoint::RIGHT_SHOULDER:
		return NUI_SKELETON_POSITION_SHOULDER_RIGHT;
	case SkeletonJoint::RIGHT_ELBOW:
		return NUI_SKELETON_POSITION_ELBOW_RIGHT;
	case SkeletonJoint::RIGHT_WRIST:
		return NUI_SKELETON_POSITION_WRIST_RIGHT;
	case SkeletonJoint::RIGHT_HAND:
		return NUI_SKELETON_POSITION_HAND_RIGHT;
	case SkeletonJoint::LEFT_HIP:
		return NUI_SKELETON_POSITION_HIP_LEFT;
	case SkeletonJoint::LEFT_KNEE:
		return NUI_SKELETON_POSITION_KNEE_LEFT;
	case SkeletonJoint::LEFT_ANKLE:
		return NUI_SKELETON_POSITION_ANKLE_LEFT;
	case SkeletonJoint::LEFT_FOOT:
		return NUI_SKELETON_POSITION_FOOT_LEFT;
	case SkeletonJoint::RIGHT_HIP:
		return NUI_SKELETON_POSITION_HIP_RIGHT;
	case SkeletonJoint::RIGHT_KNEE:
		return NUI_SKELETON_POSITION_KNEE_RIGHT;
	case SkeletonJoint::RIGHT_ANKLE:
		return NUI_SKELETON_POSITION_ANKLE_RIGHT;
	case SkeletonJoint::RIGHT_FOOT:
		return NUI_SKELETON_POSITION_FOOT_RIGHT;
	case SkeletonJoint::WAIST:
		return NUI_SKELETON_POSITION_HIP_CENTER;
	default:
		return NUI_SKELETON_POSITION_COUNT;
	}
	return NUI_SKELETON_POSITION_COUNT;
}

static NUI_SKELETON_POSITION_INDEX JointToKSDKFallbackJoint(const SkeletonJoint::Joint j)
{
	switch (j)
	{
	case SkeletonJoint::LEFT_HAND:
		return NUI_SKELETON_POSITION_WRIST_LEFT;
	case SkeletonJoint::RIGHT_HAND:
		return NUI_SKELETON_POSITION_WRIST_RIGHT;
	case SkeletonJoint::LEFT_FOOT:
		return NUI_SKELETON_POSITION_ANKLE_LEFT;
	case SkeletonJoint::RIGHT_FOOT:
		return NUI_SKELETON_POSITION_ANKLE_RIGHT;
	default:
		return NUI_SKELETON_POSITION_COUNT;
	}
	return NUI_SKELETON_POSITION_COUNT;
}

static int JointToKSDKFacePointIndex(const SkeletonJoint::Joint j)
{
	switch (j)
	{
	case SkeletonJoint::FACE_NOSE:
		return 7;
	case SkeletonJoint::FACE_LEFT_EAR:
		return 20;
	case SkeletonJoint::FACE_RIGHT_EAR:
		return 53;
	case SkeletonJoint::FACE_FOREHEAD:
		return 1;
	case SkeletonJoint::FACE_CHIN:
		return 10;
	default:
		return 0;
	}
	return 0;
}

static void NUIMatrix4ToMatrix3f(const Matrix4& nuiMat, Matrix3f& mat)
{
	mat.c[0][0] = nuiMat.M11;
	mat.c[0][1] = nuiMat.M12;
	mat.c[0][2] = nuiMat.M13;
	mat.c[1][0] = nuiMat.M21;
	mat.c[1][1] = nuiMat.M22;
	mat.c[1][2] = nuiMat.M23;
	mat.c[2][0] = nuiMat.M31;
	mat.c[2][1] = nuiMat.M32;
	mat.c[2][2] = nuiMat.M33;

	// Convert coordinate system
	Vec3f rot = mat.getRot();
	rot.y -= 180.0f ;
	rot.x *= -1.0f;
	rot.z *= -1.0f;
	mat = Matrix3f(Quaternion(degToRad(rot.x), degToRad(rot.y), degToRad(rot.z)));
}

static Fubi::Vec3f NUIVec3ToVec3f(const FT_VECTOR3D& nuiVec)
{
	return Vec3f(nuiVec.x*1000.0f, nuiVec.y*1000.0f, nuiVec.z*1000.0f);
}

static Fubi::Vec3f NUIVec2ToVec3f(const FT_VECTOR2D& nuiVec)
{
	return Vec3f(nuiVec.x, nuiVec.y, 0);
}

static Fubi::Vec3f NUITriangleIndexToVec3f(const FT_TRIANGLE& nuiTri)
{
	return Vec3f((float)nuiTri.i, (float)nuiTri.j, (float)nuiTri.k);
}

FubiKinectSDKSensor::FubiKinectSDKSensor() : m_pNuiSensor(NULL), 
	m_hNextDepthFrameEvent(NULL), m_hNextVideoFrameEvent(NULL), m_hNextSkeletonEvent(NULL),
	m_pDepthStreamHandle(NULL), m_pVideoStreamHandle(NULL), m_playerPixelBufferColor(NULL),
	m_videoBuffer(NULL), m_convertedVideoBuffer(NULL), m_depthBuffer(NULL), m_playerPixelBuffer(NULL),
	m_convertedDepthBuffer(NULL), m_registeredDepthBuffer(NULL), m_irBuffer(0x0),
	m_hThNuiProcess(NULL), m_hEvNuiProcessStop(NULL), m_bNuiInitialized(false),
	m_zoomFactor(1.0f),    
	m_imageDataNew(false), m_seatedSkeletonMode(false),
	m_depthToColorMap(NULL), m_colorToDepthDivisor(1),
	m_hasNewData(false), m_lastTrackingFrameID(-1), m_currTrackingFrameID(-1)
{
	m_options.m_type = SensorType::KINECTSDK;

	m_viewOffset.x = 0;
	m_viewOffset.y = 0;

	for (int i=0; i < FUBI_KINECT_SDK_MAX_NUM_FACES_TRACKED; ++i)
	{
		m_userContext[i].m_CountUntilFailure = 0;
		m_userContext[i].m_LastTrackSucceeded = false;
		m_userContext[i].m_SkeletonId = -1;
		m_userContext[i].m_pFaceTracker = 0x0;
		m_userContext[i].m_pFTResult = 0x0;
	}
}

HRESULT FubiKinectSDKSensor::getVideoConfiguration(FT_CAMERA_CONFIG* videoConfig)
{
	if (!videoConfig)
	{
		return E_POINTER;
	}

	UINT width = m_videoBuffer ? m_videoBuffer->GetWidth() : 0;
	UINT height =  m_videoBuffer ? m_videoBuffer->GetHeight() : 0;
	FLOAT focalLength = 0.f;

	if(width == 640 && height == 480)
	{
		focalLength = NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS;
	}
	else if(width == 1280 && height == 960)
	{
		focalLength = NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS * 2.f;
	}

	if(focalLength == 0.f)
	{
		return E_UNEXPECTED;
	}

	videoConfig->FocalLength = focalLength;
	videoConfig->Width = width;
	videoConfig->Height = height;
	return(S_OK);
}

HRESULT FubiKinectSDKSensor::getDepthConfiguration(FT_CAMERA_CONFIG* depthConfig)
{
	if (!depthConfig)
	{
		return E_POINTER;
	}

	UINT width = m_depthBuffer ? m_depthBuffer->GetWidth() : 0;
	UINT height =  m_depthBuffer ? m_depthBuffer->GetHeight() : 0;
	FLOAT focalLength = 0.f;

	if(width == 80 && height == 60)
	{
		focalLength = NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS / 4.f;
	}
	else if(width == 320 && height == 240)
	{
		focalLength = NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS;
	}
	else if(width == 640 && height == 480)
	{
		focalLength = NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS * 2.f;
	}

	if(focalLength == 0.f)
	{
		return E_UNEXPECTED;
	}

	depthConfig->FocalLength = focalLength;
	depthConfig->Width = width;
	depthConfig->Height = height;

	return S_OK;
}



bool FubiKinectSDKSensor::initWithOptions(const Fubi::SensorOptions& options)
{
	// Init sensor streams and user tracking
	HRESULT hr = E_UNEXPECTED;

	// Copy options and translate to Kinect SDK values
	m_options.m_registerStreams = options.m_registerStreams;	
	NUI_IMAGE_TYPE depthType = NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX;
	m_options.m_depthOptions.m_width = options.m_depthOptions.m_width;
	switch (options.m_depthOptions.m_width)
	{
	case 80:
		m_depthRes = NUI_IMAGE_RESOLUTION_80x60;
		m_options.m_depthOptions.m_height = 60;
		break;
	case 320:
		m_depthRes = NUI_IMAGE_RESOLUTION_320x240;
		m_options.m_depthOptions.m_height = 240;
		break;
	case 1280:
		m_depthRes =	NUI_IMAGE_RESOLUTION_1280x960;
		m_options.m_depthOptions.m_height = 960;
	default:
		m_options.m_depthOptions.m_width = 640;
		m_options.m_depthOptions.m_height = 480;
		m_depthRes = NUI_IMAGE_RESOLUTION_640x480;
	}
	NUI_IMAGE_TYPE colorType;
	bool colorValid = false;
	if (options.m_rgbOptions.isValid())
	{
		m_options.m_rgbOptions.m_width = options.m_rgbOptions.m_width;
		colorType = NUI_IMAGE_TYPE_COLOR;
		colorValid = true;
		switch (options.m_rgbOptions.m_width)
		{
		case 80:
			m_colorRes = NUI_IMAGE_RESOLUTION_80x60;
			m_options.m_rgbOptions.m_height = 60;
			break;
		case 320:
			m_colorRes = NUI_IMAGE_RESOLUTION_320x240;
			m_options.m_rgbOptions.m_height = 240;
			break;
		case 1280:
			m_colorRes =	NUI_IMAGE_RESOLUTION_1280x960;
			m_options.m_rgbOptions.m_height = 960;
		default:
			m_options.m_rgbOptions.m_width = 640;
			m_options.m_rgbOptions.m_height = 480;
			m_colorRes = NUI_IMAGE_RESOLUTION_640x480;
		}
		m_colorToDepthDivisor = m_options.m_rgbOptions.m_width / m_options.m_depthOptions.m_width;
	}
	else if (options.m_irOptions.isValid())
	{
		m_options.m_rgbOptions.invalidate();
		m_options.m_irOptions.m_fps = 30;
		m_options.m_irOptions.m_width = options.m_irOptions.m_width;
		colorValid = true;
		colorType = NUI_IMAGE_TYPE_COLOR_INFRARED;
		switch (options.m_irOptions.m_width)
		{
		case 80:
			m_colorRes = NUI_IMAGE_RESOLUTION_80x60;
			m_options.m_irOptions.m_height = 60;
			break;
		case 320:
			m_colorRes = NUI_IMAGE_RESOLUTION_320x240;
			m_options.m_irOptions.m_height = 240;
			break;
		case 1280:
			m_colorRes =	NUI_IMAGE_RESOLUTION_1280x960;
			m_options.m_irOptions.m_height = 960;
		default:
			m_options.m_irOptions.m_width = 640;
			m_options.m_irOptions.m_height = 480;
			m_colorRes = NUI_IMAGE_RESOLUTION_640x480;
		}

		delete[] m_irBuffer;
		m_irBuffer = new unsigned short[m_options.m_irOptions.m_width*m_options.m_irOptions.m_height];
		m_colorToDepthDivisor = m_options.m_irOptions.m_width / m_options.m_depthOptions.m_width;
	}
	// Copy rest of options
	m_options.m_trackingProfile = options.m_trackingProfile;
	// Activate seated mode depending on profile
	m_seatedSkeletonMode = m_options.m_trackingProfile == SkeletonTrackingProfile::UPPER_BODY || m_options.m_trackingProfile == SkeletonTrackingProfile::HEAD_HANDS;
	if (m_seatedSkeletonMode)
		// Upper body comes closest to the seated mode
		m_options.m_trackingProfile = SkeletonTrackingProfile::UPPER_BODY;
	else
		// Everything else is interpreted as all (note: also NONE as tracking cannot be disabled)
		m_options.m_trackingProfile = SkeletonTrackingProfile::ALL;

	// Init Kinect SDK data structures
	BOOL bNearMode = TRUE;
	BOOL bFallbackToDefault = TRUE;
	DWORD width = 0;
	DWORD height = 0;
	if (m_options.m_rgbOptions.isValid())
	{
		m_videoBuffer = FTCreateImage();
		m_playerPixelBufferColor = FTCreateImage();
		m_convertedVideoBuffer = FTCreateImage();
		if (!m_videoBuffer || !m_playerPixelBufferColor || !m_convertedVideoBuffer)
			return false;
		NuiImageResolutionToSize(m_colorRes, width, height);
		hr = m_videoBuffer->Allocate(width, height, FTIMAGEFORMAT_UINT8_B8G8R8X8);
		if (FAILED(hr))
			return false;
		hr = m_convertedVideoBuffer->Allocate(width, height, FTIMAGEFORMAT_UINT8_R8G8B8);
		if (FAILED(hr))
			return false;
		hr = m_playerPixelBufferColor->Allocate(width, height, FTIMAGEFORMAT_UINT16_D16);
		if (FAILED(hr))
			return false;
	}
	m_depthBuffer = FTCreateImage();
	if (!m_depthBuffer)
		return false;
	m_convertedDepthBuffer = FTCreateImage();
	m_registeredDepthBuffer = FTCreateImage();
	m_playerPixelBuffer = FTCreateImage();
	if (!m_convertedDepthBuffer || !m_registeredDepthBuffer || !m_playerPixelBuffer)
		return false;
	NuiImageResolutionToSize(m_depthRes, width, height);
	m_depthToColorMap = new LONG[width*height*2];
	hr = m_depthBuffer->Allocate(width, height, FTIMAGEFORMAT_UINT16_D13P3);
	if (FAILED(hr))
		return false;
	hr = m_convertedDepthBuffer->Allocate(width, height, FTIMAGEFORMAT_UINT16_D16);
	if (FAILED(hr))
		return false;
	hr = m_registeredDepthBuffer->Allocate(width, height, FTIMAGEFORMAT_UINT16_D13P3);
	if (FAILED(hr))
		return false;
	hr = m_playerPixelBuffer->Allocate(width, height, FTIMAGEFORMAT_UINT16_D16);
	if (FAILED(hr))
		return false;   
	for (int i = 0; i < NUI_SKELETON_COUNT; ++i)
	{
		for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; ++j)
		{
			m_skelPos[i][j] = FT_VECTOR3D(0, 0, 0);
			m_skelConf[i][j] = 0;
			m_skelRot[i][j].M11 = 1; m_skelRot[i][j].M12 = 0; m_skelRot[i][j].M13 = 0; m_skelRot[i][j].M14 = 0;
			m_skelRot[i][j].M21 = 0; m_skelRot[i][j].M22 = 1; m_skelRot[i][j].M23 = 0; m_skelRot[i][j].M24 = 0;
			m_skelRot[i][j].M31 = 0; m_skelRot[i][j].M32 = 0; m_skelRot[i][j].M33 = 1; m_skelRot[i][j].M34 = 0;
			m_skelRot[i][j].M41 = 0; m_skelRot[i][j].M42 = 0; m_skelRot[i][j].M43 = 0; m_skelRot[i][j].M44 = 1;
		}
		m_headOrient[i][0] = 0;
		m_headOrient[i][1] = 0;
		m_headOrient[i][2] = 0;
		m_headPos[i][0] = 0;
		m_headPos[i][1] = 0;
		m_headPos[i][2] = 0;
		m_headTracked[i]  = false;
		m_faceTracked[i] = false;
		m_face2DTracked[i] = false;
		m_skeletonTracked[i] = false;
	}
	// Events
	m_hNextDepthFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hNextVideoFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hNextSkeletonEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// Now initialize the Kinect SDK
	int iSensorCount = 0;
	hr = NuiGetSensorCount(&iSensorCount);
	if (FAILED(hr))
	{
		Fubi_logErr("InitWithOptions unable to start sensor!\n");
		return false;
	}
	else
	{
		DWORD dwNuiInitDepthFlag = (depthType == NUI_IMAGE_TYPE_DEPTH)? NUI_INITIALIZE_FLAG_USES_DEPTH : NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX;
		for (int i = 0; i < iSensorCount; ++i)
		{
			m_pNuiSensor = nullptr;
			hr = NuiCreateSensorByIndex(i, &m_pNuiSensor);
			if (SUCCEEDED(hr))
			{
				// Check if usable
				hr = m_pNuiSensor->NuiStatus();
				if (hr == S_OK)
				{
					hr = m_pNuiSensor->NuiInitialize(dwNuiInitDepthFlag | NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_COLOR);
					if (SUCCEEDED(hr))
						break;
					else
						m_pNuiSensor = nullptr;
				}
			}
			else
				m_pNuiSensor = nullptr;
		}
	}

	if (m_pNuiSensor == nullptr)
	{
		Fubi_logErr("InitWithOptions unable to start sensor!\n");
		return false;
	}

	m_bNuiInitialized = true;
	DWORD dwSkeletonFlags = NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE;
	if (m_seatedSkeletonMode)
	{
		dwSkeletonFlags |= NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT;
	}
	// And tracking
	hr = m_pNuiSensor->NuiSkeletonTrackingEnable( m_hNextSkeletonEvent, dwSkeletonFlags );
	if (FAILED(hr))
	{
		return false;
	}
	// Open streams
	if (colorValid)
	{
		hr = m_pNuiSensor->NuiImageStreamOpen(
			colorType,
			m_colorRes,
			0,
			2,
			m_hNextVideoFrameEvent,
			&m_pVideoStreamHandle);
		if (FAILED(hr))
		{
			return false;
		}
	}
	hr = m_pNuiSensor->NuiImageStreamOpen(
		depthType,
		m_depthRes,
		(bNearMode)? NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE : 0,
		2,
		m_hNextDepthFrameEvent,
		&m_pDepthStreamHandle );
	if (FAILED(hr))
	{
		if(bNearMode && bFallbackToDefault)
		{
			hr = m_pNuiSensor->NuiImageStreamOpen(
				depthType,
				m_depthRes,
				0,
				2,
				m_hNextDepthFrameEvent,
				&m_pDepthStreamHandle );
		}

		if(FAILED(hr))
		{
			return false;
		}
	}

	// Init face tracking
	if (m_options.m_rgbOptions.isValid())
	{
		FT_CAMERA_CONFIG videoConfig;
		getVideoConfiguration(&videoConfig);
		FT_CAMERA_CONFIG depthConfig;
		getDepthConfiguration(&depthConfig);
		for (UINT i=0; i<FUBI_KINECT_SDK_MAX_NUM_FACES_TRACKED;++i)
		{
			// Try to start the face tracker.
			m_userContext[i].m_pFaceTracker = FTCreateFaceTracker();
			if (!m_userContext[i].m_pFaceTracker)
			{
				return false;
			}

			hr = m_userContext[i].m_pFaceTracker->Initialize(&videoConfig, &depthConfig, NULL, NULL); 
			if (FAILED(hr))
			{
				return false;
			}
			m_userContext[i].m_pFaceTracker->CreateFTResult(&m_userContext[i].m_pFTResult);
			if (!m_userContext[i].m_pFTResult)
			{
				return false;
			}
			m_userContext[i].m_LastTrackSucceeded = false;
			m_userContext[i].m_SkeletonId = 0;
			m_userContext[i].m_CountUntilFailure = 0;
		}
	}

	// Start the Nui processing thread
	m_hEvNuiProcessStop=CreateEvent(NULL,TRUE,FALSE,NULL);
	m_hThNuiProcess=CreateThread(NULL,0,processThread,this,0,NULL);

	Fubi_logInfo("FubiKinectSDKSensor: succesfully initialized!\n");
	return true;
}


FubiKinectSDKSensor::~FubiKinectSDKSensor()
{
	// Stop the Nui processing thread
	if(m_hEvNuiProcessStop!=NULL)
	{
		// Signal the thread
		SetEvent(m_hEvNuiProcessStop);

		// Wait for thread to stop
		if(m_hThNuiProcess!=NULL)
		{
			WaitForSingleObject(m_hThNuiProcess,INFINITE);
			CloseHandle(m_hThNuiProcess);
			m_hThNuiProcess = NULL;
		}
		CloseHandle(m_hEvNuiProcessStop);
		m_hEvNuiProcessStop = NULL;
	}

	if (m_bNuiInitialized)
	{
		m_pNuiSensor->NuiShutdown();
		m_pNuiSensor->Release();
		m_pNuiSensor = nullptr;
	}
	m_bNuiInitialized = false;

	if (m_hNextSkeletonEvent && m_hNextSkeletonEvent != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hNextSkeletonEvent);
		m_hNextSkeletonEvent = NULL;
	}
	if (m_hNextDepthFrameEvent && m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hNextDepthFrameEvent);
		m_hNextDepthFrameEvent = NULL;
	}
	if (m_hNextVideoFrameEvent && m_hNextVideoFrameEvent != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hNextVideoFrameEvent);
		m_hNextVideoFrameEvent = NULL;
	}
	if (m_videoBuffer)
	{
		m_videoBuffer->Release();
		m_videoBuffer = NULL;
	}
	if (m_depthBuffer)
	{
		m_depthBuffer->Release();
		m_depthBuffer = NULL;
	}	
	if (m_convertedVideoBuffer)
	{
		m_convertedVideoBuffer->Release();
		m_convertedVideoBuffer = NULL;
	}
	if (m_convertedDepthBuffer)
	{
		m_convertedDepthBuffer->Release();
		m_convertedDepthBuffer = NULL;
	}
	if (m_registeredDepthBuffer)
	{
		m_registeredDepthBuffer->Release();
		m_registeredDepthBuffer = NULL;
	}
	if (m_playerPixelBuffer)
	{
		m_playerPixelBuffer->Release();
		m_playerPixelBuffer = NULL;
	}
	if (m_playerPixelBufferColor)
	{
		m_playerPixelBufferColor->Release();
		m_playerPixelBufferColor = NULL;
	}

	for (UINT i=0; i<FUBI_KINECT_SDK_MAX_NUM_FACES_TRACKED; ++i)
	{
		if (m_userContext[i].m_pFTResult != 0)
		{
			m_userContext[i].m_pFTResult->Release();
			m_userContext[i].m_pFTResult = 0;
		}
		if (m_userContext[i].m_pFaceTracker != 0)
		{
			m_userContext[i].m_pFaceTracker->Release();
			m_userContext[i].m_pFaceTracker = 0;
		}
	}

	delete[] m_irBuffer;
	delete[] m_depthToColorMap;
}

DWORD WINAPI FubiKinectSDKSensor::processThread(LPVOID pParam)
{
	FubiKinectSDKSensor*  pthis=(FubiKinectSDKSensor *) pParam;
	HANDLE          hEvents[4];

	// Configure events to be listened on
	hEvents[0]=pthis->m_hEvNuiProcessStop;
	hEvents[1]=pthis->m_hNextDepthFrameEvent;
	hEvents[2]=pthis->m_hNextVideoFrameEvent;
	hEvents[3]=pthis->m_hNextSkeletonEvent;

	// Main thread loop
	while (true)
	{
		// Wait for an event to be signaled
		WaitForMultipleObjects(sizeof(hEvents)/sizeof(hEvents[0]),hEvents,FALSE,100);

		// If the stop event is set, stop looping and exit
		if (WAIT_OBJECT_0 == WaitForSingleObject(pthis->m_hEvNuiProcessStop, 0))
		{
			break;
		}

		// Process signal events
		if (WAIT_OBJECT_0 == WaitForSingleObject(pthis->m_hNextDepthFrameEvent, 0))
		{
			pthis->gotDepthAlert();
		}
		if (WAIT_OBJECT_0 == WaitForSingleObject(pthis->m_hNextVideoFrameEvent, 0))
		{
			pthis->gotVideoAlert();
		}
		if (WAIT_OBJECT_0 == WaitForSingleObject(pthis->m_hNextSkeletonEvent, 0))
		{
			pthis->gotSkeletonAlert();
		}
	}

	return 0;
}

void FubiKinectSDKSensor::gotVideoAlert( )
{
	NUI_IMAGE_FRAME pImageFrame;
	if (FAILED(m_pNuiSensor->NuiImageStreamGetNextFrame(m_pVideoStreamHandle, 0, &pImageFrame)))
		return;

	INuiFrameTexture* pTexture = pImageFrame.pFrameTexture;
	NUI_LOCKED_RECT LockedRect;
	if (SUCCEEDED(pTexture->LockRect(0, &LockedRect, NULL, 0)))
	{
		if (LockedRect.Pitch)
		{
			if (m_options.m_rgbOptions.isValid())
			{
				// Copy video frame to face tracking
				memcpy(m_videoBuffer->GetBuffer(), PBYTE(LockedRect.pBits), min(m_videoBuffer->GetBufferSize(), UINT(pTexture->BufferLen())));
				// Convert copied data
				m_videoBuffer->CopyTo(m_convertedVideoBuffer, 0, 0, 0);
				m_imageDataNew = true;

			}
			else if (m_options.m_irOptions.isValid())
			{
				//memcpy(m_irBuffer, PBYTE(LockedRect.pBits),  min(UINT(m_options.m_irOptions.m_width*m_options.m_irOptions.m_height*sizeof(unsigned short)), UINT(pTexture->BufferLen())));
				unsigned int len = m_options.m_irOptions.m_width*m_options.m_irOptions.m_height;
				unsigned short* pSource = (unsigned short*)LockedRect.pBits;
				unsigned short* pDest = m_irBuffer;
				for (unsigned int i=0; i < len; ++i)
				{
					*pDest++ = (*pSource++) >> 6;
				}
				m_imageDataNew = true;
			}
		}

		// Release texture
		pTexture->UnlockRect(0);

		// Release frame
		m_pNuiSensor->NuiImageStreamReleaseFrame(m_pVideoStreamHandle, &pImageFrame);
	}
}


void FubiKinectSDKSensor::gotDepthAlert( )
{
	NUI_IMAGE_FRAME pImageFrame;
	if (FAILED(m_pNuiSensor->NuiImageStreamGetNextFrame(m_pDepthStreamHandle, 0, &pImageFrame)))
		return;

	INuiFrameTexture* pTexture = pImageFrame.pFrameTexture;
	NUI_LOCKED_RECT LockedRect;
	if (SUCCEEDED(pTexture->LockRect(0, &LockedRect, NULL, 0)))
	{
		if (LockedRect.Pitch)
		{
			// Copy depth frame to face tracking
			memcpy(m_depthBuffer->GetBuffer(), PBYTE(LockedRect.pBits), min(m_depthBuffer->GetBufferSize(), UINT(pTexture->BufferLen())));

			// Register to RGB image
			if (m_options.m_rgbOptions.isValid() && m_options.m_registerStreams)
			{
				registerDepthData((USHORT*)m_depthBuffer->GetBuffer(), (USHORT*)m_depthBuffer->GetBuffer(), (USHORT*)m_registeredDepthBuffer->GetBuffer(),
					m_options.m_depthOptions.m_width, m_options.m_depthOptions.m_height, m_options.m_rgbOptions.m_width, m_options.m_rgbOptions.m_height);
				// Convert to non-player-id-depth
				m_registeredDepthBuffer->CopyTo(m_convertedDepthBuffer, 0, 0, 0);
				// And extract player id
				extractPlayerID(m_registeredDepthBuffer, m_playerPixelBuffer);
			}
			else
			{
				// Convert to non-player-id-depth
				m_depthBuffer->CopyTo(m_convertedDepthBuffer, 0, 0, 0);
				// And extract player id
				extractPlayerID(m_depthBuffer, m_playerPixelBuffer);
				// In this case, we need to register the player pixel buffer for the color image
				if (m_options.m_rgbOptions.isValid())
					registerDepthData((USHORT*)m_depthBuffer->GetBuffer(), (USHORT*)m_playerPixelBuffer->GetBuffer(), (USHORT*)m_playerPixelBufferColor->GetBuffer(),
						m_options.m_depthOptions.m_width, m_options.m_depthOptions.m_height, m_options.m_rgbOptions.m_width, m_options.m_rgbOptions.m_height, true);
			}

			m_imageDataNew = true;
		}

		// Release texture
		pTexture->UnlockRect(0);
		// Release frame
		m_pNuiSensor->NuiImageStreamReleaseFrame(m_pDepthStreamHandle, &pImageFrame);
	}
}

void FubiKinectSDKSensor::registerDepthData(USHORT* depthBuffer, USHORT* srcBuffer, USHORT* dstBuffer,
	int depthWidth, int depthHeight, int colorWidth, int colorHeight, bool isPlayerPixelBuffer /*= false*/)
{
	if (SUCCEEDED(m_pNuiSensor->NuiImageGetColorPixelCoordinateFrameFromDepthPixelFrameAtResolution(
		m_colorRes, m_depthRes, depthWidth*depthHeight, depthBuffer, depthWidth*depthHeight * 2, m_depthToColorMap)))
	{
		memset(dstBuffer, 0, depthWidth*depthHeight*sizeof(USHORT));

#pragma omp parallel for schedule(dynamic)
		for (LONG y=0; y < colorHeight; ++y)
		{
			USHORT z, fullZ;
			LONG mapX, mapY;
			int depthIndex;
			USHORT* dst;
			for (LONG x=0; x < colorWidth; ++x)
			{
				depthIndex = ftoi_r(x / m_colorToDepthDivisor + y / m_colorToDepthDivisor * depthWidth);
				mapX = m_depthToColorMap[depthIndex * 2];
				mapY = m_depthToColorMap[depthIndex * 2 + 1];
				// Only copy to the registered image if not out of bounds
				if (mapX >= 0 && mapX < depthWidth && mapY >= 0 && mapY <= depthHeight)
				{
					fullZ = srcBuffer[LONG(y/m_colorToDepthDivisor*depthWidth+x/m_colorToDepthDivisor)];
					dst = dstBuffer + (mapY*depthWidth+mapX);
					if (isPlayerPixelBuffer)
					{
						// Player id already extracted
						*dst = fullZ;
					}
					else
					{
						z = fullZ >> 3;
						// Only take the closest value as there might be multiple values per coordinate
						if (*dst == 0 || (z > 0 && (*dst >> 3) > z))
							*dst = fullZ;
					}
				}
			}
		}
	}
}

void FubiKinectSDKSensor::extractPlayerID(IFTImage* depthImage, IFTImage* pixelImage)
{
	if (depthImage->GetFormat() != FTIMAGEFORMAT_UINT16_D13P3)
		return;
	int width = (int)depthImage->GetWidth();
	int height = (int)depthImage->GetHeight();
	unsigned short* depthStart = (unsigned short*) depthImage->GetBuffer();
	unsigned short* playStart = (unsigned short*) pixelImage->GetBuffer();
#pragma omp parallel for schedule(dynamic)
	for (int y = 0; y < height; ++y)
	{
		unsigned short* depthP = depthStart + y*width;
		unsigned short* playP = playStart + y*width;
		for (int x = 0; x < width; ++x)
		{
			// Extract only the 3 lowest bits as they are the player id
			*playP = (*depthP) & 0x7;
			++depthP;
			++playP;
		}
	}
}

void FubiKinectSDKSensor::gotSkeletonAlert()
{
	NUI_SKELETON_FRAME SkeletonFrame = {0};

	HRESULT hr = m_pNuiSensor->NuiSkeletonGetNextFrame(0, &SkeletonFrame);
	if(FAILED(hr))
	{		
		return;
	}

	if (m_currTrackingFrameID != SkeletonFrame.dwFrameNumber)
	{
		m_currTrackingFrameID = SkeletonFrame.dwFrameNumber;

		for( int i = 0 ; i < NUI_SKELETON_COUNT ; ++i )
		{
			if( SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED)
			{
				m_skeletonTracked[i] = true;
				NUI_SKELETON_BONE_ORIENTATION boneOrientations[NUI_SKELETON_POSITION_COUNT];
				if (SUCCEEDED(NuiSkeletonCalculateBoneOrientations(&SkeletonFrame.SkeletonData[i], boneOrientations)))
				{
					for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; ++j)
					{
						m_skelPos[i][j].x = SkeletonFrame.SkeletonData[i].SkeletonPositions[j].x;
						m_skelPos[i][j].y = SkeletonFrame.SkeletonData[i].SkeletonPositions[j].y;
						m_skelPos[i][j].z = SkeletonFrame.SkeletonData[i].SkeletonPositions[j].z;

						memcpy(&m_skelRot[i][j].M11, &boneOrientations[j].absoluteRotation.rotationMatrix.M11, sizeof(float)* 16);

						m_skelConf[i][j] = SkeletonFrame.SkeletonData[i].eSkeletonPositionTrackingState[j] / 2.0f;
					}
				}
			}
			else
			{
				m_skeletonTracked[i] = false;
			}
		}
	}
}

void FubiKinectSDKSensor::selectUsersToTrack(UINT nbUsers, FTHelperContext* pUserContexts)
{
	// First get the two closest users
	bool skeletonIsAvailable[NUI_SKELETON_COUNT];
	for (UINT i=0; i<NUI_SKELETON_COUNT; ++i)
	{
		skeletonIsAvailable[i] = false;
	}
	std::deque<unsigned int> closestUsers = Fubi::getClosestUserIDs(nbUsers); // note Fubi ids start from 1!
	for (UINT i=0; i<closestUsers.size(); ++i)
	{
		skeletonIsAvailable[closestUsers[i]-1] = true;
	}

	// Now check for already face tracked users and remove them from the available list if not lost
	for (UINT i=0; i<nbUsers; ++i)
	{
		if (pUserContexts[i].m_CountUntilFailure > 0) // currently face tracked
		{
			if (m_skeletonTracked[pUserContexts[i].m_SkeletonId]) // and body is tracked
			{
				pUserContexts[i].m_CountUntilFailure = min(5, pUserContexts[i].m_CountUntilFailure+1);
			}
			else	// Body tracking failed
			{
				pUserContexts[i].m_CountUntilFailure--;
			}

			if (pUserContexts[i].m_CountUntilFailure > 0) // still trying to track, so user is not yet available
			{
				skeletonIsAvailable[pUserContexts[i].m_SkeletonId] = false;
			}
		}
	}

	for (UINT i=0; i<nbUsers; ++i)
	{
		if (pUserContexts[i].m_CountUntilFailure == 0)	// currently no users chosen
		{
			for (UINT j=0; j<NUI_SKELETON_COUNT; ++j)
			{
				if (skeletonIsAvailable[j])	// there is still one of the closest users left, so take him
				{
					pUserContexts[i].m_SkeletonId = j;
					pUserContexts[i].m_CountUntilFailure = 1;
					skeletonIsAvailable[j] = false;
					break;
				}
			}
			if (pUserContexts[i].m_CountUntilFailure == 0)
			{
				if (pUserContexts[i].m_SkeletonId != 0)
				{
					m_headTracked[m_userContext[i].m_SkeletonId] = false;
					m_faceTracked[m_userContext[i].m_SkeletonId] = false;
					m_face2DTracked[m_userContext[i].m_SkeletonId] = false;
				}
				pUserContexts[i].m_SkeletonId = 0;
				m_userContext[i].m_LastTrackSucceeded = false;
			}
		}
	}
}

void FubiKinectSDKSensor::update()
{
	m_hasNewData = m_lastTrackingFrameID != m_currTrackingFrameID;
	m_lastTrackingFrameID = m_currTrackingFrameID;

	HRESULT hrFT = S_OK;
	// Get new stream data
	if (m_videoBuffer && m_depthBuffer && m_imageDataNew)
	{
		m_imageDataNew = false;
		FT_SENSOR_DATA sensorData(m_videoBuffer, m_depthBuffer, m_zoomFactor, &m_viewOffset);
		selectUsersToTrack(FUBI_KINECT_SDK_MAX_NUM_FACES_TRACKED, m_userContext);
		for (UINT i=0; i<FUBI_KINECT_SDK_MAX_NUM_FACES_TRACKED; ++i)
		{
			if(m_userContext[i].m_CountUntilFailure > 0) // user has been selected
			{
				m_headTracked[m_userContext[i].m_SkeletonId] = false;
				m_faceTracked[m_userContext[i].m_SkeletonId] = false;
				m_face2DTracked[m_userContext[i].m_SkeletonId] = false;

				FT_VECTOR3D hint[2];
				hint[0] =  m_skelPos[m_userContext[i].m_SkeletonId][NUI_SKELETON_POSITION_SHOULDER_CENTER];
				hint[1] =  m_skelPos[m_userContext[i].m_SkeletonId][NUI_SKELETON_POSITION_HEAD];

				if (m_userContext[i].m_LastTrackSucceeded)
				{
					hrFT = m_userContext[i].m_pFaceTracker->ContinueTracking(&sensorData, hint, m_userContext[i].m_pFTResult);
				}
				else
				{
					hrFT = m_userContext[i].m_pFaceTracker->StartTracking(&sensorData, NULL, hint, m_userContext[i].m_pFTResult);
				}
				m_userContext[i].m_LastTrackSucceeded = SUCCEEDED(hrFT) && SUCCEEDED(m_userContext[i].m_pFTResult->GetStatus());
				if (m_userContext[i].m_LastTrackSucceeded)
				{
					// Store head orientation
					static FLOAT scale;
					hrFT = m_userContext[i].m_pFTResult->Get3DPose(&scale, m_headOrient[m_userContext[i].m_SkeletonId], m_headPos[m_userContext[i].m_SkeletonId]);
					if (SUCCEEDED(hrFT))
					{
						m_headTracked[m_userContext[i].m_SkeletonId]  = true;

						IFTModel* ftModel;
						HRESULT hr = m_userContext[i].m_pFaceTracker->GetFaceModel(&ftModel);
						if (SUCCEEDED(hr))
						{
							FLOAT* pAUCOeffs;
							UINT pAUCOunt;
							m_userContext[i].m_pFTResult->GetAUCoefficients(&pAUCOeffs, &pAUCOunt);

							FLOAT* pSU = NULL;
							UINT numSU;
							BOOL suConverged;
							m_userContext[i].m_pFaceTracker->GetShapeUnits(NULL, &pSU, &numSU, &suConverged);

							FT_VECTOR2D* pPts2D;
							UINT pts2DCount;
							m_userContext[i].m_pFTResult->Get2DShapePoints(&pPts2D, &pts2DCount);
							if (pts2DCount <= 121)
							{
								m_face2DTracked[m_userContext[i].m_SkeletonId] = true;
								for(UINT j = 0; j < pts2DCount; ++j)
								{
									m_face2DPos[m_userContext[i].m_SkeletonId][j] = pPts2D[j];
								}
							}
							else
							{
								static double lastWarning = -99;
								if (Fubi::currentTime() - lastWarning > 10)
								{
									Fubi_logErr("Error in face tracking - face point count does not match!\n");
									lastWarning = Fubi::currentTime();
								}
							}


							UINT vertexCount = ftModel->GetVertexCount();
							m_convertedFacePos[m_userContext[i].m_SkeletonId].resize(vertexCount);
							if (SUCCEEDED(ftModel->Get3DShape(pSU, numSU, pAUCOeffs, pAUCOunt, scale, m_headOrient[m_userContext[i].m_SkeletonId], m_headPos[m_userContext[i].m_SkeletonId], m_facePos[m_userContext[i].m_SkeletonId], vertexCount)))
							{
								m_faceTracked[m_userContext[i].m_SkeletonId] = true;
								for (UINT j = 0; j < vertexCount; ++j)
								{
									m_convertedFacePos[m_userContext[i].m_SkeletonId][j] = NUIVec3ToVec3f(m_facePos[m_userContext[i].m_SkeletonId][j]);
								}
								FT_TRIANGLE* pTriangles;
								UINT triangleCount;
								if (SUCCEEDED(ftModel->GetTriangles(&pTriangles, &triangleCount)))
								{
									m_faceTriangleIndices[m_userContext[i].m_SkeletonId].resize(triangleCount);
									for (UINT j = 0; j < triangleCount; ++j)
									{
										m_faceTriangleIndices[m_userContext[i].m_SkeletonId][j] = NUITriangleIndexToVec3f(pTriangles[j]);
									}
								}
							}
						}
					}
				}
				else
				{
					m_userContext[i].m_pFTResult->Reset();
				}
			}
		}
	}	
}


bool FubiKinectSDKSensor::hasNewTrackingData()
{
	return m_hasNewData;
}

bool FubiKinectSDKSensor::isTracking(unsigned int id)
{
	return m_skeletonTracked[id-1];
}

void FubiKinectSDKSensor::getSkeletonJointData(unsigned int id, Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJointPosition& position, Fubi::SkeletonJointOrientation& orientation)
{
	unsigned int userIndex = id-1;
	// Special case for torso
	if (joint == SkeletonJoint::TORSO)
	{
		if (m_seatedSkeletonMode)
		{
			// Take the neck data for the torso
			getSkeletonJointData(id, SkeletonJoint::NECK, position, orientation);
			// But lower the position a bit according to its orientation
			Vec3f localPos(0, -250.0f, 0); // 250 mm lower in local coordinate system
			// Transform local position back to global positions in place
			Fubi::calculateGlobalPosition(localPos, position.m_position, orientation.m_orientation, position.m_position);
		}
		else
		{
			// Position is middle between hip and shoulder center
			Vec3f hips;
			hips.x = m_skelPos[userIndex][NUI_SKELETON_POSITION_HIP_CENTER].x * 1000.0f;
			hips.y = m_skelPos[userIndex][NUI_SKELETON_POSITION_HIP_CENTER].y * 1000.0f;
			hips.z = m_skelPos[userIndex][NUI_SKELETON_POSITION_HIP_CENTER].z * 1000.0f;
			Vec3f shoulders;
			shoulders.x = m_skelPos[userIndex][NUI_SKELETON_POSITION_SHOULDER_CENTER].x * 1000.0f;
			shoulders.y = m_skelPos[userIndex][NUI_SKELETON_POSITION_SHOULDER_CENTER].y * 1000.0f;
			shoulders.z = m_skelPos[userIndex][NUI_SKELETON_POSITION_SHOULDER_CENTER].z * 1000.0f;
			position.m_position = hips + (shoulders-hips)*0.5f;
			position.m_confidence = minf(m_skelConf[userIndex][NUI_SKELETON_POSITION_SHOULDER_CENTER], m_skelConf[userIndex][NUI_SKELETON_POSITION_HIP_CENTER]);
			// Rotation from spine
			NUIMatrix4ToMatrix3f(m_skelRot[userIndex][NUI_SKELETON_POSITION_SPINE], orientation.m_orientation);
			orientation.m_confidence = m_skelConf[userIndex][NUI_SKELETON_POSITION_SPINE];
		}
	}
	else if (joint >= SkeletonJoint::FACE_NOSE && joint <= SkeletonJoint::FACE_CHIN) // Special case for face
	{
		// First get head data as a basis
		getSkeletonJointData(id, SkeletonJoint::HEAD, position, orientation);
		if (m_headTracked[userIndex] && m_faceTracked[userIndex])
		{
			int fpIndex = JointToKSDKFacePointIndex(joint);
			// Now replace positions with face positions
			position.m_position.x = m_facePos[userIndex][fpIndex].x * 1000.0f;
			position.m_position.y = m_facePos[userIndex][fpIndex].y * 1000.0f;
			position.m_position.z = m_facePos[userIndex][fpIndex].z * 1000.0f;
		}
		else
		{
			position.m_confidence = 0.25f;
		}
	}
	else	// Default case
	{
		NUI_SKELETON_POSITION_INDEX index = JointToKSDKJoint(joint);
		position.m_confidence = m_skelConf[userIndex][index];
		position.m_position.x = m_skelPos[userIndex][index].x * 1000.0f;
		position.m_position.y = m_skelPos[userIndex][index].y * 1000.0f;
		position.m_position.z = m_skelPos[userIndex][index].z * 1000.0f;
		orientation.m_confidence = m_skelConf[userIndex][index];
		NUIMatrix4ToMatrix3f(m_skelRot[userIndex][index], orientation.m_orientation);

		// Fallback cases for feet and hands if current confidence too low
		if (position.m_confidence < 0.4f 
			&& (index == NUI_SKELETON_POSITION_FOOT_LEFT || index == NUI_SKELETON_POSITION_FOOT_RIGHT 
			|| index == NUI_SKELETON_POSITION_HAND_LEFT || index == NUI_SKELETON_POSITION_HAND_RIGHT))
		{
			index = JointToKSDKFallbackJoint(joint);
			position.m_confidence = m_skelConf[userIndex][index];
			position.m_position.x = m_skelPos[userIndex][index].x * 1000.0f;
			position.m_position.y = m_skelPos[userIndex][index].y * 1000.0f;
			position.m_position.z = m_skelPos[userIndex][index].z * 1000.0f;
			orientation.m_confidence = m_skelConf[userIndex][index];
			NUIMatrix4ToMatrix3f(m_skelRot[userIndex][index], orientation.m_orientation);
		}

		// Special case for head with face tracking activated
		if (index == NUI_SKELETON_POSITION_HEAD && m_headTracked[userIndex])
		{
			// Replace the orientation (currently no useful one) with the one from the face tracking
			orientation.m_orientation = Matrix3f(Quaternion(degToRad(m_headOrient[userIndex][0]), degToRad(m_headOrient[userIndex][1]), degToRad(m_headOrient[userIndex][2])));
			orientation.m_confidence = 1.0f; // No real confidence value, but the head is currently tracked, so this should be fine
			// Head postion by the face tracking differs significantly from that of the normal tracking
			// --> We currently don't use it...
			/*position.m_position.x = m_headPos[userIndex][0] * 1000.0f;
			position.m_position.y = m_headPos[userIndex][1] * 1000.0f;
			position.m_position.z = m_headPos[userIndex][2] * 1000.0f;
			position.m_confidence = 1.0f;*/

		}
	}
}

const unsigned short* FubiKinectSDKSensor::getDepthData()
{
	if (m_options.m_depthOptions.isValid())
	{
		if (m_convertedDepthBuffer)
			return (unsigned short*)m_convertedDepthBuffer->GetBuffer();
	}
	return 0x0;
}

const unsigned char* FubiKinectSDKSensor::getRgbData()
{
	if (m_convertedVideoBuffer && m_options.m_rgbOptions.isValid())
		return m_convertedVideoBuffer->GetBuffer();
	return 0x0;
}

const unsigned short* FubiKinectSDKSensor::getIrData()
{
	if (m_irBuffer && m_options.m_irOptions.isValid())
		return m_irBuffer;
	return 0x0;
}

const unsigned short* FubiKinectSDKSensor::getUserLabelData(Fubi::CoordinateType::Type coordType)
{
	if (m_options.m_depthOptions.isValid() && getUserIDs(0x0) > 0)
	{
		if ((coordType == CoordinateType::DEPTH || m_options.m_registerStreams) && m_playerPixelBuffer)
			return (unsigned short*)m_playerPixelBuffer->GetBuffer();
		else if ((coordType == CoordinateType::COLOR || coordType == CoordinateType::IR) && !m_options.m_registerStreams && m_playerPixelBufferColor)
			return (unsigned short*)m_playerPixelBufferColor->GetBuffer();
		// TODO: REAL_WORLD
	}
	return 0x0;
}

unsigned short FubiKinectSDKSensor::getUserIDs(unsigned int* userIDs)
{
	int index = 0;
	for (int i = 0; i < NUI_SKELETON_COUNT; ++i)
	{
		if (m_skeletonTracked[i])
		{
			if (userIDs)
				userIDs[index] = i+1;
			++index;
		}
	}
	return index;
}

Fubi::Vec3f FubiKinectSDKSensor::convertCoordinates(const Fubi::Vec3f& inputCoords, Fubi::CoordinateType::Type inputType, Fubi::CoordinateType::Type outputType)
{
	INuiCoordinateMapper* mapper = 0x0;
	if (SUCCEEDED(m_pNuiSensor->NuiGetCoordinateMapper(&mapper)))
	{
		Vec3f ret(Math::NO_INIT);

		bool success = false;
		
		if (outputType == CoordinateType::DEPTH)
		{
			if (inputType == CoordinateType::REAL_WORLD)
			{
				Vector4 inputVec;
				inputVec.x = inputCoords.x / 1000.0f;
				inputVec.y = inputCoords.y / 1000.0f;
				inputVec.z = inputCoords.z / 1000.0f;
				NUI_DEPTH_IMAGE_POINT depthPoint;
				success = SUCCEEDED(mapper->MapSkeletonPointToDepthPoint(&inputVec, m_depthRes, &depthPoint));
				ret.x = (float) depthPoint.x;
				ret.y = (float) depthPoint.y;
				ret.z = (float) depthPoint.depth;
			}
			else if (inputType == CoordinateType::COLOR || inputType == CoordinateType::IR)
			{
				/*Vec3f realWorldVec = convertCoordinates(inputCoords, inputType, CoordinateType::REAL_WORLD);
				ret = convertCoordinates(realWorldVec, CoordinateType::REAL_WORLD, outputType);
				success = true;*/
				// Dummy conversion will be done in base class!
			}
		}
		else if (outputType == CoordinateType::REAL_WORLD)
		{
			if (inputType == CoordinateType::DEPTH)
			{
				Vector4 skelPoint;
				NUI_DEPTH_IMAGE_POINT depthPoint;
				depthPoint.x = (LONG) inputCoords.x;
				depthPoint.y = (LONG) inputCoords.y;
				depthPoint.depth = (LONG) (inputCoords.z / 1000.0f);
				success = SUCCEEDED(mapper->MapDepthPointToSkeletonPoint(m_depthRes, &depthPoint, &skelPoint));
				if (success)
				{
					ret.x = skelPoint.x * 1000.0f;
					ret.y = skelPoint.y * 1000.0f;
					ret.z = skelPoint.z * 1000.0f;
				}
			}
			else if (inputType == CoordinateType::COLOR || inputType == CoordinateType::IR)
			{
				// Dummy conversion will be done in base class!
			}

		}
		else // Color or IR
		{
			NUI_COLOR_IMAGE_POINT colorPoint;
			if (inputType == CoordinateType::REAL_WORLD)
			{
				Vector4 inputVec;
				inputVec.x = inputCoords.x / 1000.0f;
				inputVec.y = inputCoords.y / 1000.0f;
				inputVec.z = inputCoords.z / 1000.0f;
				success = SUCCEEDED(mapper->MapSkeletonPointToColorPoint(&inputVec, (outputType == CoordinateType::COLOR) ? NUI_IMAGE_TYPE_COLOR : NUI_IMAGE_TYPE_COLOR_INFRARED, m_colorRes, &colorPoint));
				ret.x = (float) colorPoint.x;
				ret.y = (float) colorPoint.y;
				ret.z = inputCoords.z;
			}
			else if (inputType == CoordinateType::DEPTH)
			{
				NUI_DEPTH_IMAGE_POINT depthPoint;
				depthPoint.x = (LONG) inputCoords.x;
				depthPoint.y = (LONG) inputCoords.y;
				depthPoint.depth = (LONG) (inputCoords.z / 1000.0f);
				success = SUCCEEDED(mapper->MapDepthPointToColorPoint(m_depthRes, &depthPoint, (outputType == CoordinateType::COLOR) ? NUI_IMAGE_TYPE_COLOR : NUI_IMAGE_TYPE_COLOR_INFRARED, m_colorRes, &colorPoint));
				ret.x = (float) colorPoint.x;
				ret.y = (float) colorPoint.y;
				ret.z = inputCoords.z;
			}
		}

		if (success)
			return ret;
	}
	return FubiISensor::convertCoordinates(inputCoords, inputType, outputType);
}

bool FubiKinectSDKSensor::getFacePoints(unsigned int id, const std::vector<Fubi::Vec3f>** vertices, const std::vector<Fubi::Vec3f>** triangleIndices)
{
	if (id > 0 &&  m_faceTracked[id-1])
	{
		*vertices = &m_convertedFacePos[id - 1];

		*triangleIndices = &m_faceTriangleIndices[id - 1];
		return true;
	}
	return false;
}
#endif