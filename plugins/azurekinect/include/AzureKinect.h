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

#include <tuple>

#include <k4a/k4a.hpp> /* ATTENTION: THIS FILE WAS MODIFIED BECAUSE IT DID NOT COMPILE DUE TO A CLASH BETWEEN std::min/max AND min/max preprocessor definitions */

#include "AzureKinectDatatypes.h"
#include "AzureKinectHelpers.h"

namespace ssi {
	class Timer;
}

namespace ssi {

	using namespace AK;

#define SSI_AZUREKINECT_RGBIMAGE_PROVIDER_NAME "rgb"
#define SSI_AZUREKINECT_DEPTHIMAGE_PROVIDER_NAME "depth"
#define SSI_AZUREKINECT_IRRAWIMAGE_PROVIDER_NAME "irraw"
#define SSI_AZUREKINECT_IRIMAGE_PROVIDER_NAME "ir"

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
		const ssi_char_t* getInfo() { return "RGB image with TODO: resolution and color depth."; };
		ssi_stream_t getStream() { return stream; };
		ssi_stream_t* getStreamPtr() { return &stream; };
	protected:
		ssi_stream_t stream;
	};

	class DepthImageChannel : public IChannel {

		friend class AzureKinect;
	public:
		DepthImageChannel() {

			ssi_stream_init(stream, 0, 1, 0, SSI_IMAGE, 0);
		}
		~DepthImageChannel() {
			ssi_stream_destroy(stream);
		}
		const ssi_char_t* getName() { return SSI_AZUREKINECT_DEPTHIMAGE_PROVIDER_NAME; };
		const ssi_char_t* getInfo() { return "Depth image with 2 byte pixels."; };
		ssi_stream_t getStream() { return stream; };
		ssi_stream_t* getStreamPtr() { return &stream; };
	protected:
		ssi_stream_t stream;
	};

	class IRImageChannel : public IChannel {

		friend class AzureKinect;
	public:
		IRImageChannel() {

			ssi_stream_init(stream, 0, 1, 0, SSI_IMAGE, 0);
		}
		~IRImageChannel() {
			ssi_stream_destroy(stream);
		}
		const ssi_char_t* getName() { return SSI_AZUREKINECT_IRIMAGE_PROVIDER_NAME; };
		const ssi_char_t* getInfo() { return "IR image with 1 byte grayscale values."; };
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
			//addOption("rgbResolution", &_resolution, sizeof(), , "video resolution of the rgb camera");

			setRgbResolution(_rgbResolution);
			setDepthMode(_depthMode);
		};

		void setRgbResolution(RGB_VIDEO_RESOLUTION resolution) {
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

		void setDepthMode(DEPTH_MODE mode) {
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

	ssi_video_params_t getDepthImageParams()
	{
		ssi_video_params_t vParam;

		ssi_video_params(vParam, _options.depthVideoWidth, _options.depthVideoHeight, _options.sr, SSI_VIDEO_DEPTH_16U, 1);
		vParam.flipImage = false;
		return vParam;
	}

	ssi_video_params_t getIRImageParams()
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

	void captureImages();
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
	DepthPixel* m_depthBuffer;
	DepthPixel* m_irRawBuffer;
	IRPixel* m_irBuffer;

	RGBImageChannel m_rgb_channel;
	IProvider* m_rgb_provider;

	RGBImageChannel m_depth_channel;
	IProvider* m_depth_provider;

	RGBImageChannel m_irRaw_channel;
	IProvider* m_irRaw_provider;

	RGBImageChannel m_ir_channel;
	IProvider* m_ir_provider;

private:
	bool setImageProvider(IProvider* providerIn, IProvider* &internalProvider, IChannel &internalChannel, ssi_video_params_t params);
};

}

#endif //SSI_SENSOR_AZUREKINECT_H

