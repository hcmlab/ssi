//------------------------------------------------------------------------------
// <copyright file="KinectUtils.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

// Including SDKDDKVer.h defines the highest available Windows platform.

// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.

#include <SDKDDKVer.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif

#include "MicrosoftKinect.h"
#include "KinectUtils.h"
#include <math.h>
#include <malloc.h>
#include <memory.h>
#include "SSI_Tools.h"

using namespace ssi;

HRESULT KinectUtils::paintFaceModel(IFTImage* pColorImg, IFTModel* pModel, FT_CAMERA_CONFIG const* pCameraConfig, FLOAT const* pSUCoef, 
    FLOAT zoomFactor, POINT viewOffset, IFTResult* pAAMRlt, UINT32 color)
{
    if (!pColorImg || !pModel || !pCameraConfig || !pSUCoef || !pAAMRlt)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    UINT vertexCount = pModel->GetVertexCount();
    FT_VECTOR2D* pPts2D = reinterpret_cast<FT_VECTOR2D*>(_malloca(sizeof(FT_VECTOR2D) * vertexCount));
    if (pPts2D)
    {
        FLOAT *pAUs;
        UINT auCount;
        hr = pAAMRlt->GetAUCoefficients(&pAUs, &auCount);
        if (SUCCEEDED(hr))
        {
            FLOAT scale, rotationXYZ[3], translationXYZ[3];
            hr = pAAMRlt->Get3DPose(&scale, rotationXYZ, translationXYZ);
            if (SUCCEEDED(hr))
            {
                hr = pModel->GetProjectedShape(pCameraConfig, zoomFactor, viewOffset, pSUCoef, pModel->GetSUCount(), pAUs, auCount, 
                    scale, rotationXYZ, translationXYZ, pPts2D, vertexCount);
                if (SUCCEEDED(hr))
                {
                    POINT* p3DMdl   = reinterpret_cast<POINT*>(_malloca(sizeof(POINT) * vertexCount));
                    if (p3DMdl)
                    {
                        for (UINT i = 0; i < vertexCount; ++i)
                        {
                            p3DMdl[i].x = LONG(pPts2D[i].x + 0.5f);
                            p3DMdl[i].y = LONG(pPts2D[i].y + 0.5f);
                        }

                        FT_TRIANGLE* pTriangles;
                        UINT triangleCount;
                        hr = pModel->GetTriangles(&pTriangles, &triangleCount);
                        if (SUCCEEDED(hr))
                        {
                            struct EdgeHashTable
                            {
                                UINT32* pEdges;
                                UINT edgesAlloc;

                                void Insert(int a, int b) 
                                {
                                    UINT32 v = (min(a, b) << 16) | max(a, b);
                                    UINT32 index = (v + (v << 8)) * 49157, i;
                                    for (i = 0; i < edgesAlloc - 1 && pEdges[(index + i) & (edgesAlloc - 1)] && v != pEdges[(index + i) & (edgesAlloc - 1)]; ++i)
                                    {
                                    }
                                    pEdges[(index + i) & (edgesAlloc - 1)] = v;
                                }
                            } eht;

                            eht.edgesAlloc = 1 << UINT(log(2.f * (1 + vertexCount + triangleCount)) / log(2.f));
                            eht.pEdges = reinterpret_cast<UINT32*>(_malloca(sizeof(UINT32) * eht.edgesAlloc));
                            if (eht.pEdges)
                            {
                                ZeroMemory(eht.pEdges, sizeof(UINT32) * eht.edgesAlloc);
                                for (UINT i = 0; i < triangleCount; ++i)
                                { 
                                    eht.Insert(pTriangles[i].i, pTriangles[i].j);
                                    eht.Insert(pTriangles[i].j, pTriangles[i].k);
                                    eht.Insert(pTriangles[i].k, pTriangles[i].i);
                                }
                                for (UINT i = 0; i < eht.edgesAlloc; ++i)
                                {
                                    if(eht.pEdges[i] != 0)
                                    {
                                        pColorImg->DrawLine(p3DMdl[eht.pEdges[i] >> 16], p3DMdl[eht.pEdges[i] & 0xFFFF], color, 1);
                                    }
                                }
                                _freea(eht.pEdges);
                            }

                            // Render the face rect in magenta
                            RECT rectFace;
                            hr = pAAMRlt->GetFaceRect(&rectFace);
                            if (SUCCEEDED(hr))
                            {
                                POINT leftTop = {rectFace.left, rectFace.top};
                                POINT rightTop = {rectFace.right - 1, rectFace.top};
                                POINT leftBottom = {rectFace.left, rectFace.bottom - 1};
                                POINT rightBottom = {rectFace.right - 1, rectFace.bottom - 1};
                                UINT32 nColor = 0xff00ff;
                                SUCCEEDED(hr = pColorImg->DrawLine(leftTop, rightTop, nColor, 1)) &&
                                    SUCCEEDED(hr = pColorImg->DrawLine(rightTop, rightBottom, nColor, 1)) &&
                                    SUCCEEDED(hr = pColorImg->DrawLine(rightBottom, leftBottom, nColor, 1)) &&
                                    SUCCEEDED(hr = pColorImg->DrawLine(leftBottom, leftTop, nColor, 1));
                            }
                        }

                        _freea(p3DMdl); 
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
            }
        }
        _freea(pPts2D);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

void KinectUtils::paintBone (IFTImage *image, 
	MicrosoftKinect::SKELETON &skel,
	POINT *points,  
	MicrosoftKinect::SKELETON_JOINT::List joint0, 
	MicrosoftKinect::SKELETON_JOINT::List joint1,
	UINT32 color) {

    float joint0State = skel[joint0][MicrosoftKinect::SKELETON_JOINT_VALUE::POS_CONF];
	float joint1State = skel[joint1][MicrosoftKinect::SKELETON_JOINT_VALUE::POS_CONF];

	float NUI_SKELETON_POSITION_NOT_TRACKED = 0.25f;
	float NUI_SKELETON_POSITION_INFERRED = 0.75f;
	float NUI_SKELETON_POSITION_TRACKED = 1.25f;

    // If we can't find either of these joints, exit
    if (joint0State < NUI_SKELETON_POSITION_NOT_TRACKED || joint1State < NUI_SKELETON_POSITION_NOT_TRACKED)
    {
        return;
    }
    
    // Don't draw if both points are inferred
    if (joint0State < NUI_SKELETON_POSITION_INFERRED && joint1State < NUI_SKELETON_POSITION_INFERRED)
    {
        return;
    } 	

    // We assume all drawn bones are inferred unless BOTH joints are tracked
    if (joint0State > NUI_SKELETON_POSITION_INFERRED && joint1State > NUI_SKELETON_POSITION_INFERRED)
    {
        image->DrawLine (points[joint0], points[joint1], color, 6);
    }
    else
    {
        image->DrawLine(points[joint0], points[joint1], 0xFFFFFFFF, 2);
    }
}

void KinectUtils::paintSkeleton (IFTImage *image,
	MicrosoftKinect::SKELETON &skel,
	bool scaled,
	UINT32 color) {

	float NUI_SKELETON_POSITION_NOT_TRACKED = 0.25f;
	float NUI_SKELETON_POSITION_INFERRED = 0.75f;
	float NUI_SKELETON_POSITION_TRACKED = 1.25f;

	int i;
	POINT points[MicrosoftKinect::SKELETON_JOINT::NUM];
	if (scaled) {
		for (i = 0; i < MicrosoftKinect::SKELETON_JOINT::NUM; i++)
		{
			Vector4 point;
			point.w = skel[i][MicrosoftKinect::SKELETON_JOINT_VALUE::POS_CONF];
			point.x = skel[i][MicrosoftKinect::SKELETON_JOINT_VALUE::POS_X];
			point.y = skel[i][MicrosoftKinect::SKELETON_JOINT_VALUE::POS_Y];
			point.z = skel[i][MicrosoftKinect::SKELETON_JOINT_VALUE::POS_Z];

			points[i] = SkeletonToScreen(point, image->GetWidth (), image->GetHeight (), 320, 240 );
		}
	} else {
		for (i = 0; i < MicrosoftKinect::SKELETON_JOINT::NUM; i++)
		{
			points[i].x = ssi_cast (LONG, skel[i][MicrosoftKinect::SKELETON_JOINT_VALUE::POS_X] + 0.5f);
			points[i].y = ssi_cast (LONG, skel[i][MicrosoftKinect::SKELETON_JOINT_VALUE::POS_Y] + 0.5f);
		}
	}
	
	// Render Torso
	// DrawBone(image, skel, m_Points, NUI_SKELETON_POSITION_HEAD, NUI_SKELETON_POSITION_SHOULDER_CENTER);  //Dont Draw, cause we have the face	
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::SHOULDER_CENTER,  MicrosoftKinect::SKELETON_JOINT::SHOULDER_LEFT, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::SHOULDER_CENTER, MicrosoftKinect::SKELETON_JOINT::SHOULDER_RIGHT, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::SHOULDER_CENTER, MicrosoftKinect::SKELETON_JOINT::SPINE, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::SPINE, MicrosoftKinect::SKELETON_JOINT::HIP_CENTER, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::HIP_CENTER, MicrosoftKinect::SKELETON_JOINT::HIP_LEFT, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::HIP_CENTER, MicrosoftKinect::SKELETON_JOINT::HIP_RIGHT, color);

    // Left Arm
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::SHOULDER_LEFT, MicrosoftKinect::SKELETON_JOINT::ELBOW_LEFT, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::ELBOW_LEFT, MicrosoftKinect::SKELETON_JOINT::WRIST_LEFT, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::WRIST_LEFT, MicrosoftKinect::SKELETON_JOINT::HAND_LEFT, color);

    // Right Arm
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::SHOULDER_RIGHT, MicrosoftKinect::SKELETON_JOINT::ELBOW_RIGHT, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::ELBOW_RIGHT, MicrosoftKinect::SKELETON_JOINT::WRIST_RIGHT, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::WRIST_RIGHT, MicrosoftKinect::SKELETON_JOINT::HAND_RIGHT, color);

    // Left Leg
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::HIP_LEFT, MicrosoftKinect::SKELETON_JOINT::KNEE_LEFT, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::KNEE_LEFT, MicrosoftKinect::SKELETON_JOINT::ANKLE_LEFT, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::ANKLE_LEFT, MicrosoftKinect::SKELETON_JOINT::FOOT_LEFT, color);

    // Right Leg
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::HIP_RIGHT, MicrosoftKinect::SKELETON_JOINT::KNEE_RIGHT, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::KNEE_RIGHT, MicrosoftKinect::SKELETON_JOINT::ANKLE_RIGHT, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::ANKLE_RIGHT, MicrosoftKinect::SKELETON_JOINT::FOOT_RIGHT, color);
    
    // Draw the joints in a different color
    for (i = 0; i < MicrosoftKinect::SKELETON_JOINT::NUM; ++i) {       
		if (skel[i][MicrosoftKinect::SKELETON_JOINT_VALUE::POS_CONF] > NUI_SKELETON_POSITION_NOT_TRACKED) {
			if (skel[i][MicrosoftKinect::SKELETON_JOINT_VALUE::POS_CONF] > NUI_SKELETON_POSITION_INFERRED) {
				image->DrawLine (points[i], points[i], 0x00FFFF00, 6);
			} else {
				image->DrawLine (points[i], points[i], 0x00FFFFFF, 6);
			}
		}
	}
}

