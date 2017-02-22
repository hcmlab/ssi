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

#include "FubiKinectSDK2Sensor.h"

#ifdef FUBI_USE_KINECT_SDK_2

#pragma comment(lib, "kinect20.lib")
#pragma comment(lib, "Kinect20.Face.lib")

#include "FubiUser.h"
#include "Fubi.h"

using namespace Fubi;
using namespace std;

static JointType JointToKSDK2Joint(const SkeletonJoint::Joint j)
{
	switch (j)
	{
	case SkeletonJoint::HEAD:
		return JointType_Head;
	case SkeletonJoint::NECK:
		return JointType_Neck;
	case SkeletonJoint::TORSO:
		return JointType_SpineMid;
	case SkeletonJoint::LEFT_SHOULDER:
		return JointType_ShoulderLeft;
	case SkeletonJoint::LEFT_ELBOW:
		return JointType_ElbowLeft;
	case SkeletonJoint::LEFT_WRIST:
		return JointType_WristLeft;
	case SkeletonJoint::LEFT_HAND:
		return JointType_HandLeft;
	case SkeletonJoint::RIGHT_SHOULDER:
		return JointType_ShoulderRight;
	case SkeletonJoint::RIGHT_ELBOW:
		return JointType_ElbowRight;
	case SkeletonJoint::RIGHT_WRIST:
		return JointType_WristRight;
	case SkeletonJoint::RIGHT_HAND:
		return JointType_HandRight;
	case SkeletonJoint::LEFT_HIP:
		return JointType_HipLeft;
	case SkeletonJoint::LEFT_KNEE:
		return JointType_KneeLeft;
	case SkeletonJoint::LEFT_ANKLE:
		return JointType_AnkleLeft;
	case SkeletonJoint::LEFT_FOOT:
		return JointType_FootLeft;
	case SkeletonJoint::RIGHT_HIP:
		return JointType_HipRight;
	case SkeletonJoint::RIGHT_KNEE:
		return JointType_KneeRight;
	case SkeletonJoint::RIGHT_ANKLE:
		return JointType_AnkleRight;
	case SkeletonJoint::RIGHT_FOOT:
		return JointType_FootRight;
	case SkeletonJoint::WAIST:
		return JointType_SpineBase;
	default:
		return JointType_Count;
	}
	return JointType_Count;
}

static JointType JointToKSDK2FallbackJoint(const SkeletonJoint::Joint j)
{
	switch (j)
	{
	case SkeletonJoint::LEFT_HAND:
		return JointType_WristLeft;
	case SkeletonJoint::RIGHT_HAND:
		return JointType_WristRight;
	case SkeletonJoint::LEFT_FOOT:
		return JointType_AnkleLeft;
	case SkeletonJoint::RIGHT_FOOT:
		return JointType_AnkleRight;
	default:
		return JointType_Count;
	}
	return JointType_Count;
}

static HighDetailFacePoints JointToKSDKFacePoint(const SkeletonJoint::Joint j)
{
	switch (j)
	{
	case SkeletonJoint::FACE_NOSE:
		return HighDetailFacePoints_NoseTip;
	case SkeletonJoint::FACE_LEFT_EAR:
		return HighDetailFacePoints_LowerjawLeftend;
	case SkeletonJoint::FACE_RIGHT_EAR:
		return HighDetailFacePoints_LowerjawRightend;
	case SkeletonJoint::FACE_FOREHEAD:
		return HighDetailFacePoints_ForeheadCenter;
	case SkeletonJoint::FACE_CHIN:
		return HighDetailFacePoints_ChinCenter;
	}
	return HighDetailFacePoints_LowerjawRightend;
}

static inline Fubi::Vec3f CameraSpacePointToVec3f(const CameraSpacePoint& point)
{
	return Vec3f(point.X*1000.0f, point.Y*1000.0f, point.Z*1000.0f);
}

static inline Fubi::Matrix3f KSDK2OrientToMatrix3f(const Vector4& ksdk2Orient, bool rotateCoordinateSystem = true)
{
	// Convert coordinate system
	Vec3f rot = Matrix3f(Quaternion(ksdk2Orient.x, ksdk2Orient.y, ksdk2Orient.z, ksdk2Orient.w)).getRot(false);
	if (rotateCoordinateSystem)
	{
		rot.x *= -1.0f;
		rot.y -= degToRad(180.0f);
		rot.z *= -1.0f;
	}
	return Matrix3f::RotMat(rot);
}

FubiKinectSDK2Sensor::FubiKinectSDK2Sensor() :
m_kinectSensor(NULL), m_coordinateMapper(NULL),
m_multiSourceFrameReader(NULL), m_userLabelBuffer(0x0), m_userLabelColorBuffer(0x0),
m_rgbBuffer(0x0), m_rgbxBuffer(0x0), m_depthBuffer(0x0), m_irBuffer(0x0), m_colorIndexToDepthPointMap(0x0),
m_hasNewData(false), m_faceModel(0x0), m_faceVertexCount(0), m_depthToColorScale(1.f, 1.f, 1.f),
m_frameEvent(0x0), m_frameEventThreadHandle(0x0), m_frameArrived(false)
{
	for (int i = 0; i < BODY_COUNT; ++i)
	{
		m_bodies[i] = 0;
		for (int j = 0; j < JointType_Count; ++j)
		{
			m_joints[i][j].JointType = (JointType)j;
			m_joints[i][j].Position = { 0 };
			m_joints[i][j].TrackingState = TrackingState_NotTracked;
			m_jointOrients[i][j].JointType = (JointType)j;
			m_jointOrients[i][j].Orientation = { 0 };
		}
	}
	m_options.m_type = SensorType::KINECTSDK2;
}

