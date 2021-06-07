// AzureKinectOptions.h
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

#include "ioput/option/OptionList.h"

#include <k4a/k4a.hpp>

#include "AzureKinectDatatypes.h"
#include "AzureKinectHelpers.h"

namespace ssi {
	namespace AK {

		class Options : public OptionList {

		public:
			Options() : sr(30.0),
				rgbResolution(RGB_VIDEO_RESOLUTION::p_1920x1080),
				depthMode(DEPTH_MODE::WFOV_2x2_BINNED),
				nrOfBodiesToTrack(0),
				showBodyTracking(true),
				bodyTrackingSmoothingFactor(0.0f)
			{
				addOption("sr", &sr, 1, SSI_TIME, "sample rate in hz (Kinect only supports 5.0, 15.0, and 30.0). Default: 30.0");
				addOption("rgbResolution", &rgbResolution, 1, SSI_INT, "Resolution of the rgb video, default: 1080p. Set to OFF to deactivate. (0=2160p,1=1440p,2=1080p,3=720p,4=3072p,5=1536p,6=OFF). Option 3072p only up to sr = 15.0");
				addOption("depthMode", &depthMode, 1, SSI_INT, "Depth mode, default: WFOV_BINNED. Set to OFF to deactivate (will be activated anyway with default if e.g. bodytracking or pointcloud are used). (0=NFOV_UNBINNED,1=NFOV_BINNED,2=WFOV_UNBINNED,3=WFOV_BINNED,4=PASSIVE_IR,5=OFF). Option WFOV_UNBINNED only up to sr = 15.0");
				addOption("nrOfBodiesToTrack", &nrOfBodiesToTrack, 1, SSI_SIZE, "The number of bodies to track. Default is 0, set to at least 1 to track someone! (Azure Kinect doesn't seem to have a limit for the nr of bodies it can track)");
				addOption("showBodyTracking", &showBodyTracking, 1, SSI_BOOL, "show body tracking, default: true (if at least one body is tracked). Only paints basic joints and bones, use the SkeletonPainter plugin for more options.");
				addOption("bodyTrackingSmoothingFactor", &bodyTrackingSmoothingFactor, 1, SSI_DOUBLE, "Control bodytracking smoothing between 0.0 (none) - 1.0 (full). Default: 0.0. Less smoothing will increase the responsiveness of the detected skeletons but will cause more positional and orientational jitters.");

				//TODO: add sensor orientation (for optimal tracker configuration)

				enforceProperConfiguration();
			};

			void enforceProperConfiguration() {
				enforceProperRGBVideoConfig();
				enforceProperDepthConfig();
				enforceProperSR();

				showBodyTracking = showBodyTracking && nrOfBodiesToTrack > 0;
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

				//color format - always RGB32 as the other option (compressed JPG) is not desirable within ssi
				config.color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;

				bool synchronizeColorAndDepth = true;

				//color camera resolution
				k4a_color_resolution_t colorRes;
				switch (rgbResolution)
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
				k4a_depth_mode_t kinectDepthMode;
				switch (depthMode)
				{
				case DEPTH_MODE::NFOV_UNBINNED:
					kinectDepthMode = K4A_DEPTH_MODE_NFOV_UNBINNED;
					break;
				case DEPTH_MODE::NFOV_2x2_BINNED:
					kinectDepthMode = K4A_DEPTH_MODE_NFOV_2X2BINNED;
					break;
				case DEPTH_MODE::WFOV_UNBINNED:
					kinectDepthMode = K4A_DEPTH_MODE_WFOV_UNBINNED;
					break;
				case DEPTH_MODE::WFOV_2x2_BINNED:
					kinectDepthMode = K4A_DEPTH_MODE_WFOV_2X2BINNED;
					break;
				case DEPTH_MODE::PASSIVE_IR:
					kinectDepthMode = K4A_DEPTH_MODE_PASSIVE_IR;
					break;
				case DEPTH_MODE::OFF:
				default:
					kinectDepthMode = K4A_DEPTH_MODE_OFF;
					synchronizeColorAndDepth = false;
					break;
				}
				config.depth_mode = kinectDepthMode;

				config.synchronized_images_only = synchronizeColorAndDepth;
			}

			ssi_time_t sr;
			ssi_size_t rgbVideoWidth, rgbVideoHeight;
			RGB_VIDEO_RESOLUTION rgbResolution;
			ssi_size_t depthVideoWidth, depthVideoHeight;
			DEPTH_MODE depthMode;

			ssi_size_t nrOfBodiesToTrack;
			bool showBodyTracking;
			double bodyTrackingSmoothingFactor;

			private:
				void enforceProperRGBVideoConfig() noexcept {
					std::tie(rgbVideoWidth, rgbVideoHeight) = GetColorDimensions(rgbResolution);

					if (rgbResolution == RGB_VIDEO_RESOLUTION::p_4096x3072 && sr > 15.0) {
						sr = 15.0;
					}
				}

				void enforceProperDepthConfig() {
					std::tie(depthVideoWidth, depthVideoHeight) = GetDepthDimensions(depthMode);

					if (depthMode == DEPTH_MODE::WFOV_UNBINNED && sr > 15.0) {
						sr = 15.0;
					}
				}

				void enforceProperSR() {
					if (sr <= 5.0) {
						sr = 5.0;
					}
					else if (sr <= 15.0) {
						sr = 15.0;
					}
					else {
						sr = 30.0;
					}
				}
		};

	}
}
