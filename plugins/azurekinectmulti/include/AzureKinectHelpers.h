// AzureKinectHelpers.h
// author: Fabian Wildgrube
// created: 2021
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************
#pragma once
#ifndef SSI_AZUREKINECT_HELPERS
#define SSI_AZUREKINECT_HELPERS


#include <cstdint>
#include <exception>
#include <utility>
#include <math.h>
#include <algorithm>
#include <ssiocv.h>

#include "k4a/k4a.hpp"

#include "AzureKinectDatatypes.h"

namespace ssi {
    namespace AK {

        // Gets the dimensions of the color images that the color camera will produce for a
        // given color resolution
        //
        inline std::pair<int, int> GetColorDimensions(const RGB_VIDEO_RESOLUTION resolution) noexcept
        {
            switch (resolution)
            {
                case RGB_VIDEO_RESOLUTION::p_1280x720:
                    return { 1280, 720 };
                case RGB_VIDEO_RESOLUTION::p_3840x2160:
                    return { 3840, 2160 };
                case RGB_VIDEO_RESOLUTION::p_2560x1440:
                    return { 2560, 1440 };
                case RGB_VIDEO_RESOLUTION::p_1920x1080:
                    return { 1920, 1080 };
                case RGB_VIDEO_RESOLUTION::p_4096x3072:
                    return { 4096, 3072 };
                case RGB_VIDEO_RESOLUTION::p_2048x1536:
                    return { 2048, 1536 };
                case RGB_VIDEO_RESOLUTION::OFF:
                    return { 1, 1 };
                default:
                    return { 1, 1 };
            }
        }

        // Gets the dimensions of the depth images that the depth camera will produce for a
        // given depth mode
        //
        inline std::pair<int, int> GetDepthDimensions(const DEPTH_MODE depthMode) noexcept
        {
            switch (depthMode)
            {
                case DEPTH_MODE::NFOV_2x2_BINNED:
                    return { 320, 288 };
                case DEPTH_MODE::NFOV_UNBINNED:
                    return { 640, 576 };
                case DEPTH_MODE::WFOV_2x2_BINNED:
                    return { 512, 512 };
                case DEPTH_MODE::WFOV_UNBINNED:
                    return { 1024, 1024 };
                case DEPTH_MODE::PASSIVE_IR:
                    return { 1024, 1024 };
                case DEPTH_MODE::OFF:
                    return { 1, 1 };
                default:
                    return { 1, 1 };
            }
        }

        // Gets the range of values that we expect to see from the depth camera
        // when using a given depth mode, in millimeters
        //
        inline std::pair<uint16_t, uint16_t> GetDepthModeRange(const DEPTH_MODE depthMode)
        {
            switch (depthMode)
            {
            case DEPTH_MODE::NFOV_2x2_BINNED:
                return { (uint16_t)500, (uint16_t)5800 };
            case DEPTH_MODE::NFOV_UNBINNED:
                return { (uint16_t)500, (uint16_t)4000 };
            case DEPTH_MODE::WFOV_2x2_BINNED:
                return { (uint16_t)250, (uint16_t)3000 };
            case DEPTH_MODE::WFOV_UNBINNED:
                return { (uint16_t)250, (uint16_t)2500 };

            case DEPTH_MODE::PASSIVE_IR:
            default:
                throw std::logic_error("Invalid depth mode!");
            }
        }

        // Gets the expected min/max IR brightness levels that we expect to see
        // from the IR camera when using a given depth mode
        //
        inline std::pair<uint16_t, uint16_t> GetIrLevelsRange(const DEPTH_MODE depthMode)
        {
            switch (depthMode)
            {
            case DEPTH_MODE::PASSIVE_IR:
                return { (uint16_t)0, (uint16_t)100 };

            case DEPTH_MODE::OFF:
                throw std::logic_error("Invalid depth mode!");

            default:
                return { (uint16_t)0, (uint16_t)1000 };
            }
        }

