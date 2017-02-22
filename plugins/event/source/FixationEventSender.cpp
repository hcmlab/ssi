// FixationEventSender.cpp
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

#include "FixationEventSender.h"
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

char *FixationEventSender::ssi_log_name = "fixation__";

FixationEventSender::FixationEventSender (const ssi_char_t *file)
	: ssi_log_level (SSI_LOG_LEVEL_DEFAULT),
	_elistener (0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init (_event, SSI_ETYPE_EMPTY);
}

FixationEventSender::~FixationEventSender () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}

	ssi_event_destroy (_event);
}

bool FixationEventSender::setEventListener (IEventListener *listener) {

	_elistener = listener;
	
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

void FixationEventSender::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	reset (0);
}

void FixationEventSender::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	ssi_real_t *ptr = ssi_pcast (ssi_real_t, stream_in[0].ptr);
	ssi_time_t time = consume_info.time;
	ssi_time_t dtime = 1.0 / stream_in[0].sr;

	for (ssi_size_t i = 0; i < stream_in[0].num; i++) {

		update (*ptr, *(ptr+1));		

		if (_infix && !isfix ()) {
			if (_elistener) {				
				_event.time = ssi_cast (ssi_size_t, 1000 * _start + 0.5);
				_event.dur = ssi_cast (ssi_size_t, 1000 * (time - _start) + 0.5);		
				_event.state = SSI_ESTATE_COMPLETED;
				_elistener->update (_event);	
			}
			reset (time);
		} else if (!_infix && isfix ()) {
			if (time - _start >= _options.min_dur) {	
				_infix = true;
			}
		} else if (_infix) {
			if (time - _last >= _options.max_dur) {
				if (_elistener) {				
					_event.time = ssi_cast (ssi_size_t, 1000 * _start + 0.5);
					_event.dur = ssi_cast (ssi_size_t, 1000 * (time - _start) + 0.5);		
					_event.state = SSI_ESTATE_CONTINUED;
					_elistener->update (_event);	
					_last += _options.max_dur;
				}
			}
		} else if (!_infix && !isfix ()) {
			reset (time);
		}

		ptr += 2;
		time += dtime;
	}
}

void FixationEventSender::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {
}

SSI_INLINE void FixationEventSender::update (ssi_real_t x, ssi_real_t y) {
	
	_minx = min (x, _minx);
	_maxx = max (x, _maxx);
	_miny = min (y, _miny);
	_maxy = max (y, _maxy);
}

SSI_INLINE void FixationEventSender::reset (ssi_time_t start) {

	_start = start;
	_last = start;
	_infix = false;
	_minx = _miny = FLT_MAX;
	_maxx = _maxy = -FLT_MAX;
}

SSI_INLINE bool FixationEventSender::isfix () {

	return (_maxx - _minx) + (_maxy - _miny) <= _options.disp_thres;
}

}
