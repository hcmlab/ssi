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

#include <k4a/k4a.hpp> /* ATTENTION: THIS FILE WAS MODIFIED BECAUSE IT DID NOT COMPILE DUE TO A CLASH BETWEEN std::min/max AND min/max preprocessor definitions */

namespace ssi {
	class Timer;
}

namespace ssi {

	#define SSI_AZUREKINECT_RGBIMAGE_PROVIDER_NAME "rgb"

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
		protected:
			ssi_stream_t stream;
		};

	public:

		class Options : public OptionList {

		public:
			Options() : sr(30.0) {
				addOption("sr", &sr, 1, SSI_TIME, "sample rate in hz");
			};

			ssi_time_t sr;
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
		bool start() { return Thread::start(); };
		bool stop() { return Thread::stop(); };
		void run();
		bool disconnect();

		ssi_video_params_t getRGBImageParams()
		{
			ssi_video_params_t vParam;

			ssi_video_params(vParam, 1920, 1080, _options.sr, SSI_VIDEO_DEPTH_8U, 4);
			vParam.flipImage = false;
			return vParam;
		}

		void setLogLevel(int level) {
			ssi_log_level = level;
		}

	protected:
		AzureKinect(const ssi_char_t* file = 0);

		void process();

		void processRGBAImage();
		void processProviders();

		AzureKinect::Options _options;
		ssi_char_t* _file;

		static ssi_char_t* ssi_log_name;
		int ssi_log_level;

		Timer* m_timer;

		k4a::device m_azureKinectDevice;

		RGBQUAD* m_bgraBuffer;

		void setRGBImageProvider(IProvider* rgb_provider);
		RGBImageChannel m_rgb_channel;
		IProvider* m_rgb_provider;
	};

}

#endif //SSI_SENSOR_AZUREKINECT_H