void KinectUtils::paintBone (IFTImage *image, 
	MicrosoftKinect::SKELETON_OLD &skel,
	POINT *points,  
	MicrosoftKinect::SKELETON_JOINT::List joint0, 
	MicrosoftKinect::SKELETON_JOINT::List joint1,
	UINT32 color) {

    float joint0State = skel[joint0][MicrosoftKinect::SKELETON_JOINT_VALUE_OLD::POS_W];
	float joint1State = skel[joint1][MicrosoftKinect::SKELETON_JOINT_VALUE_OLD::POS_W];

	float NUI_SKELETON_POSITION_NOT_TRACKED = 0.25f;
	float NUI_SKELETON_POSITION_INFERRED = 0.75f;
	float NUI_SKELETON_POSITION_TRACKED = 1.25f;

    // If we can't find either of these joints, exit
    if (joint0State < NUI_SKELETON_POSITION_NOT_TRACKED || joint1State < NUI_SKELETON_POSITION_NOT_TRACKED)
    {
        return;
    }
    
    // Don't draw if both points are inferred
    if (joint0State < NUI_SKELETON_POSITION_INFERRED && joint1State < NUI_SKELETON_POSITION_INFERRED)
    {
        return;
    } 	

    // We assume all drawn bones are inferred unless BOTH joints are tracked
    if (joint0State > NUI_SKELETON_POSITION_INFERRED && joint1State > NUI_SKELETON_POSITION_INFERRED)
    {
        image->DrawLine (points[joint0], points[joint1], color, 6);
    }
    else
    {
        image->DrawLine(points[joint0], points[joint1], 0xFFFFFFFF, 2);
    }
}

