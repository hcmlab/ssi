// AzureKinect.h
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

#ifndef SSI_SENSOR_AZUREKINECT_H
#define	SSI_SENSOR_AZUREKINECT_H

#include "base/ISensor.h"
#include "base/IProvider.h"
#include "thread/Thread.h"
#include "ioput/option/OptionList.h"

#include <ssiocv.h>

#include <tuple>
#include <iostream>

#include <k4a/k4a.hpp> /* ATTENTION: THIS FILE WAS MODIFIED BECAUSE IT DID NOT COMPILE DUE TO A CLASH BETWEEN std::min/max AND min/max preprocessor definitions */

#include "AzureKinectDatatypes.h"
#include "AzureKinectHelpers.h"

namespace ssi {
	class Timer;
}

namespace ssi {

	using namespace AK;

#define SSI_AZUREKINECT_RGBIMAGE_PROVIDER_NAME "rgb"
#define SSI_AZUREKINECT_DEPTHRAWIMAGE_PROVIDER_NAME "depthraw"
#define SSI_AZUREKINECT_DEPTHVISUALISATIONIMAGE_PROVIDER_NAME "depthvisualisation"
#define SSI_AZUREKINECT_IRRAWIMAGE_PROVIDER_NAME "irraw"
#define SSI_AZUREKINECT_IRVISUALISATIONIMAGE_PROVIDER_NAME "irvisualisation"

#define SSI_AZUREKINECT_VIDEORESOLUTION_OPTION_720P "720p"
#define SSI_AZUREKINECT_VIDEORESOLUTION_OPTION_1080P "1080p"
#define SSI_AZUREKINECT_VIDEORESOLUTION_OPTION_1440P "1440p"
#define SSI_AZUREKINECT_VIDEORESOLUTION_OPTION_1536P "1536p"
#define SSI_AZUREKINECT_VIDEORESOLUTION_OPTION_2160P "2160p"
#define SSI_AZUREKINECT_VIDEORESOLUTION_OPTION_3072P "3072p"

#define SSI_AZUREKINECT_DEPTHMODE_OPTION_NFOV_BINNED "NFOV_BINNED"
#define SSI_AZUREKINECT_DEPTHMODE_OPTION_NFOV_UNBINNED "NFOV_UNBINNED"
#define SSI_AZUREKINECT_DEPTHMODE_OPTION_WFOV_BINNED "WFOV_BINNED"
#define SSI_AZUREKINECT_DEPTHMODE_OPTION_WFOV_UNBINNED "WFOV_UNBINNED"
#define SSI_AZUREKINECT_DEPTHMODE_OPTION_PASSIVE_IR "PASSIVE_IR"

class AzureKinect : public ISensor, public Thread {
public:

	class RGBImageChannel : public IChannel {

		friend class AzureKinect;
	public:
		RGBImageChannel() {

			ssi_stream_init(stream, 0, 1, 0, SSI_IMAGE, 0);
		}
		~RGBImageChannel() {
			ssi_stream_destroy(stream);
		}
		const ssi_char_t* getName() { return SSI_AZUREKINECT_RGBIMAGE_PROVIDER_NAME; };
		const ssi_char_t* getInfo() { return "RGB image with resolution as set by options and 4 byte rgba pixels."; };
		ssi_stream_t getStream() { return stream; };
		ssi_stream_t* getStreamPtr() { return &stream; };
	protected:
		ssi_stream_t stream;
	};

	class DepthVisualisationImageChannel : public IChannel {

		friend class AzureKinect;
	public:
		DepthVisualisationImageChannel() {

			ssi_stream_init(stream, 0, 1, 0, SSI_IMAGE, 0);
		}
		~DepthVisualisationImageChannel() {
			ssi_stream_destroy(stream);
		}
		const ssi_char_t* getName() { return SSI_AZUREKINECT_DEPTHVISUALISATIONIMAGE_PROVIDER_NAME; };
		const ssi_char_t* getInfo() { return "Depth visualisation with 3 byte rgb pixels ranging from blue (near) to red (far)."; };
		ssi_stream_t getStream() { return stream; };
		ssi_stream_t* getStreamPtr() { return &stream; };
	protected:
		ssi_stream_t stream;
	};

	class DepthRawImageChannel : public IChannel {

		friend class AzureKinect;
	public:
		DepthRawImageChannel() {

			ssi_stream_init(stream, 0, 1, 0, SSI_IMAGE, 0);
		}
		~DepthRawImageChannel() {
			ssi_stream_destroy(stream);
		}
		const ssi_char_t* getName() { return SSI_AZUREKINECT_DEPTHRAWIMAGE_PROVIDER_NAME; };
		const ssi_char_t* getInfo() { return "Depth image with 2 byte depth-pixels signifying raw depth values in mm."; };
		ssi_stream_t getStream() { return stream; };
		ssi_stream_t* getStreamPtr() { return &stream; };
	protected:
		ssi_stream_t stream;
	};

