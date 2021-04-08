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
	ssi_char_t* AzureKinect::ssi_log_name = "azurekinect";

	AzureKinect::AzureKinect(const ssi_char_t* file)
		: m_rgb_provider(0),
		m_bgraBuffer(0),
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
		//TODO: switch for all the different providers
		if (strcmp(name, SSI_AZUREKINECT_RGBIMAGE_PROVIDER_NAME) == 0) {
			setRGBImageProvider(provider);
			return true;
		}

		return false;
	}

	void AzureKinect::setRGBImageProvider(IProvider* rgb_provider) {

		if (m_rgb_provider) {
			ssi_wrn("rgb image provider already set");
		}

		m_rgb_provider = rgb_provider;

		if (m_rgb_provider) {

			ssi_video_params_t params = getRGBImageParams();

			m_rgb_provider->setMetaData(sizeof(params), &params);
			m_rgb_channel.stream.sr = _options.sr;

			m_rgb_channel.stream.byte = 1920 * 1080 * 4;

			m_rgb_provider->init(&m_rgb_channel);
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "rgb provider set");
		}
	}

	bool AzureKinect::connect()
	{
		//TODO: Connect to Azure Kinect here

		auto count = k4a::device::get_installed_count();

		if (count > 0) {

			m_azureKinectDevice = k4a::device::open(0);

			ssi_msg(SSI_LOG_LEVEL_DETAIL, "connected");

			k4a_device_configuration_t config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
			config.camera_fps = K4A_FRAMES_PER_SECOND_30;
			config.color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;
			config.color_resolution = K4A_COLOR_RESOLUTION_1080P;

			try
			{
				m_azureKinectDevice.start_cameras(&config);
				m_bgraBuffer = new RGBQUAD[getRGBImageParams().widthInPixels * getRGBImageParams().heightInPixels];
			}
			catch (const std::exception& e)
			{
				ssi_wrn("Could not start the kinect cameras");
			}
		}
		else {
			ssi_wrn("No Azure Kinect found!");
		}
		

		// set thread name
		Thread::setName(getName());

		return true;
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
		
		processRGBAImage();

		processProviders();		
	}

	void AzureKinect::processRGBAImage()
	{
		k4a::capture capFrame;

		try
		{
			m_azureKinectDevice.get_capture(&capFrame);
		}
		catch (const std::exception&)
		{
			ssi_wrn("Error during frame capture");
			return;
		}

		k4a::image rgbImage = capFrame.get_color_image();

		std::memcpy(m_bgraBuffer, rgbImage.get_buffer(), rgbImage.get_size());

		rgbImage.reset();
		capFrame.reset();
	}

	void AzureKinect::processProviders()
	{
		if (m_rgb_provider) {
			m_rgb_provider->provide(ssi_pcast(ssi_byte_t, m_bgraBuffer), 1);
		}
	}

	bool AzureKinect::disconnect() {

		ssi_msg(SSI_LOG_LEVEL_DETAIL, "try to disconnect sensor...");

		//TODO: Clean up all buffers and stuff
		delete[] m_bgraBuffer;

		ssi_msg(SSI_LOG_LEVEL_DETAIL, "sensor disconnected");

		return true;
	}
}