void FubiKinectSDK2Sensor::release()
{
	if (m_multiSourceFrameReader)
	{
		// Stop the event thread
		if (m_frameEventStop != 0x0)
		{
			// Signal the thread
			SetEvent(m_frameEventStop);
			// Wait for thread to stop
			if (m_frameEventThreadHandle != 0x0)
			{
				WaitForSingleObject(m_frameEventThreadHandle, INFINITE);
				CloseHandle(m_frameEventThreadHandle);
				m_frameEventThreadHandle = 0x0;
			}
			CloseHandle(m_frameEventStop);
			m_frameEventStop = NULL;
		}
		m_multiSourceFrameReader->UnsubscribeMultiSourceFrameArrived(m_frameEvent);
		m_frameEvent = 0x0;
		m_multiSourceFrameReader->Release();
		m_multiSourceFrameReader = 0x0;
	}
	if (m_coordinateMapper)
	{
		m_coordinateMapper->Release();
		m_coordinateMapper = 0x0;
	}

	if (m_kinectSensor)
	{
		m_kinectSensor->Close();
		m_kinectSensor->Release();
		m_kinectSensor = 0x0;
	}
	for (int i = 0; i < BODY_COUNT; ++i)
	{
		if (m_bodies[i])
		{
			m_bodies[i]->Release();
			m_bodies[i] = 0x0;
		}
		if (m_faces[i].source)
		{
			m_faces[i].source->Release();
			m_faces[i].source = 0x0;
		}
		if (m_faces[i].reader)
		{
			m_faces[i].reader->Release();
			m_faces[i].reader = 0x0;
		}
		if (m_faces[i].alignment)
		{
			m_faces[i].alignment->Release();
			m_faces[i].alignment = 0x0;
		}
		if (m_faces[i].vertices)
		{
			delete[] m_faces[i].vertices;
			m_faces[i].vertices = 0x0;
		}
	}
	if (m_faceModel)
	{
		m_faceModel->Release();
		m_faceModel = 0x0;
	}

	delete[] m_colorIndexToDepthPointMap;
	m_colorIndexToDepthPointMap = 0x0;
	delete[] m_depthBuffer;
	m_depthBuffer = 0x0;
	delete[] m_irBuffer;
	m_irBuffer = 0x0;
	delete[] m_userLabelBuffer;
	m_userLabelBuffer = 0x0;
	delete[] m_userLabelColorBuffer;
	m_userLabelColorBuffer = 0x0;
	delete[] m_rgbxBuffer;
	m_rgbxBuffer = 0x0;
	delete[] m_rgbBuffer;
	m_rgbBuffer = 0x0;
}


