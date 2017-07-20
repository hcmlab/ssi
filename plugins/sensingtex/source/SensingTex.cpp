// SensingTex.cpp
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 2014/09/02
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

#include "SensingTex.h"


#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


namespace ssi {
	
static char ssi_log_name[] = "sensingtex";

//https://stackoverflow.com/questions/1861294/how-to-calculate-execution-time-of-a-code-snippet-in-c

/* Returns the amount of milliseconds elapsed since the UNIX epoch. Works on both
* windows and linux. */

uint64 GetTimeMs64()
{
#ifdef _WIN32
	/* Windows */
	FILETIME ft;
	LARGE_INTEGER li;

	/* Get the amount of 100 nano seconds intervals elapsed since January 1, 1601 (UTC) and copy it
	* to a LARGE_INTEGER structure. */
	GetSystemTimeAsFileTime(&ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;

	uint64 ret = li.QuadPart;
	ret -= 116444736000000000LL; /* Convert from file time to UNIX epoch time. */
	ret /= 10000; /* From 100 nano seconds (10^-7) to 1 millisecond (10^-3) intervals */

	return ret;
#else
	/* Linux */
	struct timeval tv;

	gettimeofday(&tv, NULL);

	uint64 ret = tv.tv_usec;
	/* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
	ret /= 1000;

	/* Adds the seconds (10^0) after converting them to milliseconds (10^-3) */
	ret += (tv.tv_sec * 1000);

	return ret;
#endif
}

SensingTex::SensingTex (const ssi_char_t *file) 
	: _pressurematrix_provider (nullptr),
	_frame_size(0),
	_counter(0),
	_timer(nullptr),
	_serial_buffer (nullptr),
	_n_serial_buffer (0),
	_pressurematrix_buffer(nullptr),
	_received_cols (nullptr),
	_serial (nullptr),
	_is_connected (false),
	_file (nullptr),
	fpsValuecount(0),
	lastCall(0),
	avgFps(0.0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

SensingTex::~SensingTex () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool SensingTex::setProvider (const ssi_char_t *name, IProvider *provider) {

	if (strcmp(name, SSI_SENSINGTEX_PROVIDER_NAME) == 0) {
		setPressureMatrixProvider (provider);
		return true;
	}

	ssi_wrn ("unkown provider name '%s'", name);

	return false;
}

void SensingTex::setPressureMatrixProvider (IProvider *provider) {

	if (_pressurematrix_provider) {
		ssi_wrn ("pressurematrix provider was already set");
	}

	_pressurematrix_provider = provider;
	if (_pressurematrix_provider) {		
		_pressurematrix_channel.stream.sr = _options.sr;
		_pressurematrix_channel.stream.dim = _options.cols*_options.rows;
		_pressurematrix_provider->init (&_pressurematrix_channel);
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "pressurematrix provider set");
	}
}

void SensingTex::checkOptions () {
	if (_options.sr < 1.0 || _options.sr > 25.5) ssi_err ("sample rate (%f) out of range [1.0,25.5]!", _options.sr);
	if (_options.sr > 21.0) ssi_wrn ("sample rate (%f) out of tested range [1.0,21.0]!", _options.sr);

	if (_options.cols > 16 || _options.cols < 1) ssi_err ("column count (%u) out of range [1,16]!", _options.cols);
	if (_options.rows > 16 || _options.rows < 1) ssi_err ("row count (%u) out of range [1,16]!", _options.rows);
}

bool SensingTex::connect () {

	checkOptions();

        //check if port exists
        std::vector<serial::PortInfo> ports = serial::list_ports();

        bool exists = false;
        for (int i = 0; i < ports.size(); i++)
        {
                if (ports[i].port.compare(_options.port) == 0)
                        exists = true;
        }

        if (!exists) ssi_err("Port %s not found!", _options.port);



	//fix problem with rows / columns mix up with sensingtex microcontroller
	char _cols = _options.cols;
	_options.cols = _options.rows;
	_options.rows = _cols;

        ssi_msg (SSI_LOG_LEVEL_DETAIL, "try to connect sensor (port=%s, baud=%u)...", _options.port, 115200);

	//_n_serial_buffer = 2*sizeof(char) + sizeof(RMS_Matrix_Col_t); //message should be: ['M'][RMS_Matrix_Col_t]['\n']
	_n_serial_buffer = 2*sizeof(unsigned char) + _options.rows*sizeof(short); //the controller is just sending the configured row count per column; max is sizeof(RMS_Matrix_Col_t);
	
	_serial_buffer = new unsigned char[sizeof(RMS_Matrix_Col_t)];

	_counter = 0; 
	_frame_size = ssi_cast (ssi_size_t, _options.size * _options.sr + 0.5);

	_pressurematrix_buffer = new float[_options.cols*_options.rows*_frame_size];

	_received_cols = new bool[_options.cols];

        //this is necessary on linux!
        serial::Timeout timeout(0, 1000,1, 1000, 1);

        _serial = new serial::Serial (_options.port, 115200, timeout);
	
	_is_connected = _serial->isOpen();
	if (!_is_connected) {
                ssi_wrn ("could not connect serial sensor at port=%s", _options.port);
		return false;
	}

        ssi_msg (SSI_LOG_LEVEL_DETAIL, "connected");


        uint8_t bytes[6];

	//send settings to microcontroller
        bytes[0] = 0x43;     //C
	bytes[1] = _options.rows;		//rows
	bytes[2] = _options.cols;		//columns
	bytes[3] = 0x00;				//freq: 2nd byte               -> SSI option sr
	bytes[4] = _options.sr*10;		//freq: 1st byte   (200/10 Hz = 20 Hz) -> SSI option sr (default: 0x64)
        bytes[5] = 0x0D;     //carriage return (\r)


        _serial->write(bytes,6);

	//send start command to microcontroller
        bytes[0] = 0x53;     //S
        bytes[1] = 0x0D;     //carriage return (\r)

        _serial->write(bytes,2);


        ssi_msg (SSI_LOG_LEVEL_DETAIL, "sent configuration");

	// set thread name
	Thread::setName (getName ());

	lastCall = GetTimeMs64();

	return true;
}

void SensingTex::run () {

        if (_serial && _is_connected) {

		auto matrixDataSent = false;

		while (!matrixDataSent) {

			//find the beginning of the microcontroller message; the message should look like this: ['M'][RMS_Matrix_Col_t]['\n']
			do {
                                _serial->read (_serial_buffer, 1);
			}
                        while(_serial_buffer[0] != 'M');

                        uint8_t value = 0;
			
			//copy the bytes for "RMS_Matrix_Col_t" (the value count depends on the configured row count)
			_serial->read(_serial_buffer, _n_serial_buffer);

			//data check: read the finishing '\n'
                        _serial->read ( &value, 1);

                        #ifdef _WIN32
                            assert(value == '\n', "the end of the message wasn't '\n'");
                        #endif

                        //convert the bytes to the "RMS_Matrix_Col_t" struct
			RMS_Matrix_Col_t data;
			memcpy(&data, &_serial_buffer[0], sizeof(RMS_Matrix_Col_t));

			//check if received microcontroller row count is the same like in the options
			if (data.nrowspercol == _options.rows) {

				//copy the matrix values into the outgoing buffer
				for (auto row = 0; row < _options.rows; row++) {
					int pos = (((_options.cols*_options.rows)*_counter)+(_options.cols*row)+data.col);
					_pressurematrix_buffer[pos] = abs((data.value[row] & 0x0FFF) - 4095);
					if (_options.scale) _pressurematrix_buffer[pos] /= 4095;
				}

				//mark column of matrix as received
				_received_cols[data.col] = true;

				//check if we have a complete matrix
				auto matrixReady = true;
				for (auto i = 0; i < _options.cols; i++) {
					matrixReady &= _received_cols[i];

					if (!matrixReady) break;
				}

				if (matrixReady) {
					//fps
					long deltaMs = (GetTimeMs64() - lastCall);
					auto fps = 1.0/(deltaMs/1000.0);
					avgFps += fps;
					fpsValuecount++;

					if (fpsValuecount == 10) {
						avgFps/=fpsValuecount;
						if (_options.fps)
							ssi_msg (SSI_LOG_LEVEL_BASIC, "avg fps: %.2f", avgFps);
						avgFps = 0;
						fpsValuecount = 0;
					}

					lastCall = GetTimeMs64();

					_counter++;

					if (_counter == _frame_size) {
						_counter = 0; 

						//send the data
						if (_pressurematrix_provider) {

							//switch on for debug matrix: rows / cols are mixed in options for controller!
							if (false) {
								for (int counter = 0; counter < _frame_size; counter++) {
									for (int row = 0; row < _options.rows; row++) {
										for (int col = 0; col < _options.cols; col++) {
											int pos = (((_options.cols*_options.rows)*counter)+(_options.rows*col)+row);

											if ((col == 15 && row == 13))
												_pressurematrix_buffer[pos] = 0.5f;
											/*else if ((col == 15 && row == 0))
												_pressurematrix_buffer[pos] = 0.25f;
											else if ((col == 0 && row == 13))
												_pressurematrix_buffer[pos] = 0.125f;*/
											else
												_pressurematrix_buffer[pos] = 0.f;
										}
									}
								}
							}


							_pressurematrix_provider->provide (ssi_pcast (char, _pressurematrix_buffer), _frame_size);
							SSI_DBG (SSI_LOG_LEVEL_DEBUG, "pressurematrix data provided");
							matrixDataSent = true;
						}
					}
					for (auto i = 0; i < _options.cols; i++) _received_cols[i] = false;	//reset matrix ready

				}
			}
			else {
				ssi_wrn ("received column count from microcontroller (%u) differs from options (%u)!", data.nrowspercol, _options.cols);

			}
		}
	}
}

bool SensingTex::disconnect () {

	//send stop command to microcontroller
	uint8_t bytes[2];
    bytes[0] = 0x58;     //X
    bytes[1] = 0x0D;     //carriage return (\r)
	_serial->write(bytes,2);


	ssi_msg (SSI_LOG_LEVEL_DETAIL, "try to disconnect sensor...");

	delete[] _serial_buffer; _serial_buffer = nullptr;
	_n_serial_buffer = 0;

	delete[] _pressurematrix_buffer; _pressurematrix_buffer = nullptr;
	delete[] _received_cols; _received_cols = nullptr;

	delete _serial; _serial = nullptr;

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "sensor disconnected");

	return true;
}

}
