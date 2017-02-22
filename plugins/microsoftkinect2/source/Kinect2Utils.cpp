//------------------------------------------------------------------------------
// <copyright file="Kinect2Utils.cpp" company="Microsoft">
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

#include <Kinect.h>

#include "MicrosoftKinect2.h"
#include "Kinect2Utils.h"
#include "SSI_Cons.h"

namespace ssi {

	void Kinect2Utils::depthRaw2Image(ssi_byte_t *raw, ssi_byte_t *image, ssi_video_params_t video_format, ssi_video_params_t raw_format) {


		for (int i = 0; i < video_format.heightInPixels; i++) {

			SHORT *ppraw = ssi_pcast(SHORT, raw);
			BYTE *ppimage = ssi_pcast(BYTE, image);

			for (int j = 0; j < video_format.widthInPixels; j++) {

				SHORT realDepth = *ppraw++;

				if (realDepth > SSI_MICROSOFTKINECT2_DEPTHIMAGE_MIN_VALUE && realDepth < SSI_MICROSOFTKINECT2_DEPTHIMAGE_MAX_VALUE)
				{
					//Convert the realDepth into the 0 to 255 range for our actual distance.
					//Use only one of the following Distance assignments
					//White = Far
					//Black = Close
					//Distance = (byte)(((realDepth – MinimumDistance) * 255 / (MaximumDistance-MinimumDistance)));

					//White = Close
					//Black = Far
					*ppimage++ = (ssi_byte_t)(255 - ((realDepth - SSI_MICROSOFTKINECT2_DEPTHIMAGE_MIN_VALUE) * 255 / (SSI_MICROSOFTKINECT2_DEPTHIMAGE_MAX_VALUE - SSI_MICROSOFTKINECT2_DEPTHIMAGE_MIN_VALUE)));
				}
				//If we are closer than 500mm, then just paint it black so we know this pixel is not giving a good value
				else if (realDepth < SSI_MICROSOFTKINECT2_DEPTHIMAGE_MIN_VALUE)
				{
					*ppimage++ = 0;
				}
				//If we are further than 4500mm, then just paint it black so we know this pixel is not giving a good value
				else{
					*ppimage++ = 0;
				}
			}

			raw += ssi_video_stride(raw_format);
			image += ssi_video_stride(video_format);
		}
	}

	void Kinect2Utils::infraredRaw2Image(ssi_byte_t *raw, ssi_byte_t *image, ssi_video_params_t video_format, ssi_video_params_t raw_format) {

		for (int i = 0; i < video_format.heightInPixels; i++) {

			UINT16 *ppraw = ssi_pcast(UINT16, raw);
			BYTE *ppimage = ssi_pcast(BYTE, image);

			for (int j = 0; j < video_format.widthInPixels; j++) {

				UINT16 realDepth = *ppraw++;

				if (realDepth >= SSI_MICROSOFTKINECT2_INFRAREDIMAGE_MAX_VALUE)
				{
					//too close: white
					*ppimage++ = 255;
				}
				else{
					*ppimage++ = (ssi_byte_t)(((realDepth - SSI_MICROSOFTKINECT2_INFRAREDIMAGE_MIN_VALUE) * 255 / (SSI_MICROSOFTKINECT2_INFRAREDIMAGE_MAX_VALUE - SSI_MICROSOFTKINECT2_INFRAREDIMAGE_MIN_VALUE)));
				}

			}

			raw += ssi_video_stride(raw_format);
			image += ssi_video_stride(video_format);
		}
	}

	void Kinect2Utils::quaternionToEuler(float& eulerX, float& eulerY, float& eulerZ, const Vector4& q, bool rotateCoordSys /*= true*/)
	{
		//1) convert quaternions to rotation matrix
		float rm[3][3];

		// Calculate coefficients
		float x2 = q.x + q.x, y2 = q.y + q.y, z2 = q.z + q.z;
		float xx = q.x * x2, xy = q.x * y2, xz = q.x * z2;
		float yy = q.y * y2, yz = q.y * z2, zz = q.z * z2;
		float wx = q.w * x2, wy = q.w * y2, wz = q.w * z2;

		rm[0][0] = 1 - (yy + zz);  rm[1][0] = xy - wz;			rm[2][0] = xz + wy;
		rm[0][1] = xy + wz;        rm[1][1] = 1 - (xx + zz);	rm[2][1] = yz - wx;
		rm[0][2] = xz - wy;        rm[1][2] = yz + wx;			rm[2][2] = 1 - (xx + yy);

		//2) convert rotation matrix to euler angles

		// first one is easy
		eulerX = sinf(-rm[2][1]);

		// Special case: Cos[x] == 0 (when Sin[x] is +/-1)
		float f = abs(rm[2][1]);
		if (f > 0.999f && f < 1.001f)
		{
			// Pin arbitrarily one of y or z to 0
			// Mathematical equivalent of gimbal lock
			eulerY = 0;

			// Now: Cos[x] = 0, Sin[x] = +/-1, Cos[y] = 1, Sin[y] = 0
			// => m11 = Cos[z] and m21 = Sin[z]
			eulerZ = atan2f(-rm[0][0], -rm[1][0]);
		}
		// Standard case
		else
		{
			eulerY = atan2f(rm[2][0], rm[2][2]);
			eulerZ = atan2f(rm[0][1], rm[1][1]);
		}

		// Felix 2016-01-15:
		// Rotations in MS SDK 2 are rotated 180° around the y axis, so we rotate them back
		// so that they are the same as in OpenNI Kinect 1 provider and also in the same coordinate system as the positions
		// NOTE: does not apply to the head pose which is already rotated correctly!!
		if (rotateCoordSys)
		{
			static const float Pi = 3.141592654f;
			eulerX = -eulerX;
			eulerY = (eulerY > 0) ? (eulerY - Pi) : (eulerY + Pi);
			eulerZ = -eulerZ;
		}

		//convert to degree
		eulerX = radToDeg(eulerX);
		eulerY = radToDeg(eulerY);
		eulerZ = radToDeg(eulerZ);
	}

