#pragma once

#include "ioput/option/OptionList.h"

#include <k4a/k4a.hpp>

#include "AzureKinectDatatypes.h"
#include "AzureKinectHelpers.h"

#define SSI_AZUREKINECT_VIDEORESOLUTION_OPTION_720P "720p"
#define SSI_AZUREKINECT_VIDEORESOLUTION_OPTION_1080P "1080p"
#define SSI_AZUREKINECT_VIDEORESOLUTION_OPTION_1440P "1440p"
#define SSI_AZUREKINECT_VIDEORESOLUTION_OPTION_1536P "1536p"
#define SSI_AZUREKINECT_VIDEORESOLUTION_OPTION_2160P "2160p"
#define SSI_AZUREKINECT_VIDEORESOLUTION_OPTION_3072P "3072p"
#define SSI_AZUREKINECT_VIDEORESOLUTION_OPTION_OFF "OFF"

#define SSI_AZUREKINECT_DEPTHMODE_OPTION_NFOV_BINNED "NFOV_BINNED"
#define SSI_AZUREKINECT_DEPTHMODE_OPTION_NFOV_UNBINNED "NFOV_UNBINNED"
#define SSI_AZUREKINECT_DEPTHMODE_OPTION_WFOV_BINNED "WFOV_BINNED"
#define SSI_AZUREKINECT_DEPTHMODE_OPTION_WFOV_UNBINNED "WFOV_UNBINNED"
#define SSI_AZUREKINECT_DEPTHMODE_OPTION_PASSIVE_IR "PASSIVE_IR"
#define SSI_AZUREKINECT_DEPTHMODE_OPTION_PASSIVE_OFF "OFF"

namespace ssi {
	namespace AK {

		class Options : public OptionList {

		public:
			Options() : sr(30.0),
				_rgbResolution(RGB_VIDEO_RESOLUTION::p_1920x1080),
				_depthMode(DEPTH_MODE::WFOV_2x2_BINNED),
				nrOfBodiesToTrack(1),
				showBodyTracking(true)
			{
				addOption("sr", &sr, 1, SSI_TIME, "sample rate in hz");
				addOption("rgbResolution", videoResolutionIn, SSI_MAX_CHAR, SSI_CHAR, "Resolution of the rgb video. Must be one of ['720p', '1080p', '1440p', '1536p', '2160p', '3072p']. Set to 'OFF' to deactivate.");
				addOption("depthMode", depthModeIn, SSI_MAX_CHAR, SSI_CHAR, "Depth mode. Must be one of ['NFOV_BINNED', 'NFOV_UNBINNED', 'WFOV_BINNED', 'WFOV_UNBINNED', 'PASSIVE_IR']. Set to 'OFF' to deactivate.");
				addOption("nrOfBodiesToTrack", &nrOfBodiesToTrack, 0, SSI_SIZE, "The number of bodies to track. Default is 0, set to at least 1 to track someone! (Azure Kinect doesn't seem to have a limit for the nr of bodies it can track)");
				addOption("showBodyTracking", &showBodyTracking, 1, SSI_BOOL, "show body tracking (only paints basic joints and bones, use the SkeletonPainter plugin for more options");

				//TODO: add sensor orientation (for optimal tracker configuration)

				//defaults:
				std::tie(rgbVideoWidth, rgbVideoHeight) = GetColorDimensions(_rgbResolution);
				std::tie(depthVideoWidth, depthVideoHeight) = GetDepthDimensions(_depthMode);
			};

			void setVideoResolutionIn(const ssi_char_t* resolutionString) {
				std::cout << "Setting AK video resolution: " << resolutionString << "\n";

				RGB_VIDEO_RESOLUTION resolution;

				if (strcmp(resolutionString, SSI_AZUREKINECT_VIDEORESOLUTION_OPTION_1080P) == 0) {
					resolution = RGB_VIDEO_RESOLUTION::p_1920x1080;
				}
				else if (strcmp(resolutionString, SSI_AZUREKINECT_VIDEORESOLUTION_OPTION_1440P) == 0) {
					resolution = RGB_VIDEO_RESOLUTION::p_2560x1440;
				}
				else if (strcmp(resolutionString, SSI_AZUREKINECT_VIDEORESOLUTION_OPTION_1536P) == 0) {
					resolution = RGB_VIDEO_RESOLUTION::p_2048x1536;
				}
				else if (strcmp(resolutionString, SSI_AZUREKINECT_VIDEORESOLUTION_OPTION_2160P) == 0) {
					resolution = RGB_VIDEO_RESOLUTION::p_3840x2160;
				}
				else if (strcmp(resolutionString, SSI_AZUREKINECT_VIDEORESOLUTION_OPTION_3072P) == 0) {
					resolution = RGB_VIDEO_RESOLUTION::p_4096x3072;
				}
				else if (strcmp(resolutionString, SSI_AZUREKINECT_VIDEORESOLUTION_OPTION_720P) == 0) {
					resolution = RGB_VIDEO_RESOLUTION::p_1280x720;
				}
				else {
					resolution = RGB_VIDEO_RESOLUTION::OFF;
				}

				std::tie(rgbVideoWidth, rgbVideoHeight) = GetColorDimensions(resolution);

				if (resolution == RGB_VIDEO_RESOLUTION::p_4096x3072 && sr > 15.0) {
					sr = 15.0;
				}

				_rgbResolution = resolution;
			}

