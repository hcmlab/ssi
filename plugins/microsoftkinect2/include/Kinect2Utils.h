//------------------------------------------------------------------------------
// based on
// <copyright file="KinectSensor.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#ifndef SSI_FILTER_MICROSOFTKINECT2UTILS_H
#define SSI_FILTER_MICROSOFTKINECT2UTILS_H

#include "MicrosoftKinect2.h"

namespace ssi {

	class Kinect2Utils
	{
	public:

		static void Kinect2Utils::paint_point(BYTE *image, ssi_video_params_t &params, int point_x, int point_y, int border, int r_value, int g_value, int b_value);
		static void Kinect2Utils::paint_line(BYTE *image, ssi_video_params_t &params, int x1, int x2, int y1, int y2, int thickness, int r_value, int g_value, int b_value);
		static int Kinect2Utils::getParentJoint(int j);

		static void depthRaw2Image(ssi_byte_t *raw, ssi_byte_t *image, ssi_video_params_t video_format, ssi_video_params_t raw_format);
		static void infraredRaw2Image(ssi_byte_t *raw, ssi_byte_t *image, ssi_video_params_t video_format, ssi_video_params_t raw_format);
		static void quaternionToEuler(float& eulerX, float& eulerY, float& eulerZ, const Vector4& q, bool rotateCoordSys = true);

		// taken from Horde3D Utils
		static inline float degToRad(float f);
		static inline float radToDeg(float f);
	};

}

#endif