// WiiBBoard.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/05/20
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

#include "WiiBBoard.h"
#include "wiimote.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *WiiBBoard::ssi_log_name = "wiiboard__";

WiiBBoard::WiiBBoard (const ssi_char_t *file) 
	: _wiimote (0),
	_buffer_size (0),
	_buffer_counter (0),
	_raw_provider (0),
	_raw_buffer (0),
	_flt_provider (0),
	_flt_buffer (0),
	_timer (0),
	_file (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

WiiBBoard::~WiiBBoard () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

bool WiiBBoard::setProvider (const ssi_char_t *name, IProvider *provider) {

	if (strcmp (name, SSI_WIIBBOARD_RAW_PROVIDER_NAME) == 0) {
		setRawProvider (provider);
		return true;
	} else if (strcmp (name, SSI_WIIBBOARD_FLT_PROVIDER_NAME) == 0) {
		setFltProvider (provider);
		return true;
	}

	ssi_wrn ("unkown provider name '%s'", name);

	return false;
}

void WiiBBoard::setRawProvider (IProvider *provider) {

	if (_raw_provider) {
		ssi_wrn ("already set");
	}

	_raw_provider = provider;
	if (_raw_provider) {
		_raw_provider->init (&_raw_channel);
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "wii bboard raw provider set");
	}
}

void WiiBBoard::setFltProvider (IProvider *provider) {

	if (_flt_provider) {
		ssi_wrn ("already set");
	}

	_flt_provider = provider;
	if (_flt_provider) {
		_flt_provider->init (&_flt_channel);
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "wii bboard flt provider set");
	}
}

bool WiiBBoard::connect () {

	_buffer_size = ssi_cast (ssi_size_t, SSI_WIIBBOARD_SAMPLE_RATE * _options.size + 0.5);

	if (_raw_provider) {
		_raw_buffer = new SSI_WIIBBOARD_RAW_SAMPLE_TYPE[_buffer_size * _raw_channel.stream.dim];
		_raw_buffer_ptr = _raw_buffer;
	}

	if (_flt_provider) {
		_flt_buffer = new SSI_WIIBBOARD_FLT_SAMPLE_TYPE[_buffer_size * _flt_channel.stream.dim];
		_flt_buffer_ptr = _flt_buffer;
	}

	// connect balance board
	_wiimote = new wiimote ();
	bool status = _wiimote->Connect (ssi_cast (unsigned int, _options.device == wiimote::FIRST_AVAILABLE ? wiimote::FIRST_AVAILABLE : _options.device + 1));
	
	if (!_wiimote->IsBalanceBoard ()) {
		ssi_wrn ("selected device is not a balance board");
		status = false;
	}

	if (!status) {
		ssi_wrn ("could not connect [%d]", _options.device);
	}

	_buffer_counter = 0;
	_timer = 0;

	if (status) {

		ssi_msg (SSI_LOG_LEVEL_BASIC, "connected [%d]", _options.device);

		if (_raw_provider && WiiBBoard::ssi_log_level >= SSI_LOG_LEVEL_DETAIL) {
			ssi_print ("\
         acceleration\n\
         rate\t= %.2lf\t\n\
         size\t= %.2lf\n\
         dim\t= %d\n\
         bytes\t= %d\n",
				SSI_WIIBBOARD_SAMPLE_RATE, 
				_buffer_size / SSI_WIIBBOARD_SAMPLE_RATE,
				_raw_channel.stream.dim, 
				_raw_channel.stream.byte);
		}

		if (_flt_provider && WiiBBoard::ssi_log_level >= SSI_LOG_LEVEL_DETAIL) {
			ssi_print ("\
         orientation\n\
         rate\t= %.2lf\t\n\
         size\t= %.2lf\n\
         dim\t= %d\n\
         bytes\t= %d\n",
				SSI_WIIBBOARD_SAMPLE_RATE, 
				_buffer_size / SSI_WIIBBOARD_SAMPLE_RATE,
				_flt_channel.stream.dim,
				_flt_channel.stream.byte);
		}

	}

	return status;
}

bool WiiBBoard::disconnect () {

	_wiimote->Disconnect ();
	Sleep (200);
	delete _wiimote;
	_wiimote = 0;
	
	ssi_msg (SSI_LOG_LEVEL_BASIC, "disconnected [%d]", _options.device);

	delete _timer;
	delete[] _raw_buffer;
	_raw_buffer = 0;
	delete[] _flt_buffer;
	_flt_buffer = 0;

	_buffer_counter = 0;
	_buffer_size = 0;

	return true;
}


void WiiBBoard::run () {

	if (!_timer) {
		_timer = new Timer (1.0 / SSI_WIIBBOARD_SAMPLE_RATE);
	}

	int status = _wiimote->RefreshState ();

	if (_wiimote->ConnectionLost ()) {
		ssi_wrn ("connection lost [%d]", _options.device);
		sleep_s (1.0);
		return;
	}

	++_buffer_counter;

	if (_raw_provider) {

		*_raw_buffer_ptr++ = _wiimote->BalanceBoard.Raw.BottomL;
		*_raw_buffer_ptr++ = _wiimote->BalanceBoard.Raw.BottomR;
		*_raw_buffer_ptr++ = _wiimote->BalanceBoard.Raw.TopL;
		*_raw_buffer_ptr++ = _wiimote->BalanceBoard.Raw.TopR;

		if (_buffer_counter == _buffer_size) {

			_raw_provider->provide (ssi_pcast (ssi_byte_t, _raw_buffer), _buffer_size);
			_raw_buffer_ptr = _raw_buffer;

			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "provide raw data");
		}
	}

	if (_flt_provider) {

		*_flt_buffer_ptr++ = _wiimote->BalanceBoard.Kg.BottomL;	
		*_flt_buffer_ptr++ = _wiimote->BalanceBoard.Kg.BottomR;	
		*_flt_buffer_ptr++ = _wiimote->BalanceBoard.Kg.TopL;	
		*_flt_buffer_ptr++ = _wiimote->BalanceBoard.Kg.TopR;

		if (_buffer_counter == _buffer_size) {

			_flt_provider->provide (ssi_pcast (ssi_byte_t, _flt_buffer), _buffer_size);
			_flt_buffer_ptr = _flt_buffer;

			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "provide flt data");
		}
	}

	if (_buffer_counter == _buffer_size) {
		_buffer_counter = 0;
	}
	_timer->wait ();
}

}
