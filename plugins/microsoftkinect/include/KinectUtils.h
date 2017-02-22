//------------------------------------------------------------------------------
// based on
// <copyright file="KinectSensor.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include <FaceTrackLib.h>
#include <NuiApi.h>
#include "MicrosoftKinect.h"

class KinectUtils
{
public:

	static void paintFacePoints(IFTImage *image, ssi::MicrosoftKinect::FACEPOINTS &face, bool scaled, UINT32 color = 0x00FFFF00);
	static HRESULT paintFaceModel(IFTImage* pColorImg, IFTModel* pModel, FT_CAMERA_CONFIG const* pCameraConfig, FLOAT const* pSUCoef, 
		FLOAT zoomFactor, POINT viewOffset, IFTResult* pAAMRlt, UINT32 color);

	static void paintSkeleton(IFTImage *image, ssi::MicrosoftKinect::SKELETON &skel, bool scaled, UINT32 color = 0x0000FF00);
	static void paintBone(IFTImage *image, ssi::MicrosoftKinect::SKELETON &skel, POINT *points, ssi::MicrosoftKinect::SKELETON_JOINT::List joint0, ssi::MicrosoftKinect::SKELETON_JOINT::List joint1, UINT32 color);
	static void paintSkeleton (IFTImage *image, ssi::MicrosoftKinect::SKELETON_OLD &skel, bool scaled, UINT32 color = 0x0000FF00);
	static void paintBone (IFTImage *image, ssi::MicrosoftKinect::SKELETON_OLD &skel, POINT *points, ssi::MicrosoftKinect::SKELETON_JOINT::List joint0, ssi::MicrosoftKinect::SKELETON_JOINT::List joint1, UINT32 color);
	
	static void depthRaw2Image (ssi_byte_t *raw, ssi_byte_t *image, ssi_video_params_t video_format, ssi_video_params_t raw_format);
	static POINT SkeletonToScreen(Vector4 skeletonPoint, int width, int height, int cScreenWidth, int cScreenHeight);
	static void rotMatToRotation(float& eulerX, float& eulerY, float& eulerZ, const _Matrix4& x);
	
	static HRESULT GetVideoConfiguration(FT_CAMERA_CONFIG* videoConfig, UINT width, UINT height);
	static HRESULT GetDepthConfiguration(FT_CAMERA_CONFIG* videoConfig, UINT width, UINT height);	

	// taken from Horde3D Utils
	static inline float degToRad( float f );
	static inline float radToDeg( float f );
};