bool FubiKinectSDK2Sensor::initWithOptions(const Fubi::SensorOptions& options)
{
	bool ret = false;

	release();

	m_options.m_depthOptions.invalidate();
	m_options.m_rgbOptions.invalidate();
	m_options.m_irOptions.invalidate();
	m_options.m_mirrorStreams = true;
	m_options.m_registerStreams = false;
	m_options.m_trackingProfile = SkeletonTrackingProfile::NONE;
	m_faceTriangles.clear();

	// To be on the safe side, we check all settings:
	// if they are only set somehow the stream will get activated with the default config
	DWORD frameTypes = 0;
	if (options.m_rgbOptions.isValid())
	{
		m_options.m_rgbOptions.m_width = 1920;
		m_options.m_rgbOptions.m_height = 1080;
		m_options.m_rgbOptions.m_fps = 30;
		frameTypes |= FrameSourceTypes::FrameSourceTypes_Color;
		m_rgbBuffer = new unsigned char[m_options.m_rgbOptions.m_width*m_options.m_rgbOptions.m_height * 3];
		memset(m_rgbBuffer, 0, m_options.m_rgbOptions.m_width*m_options.m_rgbOptions.m_height * 3 * sizeof(unsigned char));
		m_rgbxBuffer = new RGBQUAD[m_options.m_rgbOptions.m_width*m_options.m_rgbOptions.m_height];
	}
	if (options.m_irOptions.isValid())
	{
		m_options.m_irOptions.m_width = 512;
		m_options.m_irOptions.m_height = 424;
		m_options.m_irOptions.m_fps = 30;
		frameTypes |= FrameSourceTypes::FrameSourceTypes_Infrared;
		m_irBuffer = new unsigned short[m_options.m_irOptions.m_width*m_options.m_irOptions.m_height];
		memset(m_irBuffer, 0, m_options.m_irOptions.m_width*m_options.m_irOptions.m_height*sizeof(unsigned short));
	}
	if (options.m_depthOptions.isValid())
	{
		m_options.m_depthOptions.m_width = 512;
		m_options.m_depthOptions.m_height = 424;
		m_options.m_depthOptions.m_fps = 30;
		frameTypes |= FrameSourceTypes::FrameSourceTypes_Depth;

		int numPixels = m_options.m_depthOptions.m_width*m_options.m_depthOptions.m_height;

		m_depthBuffer = new unsigned short[numPixels];
		memset(m_depthBuffer, 0, numPixels*sizeof(unsigned short));

		if (options.m_trackingProfile != SkeletonTrackingProfile::NONE)
		{
			frameTypes |= FrameSourceTypes::FrameSourceTypes_BodyIndex | FrameSourceTypes::FrameSourceTypes_Body;
			m_userLabelBuffer = new unsigned short[numPixels];
			memset(m_userLabelBuffer, 0, numPixels*sizeof(unsigned short));
			m_userLabelColorBuffer = new unsigned short[numPixels];
			memset(m_userLabelColorBuffer, 0, numPixels*sizeof(unsigned short));
			m_options.m_trackingProfile = SkeletonTrackingProfile::ALL;
		}

		if (options.m_rgbOptions.isValid())
		{
			m_colorIndexToDepthPointMap = new DepthSpacePoint[m_options.m_rgbOptions.m_width*m_options.m_rgbOptions.m_height];
			memset(m_colorIndexToDepthPointMap, 0, 2 * sizeof(float) * m_options.m_rgbOptions.m_width*m_options.m_rgbOptions.m_height);
			m_options.m_registerStreams = options.m_registerStreams;
		}
	}

	if (m_options.m_depthOptions.isValid() && m_options.m_rgbOptions.isValid())
	{
		m_depthToColorScale.x = (float)(m_options.m_rgbOptions.m_width - 1) / (m_options.m_depthOptions.m_width - 1);
		m_depthToColorScale.y = (float)(m_options.m_rgbOptions.m_height - 1) / (m_options.m_depthOptions.m_height - 1);
	}

	if (SUCCEEDED(GetDefaultKinectSensor(&m_kinectSensor)) && m_kinectSensor)
	{
		// Initialize the Kinect and get coordinate mapper and the body reader
		if (SUCCEEDED(m_kinectSensor->Open()))
		{
			if (SUCCEEDED(m_kinectSensor->OpenMultiSourceFrameReader(frameTypes, &m_multiSourceFrameReader)))
			{

				if (FAILED(m_kinectSensor->get_CoordinateMapper(&m_coordinateMapper)))
					m_coordinateMapper = 0x0;

				// Face model init
				unsigned int triangleCount;
				if (SUCCEEDED(GetFaceModelVertexCount(&m_faceVertexCount)) && SUCCEEDED(GetFaceModelTriangleCount(&triangleCount)))
				{
					unsigned int* triangles = new unsigned int[triangleCount * 3];
					if (SUCCEEDED(GetFaceModelTriangles(triangleCount * 3, triangles)))
					{
						for (unsigned int i = 0; i < triangleCount; ++i)
							m_faceTriangles.push_back(Vec3f((float)triangles[i * 3], (float)triangles[i * 3 + 1], (float)triangles[i * 3 + 2]));
					}
					delete[] triangles;
					// Create a face frame source + reader to track each body in the fov
					for (int i = 0; i < BODY_COUNT; ++i)
					{
						if (SUCCEEDED(CreateHighDefinitionFaceFrameSource(m_kinectSensor, &m_faces[i].source)))
						{
							// open the corresponding reader
							if (FAILED(m_faces[i].source->OpenReader(&m_faces[i].reader)))
								Fubi_logInfo("FubiKinectSDK2Sensor: failed initinalizing face reader!\n");
							if (FAILED(CreateFaceAlignment(&m_faces[i].alignment)))
								Fubi_logInfo("FubiKinectSDK2Sensor: failed initinalizing face alignment!\n");
							float faceShapeDeformations[FaceShapeDeformations_Count];
							memset(faceShapeDeformations, 0, sizeof(faceShapeDeformations));
							if (FAILED(CreateFaceModel(1.0f, FaceShapeDeformations_Count, faceShapeDeformations, &m_faceModel)))
								Fubi_logInfo("FubiKinectSDK2Sensor: failed initinalizing face model!\n");

							m_faces[i].vertices = new CameraSpacePoint[m_faceVertexCount];
							m_faces[i].convertedVertices.resize(m_faceVertexCount);
						}
						else
							Fubi_logInfo("FubiKinectSDK2Sensor: failed initinalizing face frame source!\n");
					}
				}

				// Register for frame events
				m_frameEventStop = CreateEvent(NULL, TRUE, FALSE, NULL);
				m_multiSourceFrameReader->SubscribeMultiSourceFrameArrived(&m_frameEvent);
				m_frameEventThreadHandle = CreateThread(NULL, 0, frameEventThread, this, 0, NULL);

				Fubi_logInfo("FubiKinectSDK2Sensor: successfully initialized!\n");
				ret = true;
			}
		}
	}
	if (!ret)
	{
		m_options.m_depthOptions.invalidate();
		m_options.m_rgbOptions.invalidate();
		m_options.m_irOptions.invalidate();
		m_options.m_trackingProfile = SkeletonTrackingProfile::NONE;
	}
	return ret;
}


