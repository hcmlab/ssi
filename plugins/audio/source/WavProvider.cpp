// WavProvider.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/01/12
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

#include "WavProvider.h"
#include "base/Factory.h"

namespace ssi {

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

const ssi_char_t *WavProvider::ssi_log_name = "wavprovide";

WavProvider::WavProvider(const char *file)
        #if _WIN32|_WIN64
	: _writer (0),
	_file (0),
        _call_flush (false)
        #endif
{
        #if _WIN32|_WIN64
	_writer = ssi_pcast (WavWriter, WavWriter::Create (0));	
        #endif
	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

WavProvider::~WavProvider() {
#if _WIN32|_WIN64
	if (_call_flush) {
		_writer->consume_flush (1, &_stream);
	}
#endif
	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void WavProvider::init (IChannel *channel) {

	_stream = channel->getStream ();
#if _WIN32|_WIN64
        _writer->getOptions ()->setPath (_options.path);
#endif
	_format = ssi_create_wfx (_stream.sr, _stream.dim, _stream.byte);

#if _WIN32|_WIN64
	_info.status = IConsumer::NO_TRIGGER;
	_info.time = 0;

        _writer->consume_enter (1, &_stream);
#endif
	_call_flush = true;
}

bool WavProvider::provide (ssi_byte_t *data, 
	ssi_size_t sample_number) {

	_stream.num_real = _stream.num = sample_number;
	_stream.tot_real = _stream.tot = sample_number * _stream.dim * _stream.byte;
	_stream.ptr = data;
        #if _WIN32|_WIN64
	_info.dur = sample_number * (1.0 / _stream.sr);	    
	_writer->consume (_info, 1, &_stream);
	_info.time += _info.dur;
        #endif
	return true;
}


}
