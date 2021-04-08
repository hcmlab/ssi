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

namespace ssi {
	class Timer;
}

namespace ssi {

	class AzureKinect : public ISensor, public Thread {
	public:

		class Options : public OptionList {

		public:
			Options() : sr(20.0) {
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

		ssi_size_t getChannelSize() { return 0; };
		IChannel* getChannel(ssi_size_t index) {
			return 0;
		};
		bool setProvider(const ssi_char_t* name, IProvider* provider);
		bool connect();
		bool start() { return Thread::start(); };
		bool stop() { return Thread::stop(); };
		void run();
		bool disconnect();

		void setLogLevel(int level) {
			ssi_log_level = level;
		}

	protected:
		AzureKinect(const ssi_char_t* file = 0);

		void AzureKinect::process();

		AzureKinect::Options _options;
		ssi_char_t* _file;

		Timer* m_timer;

		static ssi_char_t* ssi_log_name;
		int ssi_log_level;
	};

}

#endif //SSI_SENSOR_AZUREKINECT_H