FubiKinectSDK2Sensor::~FubiKinectSDK2Sensor()
{
	release();
}

DWORD WINAPI FubiKinectSDK2Sensor::frameEventThread(LPVOID pParam)
{
	FubiKinectSDK2Sensor*  pthis = (FubiKinectSDK2Sensor *)pParam;
	HANDLE          hEvents[] = { pthis->m_frameEventStop, (HANDLE)pthis->m_frameEvent };
	int idx;
	bool running = true;
	while (running)
	{
		idx = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
		switch (idx)
		{
		case WAIT_TIMEOUT:
			break;
		case WAIT_OBJECT_0 + 1:
			if (pthis->m_multiSourceFrameReader)
			{
				IMultiSourceFrameArrivedEventArgs *frameArgs = 0x0;
				if (SUCCEEDED(pthis->m_multiSourceFrameReader->GetMultiSourceFrameArrivedEventData(pthis->m_frameEvent, &frameArgs)))
				{
					IMultiSourceFrameReference *frameReference = 0x0;
					if (SUCCEEDED(frameArgs->get_FrameReference(&frameReference)))
					{
						IMultiSourceFrame* frame;
						if (SUCCEEDED(frameReference->AcquireFrame(&frame)))
						{

							IBodyIndexFrame* bodyIndexFrame = NULL;
							BYTE *bodyIndexBuffer = NULL;
							if (pthis->m_options.m_trackingProfile != SkeletonTrackingProfile::NONE)
							{
								IBodyFrameReference* pBodyFrameReference = NULL;
								if (SUCCEEDED(frame->get_BodyFrameReference(&pBodyFrameReference)))
								{
									IBodyFrame* bodyFrame = NULL;
									if (SUCCEEDED(pBodyFrameReference->AcquireFrame(&bodyFrame)))
									{
										if (SUCCEEDED(bodyFrame->GetAndRefreshBodyData(BODY_COUNT, pthis->m_bodies)))
										{
											for (int i = 0; i < BODY_COUNT; ++i)
											{
												IBody* pBody = pthis->m_bodies[i];
												BOOLEAN bTracked = false;
												if (pBody)
												{
													if (SUCCEEDED(pBody->get_IsTracked(&bTracked)) && bTracked)
													{
														/*HandState leftHandState = HandState_Unknown;
														HandState rightHandState = HandState_Unknown;
														pBody->get_HandLeftState(&leftHandState);
														pBody->get_HandRightState(&rightHandState);*/
														bTracked = SUCCEEDED(pBody->GetJoints(JointType_Count, pthis->m_joints[i]))
															&& SUCCEEDED(pBody->GetJointOrientations(JointType_Count, pthis->m_jointOrients[i]));
													}
												}
												// Retrieve the latest face frame from this reader
												IHighDefinitionFaceFrame* pFaceFrame = 0x0;
												FaceInfo& face = pthis->m_faces[i];
												face.isTracked = false;
												if (face.reader != 0x0 && SUCCEEDED(face.reader->AcquireLatestFrame(&pFaceFrame)) && 0x0 != pFaceFrame)
												{
													// check if a valid face is tracked in this face frame
													if (SUCCEEDED(pFaceFrame->get_IsFaceTracked(&face.isTracked)) && face.isTracked)
													{
														if (SUCCEEDED(pFaceFrame->GetAndRefreshFaceAlignmentResult(face.alignment)))
														{
															face.alignment->get_FaceOrientation(&face.orientation);
															face.alignment->get_Quality(&face.alignmentQuality);
															if (pthis->m_faceModel)
															{
																pthis->m_faceModel->CalculateVerticesForAlignment(face.alignment, pthis->m_faceVertexCount, face.vertices);
																for (unsigned int j = 0; j < pthis->m_faceVertexCount; ++j)
																	face.convertedVertices[j] = CameraSpacePointToVec3f(face.vertices[j]);
															}
														}
													}
													pFaceFrame->Release();
												}
												if (!face.isTracked && bTracked)
												{
													// face tracking is not valid - attempt to fix the issue
													UINT64 bodyTId;
													if (SUCCEEDED(pBody->get_TrackingId(&bodyTId)))
													{
														// update tracking ID
														face.source->put_TrackingId(bodyTId);
													}
												}
											}
										}
										bodyFrame->Release();
									}
									pBodyFrameReference->Release();
								}

								if (pthis->m_userLabelBuffer)
								{
									IBodyIndexFrameReference* pBodyIndexFrameReference = NULL;
									if (SUCCEEDED(frame->get_BodyIndexFrameReference(&pBodyIndexFrameReference)))
									{
										if (SUCCEEDED(pBodyIndexFrameReference->AcquireFrame(&bodyIndexFrame)))
										{
											// Convert to unsigned short
											UINT nBufferSize;
											bodyIndexFrame->AccessUnderlyingBuffer(&nBufferSize, &bodyIndexBuffer);
										}
										pBodyIndexFrameReference->Release();
									}
								}
							}

							if (pthis->m_options.m_depthOptions.isValid())
							{
								IDepthFrameReference* pDepthFrameReference;
								if (SUCCEEDED(frame->get_DepthFrameReference(&pDepthFrameReference)))
								{
									IDepthFrame* depthFrame;
									if (SUCCEEDED(pDepthFrameReference->AcquireFrame(&depthFrame)))
									{
										UINT nBufferSize;
										UINT16 *pBuffer = NULL;
										if (SUCCEEDED(depthFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer)))
										{
											if (pthis->m_coordinateMapper)
											{
												// Register depth image to color space if wanted or calculate the color to depth map if requested
												if (SUCCEEDED(pthis->m_coordinateMapper->MapColorFrameToDepthSpace(
													pthis->m_options.m_depthOptions.m_width*pthis->m_options.m_depthOptions.m_height, pBuffer,
													pthis->m_options.m_rgbOptions.m_width*pthis->m_options.m_rgbOptions.m_height, pthis->m_colorIndexToDepthPointMap))
													&& pthis->m_options.m_registerStreams) // Check again, else we only need to compute the map
												{
#pragma omp parallel for schedule(dynamic)
													for (int h = 0; h < pthis->m_options.m_depthOptions.m_height; ++h)
													{
														unsigned short* pDest = pthis->m_depthBuffer + pthis->m_options.m_depthOptions.m_width*h;
														unsigned short* pLabelDest = bodyIndexBuffer
															? (pthis->m_userLabelBuffer + pthis->m_options.m_depthOptions.m_width*h)
															: 0x0;
														for (int w = 0; w < pthis->m_options.m_depthOptions.m_width; ++w)
														{
															// Retrieve the depth to color mapping for the current depth pixel
															int mappedIndex = ftoi_r(h*pthis->m_depthToColorScale.y)*pthis->m_options.m_rgbOptions.m_width + ftoi_r(w*pthis->m_depthToColorScale.x);
															const DepthSpacePoint& point = pthis->m_colorIndexToDepthPointMap[mappedIndex];
															// Make sure the depth pixel maps to a valid point in depth space again
															int depthX = ftoi_r(point.X);
															int depthY = ftoi_r(point.Y);
															BYTE id = 0xff;
															if ((depthX >= 0) && (depthX < pthis->m_options.m_depthOptions.m_width) && (depthY >= 0) && (depthY < pthis->m_options.m_depthOptions.m_height))
															{
																// Calculate new index in depth array
																int depthIndex = depthX + (depthY * pthis->m_options.m_depthOptions.m_width);
																// Now copy the depth pixel to the new coordinate
																*pDest = pBuffer[depthIndex];
																if (pLabelDest)
																	id = bodyIndexBuffer[depthIndex];
															}
															else
															{
																*pDest = 0;
															}
															if (pLabelDest)
															{
																if (id != 0xff)
																	*pLabelDest = id + 1;
																else
																	*pLabelDest = 0;
																pLabelDest++;
															}
															++pDest;
														}
													}
													if (bodyIndexFrame)
													{
														bodyIndexFrame->Release();
														bodyIndexFrame = 0;
													}
												}
												else
												{
													// No registration configured, or it did not work
													memcpy(pthis->m_depthBuffer, pBuffer, nBufferSize*sizeof(unsigned short));
												}
											}
											else
											{
												// Registration failed so copy directly
												memcpy(pthis->m_depthBuffer, pBuffer, nBufferSize*sizeof(unsigned short));
											}
										}
										depthFrame->Release();
									}
									pDepthFrameReference->Release();
								}
							}

							// Fallback: if the body index frame still exists, we need to convert it now
							if (bodyIndexFrame)
							{
								if (bodyIndexBuffer)
								{
									int numPixels = pthis->m_options.m_depthOptions.m_width*pthis->m_options.m_depthOptions.m_height;
									BYTE* pSrc = bodyIndexBuffer;
									unsigned short* pDst = pthis->m_userLabelBuffer;
									for (int i = 0; i < numPixels; ++i)
									{
										BYTE id = *pSrc++;
										if (id != 0xff)
											*pDst++ = id + 1;
										else
											*pDst++ = 0;
									}
									// Register labels for the color stream as well
									if (pthis->m_colorIndexToDepthPointMap)
									{
#pragma omp parallel for schedule(dynamic)
										for (int h = 0; h < pthis->m_options.m_depthOptions.m_height; ++h)
										{
											unsigned short* pLabelDest = pthis->m_userLabelColorBuffer + pthis->m_options.m_depthOptions.m_width*h;
											for (int w = 0; w < pthis->m_options.m_depthOptions.m_width; ++w)
											{
												// Retrieve the depth to color mapping for the current depth pixel
												int mappedIndex = ftoi_r(h*pthis->m_depthToColorScale.y)*pthis->m_options.m_rgbOptions.m_width + ftoi_r(w*pthis->m_depthToColorScale.x);
												const DepthSpacePoint& point = pthis->m_colorIndexToDepthPointMap[mappedIndex];
												// Make sure the depth pixel maps to a valid point in depth space again
												int depthX = ftoi_r(point.X);
												int depthY = ftoi_r(point.Y);
												BYTE id = 0xff;
												if ((depthX >= 0) && (depthX < pthis->m_options.m_depthOptions.m_width) && (depthY >= 0) && (depthY < pthis->m_options.m_depthOptions.m_height))
												{
													// Calculate new index in depth array
													int depthIndex = depthX + (depthY * pthis->m_options.m_depthOptions.m_width);
													// Now copy the depth pixel to the new coordinate
													id = bodyIndexBuffer[depthIndex];
												}
												if (id != 0xff)
													*pLabelDest = id + 1;
												else
													*pLabelDest = 0;
												pLabelDest++;
											}
										}
									}
								}
								bodyIndexFrame->Release();
							}

							if (pthis->m_options.m_irOptions.isValid())
							{
								IInfraredFrameReference* pIRFrameReference;
								if (SUCCEEDED(frame->get_InfraredFrameReference(&pIRFrameReference)))
								{
									IInfraredFrame* iFrame;
									if (SUCCEEDED(pIRFrameReference->AcquireFrame(&iFrame)))
									{
										UINT nBufferSize;
										UINT16 *pBuffer = NULL;
										if (SUCCEEDED(iFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer)))
										{
											if (pthis->m_coordinateMapper && pthis->m_options.m_registerStreams)
											{
												for (int h = 0; h < pthis->m_options.m_irOptions.m_height; ++h)
												{
													unsigned short* pDest = pthis->m_irBuffer + pthis->m_options.m_irOptions.m_width*h;
													for (int w = 0; w < pthis->m_options.m_irOptions.m_width; ++w)
													{
														// Retrieve the depth to color mapping for the current depth pixel
														int mappedIndex = ftoi_r(h*pthis->m_depthToColorScale.y)*pthis->m_options.m_rgbOptions.m_width + ftoi_r(w*pthis->m_depthToColorScale.x);
														DepthSpacePoint point = pthis->m_colorIndexToDepthPointMap[mappedIndex];
														// Make sure the depth pixel maps to a valid point in ir space again
														int irX = ftoi_r(point.X);
														int irY = ftoi_r(point.Y);
														BYTE id = 0;
														if ((irX >= 0) && (irX < pthis->m_options.m_irOptions.m_width) && (irY >= 0) && (irY < pthis->m_options.m_irOptions.m_height))
														{
															// Calculate new index in ir array
															int irIndex = irX + (irY * pthis->m_options.m_irOptions.m_width);
															// Now copy the ir pixel to the new coordinate
															*pDest = *(pBuffer + irIndex);
														}
														else
														{
															*pDest = 0;
														}

														++pDest;
													}
												}
											}
											else
												memcpy(pthis->m_irBuffer, pBuffer, nBufferSize*sizeof(unsigned short));
										}
										iFrame->Release();
									}
									pIRFrameReference->Release();
								}
							}

							if (pthis->m_options.m_rgbOptions.isValid())
							{
								IColorFrameReference* pColorFrameReference;
								if (SUCCEEDED(frame->get_ColorFrameReference(&pColorFrameReference)))
								{
									IColorFrame* colorFrame = NULL;
									if (SUCCEEDED(pColorFrameReference->AcquireFrame(&colorFrame)))
									{
										UINT nBufferSize;
										BYTE *pBuffer = NULL;
										ColorImageFormat imageFormat = ColorImageFormat_None;
										if (SUCCEEDED(colorFrame->get_RawColorImageFormat(&imageFormat))
											&& SUCCEEDED(colorFrame->AccessRawUnderlyingBuffer(&nBufferSize, &pBuffer)))
										{
											// Convert (from YUV2) to BGRA
											RGBQUAD* rgbxBuffer = 0x0;
											if (imageFormat == ColorImageFormat_Bgra)
											{
												rgbxBuffer = (RGBQUAD*)pBuffer;
											}
											else
											{
												nBufferSize = pthis->m_options.m_rgbOptions.m_width*pthis->m_options.m_rgbOptions.m_height * sizeof(RGBQUAD);
												if (SUCCEEDED(colorFrame->CopyConvertedFrameDataToArray(nBufferSize, reinterpret_cast<BYTE*>(pthis->m_rgbxBuffer), ColorImageFormat_Bgra)))
													rgbxBuffer = pthis->m_rgbxBuffer;
											}
											if (rgbxBuffer)
											{
												// Now remove the A channel
												int numPixels = pthis->m_options.m_rgbOptions.m_width*pthis->m_options.m_rgbOptions.m_height;
												unsigned char* pSource = (unsigned char*)rgbxBuffer;
												unsigned char* pDest = pthis->m_rgbBuffer;
												for (int i = 0; i < numPixels; ++i)
												{
													unsigned char b = *pSource++;
													unsigned char g = *pSource++;
													unsigned char r = *pSource++;
													pSource++; // skip A
													*pDest++ = r;
													*pDest++ = g;
													*pDest++ = b;
												}
											}
										}
										colorFrame->Release();
									}
									pColorFrameReference->Release();
								}
							}

							frame->Release();
							pthis->m_frameArrived = true;
						}

						frameReference->Release();
					}
				}
				frameArgs->Release();
			}
			break;
		case WAIT_OBJECT_0:
			running = false;
			break;
		}
	}
	return 0;
}

