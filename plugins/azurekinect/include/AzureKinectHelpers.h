#pragma once
#ifndef SSI_AZUREKINECT_HELPERS
#define SSI_AZUREKINECT_HELPERS


#include <cstdint>
#include <exception>
#include <utility>

#include "k4a/k4a.hpp"

#include "AzureKinectDatatypes.h"

namespace ssi {
    namespace AK {
        // Gets the dimensions of the color images that the color camera will produce for a
    // given color resolution
    //
        inline std::pair<int, int> GetColorDimensions(const RGB_VIDEO_RESOLUTION resolution)
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

            default:
                throw std::logic_error("Invalid color dimensions value!");
            }
        }

        // Gets the dimensions of the depth images that the depth camera will produce for a
        // given depth mode
        //
        inline std::pair<int, int> GetDepthDimensions(const DEPTH_MODE depthMode)
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

            default:
                throw std::logic_error("Invalid depth dimensions value!");
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
        inline std::pair<uint16_t, uint16_t> GetIrLevels(const DEPTH_MODE depthMode)
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
    } //namespace AK
} //namespace ssi



#endif // !SSI_AZUREKINECT_HELPERS