			void setDepthModeIn(const ssi_char_t* depthModeString) {
				std::cout << "Setting AK depth mode: " << depthModeString << "\n";
				DEPTH_MODE mode;

				if (strcmp(depthModeString, SSI_AZUREKINECT_DEPTHMODE_OPTION_NFOV_UNBINNED) == 0) {
					mode = DEPTH_MODE::NFOV_UNBINNED;
				}
				else if (strcmp(depthModeString, SSI_AZUREKINECT_DEPTHMODE_OPTION_NFOV_BINNED) == 0) {
					mode = DEPTH_MODE::NFOV_2x2_BINNED;
				}
				else if (strcmp(depthModeString, SSI_AZUREKINECT_DEPTHMODE_OPTION_WFOV_BINNED) == 0) {
					mode = DEPTH_MODE::WFOV_2x2_BINNED;
				}
				else if (strcmp(depthModeString, SSI_AZUREKINECT_DEPTHMODE_OPTION_WFOV_UNBINNED) == 0) {
					mode = DEPTH_MODE::WFOV_UNBINNED;
				}
				else if (strcmp(depthModeString, SSI_AZUREKINECT_DEPTHMODE_OPTION_PASSIVE_IR) == 0) {
					mode = DEPTH_MODE::PASSIVE_IR;
				}
				else {
					mode = DEPTH_MODE::OFF;
				}

				std::tie(depthVideoWidth, depthVideoHeight) = GetDepthDimensions(mode);

				if (mode == DEPTH_MODE::WFOV_UNBINNED && sr > 15.0) {
					sr = 15.0;
				}

				_depthMode = mode;
			}


			void applyToCameraConfiguration(k4a_device_configuration_t& config)
			{
				//fps
				if (sr <= 5.0) {
					config.camera_fps = K4A_FRAMES_PER_SECOND_5;
				}
				else if (sr <= 15.0) {
					config.camera_fps = K4A_FRAMES_PER_SECOND_15;
				}
				else {
					config.camera_fps = K4A_FRAMES_PER_SECOND_30;
				}

				//color format - always RGB32 as the other option (compressed JPG) is not desirable
				config.color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;

				bool synchronizeColorAndDepth = true;

				//color camera resolution
				k4a_color_resolution_t colorRes;
				switch (_rgbResolution)
				{
				case RGB_VIDEO_RESOLUTION::p_3840x2160:
					colorRes = K4A_COLOR_RESOLUTION_2160P;
					break;
				case RGB_VIDEO_RESOLUTION::p_2560x1440:
					colorRes = K4A_COLOR_RESOLUTION_1440P;
					break;
				case RGB_VIDEO_RESOLUTION::p_1920x1080:
					colorRes = K4A_COLOR_RESOLUTION_1080P;
					break;
				case RGB_VIDEO_RESOLUTION::p_1280x720:
					colorRes = K4A_COLOR_RESOLUTION_720P;
					break;
				case RGB_VIDEO_RESOLUTION::p_4096x3072:
					colorRes = K4A_COLOR_RESOLUTION_3072P;
					break;
				case RGB_VIDEO_RESOLUTION::p_2048x1536:
					colorRes = K4A_COLOR_RESOLUTION_1536P;
					break;
				case RGB_VIDEO_RESOLUTION::OFF:
				default:
					colorRes = K4A_COLOR_RESOLUTION_OFF;
					synchronizeColorAndDepth = false;
					break;
				}
				config.color_resolution = colorRes;

				//depth sensor mode
				k4a_depth_mode_t depthMode;
				switch (_depthMode)
				{
				case DEPTH_MODE::NFOV_UNBINNED:
					depthMode = K4A_DEPTH_MODE_NFOV_UNBINNED;
					break;
				case DEPTH_MODE::NFOV_2x2_BINNED:
					depthMode = K4A_DEPTH_MODE_NFOV_2X2BINNED;
					break;
				case DEPTH_MODE::WFOV_UNBINNED:
					depthMode = K4A_DEPTH_MODE_WFOV_UNBINNED;
					break;
				case DEPTH_MODE::WFOV_2x2_BINNED:
					depthMode = K4A_DEPTH_MODE_WFOV_2X2BINNED;
					break;
				case DEPTH_MODE::PASSIVE_IR:
					depthMode = K4A_DEPTH_MODE_PASSIVE_IR;
					break;
				case DEPTH_MODE::OFF:
				default:
					depthMode = K4A_DEPTH_MODE_OFF;
					synchronizeColorAndDepth = false;
					break;
				}
				config.depth_mode = depthMode;

				config.synchronized_images_only = synchronizeColorAndDepth;
			}


			ssi_time_t sr;
			ssi_size_t rgbVideoWidth, rgbVideoHeight;
			RGB_VIDEO_RESOLUTION _rgbResolution;
			ssi_size_t depthVideoWidth, depthVideoHeight;
			DEPTH_MODE _depthMode;

			ssi_size_t nrOfBodiesToTrack;
			bool showBodyTracking;

		private:
			ssi_char_t videoResolutionIn[SSI_MAX_CHAR];
			ssi_char_t depthModeIn[SSI_MAX_CHAR];
		};

	}
}
