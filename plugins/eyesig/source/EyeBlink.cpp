// EyeBlink.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/04
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

#include "EyeBlink.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

char *EyeBlink::ssi_log_name = "eyeblink__";

EyeBlink::EyeBlink (const ssi_char_t *file)
	: _elistener (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init (_event, SSI_ETYPE_STRING,0,0,0,0,256);

}

EyeBlink::~EyeBlink () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}

	ssi_event_destroy (_event);
}


bool EyeBlink::setEventListener (IEventListener *listener) {

		_elistener = listener;
		_event.sender_id = Factory::AddString(_options.sname);
		if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}
		_event.event_id = Factory::AddString(_options.ename);
		if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}

		_event_address.setSender (_options.sname);
		_event_address.setEvents (_options.ename);
		_event.prob = 1.0;
		return true;
	}


void EyeBlink::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	reset (0);
}

void EyeBlink::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	int *ptr = ssi_pcast (int, stream_in[0].ptr);
	double time = consume_info.time;
	double dtime = 1.0 / stream_in[0].sr;

	for (ssi_size_t i = 0; i < stream_in[0].num; i++) {

		if (_inblink && !isblink(*ptr, *(ptr + 1))) 
		{			
			if (time - _start <= _options.maxdur)
				blinkend (time);

			reset (time);
		}
		else if (!_inblink && isblink (*ptr, *(ptr+1))) 
		{
			if (time - _start >= _options.mindur) {
				blinkstart (_start);		
				_inblink = true;
			}
		} else if (!_inblink && !isblink (*ptr, *(ptr+1))) {
			reset (time);
		}

		ptr += 2;
		time += dtime;
	}
}

SSI_INLINE void EyeBlink::reset (double start) {

	_start = start;
	_inblink = false;
}

bool EyeBlink::isblink (int x, int y) {

	return (x < 0 && y < 0);
}

void EyeBlink::blinkstart (double time) { 	
	
	if(_options.console)	
	{
		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "start of blink @ %.2lf", time);	
	}	
}

void EyeBlink::blinkend (double time) {
		
	
	if(_options.console)	
	{
		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "end of blink @ %.2lf", time);
	}
	else
	{
		if (_elistener) 
		{
					
			_event.dur = ssi_cast (ssi_size_t, 1000 * (time - _start) + 0.5);
			_event.time = ssi_cast (ssi_size_t, 1000 * _start + 0.5);
			strcpy(_event.ptr, _options.ename);
			_event.state =  SSI_ESTATE_COMPLETED;
			_event.prob = 1.0;
			_elistener->update (_event);
					
		}
	}	
}

}