void KinectUtils::paintSkeleton (IFTImage *image,
	MicrosoftKinect::SKELETON_OLD &skel,
	bool scaled,
	UINT32 color) {

	float NUI_SKELETON_POSITION_NOT_TRACKED = 0.25f;
	float NUI_SKELETON_POSITION_INFERRED = 0.75f;
	float NUI_SKELETON_POSITION_TRACKED = 1.25f;

	int i;
	POINT points[MicrosoftKinect::SKELETON_JOINT::NUM];
	if (scaled) {
		for (i = 0; i < MicrosoftKinect::SKELETON_JOINT::NUM; i++)
		{
			Vector4 point;
			point.w = skel[i][MicrosoftKinect::SKELETON_JOINT_VALUE_OLD::POS_W];
			point.x = skel[i][MicrosoftKinect::SKELETON_JOINT_VALUE_OLD::POS_X];
			point.y = skel[i][MicrosoftKinect::SKELETON_JOINT_VALUE_OLD::POS_Y];
			point.z = skel[i][MicrosoftKinect::SKELETON_JOINT_VALUE_OLD::POS_Z];

			points[i] = SkeletonToScreen(point, image->GetWidth (), image->GetHeight (), 320, 240 );
		}
	} else {
		for (i = 0; i < MicrosoftKinect::SKELETON_JOINT::NUM; i++)
		{
			points[i].x = ssi_cast (LONG, skel[i][MicrosoftKinect::SKELETON_JOINT_VALUE_OLD::POS_X] + 0.5f);
			points[i].y = ssi_cast (LONG, skel[i][MicrosoftKinect::SKELETON_JOINT_VALUE_OLD::POS_Y] + 0.5f);
		}
	}
	
	// Render Torso
	// DrawBone(image, skel, m_Points, NUI_SKELETON_POSITION_HEAD, NUI_SKELETON_POSITION_SHOULDER_CENTER);  //Dont Draw, cause we have the face	
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::SHOULDER_CENTER,  MicrosoftKinect::SKELETON_JOINT::SHOULDER_LEFT, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::SHOULDER_CENTER, MicrosoftKinect::SKELETON_JOINT::SHOULDER_RIGHT, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::SHOULDER_CENTER, MicrosoftKinect::SKELETON_JOINT::SPINE, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::SPINE, MicrosoftKinect::SKELETON_JOINT::HIP_CENTER, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::HIP_CENTER, MicrosoftKinect::SKELETON_JOINT::HIP_LEFT, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::HIP_CENTER, MicrosoftKinect::SKELETON_JOINT::HIP_RIGHT, color);

    // Left Arm
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::SHOULDER_LEFT, MicrosoftKinect::SKELETON_JOINT::ELBOW_LEFT, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::ELBOW_LEFT, MicrosoftKinect::SKELETON_JOINT::WRIST_LEFT, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::WRIST_LEFT, MicrosoftKinect::SKELETON_JOINT::HAND_LEFT, color);

    // Right Arm
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::SHOULDER_RIGHT, MicrosoftKinect::SKELETON_JOINT::ELBOW_RIGHT, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::ELBOW_RIGHT, MicrosoftKinect::SKELETON_JOINT::WRIST_RIGHT, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::WRIST_RIGHT, MicrosoftKinect::SKELETON_JOINT::HAND_RIGHT, color);

    // Left Leg
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::HIP_LEFT, MicrosoftKinect::SKELETON_JOINT::KNEE_LEFT, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::KNEE_LEFT, MicrosoftKinect::SKELETON_JOINT::ANKLE_LEFT, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::ANKLE_LEFT, MicrosoftKinect::SKELETON_JOINT::FOOT_LEFT, color);

    // Right Leg
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::HIP_RIGHT, MicrosoftKinect::SKELETON_JOINT::KNEE_RIGHT, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::KNEE_RIGHT, MicrosoftKinect::SKELETON_JOINT::ANKLE_RIGHT, color);
    paintBone (image, skel, points, MicrosoftKinect::SKELETON_JOINT::ANKLE_RIGHT, MicrosoftKinect::SKELETON_JOINT::FOOT_RIGHT, color);
    
    // Draw the joints in a different color
    for (i = 0; i < MicrosoftKinect::SKELETON_JOINT::NUM; ++i) {       
		if (skel[i][MicrosoftKinect::SKELETON_JOINT_VALUE_OLD::POS_W] > NUI_SKELETON_POSITION_NOT_TRACKED) {
			if (skel[i][MicrosoftKinect::SKELETON_JOINT_VALUE_OLD::POS_W] > NUI_SKELETON_POSITION_INFERRED) {
				image->DrawLine (points[i], points[i], 0x00FFFF00, 6);
			} else {
				image->DrawLine (points[i], points[i], 0x00FFFFFF, 6);
			}
		}
	}
}