void FubiKinectSDK2Sensor::update()
{
	if (m_frameArrived)
	{
		m_hasNewData = true;
		m_frameArrived = false;
	}
	else
		m_hasNewData = false;
}


bool FubiKinectSDK2Sensor::hasNewTrackingData()
{
	return m_hasNewData;
}

bool FubiKinectSDK2Sensor::isTracking(unsigned int id)
{
	BOOLEAN isTracked = FALSE;
	if (m_bodies[id - 1])
	{
		m_bodies[id - 1]->get_IsTracked(&isTracked);
	}
	return isTracked == TRUE;
}

void FubiKinectSDK2Sensor::getSkeletonJointData(unsigned int id, Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJointPosition& position, Fubi::SkeletonJointOrientation& orientation)
{
	unsigned int userIndex = id - 1;
	Joint* joints = m_joints[userIndex];
	JointOrientation* jointOrients = m_jointOrients[userIndex];

	if (joint >= SkeletonJoint::FACE_NOSE && joint <= SkeletonJoint::FACE_CHIN) // Special case for face
	{
		// First get head data as a basis
		getSkeletonJointData(id, SkeletonJoint::HEAD, position, orientation);
		if (m_faces[userIndex].isTracked)
		{
			int fpIndex = JointToKSDKFacePoint(joint);
			// Now replace positions with face positions
			position.m_position = m_faces[userIndex].convertedVertices[fpIndex];
			position.m_confidence = (m_faces[userIndex].alignmentQuality == FaceAlignmentQuality_High) ? 1.0f : 0.5f;
		}
		else
		{
			position.m_confidence = 0.25f;
		}
	}
	else	// Default case
	{
		JointType index = JointToKSDK2Joint(joint);
		orientation.m_confidence = position.m_confidence = joints[index].TrackingState / (float)TrackingState_Tracked;
		position.m_position.x = joints[index].Position.X * 1000.0f;
		position.m_position.y = joints[index].Position.Y * 1000.0f;
		position.m_position.z = joints[index].Position.Z * 1000.0f;
		orientation.m_orientation = KSDK2OrientToMatrix3f(jointOrients[index].Orientation);

		// Fallback cases for feet and hands if current confidence too low
		if (position.m_confidence < 0.4f
			&& (index == JointType_FootLeft || index == JointType_FootRight || index == JointType_HandLeft || index == JointType_HandRight))
		{
			index = JointToKSDK2FallbackJoint(joint);
			orientation.m_confidence = position.m_confidence = joints[index].TrackingState / (float)TrackingState_Tracked;
			position.m_position.x = joints[index].Position.X * 1000.0f;
			position.m_position.y = joints[index].Position.Y * 1000.0f;
			position.m_position.z = joints[index].Position.Z * 1000.0f;
			orientation.m_orientation = KSDK2OrientToMatrix3f(jointOrients[index].Orientation);
		}
		// Special case for head with face tracking activated
		else if (index == JointType_Head && m_faces[userIndex].isTracked)
		{
			// Replace the orientation (currently no useful one) with the one from the face tracking
			// For some reason this orientation is already in the correct coordinate system...
			orientation.m_orientation = KSDK2OrientToMatrix3f(m_faces[userIndex].orientation, false);
			orientation.m_confidence = 1.0f;
		}
	}
}

