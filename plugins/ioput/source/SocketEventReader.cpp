// SocketEventReader.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/06/02
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

#include "SocketEventReader.h"
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

int SocketEventReader::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
ssi_char_t *SocketEventReader::ssi_log_name = "sockerecv_";

SocketEventReader::SocketEventReader (const ssi_char_t *file)
	: _file (0),
	_elistener (0),
	_frame (0),
	_socket (0),
	_socket_osc (0),
	_buffer (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init (_event_string, SSI_ETYPE_STRING);
};

SocketEventReader::~SocketEventReader () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
	
	ssi_event_destroy (_event_string);
}

bool SocketEventReader::setEventListener (IEventListener *listener) {

	_elistener = listener;


	if (_options.address[0] != '\0') {

		SSI_OPTIONLIST_SET_ADDRESS(_options.address, _event_address, _event_string);

	} else {

		ssi_wrn("use of deprecated option 'sname' and 'ename', use 'address' instead")

		_event_string.sender_id = Factory::AddString(_options.sname);
		if (_event_string.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}
		_event_string.event_id = Factory::AddString(_options.ename);
		if (_event_string.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}
		_event_address.setSender(_options.sname);
		_event_address.setEvents(_options.ename);

	}

	_event_string.state = SSI_ESTATE_COMPLETED;
	_event_string.prob = 1.0f;

	return true;
}

void SocketEventReader::message (const char *from,
	const ssi_char_t *sender_id,
	const ssi_char_t *event_id,
	osc_int32 time,
	osc_int32 dur,
	const ssi_char_t *msg) {
	
	if (_elistener) {
		_event_string.time = _frame->GetElapsedTimeMs ();
		_event_string.dur = dur;
		_event_string.event_id = Factory::AddString(event_id);
		_event_string.sender_id = Factory::AddString(sender_id);
		ssi_event_adjust (_event_string, ssi_strlen (msg) + 1);
		strcpy (_event_string.ptr, msg);
		_elistener->update (_event_string);
	}
}

void SocketEventReader::event (const char *from,
	const ssi_char_t *sender_id,
	const ssi_char_t *event_id,
	osc_int32 time,
	osc_int32 dur,
	osc_int32 state,
	osc_int32 n_events,
	const ssi_char_t **events,
	const ssi_real_t *values) {		

	if (_elistener) {

		ssi_event_t e;
		ssi_size_t time_ms = _options.reltime ? _frame->GetElapsedTimeMs() - time : time;

		if (n_events == 0)
		{
			ssi_event_init(e, SSI_ETYPE_EMPTY, Factory::AddString(sender_id), Factory::AddString(event_id), time_ms, dur, 0, 1.0f, ssi_estate_t(state));
		}
		else
		{
			ssi_event_init(e, SSI_ETYPE_MAP, Factory::AddString(sender_id), Factory::AddString(event_id), time_ms, dur, n_events * sizeof(ssi_event_map_t), 1.0f, ssi_estate_t(state));
			ssi_event_map_t *t = ssi_pcast(ssi_event_map_t, e.ptr);
			for (osc_int32 i = 0; i < n_events; i++) {
				t[i].id = Factory::AddString(events[i]);
				t[i].value = values[i];
			}
		}

		_elistener->update(e);
		ssi_event_destroy(e);
	}
};

void SocketEventReader::enter () {

	_socket = Socket::CreateAndConnect (_options.type, Socket::SERVER, _options.port, _options.host);	
	if (_options.osc) {
		_socket_osc = new SocketOsc (*_socket, _options.size);
	} else {
		_buffer = new ssi_byte_t[_options.size];
	}
	
	if (!_socket) {
		ssi_wrn ("could not connect '%s@%u'", _options.host, _options.port);
	}

	_frame = Factory::GetFramework ();
	
	ssi_char_t string[SSI_MAX_CHAR];
	ssi_sprint (string, "SocketEventReader@'%s@%u'", _options.host, _options.port); 	
	Thread::setName (string);

	ssi_sprint(string, "%s@%u", _options.host, _options.port);
	ssi_msg(SSI_LOG_LEVEL_BASIC, "started '%s'", string);
}

void SocketEventReader::run () {

	int result = 0;
	
	if (_options.osc) {		
		result = _socket_osc->recv (this, _options.timeout); 
	} else if (_socket->isConnected()) {
		result = _socket->recv (_buffer, _options.size, _options.timeout); 
		if (result > 0) {
			// make sure we have an ending null-byte
			bool has_null_byte = false;
			for (int i = 0; i < result; i++) {
				if (_buffer[i] == '\0') {
					has_null_byte = true;
					break;
				}
			}
			// send event
			if (_elistener) {
				_event_string.time = _frame->GetElapsedTimeMs ();
				_event_string.dur = 0;
				if (has_null_byte) {
					ssi_event_adjust(_event_string, ssi_strlen(_buffer) + 1);
					strcpy(_event_string.ptr, _buffer);					
				} else {
					ssi_event_adjust(_event_string, result + 1);
					memcpy(_event_string.ptr, _buffer, result * sizeof(ssi_byte_t));
					_event_string.ptr[result] = '\0';
				}				
				_elistener->update (_event_string);
			}	
		}
	} else {
		Sleep(10);
	}

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "received %d bytes", result);
};

void SocketEventReader::flush () {
	
	delete _socket; _socket = 0;
	delete _socket_osc; _socket_osc = 0;
	delete[] _buffer; _buffer = 0;

	ssi_char_t string[SSI_MAX_CHAR];
	ssi_sprint(string, "%s@%u", _options.host, _options.port);
	ssi_msg(SSI_LOG_LEVEL_BASIC, "stopped'%s'", string);
}

}