void KinectUtils::paintFacePoints (IFTImage *image, ssi::MicrosoftKinect::FACEPOINTS &face, bool scaled, UINT color) {

    POINT points[MicrosoftKinect::FACEPOINT::NUM];
	for (ssi_size_t i = 0; i < MicrosoftKinect::FACEPOINT::NUM; i++) {
		points[i].x = ssi_cast (LONG, scaled ? face[i][MicrosoftKinect::FACEPOINT_VALUE::X] * image->GetWidth() + 0.5f : face[i][MicrosoftKinect::FACEPOINT_VALUE::X] + 0.5f);
		points[i].y = ssi_cast (LONG, scaled ? face[i][MicrosoftKinect::FACEPOINT_VALUE::Y] * image->GetHeight () + 0.5f : face[i][MicrosoftKinect::FACEPOINT_VALUE::Y] + 0.5f);
	}
	
    for (UINT ipt = 0; ipt < 8; ++ipt)
    {
        POINT ptStart = points[ipt];
        POINT ptEnd = points[(ipt+1)%8];
        image->DrawLine(ptStart, ptEnd, color, 1);
    }

    for (UINT ipt = 8; ipt < 16; ++ipt)
    {
        POINT ptStart = points[ipt];
        POINT ptEnd = points[(ipt-8+1)%8+8];
        image->DrawLine(ptStart, ptEnd, color, 1);
    }

    for (UINT ipt = 16; ipt < 26; ++ipt)
    {
        POINT ptStart = points[ipt];
        POINT ptEnd = points[(ipt-16+1)%10+16];
        image->DrawLine(ptStart, ptEnd, color, 1);
    }

    for (UINT ipt = 26; ipt < 36; ++ipt)
    {
        POINT ptStart = points[ipt];
        POINT ptEnd = points[(ipt-26+1)%10+26];
        image->DrawLine(ptStart, ptEnd, color, 1);
    }

    for (UINT ipt = 36; ipt < 47; ++ipt)
    {
        POINT ptStart = points[ipt];
        POINT ptEnd = points[ipt+1];
        image->DrawLine(ptStart, ptEnd, color, 1);
    }

    for (UINT ipt = 48; ipt < 60; ++ipt)
    {
        POINT ptStart = points[ipt];
        POINT ptEnd = points[(ipt-48+1)%12+48];
        image->DrawLine(ptStart, ptEnd, color, 1);
    }

    for (UINT ipt = 60; ipt < 68; ++ipt)
    {
        POINT ptStart = points[ipt];
        POINT ptEnd = points[(ipt-60+1)%8+60];
        image->DrawLine(ptStart, ptEnd, color, 1);
    }

    for (UINT ipt = 68; ipt < 86; ++ipt)
    {
        POINT ptStart = points[ipt];
        POINT ptEnd = points[ipt+1];
        image->DrawLine(ptStart, ptEnd, color, 1);
    }

}