	void Kinect2Utils::paint_point(BYTE *image, ssi_video_params_t &params, int point_x, int point_y, int border, int r_value, int g_value, int b_value) {

		int stride = ssi_video_stride(params);
		int width = params.widthInPixels;
		int height = params.heightInPixels;
		int depth = params.numOfChannels;
		point_y = params.flipImage ? height - point_y : point_y;

		for (int x = max(0, point_x - border); x < min(width, point_x + border); ++x) {
			for (int y = max(0, point_y - border); y < min(height, point_y + border); ++y) {
				BYTE *pixel = image + depth * x + y * stride;
				pixel[0] = b_value;
				pixel[1] = g_value;
				pixel[2] = r_value;

			}
		}
	}

	void Kinect2Utils::paint_line(BYTE *image, ssi_video_params_t &params, int x1, int x2, int y1, int y2, int thickness, int r_value, int g_value, int b_value) {

		if (x1 > 0 && y1 > 0 && x2 > 0 && y2 > 0){

			int stride = ssi_video_stride(params);
			int width = params.widthInPixels;
			int height = params.heightInPixels;
			int depth = params.numOfChannels;

			bool horizontal = std::abs(x2 - x1) > std::abs(y2 - y1);
			int x, y, xd, yd, tmp, xPaint, yPaint;

			if (horizontal){
				if (x1 > x2){
					tmp = x1;
					x1 = x2;
					x2 = tmp;

					tmp = y1;
					y1 = y2;
					y2 = tmp;
				}

				yd = (y2 - y1);
				xd = (x2 - x1);

				for (x = x1; x <= x2; x++)
				{
					if (xd != 0){
						y = y1 + (yd * (x - x1) / xd);
					}
					else{
						y = y1;
					}


					for (int t = 0; t <= thickness; t++){

						yPaint = y - (thickness / 2) + t;
						if (x > 0 && yPaint > 0 && x < width && yPaint < height){
							BYTE *pixel = image + depth * x + yPaint * stride;
							pixel[0] = b_value;
							pixel[1] = g_value;
							pixel[2] = r_value;
						}
					}
				}

			}
			else{
				if (y1 > y2){
					tmp = y1;
					y1 = y2;
					y2 = tmp;

					tmp = x1;
					x1 = x2;
					x2 = tmp;

				}

				yd = (y2 - y1);
				xd = (x2 - x1);


				for (y = y1; y <= y2; y++)
				{
					if (yd != 0){
						x = x1 + (xd * (y - y1) / yd);
					}
					else{
						x = x1;
					}

					for (int t = 0; t <= thickness; t++){
						xPaint = x - (thickness / 2) + t;
						if (xPaint > 0 && y > 0 && xPaint < width && y < height){
							BYTE *pixel = image + depth * xPaint + y * stride;
							pixel[0] = b_value;
							pixel[1] = g_value;
							pixel[2] = r_value;
						}
					}

				}
			}
		}



	}

	int Kinect2Utils::getParentJoint(int j){

		//https://social.msdn.microsoft.com/Forums/en-US/1c068899-cd0d-437d-9a95-dbf2eac42487/kinect-v20-get-hierarchical-joint-orientation-c?forum=kinectv2sdk

		switch (j)
		{
		case JointType_SpineBase:
			return -1; //root has no parent
		case JointType_SpineMid:
			return JointType_SpineBase;
		case JointType_Neck:
			return JointType_SpineShoulder;
		case JointType_Head:
			return JointType_Neck;
		case JointType_ShoulderLeft:
			return JointType_SpineShoulder;
		case JointType_ElbowLeft:
			return JointType_ShoulderLeft;
		case JointType_WristLeft:
			return JointType_ElbowLeft;
		case JointType_HandLeft:
			return JointType_WristLeft;
		case JointType_ShoulderRight:
			return JointType_SpineShoulder;
		case JointType_ElbowRight:
			return JointType_ShoulderRight;
		case JointType_WristRight:
			return JointType_ElbowRight;
		case JointType_HandRight:
			return JointType_WristRight;
		case JointType_HipLeft:
			return JointType_SpineBase;
		case JointType_KneeLeft:
			return JointType_HipLeft;
		case JointType_AnkleLeft:
			return JointType_KneeLeft;
		case JointType_FootLeft:
			return JointType_AnkleLeft;
		case JointType_HipRight:
			return JointType_SpineBase;
		case JointType_KneeRight:
			return JointType_HipRight;
		case JointType_AnkleRight:
			return JointType_KneeRight;
		case JointType_FootRight:
			return JointType_AnkleRight;
		case JointType_SpineShoulder:
			return JointType_SpineMid;
		case JointType_HandTipLeft:
			return JointType_HandLeft;
		case JointType_ThumbLeft:
			return JointType_HandLeft;
		case JointType_HandTipRight:
			return JointType_HandRight;
		case JointType_ThumbRight:
			return JointType_HandRight;
		default:
			return -1;
		}

	}


	float Kinect2Utils::degToRad(float f)
	{
		return f * 0.017453293f;
	}

	float Kinect2Utils::radToDeg(float f)
	{
		return f * 57.29577951f;
	}

}


