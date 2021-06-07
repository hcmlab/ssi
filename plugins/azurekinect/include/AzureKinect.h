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

#include <ssiocv.h>

#include <tuple>
#include <iostream>

//azure kinect sensor sdk
#include <k4a/k4a.hpp> /* ATTENTION: THIS FILE WAS MODIFIED BECAUSE IT DID NOT COMPILE DUE TO A CLASH BETWEEN std::min/max AND min/max preprocessor definitions */
//azure kinect body tracking
#include <k4abt.hpp>

#include "AzureKinectDatatypes.h"
#include "AzureKinectHelpers.h"
#include "AzureKinectOptions.h"

#define SSI_AZUREKINECT_BYTESPERDEPTHCLOUDVOXEL 6

#define SSI_AZUREKINECT_RGBIMAGE_PROVIDER_NAME "rgb"
#define SSI_AZUREKINECT_DEPTHRAWIMAGE_PROVIDER_NAME "depthraw"
#define SSI_AZUREKINECT_DEPTHVISUALISATIONIMAGE_PROVIDER_NAME "depthvisualisation"
#define SSI_AZUREKINECT_IRRAWIMAGE_PROVIDER_NAME "irraw"
#define SSI_AZUREKINECT_IRVISUALISATIONIMAGE_PROVIDER_NAME "irvisualisation"
#define SSI_AZUREKINECT_SKELETON_PROVIDER_NAME "skeleton"
#define SSI_AZUREKINECT_POINTCLOUD_PROVIDER_NAME "pointcloud"

namespace ssi {
	class Timer;
}

namespace ssi {

	using namespace AK;

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
		const ssi_char_t* getInfo() { return "Depth visualisation with 3 byte rgb pixels ranging from red (near) to blue (far)."; };
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

	class PointCloudChannel : public IChannel {

		friend class AzureKinect;

	public:
		PointCloudChannel() {
			ssi_stream_init(stream, 0, 0, sizeof(unsigned char), SSI_UCHAR, 0);
		}
		~PointCloudChannel() {
			ssi_stream_destroy(stream);
		}
		const ssi_char_t* getName() { return SSI_AZUREKINECT_POINTCLOUD_PROVIDER_NAME; };
		const ssi_char_t* getInfo() { return "3-D coordinates (relative to Kinect camera) for every pixel in the depth image. 2 byte integer per dimension, unit: mm, positive Y points towards ground!"; };
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

	class SkeletonChannel : public IChannel {

		friend class AzureKinect;

	public:
		SkeletonChannel() {
			ssi_stream_init(stream, 0, 0, sizeof(float), SSI_FLOAT, 0);
		}
		~SkeletonChannel() {
			ssi_stream_destroy(stream);
		}
		const ssi_char_t* getName() { return SSI_AZUREKINECT_SKELETON_PROVIDER_NAME; };
		const ssi_char_t* getInfo() { return "Reports 3D skeleton positions of 32 joints."; };
		ssi_stream_t getStream() { return stream; };
	protected:
		ssi_stream_t stream;
	};

public:

	static const ssi_char_t* GetCreateName() { return "AzureKinect"; };
	static IObject* Create(const ssi_char_t* file) { return new AzureKinect(file); };
	~AzureKinect();
	AzureKinect::Options* getOptions() { return &_options; };
	const ssi_char_t* getName() { return GetCreateName(); };
	const ssi_char_t* getInfo() { return "Azure Kinect DK Sensor. Before first use, make sure AzureKinect SensorSDK and BodyTrackingSDK are installed and added to the PATH."; };

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

	ssi_video_params_t getPointCloudImageParams()
	{
		ssi_video_params_t vParam;

		ssi_video_params(vParam, _options.depthVideoWidth, _options.depthVideoHeight, _options.sr, SSI_VIDEO_DEPTH_16S, 3);
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

	void initBuffers();

	void startCameras();
	void stopCameras();

	void process();

	void getCapture();
	void passCaptureToBodyTracker();
	void releaseCapture();
	void processRGBImage();
	void processDepthImage();
	void processIRImage();
	void processBodyTracking();
	void visualizeTrackedBodies();
	void processProviders();

	Options _options;
	ssi_char_t* _file;

	static ssi_char_t* ssi_log_name;
	int ssi_log_level;

	Timer* m_timer;

	k4a::device m_azureKinectDevice;
	k4a_device_configuration_t m_azureKinectConfig;
	k4a::calibration m_sensorCalibration;
	k4a::transformation m_transformation;
	bool m_camerasStarted;

	k4abt::tracker m_bodyTracker;

	k4a::capture m_capturedFrame;

	bool m_firstFrame;

	BgraPixel* m_bgraBuffer;
	DepthPixel* m_depthRawBuffer;
	BgrPixel* m_depthVisualisationBuffer;
	cv::Mat m_depthHSVConversionMat; //helper, constructed over the m_depthBuffer to use opencv color conversion functions on it
	DepthPixel* m_irRawBuffer;
	IRPixel* m_irVisualisationBuffer;
	PointCloudPixel* m_pointCloudBuffer;
	k4a::image m_pointCloudKinectBufferImage;

	ssi_size_t m_nrOfSkeletons;
	SKELETON* m_skeletons;

	RGBImageChannel m_rgb_channel;
	IProvider* m_rgb_provider;

	DepthRawImageChannel m_depthRaw_channel;
	IProvider* m_depthRaw_provider;

	DepthVisualisationImageChannel m_depthVisualisation_channel;
	IProvider* m_depthVisualisation_provider;
	
	bool setPointCloudProvider(IProvider* pointcloud_provider);
	PointCloudChannel m_pointCloud_channel;
	IProvider* m_pointCloud_provider;

	IRRawImageChannel m_irRaw_channel;
	IProvider* m_irRaw_provider;

	IRVisualisationImageChannel m_irVisualisation_channel;
	IProvider* m_irVisualisation_provider;

	bool setSkeletonProvider(IProvider* skeleton_provider);
	SkeletonChannel m_skeleton_channel;
	IProvider* m_skeleton_provider;

private:
	bool setImageProvider(IProvider* providerIn, IProvider* &internalProvider, IChannel &internalChannel, ssi_video_params_t params);
};

}

#endif //SSI_SENSOR_AZUREKINECT_H

