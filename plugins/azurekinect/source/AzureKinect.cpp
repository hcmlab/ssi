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
		: _file(0),
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
	}

	bool AzureKinect::setProvider(const ssi_char_t* name, IProvider* provider)
	{
		//TODO: switch for all the different providers

		return false;
	}

	bool AzureKinect::connect()
	{
		//TODO: Connect to Azure Kinect here
		
		//ssi_msg(SSI_LOG_LEVEL_DETAIL, "connected");

		// set thread name
		Thread::setName(getName());

		return false;
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
	}

	bool AzureKinect::disconnect() {

		ssi_msg(SSI_LOG_LEVEL_DETAIL, "try to disconnect sensor...");

		//TODO: Clean up all buffers and stuff

		ssi_msg(SSI_LOG_LEVEL_DETAIL, "sensor disconnected");

		return true;
	}
}