const unsigned short* FubiKinectSDK2Sensor::getDepthData()
{
	return m_depthBuffer;
}

const unsigned char* FubiKinectSDK2Sensor::getRgbData()
{
	return m_rgbBuffer;
}

const unsigned short* FubiKinectSDK2Sensor::getIrData()
{
	return m_irBuffer;
}

const unsigned short* FubiKinectSDK2Sensor::getUserLabelData(Fubi::CoordinateType::Type coordType)
{
	if (getUserIDs(0x0) > 0)
	{
		if (coordType == CoordinateType::DEPTH || coordType == CoordinateType::IR || m_options.m_registerStreams)
			return m_userLabelBuffer;
		else if (coordType == CoordinateType::COLOR)
			return m_userLabelColorBuffer;
	}

	return 0x0;
}

unsigned short FubiKinectSDK2Sensor::getUserIDs(unsigned int* userIDs)
{
	int index = 0;
	for (int i = 0; i < BODY_COUNT; ++i)
	{
		if (isTracking(i + 1))
		{
			if (userIDs)
				userIDs[index] = i + 1;
			++index;
		}
	}
	return index;
}

Fubi::Vec3f FubiKinectSDK2Sensor::convertCoordinates(const Fubi::Vec3f& inputCoords, Fubi::CoordinateType::Type inputType, Fubi::CoordinateType::Type outputType)
{
	Vec3f ret(Math::NO_INIT);
	bool success = false;

	if (outputType == CoordinateType::DEPTH || outputType == CoordinateType::IR)
	{
		if (inputType == CoordinateType::REAL_WORLD)
		{
			if (m_options.m_registerStreams)
			{
				// The output type is equal to color space divided by depth to color scale
				ret = convertCoordinates(inputCoords, CoordinateType::REAL_WORLD, CoordinateType::COLOR);
				ret /= m_depthToColorScale;
			}
			else
			{
				// Actual real world to depth mapping
				CameraSpacePoint bodyPoint = { inputCoords.x / 1000.0f, inputCoords.y / 1000.0f, inputCoords.z / 1000.0f };
				DepthSpacePoint depthPoint = { 0 };
				m_coordinateMapper->MapCameraPointToDepthSpace(bodyPoint, &depthPoint);
				ret.x = depthPoint.X;
				ret.y = depthPoint.Y;
				ret.z = inputCoords.z;
			}
			success = true;
		}
		else if (inputType == CoordinateType::COLOR)
		{
			if (m_options.m_registerStreams)
			{
				ret = inputCoords / m_depthToColorScale;
				success = true;
			}
			else if (m_colorIndexToDepthPointMap)
			{
				int mappedIndex = ftoi_r(inputCoords.y)*m_options.m_rgbOptions.m_width + ftoi_r(inputCoords.x);
				DepthSpacePoint point = m_colorIndexToDepthPointMap[mappedIndex];
				ret.x = point.X;
				ret.y = point.Y;
				ret.z = inputCoords.z;
				success = true;
			}
		}
		else // conversion between depth and ir is equal
		{
			return inputCoords;
		}
	}
	else if (outputType == CoordinateType::REAL_WORLD)
	{
		CameraSpacePoint bodyPoint = { 0 };
		if (inputType == CoordinateType::COLOR)
		{
			// There is no direct mapping, so we first convert from color to depth
			Vec3f depthVec = convertCoordinates(inputCoords, CoordinateType::COLOR, CoordinateType::DEPTH);
			// And now from depth to real world
			ret = convertCoordinates(depthVec, CoordinateType::DEPTH, CoordinateType::REAL_WORLD);
			success = true;
		}
		else if (inputType == CoordinateType::DEPTH || inputType == CoordinateType::IR)
		{
			DepthSpacePoint depthPoint;
			if (m_options.m_registerStreams)
			{
				int mappedIndex = ftoi_r(inputCoords.y*m_depthToColorScale.y)*m_options.m_rgbOptions.m_width + ftoi_r(inputCoords.x*m_depthToColorScale.x);
				depthPoint = m_colorIndexToDepthPointMap[mappedIndex];
			}
			else
			{
				depthPoint = { inputCoords.x, inputCoords.y };
			}
			m_coordinateMapper->MapDepthPointToCameraSpace(depthPoint, (UINT16)(inputCoords.z / 1000.0f), &bodyPoint);
			ret.x = bodyPoint.X * 1000.0f;
			ret.y = bodyPoint.Y * 1000.0f;
			ret.z = bodyPoint.Z * 1000.0f;
			success = true;
		}
	}
	else if (outputType == CoordinateType::COLOR)
	{
		ColorSpacePoint colorPoint = { 0 };
		if (inputType == CoordinateType::REAL_WORLD)
		{
			CameraSpacePoint bodyPoint = { inputCoords.x / 1000.0f, inputCoords.y / 1000.0f, inputCoords.z / 1000.0f };
			m_coordinateMapper->MapCameraPointToColorSpace(bodyPoint, &colorPoint);
			ret.x = colorPoint.X;
			ret.y = colorPoint.Y;
			ret.z = inputCoords.z;
			success = true;
		}
		else if (inputType == CoordinateType::DEPTH || inputType == CoordinateType::IR)
		{
			if (m_options.m_registerStreams)	// already registered to color
			{
				ret = inputCoords * m_depthToColorScale;
			}
			else
			{
				DepthSpacePoint depthPoint = { inputCoords.x / 1000.0f, inputCoords.y / 1000.0f };
				m_coordinateMapper->MapDepthPointToColorSpace(depthPoint, (UINT16)(inputCoords.z / 1000.0f), &colorPoint);
				ret.x = colorPoint.X;
				ret.y = colorPoint.Y;
				ret.z = inputCoords.z;
			}
			success = true;
		}
	}

	if (success)
		return ret;
	return FubiISensor::convertCoordinates(inputCoords, inputType, outputType);
}

bool FubiKinectSDK2Sensor::getFacePoints(unsigned int id, const std::vector<Fubi::Vec3f>** vertices, const std::vector<Fubi::Vec3f>** triangleIndices)
{
	if (id > 0 && m_faces[id - 1].isTracked)
	{
		*vertices = &m_faces[id - 1].convertedVertices;

		*triangleIndices = &m_faceTriangles;
		return true;
	}
	return false;
}
#endif