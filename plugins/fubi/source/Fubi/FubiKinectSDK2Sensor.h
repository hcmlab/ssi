// ****************************************************************************************
//
// Fubi Kinect SDK 2 sensor
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler 
// 
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
// 
// ****************************************************************************************
#pragma once

#include "FubiConfig.h"

#ifdef FUBI_USE_KINECT_SDK_2
#include "FubiISensor.h"

#include <Kinect.h>
#include <Kinect.Face.h>


// The FubiKinectSDK2Sensor class is responsible for integrating the Kinect SDK
class FubiKinectSDK2Sensor : public FubiISensor
{
public:
	FubiKinectSDK2Sensor();
	virtual ~FubiKinectSDK2Sensor();

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

	virtual inline int getMaxRawStreamValue(Fubi::ImageType::Type imageType)
	{
		switch (imageType)
		{
		case Fubi::ImageType::IR:
			return Fubi::Math::MaxUShort16;
		case Fubi::ImageType::Depth:
			return Fubi::MaxDepth;
		}
		return 255;
	}

private:
	void release();
	static DWORD WINAPI frameEventThread(LPVOID pParam);

	// Current Kinect
	IKinectSensor*				m_kinectSensor;
	ICoordinateMapper*			m_coordinateMapper;

	// Image data
	IMultiSourceFrameReader*	m_multiSourceFrameReader;
	DepthSpacePoint*			m_colorIndexToDepthPointMap;
	unsigned short				*m_depthBuffer, *m_irBuffer;
	unsigned short				*m_userLabelBuffer, *m_userLabelColorBuffer;
	unsigned char*				m_rgbBuffer;
	RGBQUAD*					m_rgbxBuffer;
	Fubi::Vec3f					m_depthToColorScale;

	// Tracking
	IBody*						m_bodies[BODY_COUNT];
	Joint						m_joints[BODY_COUNT][JointType_Count];
	JointOrientation			m_jointOrients[BODY_COUNT][JointType_Count];
	bool						m_hasNewData;

	// Face tracking
	struct FaceInfo
	{
		IHighDefinitionFaceFrameSource*		source;
		IHighDefinitionFaceFrameReader*		reader;
		BOOLEAN								isTracked;
		IFaceAlignment*						alignment;
		FaceAlignmentQuality				alignmentQuality;
		Vector4								orientation;
		CameraSpacePoint*					vertices;
		std::vector<Fubi::Vec3f>			convertedVertices;
		unsigned int						numVerts;
		FaceInfo()
		{
			source = 0x0;
			reader = 0x0;
			isTracked = false;
			alignment = 0x0;
			alignmentQuality = FaceAlignmentQuality_Low;
			orientation = { 0 };
			numVerts = 0;
			vertices = 0x0;
		};
	};
	FaceInfo m_faces[BODY_COUNT];
	
	unsigned int				m_faceVertexCount;
	IFaceModel*					m_faceModel;
	std::vector<Fubi::Vec3f>	m_faceTriangles;

	HANDLE						m_frameEventThreadHandle, m_frameEventStop;
	WAITABLE_HANDLE				m_frameEvent;
	bool						m_frameArrived;
};

#endif