        // Computes a greyscale representation of a depth pixel.
        //
        inline IRPixel ColorizeGreyscale(const DepthPixel& value, const DepthPixel& minimum, const DepthPixel& maximum)
        {
            // Clamp to max
            DepthPixel pixelValue = (std::min)(value, maximum);

            constexpr uint8_t PixelMax = (std::numeric_limits<uint8_t>::max)();
            const auto normalizedValue = static_cast<uint8_t>((pixelValue - minimum) * (double(PixelMax) / (maximum - minimum)));

            return normalizedValue;
        }

        // Computes a blue - to red representation of depth pixels as HSV values
        inline HSVPixel ColorizeBlueToRed_HSV(const DepthPixel& depthPixel, const DepthPixel& minimum, const DepthPixel& maximum)
        {
            constexpr uint8_t PixelMax = (std::numeric_limits<uint8_t>::max)();

            // Default to opaque black.
            //
            HSVPixel result = { 0, 0, 0 };

            // If the pixel is actual zero and not just below the min value, make it black
            //
            if (depthPixel == 0)
            {
                return result;
            }

            uint16_t clampedValue = depthPixel;
            clampedValue = (std::min)(clampedValue, maximum);
            clampedValue = (std::max)(clampedValue, minimum);

            // Normalize to [0, 1]
            //
            float hue = (clampedValue - minimum) / static_cast<float>(maximum - minimum);

            // The 'hue' coordinate in HSV is a polar coordinate, so it 'wraps'.
            // Purple starts after blue and is close enough to red to be a bit unclear,
            // so we want to go from red to blue.  Purple starts around .5,
            // so we want to normalize to [0, .5].
            //
            constexpr float range = 0.5f;
            hue *= range;

            //return hsv signal
            result.Blue = static_cast<uint8_t>(255 * hue);
            result.Red = 255;
            result.Green = 255;

            return result;
        }

        inline std::pair<bool, cv::Point> convert3DDepthTo2DColorCoordinate(float x, float y, float z, const k4a::calibration& calibration)
        {
            bool isValid = false;
            k4a_float3_t jointPos3D = { { x, y, z } };
            k4a_float2_t jointPos2D;
            try
            {
                isValid = calibration.convert_3d_to_2d(jointPos3D, K4A_CALIBRATION_TYPE_DEPTH, K4A_CALIBRATION_TYPE_COLOR, &jointPos2D);
            }
            catch (const std::exception&)
            {
                isValid = false;
            }

            //points that fall outside the visible area of the color camera are set to 0
            //according to https://docs.microsoft.com/en-us/azure/kinect-dk/use-calibration-functions
            isValid = jointPos2D.xy.x > 0.0f && jointPos2D.xy.y > 0.0f;

            //subpixel coordinates have the center of the pixel at its "int" coordinate. i.e. (0, 0) ranges from (-0.4999, -0.4999) to (0.4999, 0.4999)
            //see: https://docs.microsoft.com/en-us/azure/kinect-dk/coordinate-systems
            //therefore a normal round (i.e. >= 0.5) goes to the next higher number, which is the correct integer pixel-coordinate for the subpixel coordinate
            return std::make_pair(isValid, cv::Point(static_cast<int>(std::round(jointPos2D.xy.x)), static_cast<int>(std::round(jointPos2D.xy.y))));
        }

        inline void transform(int16_t* result, const int16_t* vector, const double* matrix)
        {
            result[0] = (vector[0] * matrix[0] + vector[1] * matrix[1] + vector[2] * matrix[2] + matrix[3]);
            result[1] = (vector[0] * matrix[4] + vector[1] * matrix[5] + vector[2] * matrix[6] + matrix[7]);
            result[2] = (vector[0] * matrix[8] + vector[1] * matrix[9] + vector[2] * matrix[10] + matrix[11]);
        }
    } //namespace AK
} //namespace ssi



#endif // !SSI_AZUREKINECT_HELPERS