	class IRVisualisationImageChannel : public IChannel {

		friend class AzureKinect;
	public:
		IRVisualisationImageChannel() {

			ssi_stream_init(stream, 0, 1, 0, SSI_IMAGE, 0);
		}
		~IRVisualisationImageChannel() {
			ssi_stream_destroy(stream);
		}
		const ssi_char_t* getName() { return SSI_AZUREKINECT_IRVISUALISATIONIMAGE_PROVIDER_NAME; };
		const ssi_char_t* getInfo() { return "IR image visualisation with 1 byte grayscale values."; };
		ssi_stream_t getStream() { return stream; };
		ssi_stream_t* getStreamPtr() { return &stream; };
	protected:
		ssi_stream_t stream;
	};

	class IRRawImageChannel : public IChannel {

		friend class AzureKinect;
	public:
		IRRawImageChannel() {

			ssi_stream_init(stream, 0, 1, 0, SSI_IMAGE, 0);
		}
		~IRRawImageChannel() {
			ssi_stream_destroy(stream);
		}
		const ssi_char_t* getName() { return SSI_AZUREKINECT_IRRAWIMAGE_PROVIDER_NAME; };
		const ssi_char_t* getInfo() { return "IR image with 2 byte raw values."; };
		ssi_stream_t getStream() { return stream; };
		ssi_stream_t* getStreamPtr() { return &stream; };
	protected:
		ssi_stream_t stream;
	};

public:

	class Options : public OptionList {

	public:
		Options() : sr(30.0),
					_rgbResolution(RGB_VIDEO_RESOLUTION::p_1920x1080),
					_depthMode(DEPTH_MODE::NFOV_UNBINNED)
		{
			addOption("sr", &sr, 1, SSI_TIME, "sample rate in hz");
			addOption("rgbResolution", videoResolutionIn, SSI_MAX_CHAR, SSI_CHAR, "Resolution of the rgb video. Must be one of ['720p', '1080p', '1440p', '1536p', '2160p', '3072p']");
			addOption("depthMode", depthModeIn, SSI_MAX_CHAR, SSI_CHAR, "Depth mode. Must be one of ['NFOV_BINNED', 'NFOV_UNBINNED', 'WFOV_BINNED', 'WFOV_UNBINNED', 'PASSIVE_IR']");

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
				resolution = RGB_VIDEO_RESOLUTION::p_1280x720;
			}

			try
			{
				std::tie(rgbVideoWidth, rgbVideoHeight) = GetColorDimensions(resolution);

				if (resolution == RGB_VIDEO_RESOLUTION::p_4096x3072 && sr > 15.0) {
					sr = 15.0;
				}

				_rgbResolution = resolution;
			}
			catch (const std::exception&)
			{
				rgbVideoWidth = 0;
				rgbVideoHeight = 0;
				_rgbResolution = RGB_VIDEO_RESOLUTION::OFF;
			}
		}

		void setDepthModeIn(const ssi_char_t* depthModeString) {
			std::cout << "Setting AK depth mode: " << depthModeString << "\n";
			DEPTH_MODE mode;

			if (strcmp(depthModeString, SSI_AZUREKINECT_DEPTHMODE_OPTION_NFOV_UNBINNED) == 0) {
				mode = DEPTH_MODE::NFOV_UNBINNED;
			} else if (strcmp(depthModeString, SSI_AZUREKINECT_DEPTHMODE_OPTION_NFOV_BINNED) == 0) {
				mode = DEPTH_MODE::NFOV_2x2_BINNED;
			} else if (strcmp(depthModeString, SSI_AZUREKINECT_DEPTHMODE_OPTION_WFOV_BINNED) == 0) {
				mode = DEPTH_MODE::WFOV_2x2_BINNED;
			} else if (strcmp(depthModeString, SSI_AZUREKINECT_DEPTHMODE_OPTION_WFOV_UNBINNED) == 0) {
				mode = DEPTH_MODE::WFOV_UNBINNED;
			} else if (strcmp(depthModeString, SSI_AZUREKINECT_DEPTHMODE_OPTION_PASSIVE_IR) == 0) {
				mode = DEPTH_MODE::PASSIVE_IR;
			}
			else {
				mode = DEPTH_MODE::NFOV_UNBINNED;
			}

			try
			{
				std::tie(depthVideoWidth, depthVideoHeight) = GetDepthDimensions(mode);

				if (mode == DEPTH_MODE::WFOV_UNBINNED && sr > 15.0) {
					sr = 15.0;
				}

				_depthMode = mode;
			}
			catch (const std::exception&)
			{
				depthVideoWidth = 0;
				depthVideoHeight = 0;
				_depthMode = DEPTH_MODE::OFF;
			}
		}

