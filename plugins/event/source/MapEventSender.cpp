// MapEventSender.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/01/12
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

#include "MapEventSender.h"
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

ssi_char_t *MapEventSender::ssi_log_name = "mapesend__";

MapEventSender::MapEventSender (const ssi_char_t *file)
	: _file (0),
	_listener (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init (_event, SSI_ETYPE_MAP);
}

MapEventSender::~MapEventSender () {

	ssi_event_destroy (_event);

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

bool MapEventSender::setEventListener (IEventListener *listener) {

	_listener = listener;

	if (_options.address[0] != '\0') {

		SSI_OPTIONLIST_SET_ADDRESS(_options.address, _event_address, _event);

	} else {

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

void MapEventSender::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (stream_in[0].type != SSI_REAL) {
		ssi_err ("type '%s' not supported", SSI_TYPE_NAMES[stream_in[0].type]);
	}

	ssi_size_t dim = stream_in[0].dim;

	ssi_event_adjust(_event, sizeof(ssi_event_map_t) * dim);
	ssi_event_map_t *dst = ssi_pcast(ssi_event_map_t, _event.ptr);

	ssi_char_t **keys = 0;
	if (_options.keys[0] != '\0') {
		ssi_size_t n = ssi_split_string_count(_options.keys, ',');
		keys = new ssi_char_t *[dim];
		ssi_split_string(n, keys, _options.keys, ',');
		for (ssi_size_t i = n; i < dim; i++) {			
			keys[i] = ssi_strcpy(keys[i%n]);
		}
	}

	ssi_char_t str[512];
	if (keys) {
		for (ssi_size_t i = 0; i < dim; i++) {
			dst++->id = Factory::AddString(keys[i]);
		}
	} else {
		for (ssi_size_t i = 0; i < dim; i++) {
			ssi_sprint(str, "dim#%u", i);
			dst++->id = Factory::AddString(str);
		}
	}

	if (keys) {
		for (ssi_size_t i = 0; i < dim; i++) {
			delete[] keys[i];
		}
		delete[] keys; keys = 0;
	}
}

void MapEventSender::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	ssi_size_t dim = stream_in[0].dim;
	ssi_size_t num = stream_in[0].num;

	_event.time = ssi_cast (ssi_size_t, 1000 * consume_info.time + 0.5);
	_event.dur = ssi_cast (ssi_size_t, 1000 * consume_info.dur + 0.5);	
	_event.state = consume_info.status == IConsumer::COMPLETED ? SSI_ESTATE_COMPLETED : SSI_ESTATE_CONTINUED;
	
	if (_options.mean) {			
		ssi_real_t *src = ssi_pcast(ssi_real_t, stream_in[0].ptr);
		ssi_event_map_t *dst = ssi_pcast(ssi_event_map_t, _event.ptr);
		ssi_real_t *mean = new ssi_real_t[dim];
		ssi_mean(num, dim, src, mean);
		for (ssi_size_t i = 0; i < dim; i++) {
			dst++->value = mean[i];
		}		
		if (_listener) {
			_listener->update(_event);
		}
		delete[] mean;
	} else {
		ssi_real_t *src = ssi_pcast(ssi_real_t, stream_in[0].ptr);
		ssi_event_map_t *dst = ssi_pcast(ssi_event_map_t, _event.ptr);
		for (ssi_size_t i = 0; i < num; i++) {
			for (ssi_size_t j = 0; j < dim; j++) {
				dst++->value = *src++;
			}
			if (_listener) {
				_listener->update(_event);
			}
		}
	}
}

void MapEventSender::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	ssi_event_reset (_event);
}



}
