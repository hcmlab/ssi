// Shimmer3GSRPlus.cpp
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 9/3/2015
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "Shimmer3GSRplus.h"
#include "Serial.h"
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

static char ssi_log_name[] = "shimmer3gsr+";



//https://stackoverflow.com/questions/1861294/how-to-calculate-execution-time-of-a-code-snippet-in-c
_int64 GetTimeMicros64()
{
	FILETIME ft;
	LARGE_INTEGER li;

	GetSystemTimeAsFileTime(&ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;

	unsigned _int64 ret = li.QuadPart;
	ret -= 116444736000000000LL; /* Convert from file time to UNIX epoch time. */
	ret /= 10; /* From 100 nano seconds (10^-7) to 1 µsec intervals */

	return ret;
}


Shimmer3GSRPlus::Shimmer3GSRPlus (const ssi_char_t *file) 
	: _serial_provider (0),
	_is_connected (false),
	_file (0),
	_serial_channel (0),
	_buffer (0), 
	_buffer_ptr (0),
	fpsValuecount(0),
	avgFps(0.0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {
	
	getOptions()->setStartCMD(0);
	getOptions()->setStopCMD(0);
	getOptions()->setDeviceInstanceId(0);

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	//possbile baud rates from windows
	baudRates[110]		= 110UL;
	baudRates[300]		= 300UL;
	baudRates[600]		= 600UL;
	baudRates[1200]		= 1200UL;
	baudRates[2400]		= 2400UL;
	baudRates[4800]		= 4800UL;
	baudRates[9600]		= 9600UL;
	baudRates[14400]	= 14400UL;
	baudRates[19200]	= 19200UL;
	baudRates[38400]	= 38400UL;
	baudRates[56000]	= 56000UL;
	baudRates[57600]	= 57600UL;
	baudRates[115200]	= 115200UL;
	baudRates[128000]	= 128000UL;
	baudRates[256000]	= 256000UL;
}

Shimmer3GSRPlus::~Shimmer3GSRPlus () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}

	delete _serial_channel;
}

bool Shimmer3GSRPlus::setProvider (const ssi_char_t *name, IProvider *provider) {

	if (strcmp (name, SSI_GENERICSERIAL_PROVIDER_NAME) == 0) {
		setSerialProvider (provider);
		return true;
	}

	ssi_wrn ("unkown provider name '%s'", name);

	return false;
}

void Shimmer3GSRPlus::setSerialProvider (IProvider *provider) {

	if (_serial_provider) {
		ssi_wrn ("serial provider was already set");
	}

	_serial_provider = provider;
	if (_serial_provider) {		
		_serial_provider->init (getChannel (0));
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "serial provider set");
	}
}

bool Shimmer3GSRPlus::connect () {

	_device = std::make_unique<Shimmer3LogAndStreamDevice>(_options.port, _options.baud);

	if (!_device->connect()) {
		return false;
	}

	_frame_size = ssi_cast (ssi_size_t, _options.size * _options.sr + 0.5);
	_counter = 0;
	_buffer = new ssi_real_t[ _frame_size * _serial_channel->getStream().dim];
	_buffer_ptr = _buffer;

	// set thread name
	Thread::setName (getName ());

	lastCall = GetTimeMicros64();

	return true;
}

void Shimmer3GSRPlus::run () {
	/*
	if (_serial && _is_connected) {

		int pos = 0;
		bool found = false;

		std::string str = "";

		//search for new line
		do {
			char c;
			int r = _serial->ReadData (&c, 1);
			std::cout << "Received " << r << " byte(s): " << (c == 0 ? "[packet-start]" : "") << std::hex << (int)c << std::endl;
			str += c;
			if (r > 0 && c == '\n') {
				found = true;
			}
		} while (!found);


		//ssi_msg(SSI_LOG_LEVEL::SSI_LOG_LEVEL_BASIC, "%s", str.c_str());

		if (found) {
			std::vector<float> buffer;
			std::stringstream ss(str);

			float i;
			while (ss >> i) {
				buffer.push_back(i);

				if (ss.peek() == getOptions()->separator)
					ss.ignore();
			}

			//check if value count is ok
			if (buffer.size() == getOptions()->dim) {
				//copy from vector into outgoing buffer
				memcpy(_buffer_ptr, &buffer[0], buffer.size() * sizeof(float));
				_buffer_ptr += buffer.size();


				if (_options.showDebugSR) {
					//fps
					double deltaMs = ((GetTimeMicros64() - lastCall) / 1000.0);

					double fps = 1.0 / (deltaMs / 1000.0);

					if (deltaMs != 0) {
						avgFps += fps;
						fpsValuecount++;
					}

					if (fpsValuecount == 10) {
						avgFps /= fpsValuecount;
						ssi_msg(SSI_LOG_LEVEL_BASIC, "avg input sr: %.3f", avgFps);
						avgFps = 0;
						fpsValuecount = 0;
					}

					lastCall = GetTimeMicros64();

				}



				_counter++;
			}
			else {
				ssi_wrn("Value number (%llu) from serial port differs from dim () -> dropping!", buffer.size(), getOptions()->dim);
			}

			buffer.clear();

		}


		if (_counter == _frame_size) {
			_counter = 0;
			if (_serial_provider) {
				_serial_provider->provide (ssi_pcast (char, _buffer), _frame_size);
				SSI_DBG (SSI_LOG_LEVEL_DEBUG, "serial data provided");

				
			}

			_buffer_ptr = _buffer;
		}
	}
	*/
}

bool Shimmer3GSRPlus::disconnect() {

	if (!_device) {
		ssi_wrn("sensor not connected");
		return false;
	}

	if (!_device->isConnected()) {
		ssi_wrn("sensor not connected");
		_device.reset(nullptr);
		return false;
	}

	ssi_msg(SSI_LOG_LEVEL_BASIC, "sending stop message ...");
	_device->stopStreaming();

	ssi_msg(SSI_LOG_LEVEL_BASIC, "try to disconnect sensor ...");

	_device.reset(nullptr);

	if (_buffer) {
		delete[] _buffer; _buffer = 0;
		_buffer_ptr = 0;
	}

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "sensor disconnected");

	return true;
}

}