		ssi_time_t sr;
		ssi_size_t rgbVideoWidth, rgbVideoHeight;
		RGB_VIDEO_RESOLUTION _rgbResolution;
		ssi_size_t depthVideoWidth, depthVideoHeight;
		DEPTH_MODE _depthMode;

	private:
		ssi_char_t videoResolutionIn[SSI_MAX_CHAR];
		ssi_char_t depthModeIn[SSI_MAX_CHAR];
	};

public:

	static const ssi_char_t* GetCreateName() { return "AzureKinect"; };
	static IObject* Create(const ssi_char_t* file) { return new AzureKinect(file); };
	~AzureKinect();
	AzureKinect::Options* getOptions() { return &_options; };
	const ssi_char_t* getName() { return GetCreateName(); };
	const ssi_char_t* getInfo() { return "Azure Kinect DK Sensor"; };

	ssi_size_t getChannelSize() { return 1; };
	IChannel* getChannel(ssi_size_t index) {
		return &m_rgb_channel;
	};
	bool setProvider(const ssi_char_t* name, IProvider* provider);
	bool connect();
	bool start();
	bool stop();
	void run();
	bool disconnect();

	ssi_video_params_t getRGBImageParams()
	{
		ssi_video_params_t vParam;

		ssi_video_params(vParam, _options.rgbVideoWidth, _options.rgbVideoHeight, _options.sr, SSI_VIDEO_DEPTH_8U, 4);
		vParam.flipImage = false;
		return vParam;
	}

	ssi_video_params_t getDepthVisualisationImageParams()
	{
		ssi_video_params_t vParam;

		ssi_video_params(vParam, _options.depthVideoWidth, _options.depthVideoHeight, _options.sr, SSI_VIDEO_DEPTH_8U, 3);
		vParam.flipImage = false;
		return vParam;
	}

	ssi_video_params_t getDepthRawImageParams()
	{
		ssi_video_params_t vParam;

		ssi_video_params(vParam, _options.depthVideoWidth, _options.depthVideoHeight, _options.sr, SSI_VIDEO_DEPTH_16U, 1);
		vParam.flipImage = false;
		return vParam;
	}

	ssi_video_params_t getIRVisualisationImageParams()
	{
		ssi_video_params_t vParam;

		ssi_video_params(vParam, _options.depthVideoWidth, _options.depthVideoHeight, _options.sr, SSI_VIDEO_DEPTH_8U, 1);
		vParam.flipImage = false;
		return vParam;
	}

	ssi_video_params_t getIRRawImageParams()
	{
		ssi_video_params_t vParam;

		ssi_video_params(vParam, _options.depthVideoWidth, _options.depthVideoHeight, _options.sr, SSI_VIDEO_DEPTH_16U, 1);
		vParam.flipImage = false;
		return vParam;
	}

	void setLogLevel(int level) {
		ssi_log_level = level;
	}

protected:
	AzureKinect(const ssi_char_t* file = 0);

	void startCameras();
	void stopCameras();

	void process();

	void applyOptionsToCameraConfiguration(k4a_device_configuration_t & config);

	void getCapture();
	void releaseCapture();
	void processRGBAImage();
	void processDepthImage();
	void processIRImage();
	void processProviders();

	AzureKinect::Options _options;
	ssi_char_t* _file;

	static ssi_char_t* ssi_log_name;
	int ssi_log_level;

	Timer* m_timer;

	k4a::device m_azureKinectDevice;
	bool m_camerasStarted;

	k4a::capture m_capturedFrame;

	BgraPixel* m_bgraBuffer;
	DepthPixel* m_depthRawBuffer;
	BgrPixel* m_depthVisualisationBuffer;
	cv::Mat m_depthHSVConversionMat; //helper, constructed over the m_depthBuffer to use opencv color conversion functions on it
	DepthPixel* m_irRawBuffer;
	IRPixel* m_irVisualisationBuffer;

	RGBImageChannel m_rgb_channel;
	IProvider* m_rgb_provider;

	DepthRawImageChannel m_depthRaw_channel;
	IProvider* m_depthRaw_provider;

	DepthVisualisationImageChannel m_depthVisualisation_channel;
	IProvider* m_depthVisualisation_provider;

	IRRawImageChannel m_irRaw_channel;
	IProvider* m_irRaw_provider;

	IRVisualisationImageChannel m_irVisualisation_channel;
	IProvider* m_irVisualisation_provider;

private:
	bool setImageProvider(IProvider* providerIn, IProvider* &internalProvider, IChannel &internalChannel, ssi_video_params_t params);
};

}

#endif //SSI_SENSOR_AZUREKINECT_H