void KinectUtils::depthRaw2Image (ssi_byte_t *raw, ssi_byte_t *image, ssi_video_params_t video_format, ssi_video_params_t raw_format) {

	for (int i = 0; i < video_format.heightInPixels; i++) {

		SHORT *ppraw = ssi_pcast (SHORT, raw);
		BYTE *ppimage = ssi_pcast (BYTE, image);

		for (int j = 0; j < video_format.widthInPixels; j++) {

			SHORT realDepth = *ppraw++;

			if (realDepth > SSI_MICROSOFTKINECT_DEPTHIMAGE_MIN_VALUE)
			{
				//Convert the realDepth into the 0 to 255 range for our actual distance.
				//Use only one of the following Distance assignments
				//White = Far
				//Black = Close
				//Distance = (byte)(((realDepth – MinimumDistance) * 255 / (MaximumDistance-MinimumDistance)));

				//White = Close
				//Black = Far
				*ppimage++ = (ssi_byte_t) (255 - ((realDepth - SSI_MICROSOFTKINECT_DEPTHIMAGE_MIN_VALUE) * 255 / (SSI_MICROSOFTKINECT_DEPTHIMAGE_MAX_VALUE - SSI_MICROSOFTKINECT_DEPTHIMAGE_MIN_VALUE)));
			}
			//If we are closer than 800mm, the just paint it red so we know this pixel is not giving a good value
			else
			{
				*ppimage++ = 0;
			}
		}

		raw += ssi_video_stride (raw_format);
		image += ssi_video_stride (video_format);
	}
}



