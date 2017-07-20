// StringEventSender.cpp
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

#include "StringEventSender.h"
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

ssi_char_t *StringEventSender::ssi_log_name = "stringsend";

StringEventSender::StringEventSender (const ssi_char_t *file)
	: _file (0),
	_listener (0),
	_buffer(0),
	_n_buffer(0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init(_event, SSI_ETYPE_STRING);
}

StringEventSender::~StringEventSender () {

	ssi_event_destroy (_event);

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool StringEventSender::setEventListener (IEventListener *listener) {

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

void StringEventSender::consume_enter(ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (stream_in[0].type != SSI_REAL) {
		ssi_err("type '%s' not supported", SSI_TYPE_NAMES[stream_in[0].type]);
	}

	_n_buffer = _options.n_buffer;
	_buffer = new ssi_char_t[_n_buffer];
}

void StringEventSender::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {
	
	_event.time = ssi_cast (ssi_size_t, 1000 * consume_info.time + 0.5);
	_event.dur = ssi_cast (ssi_size_t, 1000 * consume_info.dur + 0.5);	
	_event.state = consume_info.status == IConsumer::COMPLETED ? SSI_ESTATE_COMPLETED : SSI_ESTATE_CONTINUED;

	ssi_real_t *ptr = ssi_pcast (ssi_real_t, stream_in[0].ptr);

	if (_options.mean) {		
		ssi_real_t *dst = new ssi_real_t[stream_in[0].dim];
		ssi_mean (stream_in[0].num, stream_in[0].dim, ptr, dst);
		ssi_size_t n = ssi_stream_print(dst, stream_in[0].type, 1, stream_in[0].dim, stream_in[0].byte, _n_buffer, _buffer, ',', _options.delim);
		delete[] dst;
		if (n > 0) {
			ssi_event_adjust(_event, n+1);
			ssi_strcpy(_event.ptr, _buffer);
			_listener->update(_event);
		} else {
            ssi_wrn("string buffer too short '%d'", _n_buffer);
		}
	} else {
		ssi_size_t n = ssi_stream_print(stream_in[0], _n_buffer, _buffer, ',', _options.delim);
		if (n > 0) {
			ssi_event_adjust(_event, n+1);
			ssi_strcpy(_event.ptr, _buffer);
			_listener->update(_event);
		}
		else {
            ssi_wrn("string buffer too short '%d'", _n_buffer);
		}
	}
}

void StringEventSender::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	ssi_event_reset (_event);
	delete[] _buffer; _buffer = 0;
	_n_buffer = 0;
}



}
