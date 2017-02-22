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
#pragma once

#include "FubiConfig.h"

#ifdef FUBI_USE_KINECT_SDK
#include "FubiISensor.h"

#include <FaceTrackLib.h>
#include <NuiApi.h>

// The FubiKinectSDKSensor class is responsible for integrating the Kinect SDK
class FubiKinectSDKSensor : public FubiISensor
{
public:
	FubiKinectSDKSensor();
	virtual ~FubiKinectSDKSensor();

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

	// Get the current tracked face points of a user
	virtual bool getFacePoints(unsigned int id, const std::vector<Fubi::Vec3f>** vertices, const std::vector<Fubi::Vec3f>** triangleIndices);

	// Get Stream data
	virtual const unsigned short* getDepthData();
	virtual const unsigned char* getRgbData();
	virtual const unsigned short* getIrData();
	virtual const unsigned short* getUserLabelData(Fubi::CoordinateType::Type coordType);

	// Get the floor plane
	//virtual Fubi::Plane getFloor();

	// Resets the tracking of a users
	//virtual void resetTracking(unsigned int id);

	// Convert coordinates between real world, depth, color, or IR image
	// inputType should be != outputType
	virtual Fubi::Vec3f convertCoordinates(const Fubi::Vec3f& inputCoords, Fubi::CoordinateType::Type inputType, Fubi::CoordinateType::Type outputType);

private:
	struct FTHelperContext
	{
		IFTFaceTracker*     m_pFaceTracker;
		IFTResult*          m_pFTResult;
		FT_VECTOR3D         m_hint3D[2];
		bool                m_LastTrackSucceeded;
		int                 m_CountUntilFailure;
		UINT                m_SkeletonId;
	};

	HRESULT     getVideoConfiguration(FT_CAMERA_CONFIG* videoConfig);
	HRESULT     getDepthConfiguration(FT_CAMERA_CONFIG* depthConfig);

	static DWORD WINAPI processThread(PVOID pParam);
	
	void gotVideoAlert();
	void gotDepthAlert();
	void gotSkeletonAlert();

	HRESULT getClosestHint(FT_VECTOR3D* pHint3D);
	void selectUsersToTrack(UINT nbUsers, FTHelperContext* pUserContexts);

	void extractPlayerID(IFTImage* depthImage, IFTImage* pixelImage);
	void registerDepthData(USHORT* depthBuffer, USHORT* srcBuffer, USHORT* dstBuffer, int depthWidth, int depthHeight, int colorWidth, int colorHeight, bool isPlayerPixelBuffer = false);

	IFTImage*		m_videoBuffer;
	IFTImage*		m_depthBuffer;
	IFTImage*		m_convertedVideoBuffer;
	IFTImage*		m_convertedDepthBuffer;
	IFTImage*		m_registeredDepthBuffer;
	IFTImage		*m_playerPixelBuffer, *m_playerPixelBufferColor;
	unsigned short*	m_irBuffer;
	LONG*           m_depthToColorMap;
	double          m_colorToDepthDivisor;

	FT_VECTOR3D m_skelPos[NUI_SKELETON_COUNT][NUI_SKELETON_POSITION_COUNT];
	Matrix4		m_skelRot[NUI_SKELETON_COUNT][NUI_SKELETON_POSITION_COUNT];
	float		m_skelConf[NUI_SKELETON_COUNT][NUI_SKELETON_POSITION_COUNT];
	bool        m_skeletonTracked[NUI_SKELETON_COUNT];
	bool		m_imageDataNew;

	float		m_headOrient[NUI_SKELETON_COUNT][3];
	float		m_headPos[NUI_SKELETON_COUNT][3];
	bool		m_headTracked[NUI_SKELETON_COUNT];
	FT_VECTOR3D m_facePos[NUI_SKELETON_COUNT][121];
	std::vector<Fubi::Vec3f> m_convertedFacePos[NUI_SKELETON_COUNT];
	std::vector<Fubi::Vec3f> m_faceTriangleIndices[NUI_SKELETON_COUNT];
	FT_VECTOR2D m_face2DPos[NUI_SKELETON_COUNT][121];
	bool		m_faceTracked[NUI_SKELETON_COUNT];
	bool		m_face2DTracked[NUI_SKELETON_COUNT];
	FTHelperContext	m_userContext[FUBI_KINECT_SDK_MAX_NUM_FACES_TRACKED];

	FLOAT       m_zoomFactor;   // video frame zoom factor (it is 1.0f if there is no zoom)
	POINT       m_viewOffset;   // Offset of the view from the top left corner.
	NUI_IMAGE_RESOLUTION m_depthRes;
	NUI_IMAGE_RESOLUTION m_colorRes;

	HANDLE      m_hNextDepthFrameEvent;
	HANDLE      m_hNextVideoFrameEvent;
	HANDLE      m_hNextSkeletonEvent;
	HANDLE      m_pDepthStreamHandle;
	HANDLE      m_pVideoStreamHandle;
	HANDLE      m_hThNuiProcess;
	HANDLE      m_hEvNuiProcessStop;

	INuiSensor*	m_pNuiSensor;

	bool        m_bNuiInitialized;
	bool		m_seatedSkeletonMode;
	bool		m_hasNewData;
	DWORD		m_lastTrackingFrameID;
	DWORD		m_currTrackingFrameID;
};

#endif