/// <summary>
/// Converts a skeleton point to screen space
/// </summary>
/// <param name="skeletonPoint">skeleton point to tranform</param>
/// <param name="width">width (in pixels) of output buffer</param>
/// <param name="height">height (in pixels) of output buffer</param>
/// <returns>point in screen-space</returns>
POINT KinectUtils::SkeletonToScreen(Vector4 skeletonPoint, int width, int height, int cScreenWidth, int cScreenHeight)
{
    LONG x, y;
    USHORT depth;

    // Calculate the skeleton's position on the screen
    // NuiTransformSkeletonToDepthImage returns coordinates in NUI_IMAGE_RESOLUTION_320x240 space
    NuiTransformSkeletonToDepthImage(skeletonPoint, &x, &y, &depth);

    float screenPointX = static_cast<float>(x * width) / cScreenWidth;
    float screenPointY = static_cast<float>(y * height) / cScreenHeight;

	POINT point;
	point.x = (LONG) (screenPointX + 0.5f);
	point.y = (LONG) (screenPointY + 0.5f);

    return point;
}

HRESULT KinectUtils::GetVideoConfiguration(FT_CAMERA_CONFIG* videoConfig, UINT width, UINT height)
{
	if (!videoConfig)
	{
		return E_POINTER;
	}

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
	return S_OK;
}

HRESULT KinectUtils::GetDepthConfiguration(FT_CAMERA_CONFIG* depthConfig, UINT width, UINT height)
{
	if (!depthConfig)
	{
		return E_POINTER;
	}
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

void KinectUtils::rotMatToRotation(float& eulerX, float& eulerY, float& eulerZ, const _Matrix4& x)
{
	// first one is easy
	eulerX = sinf(-x.M32);

	// Special case: Cos[x] == 0 (when Sin[x] is +/-1)
	float f = abs(x.M32);
	if (f > 0.999f && f < 1.001f)
	{
		// Pin arbitrarily one of y or z to 0
		// Mathematical equivalent of gimbal lock
		eulerY = 0;

		// Now: Cos[x] = 0, Sin[x] = +/-1, Cos[y] = 1, Sin[y] = 0
		// => m11 = Cos[z] and m21 = Sin[z]
		eulerZ = atan2f(-x.M11, -x.M21);
	}
	// Standard case
	else
	{
		eulerY = atan2f(x.M31, x.M33);
		eulerZ = atan2f(x.M12, x.M22);
	}

	// Rotations in MS SDK are rotated 180° around the y axis, so we rotate them back
	// so that they are the same as in OpenNI and also in the same coordinate system
	// as the positions (why does MS use different ones anyway??)
	static const float Pi = 3.141592654f;
	eulerY = (eulerY > 0) ? (eulerY - Pi) : (eulerY + Pi);

	//convert to degree (additional invert x and z axis, see comment above)
	eulerX = -radToDeg(eulerX);
	eulerY = radToDeg(eulerY);
	eulerZ = -radToDeg(eulerZ);
}

float KinectUtils::degToRad( float f ) 
{
	return f * 0.017453293f;
}

float KinectUtils::radToDeg( float f ) 
{
	return f * 57.29577951f;
}


