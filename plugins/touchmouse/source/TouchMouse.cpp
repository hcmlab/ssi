// TouchMouse.cpp
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 2014/11/03
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

#include "TouchMouse.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


namespace ssi {
	
static char ssi_log_name[] = "touchmouse";


BYTE* _buffer_pabImage = 0;
Mutex _buffer_pabImage_mutex;

// Function receiving callback from mouse.
// pTouchMouseStatus - values indicating status of mouse.
// pabImage - bytes forming image, 13 rows of 15 columns.
// dwImageSize - size of image, assumed to always be 195 (13x15).
void __declspec(cdecl) MyTouchMouseCallback(const TOUCHMOUSESTATUS* const pTouchMouseStatus, 
                                            const BYTE* const pabImage, 
                                            DWORD /* dwImageSize */ )
{
	_buffer_pabImage_mutex.acquire();
	memcpy(_buffer_pabImage,pabImage,13*15);
	_buffer_pabImage_mutex.release();
}


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

TouchMouse::TouchMouse (const ssi_char_t *file) 
	: _pressurematrix_provider (0),
	_pressurematrix_buffer(0),
	_file (0),
	fpsValuecount(0),
	avgFps(0.0),
	cols(13), 
	rows(15),
	_timer(0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

TouchMouse::~TouchMouse () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

bool TouchMouse::setProvider (const ssi_char_t *name, IProvider *provider) {

	if (strcmp(name, SSI_TOUCHMOUSE_PROVIDER_NAME) == 0) {
		setPressureMatrixProvider (provider);
		return true;
	}

	ssi_wrn ("unkown provider name '%s'", name);

	return false;
}

void TouchMouse::setPressureMatrixProvider (IProvider *provider) {

	if (_pressurematrix_provider) {
		ssi_wrn ("pressurematrix provider was already set");
	}

	_pressurematrix_provider = provider;
	if (_pressurematrix_provider) {		
		_pressurematrix_channel.stream.sr = _options.sr;
		_pressurematrix_channel.stream.dim = cols*rows;
		_pressurematrix_provider->init (&_pressurematrix_channel);
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "pressurematrix provider set");
	}
}

void TouchMouse::checkOptions () {
	//if (_options.sr < 1.0 || _options.sr > 200.0) ssi_err ("sample rate (%f) out of range [1.0,25.5]!", _options.sr);
	//if (_options.sr > 120.0) ssi_wrn ("sample rate (%f) out of tested range [1.0,21.0]!", _options.sr);
}

bool TouchMouse::connect () {

	checkOptions();

	ssi_msg (SSI_LOG_LEVEL_DEBUG, "setting up touch mouse callback...");

	_frame_size = ssi_cast (ssi_size_t, _options.size * _options.sr + 0.5);
	_pressurematrix_buffer = new float[cols*rows*_frame_size];
	_buffer_pabImage = new BYTE[cols*rows];

	std::fill_n(_buffer_pabImage, cols*rows, 0);
	
	_counter = 0; 
	_frame_size = ssi_cast (ssi_size_t, _options.size * _options.sr + 0.5);

	Thread::setName (getName ());

	lastCall = GetTimeMicros64();

	// Set up callback with TouchMouseSensor DLL.
    RegisterTouchMouseCallback(MyTouchMouseCallback);

	return true;
}

void TouchMouse::run () {

	//fps
	double deltaMs = ((GetTimeMicros64() - lastCall) / 1000.0);
	double fps = 1.0/(deltaMs/1000.0);

	avgFps += fps;
	fpsValuecount++;

	if (fpsValuecount == 10) {
		avgFps/=fpsValuecount;
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "avg fps: %.2f", avgFps);
		avgFps = 0;
		fpsValuecount = 0;
	}
	
	lastCall = GetTimeMicros64();

	//copy data from mouse
	_buffer_pabImage_mutex.acquire();

	// Iterate over rows.
	for (DWORD y = 0 ; y < rows ; y++)
	{
		// Iterate over columns.
		for (DWORD x = 0 ; x < cols ; x++)
		{
			int pos = (((cols*rows)*_counter)+(cols*y)+x);
			int pixel = _buffer_pabImage[rows * x + y];

			// Get the pixel value at current position.
				if (_options.scale) _pressurematrix_buffer[pos] = (float)(pixel / 255.f);
				else _pressurematrix_buffer[pos] = pixel;
		}
	}

	_buffer_pabImage_mutex.release();

	_counter++;

	if (_counter == _frame_size) {
		_counter = 0; 

		//send the data
		if (_pressurematrix_provider) {
			_pressurematrix_provider->provide (ssi_pcast (char, _pressurematrix_buffer), _frame_size);
		}

	}

	// init timer
	if (!_timer) {
		_timer = new Timer (1/_options.sr, true);
	}
	_timer->wait (); 
}

bool TouchMouse::disconnect () {
	UnregisterTouchMouseCallback();

	delete[] _pressurematrix_buffer; _pressurematrix_buffer = 0;
	delete[]  _buffer_pabImage;  _buffer_pabImage = 0;
	
	ssi_msg (SSI_LOG_LEVEL_DEBUG, "touch mouse disconnected");

	return true;
}

}
