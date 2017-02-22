// ClockEventSender.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/02/01
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "ClockEventSender.h"
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

ssi_char_t *ClockEventSender::ssi_log_name = "clocksend_";

ClockEventSender::ClockEventSender (const ssi_char_t *file)
	: _file (0),
	_clocks(0),
	_n_clocks(0),
	_listener (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init (_event, SSI_ETYPE_TUPLE);
	_frame = Factory::GetFramework();
}

ClockEventSender::~ClockEventSender () {

	ssi_event_destroy (_event);

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

bool ClockEventSender::setEventListener (IEventListener *listener) {

	if (_options.empty) {
		ssi_event_init(_event, SSI_ETYPE_EMPTY);
	}
	else {
		ssi_event_init(_event, SSI_ETYPE_STRING);
		ssi_event_adjust(_event, ssi_strlen(_options.string) + 1);
		ssi_strcpy(_event.ptr, _options.string);
	}

	_listener = listener;
	
	if (_options.address[0] != '\0') {

		SSI_OPTIONLIST_SET_ADDRESS(_options.address, _event_address, _event);

	}
	else {

		ssi_wrn("use of deprecated option 'sname' and 'ename', use 'address' instead")

		_event.sender_id = Factory::AddString(_options.sname);
		if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}
		_event.event_id = Factory::AddString(_options.ename);
		if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}

		_event_address.setSender(_options.sname);
		_event_address.setEvents(_options.ename);
	}

	return true;
}

void ClockEventSender::initClocks() {

	if (_options.clocks[0] != '\0') {
		_n_clocks = ssi_string2array_count(_options.clocks, ';');
		_clocks = new ssi_size_t[_n_clocks];
		ssi_string2array(_n_clocks, _clocks, _options.clocks, ';');
	} else {
		_n_clocks = 1;
		_clocks = new ssi_size_t[_n_clocks];
		_clocks[0] = _options.clock;
	}
}

void ClockEventSender::enter() {

	initClocks();
	_next = 0;
	_event.time = 0;
	_event.dur = 0;
	_init = true;
}

void ClockEventSender::run() {

	if (_frame->IsInIdleMode()) {
		sleep_ms(10);
		return;
	}

	_timer.reset();

    if ((_listener&& (!_options.silence)) && (!_init || _options.init)) {
		_listener->update(_event);		
	}
	_init = false;

	_event.time += _clocks[_next];
	_timer.setClockMs(_clocks[_next]);
	if (++_next >= _n_clocks) {
		_next = 0;
	}

	_timer.wait();
}

void ClockEventSender::flush() {

	_n_clocks = 0;
	delete[] _clocks; _clocks = 0;
}


}
