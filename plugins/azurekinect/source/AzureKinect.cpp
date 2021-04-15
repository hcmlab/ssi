// AzureKinect.cpp
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

#include "AzureKinect.h"
#include "thread/Timer.h"

#include <iostream>

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {
	using namespace AK;

	ssi_char_t* AzureKinect::ssi_log_name = "azurekinect";

	AzureKinect::AzureKinect(const ssi_char_t* file)
		: m_rgb_provider(0),
		m_bgraBuffer(0),
		m_depthVisualisation_provider(0),
		m_depthVisualisationBuffer(0),
		m_depthRaw_provider(0),
		m_depthRawBuffer(0),
		m_irVisualisation_provider(0),
		m_irVisualisationBuffer(0),
		m_irRaw_provider(0),
		m_irRawBuffer(0),
		m_camerasStarted(false),
		_file(0),
		m_timer(0),
		ssi_log_level(SSI_LOG_LEVEL_DEFAULT)
	{
		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}
	}

	AzureKinect::~AzureKinect()
	{
		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}

		//TODO: Deinit the AzureKinect resources

		if (m_azureKinectDevice) {
			m_azureKinectDevice.close();
		}
	}

	bool AzureKinect::setProvider(const ssi_char_t* name, IProvider* provider)
	{
		bool providerWasAlreadySet = false;

		std::string providerNameStr = std::string("Provider ") + name;

		if (strcmp(name, SSI_AZUREKINECT_RGBIMAGE_PROVIDER_NAME) == 0) {
			providerWasAlreadySet = setImageProvider(provider, m_rgb_provider, m_rgb_channel, getRGBImageParams());
		}
		else if (strcmp(name, SSI_AZUREKINECT_DEPTHVISUALISATIONIMAGE_PROVIDER_NAME) == 0) {
			providerWasAlreadySet = setImageProvider(provider, m_depthVisualisation_provider, m_depthVisualisation_channel, getDepthVisualisationImageParams());
		}
		else if (strcmp(name, SSI_AZUREKINECT_DEPTHRAWIMAGE_PROVIDER_NAME) == 0) {
			providerWasAlreadySet = setImageProvider(provider, m_depthRaw_provider, m_depthRaw_channel, getDepthRawImageParams());
		}
		else if (strcmp(name, SSI_AZUREKINECT_IRVISUALISATIONIMAGE_PROVIDER_NAME) == 0) {
			providerWasAlreadySet = setImageProvider(provider, m_irVisualisation_provider, m_irVisualisation_channel, getIRVisualisationImageParams());
		}
		else if (strcmp(name, SSI_AZUREKINECT_IRRAWIMAGE_PROVIDER_NAME) == 0) {
			providerWasAlreadySet = setImageProvider(provider, m_irRaw_provider, m_irRaw_channel,getIRRawImageParams());
		}
		else {
			ssi_wrn((providerNameStr + " does not exist on this sensor").c_str());

			return false;
		}

		if (providerWasAlreadySet) {
			ssi_wrn((providerNameStr + " was already set before.").c_str());
		}

		ssi_msg(SSI_LOG_LEVEL_DETAIL,(providerNameStr + " set.").c_str());

		return true;
	}

	bool AzureKinect::setImageProvider(IProvider* providerIn, IProvider* &internalProvider, IChannel &internalChannel, ssi_video_params_t params) {
		bool providerWasDefined = false;
		if (internalProvider) {
			providerWasDefined = true;
		}

		internalProvider = providerIn;

		if (internalProvider) {
			internalProvider->setMetaData(sizeof(params), &params);

			internalChannel.getStreamPtr()->sr = _options.sr;
			internalChannel.getStreamPtr()->byte = params.widthInPixels * params.heightInPixels * params.numOfChannels * params.depthInBitsPerChannel / 8;

			internalProvider->init(&internalChannel);
		}

		return providerWasDefined;
	}

	bool AzureKinect::connect()
	{
		//TODO: Connect to Azure Kinect here

		auto count = k4a::device::get_installed_count();

		if (count > 0) {

			if (count > 1) {
				ssi_wrn("Multiple Azure Kinects found. Connecting to the first one");
			}

			try {

				m_azureKinectDevice = k4a::device::open(0);
			} 
			catch (const std::exception& e)
			{
				ssi_wrn("Could not connect to azure kinect");
				m_azureKinectDevice.close();
				return false;
			}

			ssi_msg(SSI_LOG_LEVEL_DETAIL, "connected");
		}
		else {
			ssi_wrn("No Azure Kinect found!");
		}
		

		// set thread name
		Thread::setName(getName());

		return true;
	}

	bool AzureKinect::start()
	{
		startCameras();
		return Thread::start();
	}

	bool AzureKinect::stop()
	{
		
		return Thread::stop();
	}


	void AzureKinect::startCameras()
	{
		k4a_device_configuration_t config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
		applyOptionsToCameraConfiguration(config);

		try
		{
			m_azureKinectDevice.start_cameras(&config);
			m_camerasStarted = true;
			m_bgraBuffer = new BgraPixel[getRGBImageParams().widthInPixels * getRGBImageParams().heightInPixels];
			m_depthRawBuffer = new DepthPixel[getDepthVisualisationImageParams().widthInPixels * getDepthVisualisationImageParams().heightInPixels];
			m_depthVisualisationBuffer = new BgrPixel[getDepthVisualisationImageParams().widthInPixels * getDepthVisualisationImageParams().heightInPixels];
			m_depthHSVConversionMat = cv::Mat(getDepthVisualisationImageParams().widthInPixels, getDepthVisualisationImageParams().heightInPixels, CV_8UC3, m_depthVisualisationBuffer);
			m_irRawBuffer = new DepthPixel[getIRRawImageParams().widthInPixels * getIRRawImageParams().heightInPixels];
			m_irVisualisationBuffer = new IRPixel[getIRVisualisationImageParams().widthInPixels * getIRVisualisationImageParams().heightInPixels];
		}
		catch (const std::exception& e)
		{
			ssi_wrn("Could not start the kinect cameras");
		}
	}

	void AzureKinect::stopCameras() {
		if (m_azureKinectDevice) {
			m_azureKinectDevice.stop_cameras();
			m_camerasStarted = false;
		}
	}

	void AzureKinect::applyOptionsToCameraConfiguration(k4a_device_configuration_t& config)
	{
		//fps
		if (_options.sr <= 5.0) {
			config.camera_fps = K4A_FRAMES_PER_SECOND_5;
		}
		else if (_options.sr <= 15.0) {
			config.camera_fps = K4A_FRAMES_PER_SECOND_15;
		}
		else {
			config.camera_fps = K4A_FRAMES_PER_SECOND_30;
		}

		//color format - always RGB32 as the other option (compressed JPG) is not desirable
		config.color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;

		//color camera resolution
		k4a_color_resolution_t colorRes;
		switch (_options._rgbResolution)
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
				break;
		}
		config.color_resolution = colorRes;

		//depth sensor mode
		k4a_depth_mode_t depthMode;
		switch (_options._depthMode)
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
				break;
		}
		config.depth_mode = depthMode;

		config.synchronized_images_only = true;
	}

	void AzureKinect::run()
	{

		if (!m_timer) {
			m_timer = new Timer(1 / _options.sr);
		}

		process();
		m_timer->wait();
	}

	void AzureKinect::process()
	{
		if (!m_azureKinectDevice) return;

		getCapture();

		processRGBAImage();
		processDepthImage();
		processIRImage();

		releaseCapture();

		processProviders();		
	}

	void AzureKinect::getCapture()
	{
		if (m_azureKinectDevice && m_camerasStarted) {
			try
			{
				m_azureKinectDevice.get_capture(&m_capturedFrame);
			}
			catch (const std::exception& e)
			{
				releaseCapture();
				ssi_wrn("Error during frame capture");
				ssi_wrn(e.what());
				return;
			}
		}
	}


	void AzureKinect::releaseCapture() {
		m_capturedFrame.reset();
	}


	void AzureKinect::processRGBAImage()
	{
		if (m_capturedFrame) {
			k4a::image rgbImage = m_capturedFrame.get_color_image();
			if (rgbImage) {
				std::memcpy(m_bgraBuffer, rgbImage.get_buffer(), rgbImage.get_size());
				rgbImage.reset(); //release the image after getting its data
			}
		}
	}

	void AzureKinect::processDepthImage()
	{
		if (m_capturedFrame) {
			k4a::image depthImage = m_capturedFrame.get_depth_image();
			if (depthImage) {
				std::memcpy(m_depthRawBuffer, depthImage.get_buffer(), depthImage.get_size());
				depthImage.reset(); //release the image after getting its data

				if (m_depthVisualisation_provider) {
					const int width = getDepthVisualisationImageParams().widthInPixels;
					const int height = getDepthVisualisationImageParams().heightInPixels;
					const auto valueRange = GetDepthModeRange(_options._depthMode);

					for (int h = 0; h < height; ++h)
					{
						for (int w = 0; w < width; ++w)
						{
							const size_t currentPixel = static_cast<size_t>(h * width + w);
							m_depthVisualisationBuffer[currentPixel] = ColorizeBlueToRed_HSV(m_depthRawBuffer[currentPixel], valueRange.first, valueRange.second);
						}
					}

					cv::cvtColor(m_depthHSVConversionMat, m_depthHSVConversionMat, cv::COLOR_HSV2BGR);
				}
			}
		}
	}

	void AzureKinect::processIRImage()
	{
		if (m_capturedFrame) {
			k4a::image irImage = m_capturedFrame.get_ir_image();
			if (irImage) {
				std::memcpy(m_irRawBuffer, irImage.get_buffer(), irImage.get_size());
				irImage.reset(); //release the image after getting its data

				if (m_irVisualisation_provider) {
					const int width = getIRRawImageParams().widthInPixels;
					const int height = getIRRawImageParams().heightInPixels;
					const auto valueRange = GetIrLevelsRange(_options._depthMode);

					for (int h = 0; h < height; ++h)
					{
						for (int w = 0; w < width; ++w)
						{
							const size_t currentPixel = static_cast<size_t>(h * width + w);
							m_irVisualisationBuffer[currentPixel] = ColorizeGreyscale(m_irRawBuffer[currentPixel], valueRange.first, valueRange.second);
						}
					}
				}
			}
		}
	}

	void AzureKinect::processProviders()
	{
		if (m_rgb_provider) {
			m_rgb_provider->provide(ssi_pcast(ssi_byte_t, m_bgraBuffer), 1);
		}

		if (m_depthRaw_provider) {
			m_depthRaw_provider->provide(ssi_pcast(ssi_byte_t, m_depthRawBuffer), 1);
		}

		if (m_depthVisualisation_provider) {
			m_depthVisualisation_provider->provide(ssi_pcast(ssi_byte_t, m_depthVisualisationBuffer), 1);
		}

		if (m_irRaw_provider) {
			m_irRaw_provider->provide(ssi_pcast(ssi_byte_t, m_irRawBuffer), 1);
		}

		if (m_irVisualisation_provider) {
			m_irVisualisation_provider->provide(ssi_pcast(ssi_byte_t, m_irVisualisationBuffer), 1);
		}
	}

	bool AzureKinect::disconnect() {

		ssi_msg(SSI_LOG_LEVEL_DETAIL, "try to disconnect sensor...");

		//TODO: Clean up all buffers and stuff
		delete[] m_bgraBuffer;
		delete[] m_depthRawBuffer;
		delete[] m_depthVisualisationBuffer;
		delete[] m_irRawBuffer;
		delete[] m_irVisualisationBuffer;

		ssi_msg(SSI_LOG_LEVEL_DETAIL, "sensor disconnected");

		return true;
	}
}