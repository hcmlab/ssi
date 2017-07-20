// WiiRemoteButtonEventSender.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/10/14
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

#include "WiiRemote.h"
#include "WiiRemoteButtonEventSender.h"
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

ssi_char_t *WiiRemoteButtonEventSender::ssi_log_name = "wiibutsend";

WiiRemoteButtonEventSender::WiiRemoteButtonEventSender (const ssi_char_t *file)
	: _file (0),
	_listener (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init (_event, SSI_ETYPE_STRING);
}

WiiRemoteButtonEventSender::~WiiRemoteButtonEventSender () {

	ssi_event_destroy (_event);

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool WiiRemoteButtonEventSender::setEventListener (IEventListener *listener) {

	_listener = listener;
	_event.sender_id = Factory::AddString (_options.sname);
	if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	_event.event_id = Factory::AddString (_options.ename);
	if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	
	_event_address.setSender (_options.sname);
	_event_address.setEvents (_options.ename);

	return true;
}

void WiiRemoteButtonEventSender::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (stream_in[0].type != SSI_SHORT) {
		ssi_err ("type '%s' not supported", SSI_TYPE_NAMES[stream_in[0].type]);
	}

	if (stream_in[0].dim != 1) {
		ssi_err ("dimension != 1 not supported");
	}

	ssi_event_adjust (_event, 256 * sizeof (ssi_char_t));
}

void WiiRemoteButtonEventSender::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {
	
	_event.time = ssi_cast (ssi_size_t, 1000 * consume_info.time + 0.5);
	_event.dur = ssi_cast (ssi_size_t, 1000 * consume_info.dur + 0.5);		

	short *ptr = ssi_pcast (short, stream_in[0].ptr);
	short button = 0;

	for (ssi_size_t i = 0; i < stream_in[0].num; i++) {
		button |= *ptr++;
	}

	ssi_char_t *string = ssi_pcast (ssi_char_t, _event.ptr);
	string[0] = '\0';
	bool first = true;
	if (button & WiiRemote::LEFT) {
		if (!first) {
			strcat (string, ",");
		}
		strcat (string, "left");
		first = false;
	}
	if (button & WiiRemote::RIGHT) {
		if (!first) {
			strcat (string, ",");
		}
		strcat (string, "right");
		first = false;
	}
	if (button & WiiRemote::DOWN) {
		if (!first) {
			strcat (string, ",");
		}
		strcat (string, "down");
		first = false;
	}
	if (button & WiiRemote::UP) {
		if (!first) {
			strcat (string, ",");
		}
		strcat (string, "up");
		first = false;
	}
	if (button & WiiRemote::PLUS) {
		if (!first) {
			strcat (string, ",");
		}
		strcat (string, "+");
		first = false;
	}
	if (button & WiiRemote::TWO) {
		if (!first) {
			strcat (string, ",");
		}
		strcat (string, "2");
		first = false;
	}
	if (button & WiiRemote::ONE) {
		if (!first) {
			strcat (string, ",");
		}
		strcat (string, "1");
		first = false;
	}
	if (button & WiiRemote::B) {
		if (!first) {
			strcat (string, ",");
		}
		strcat (string, "b");
		first = false;
	}
	if (button & WiiRemote::A) {
		if (!first) {
			strcat (string, ",");
		}
		strcat (string, "a");
		first = false;
	}
	if (button & WiiRemote::MINUS) {
		if (!first) {
			strcat (string, ",");
		}
		strcat (string, "-");
		first = false;
	}
	if (button & WiiRemote::HOME) {
		if (!first) {
			strcat (string, ",");
		}
		strcat (string, "home");
		first = false;
	}

	if (_listener) {
		_listener->update (_event);
	}
}

void WiiRemoteButtonEventSender::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	ssi_event_reset (_event);
}



}
