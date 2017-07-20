// FunctionalsEventSender.cpp
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

#include "FunctionalsEventSender.h"
#include "Functionals.h"
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

ssi_char_t *FunctionalsEventSender::ssi_log_name = "functsend_";

FunctionalsEventSender::FunctionalsEventSender (const ssi_char_t *file)
	: _file (0),
	_listener (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	_board = Factory::GetEventBoard ();
	ssi_event_init (_event, SSI_ETYPE_MAP);
}

FunctionalsEventSender::~FunctionalsEventSender () {

	ssi_event_destroy (_event);

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool FunctionalsEventSender::setEventListener (IEventListener *listener) {

	_listener = listener;

	if (_options.address[0] != '\0') {

		SSI_OPTIONLIST_SET_ADDRESS(_options.address, _event_address, _event);

	} else {

		ssi_wrn("use of deprecated option 'sname' and 'ename', use 'address' instead");

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

void FunctionalsEventSender::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (stream_in[0].type != SSI_REAL) {
		ssi_err ("type '%s' not supported", SSI_TYPE_NAMES[stream_in[0].type]);
	}

	Functionals *functionals = ssi_create (Functionals, 0, false);
	ssi_strcpy (functionals->getOptions ()->names, _options.names);
	functionals->getOptions ()->delta = _options.delta;
	ssi_size_t dim = functionals->getSampleDimensionOut (stream_in[0].dim);
	ssi_size_t byte = functionals->getSampleBytesOut (stream_in[0].byte);
	ssi_type_t type = functionals->getSampleTypeOut (stream_in[0].type);
	ssi_stream_init (_fstream, 1, dim, byte, type, 0);
	functionals->transform_enter (stream_in[0], _fstream);
	_functionals = functionals;

	ssi_event_adjust (_event, dim * sizeof (ssi_event_map_t));
	ssi_event_map_t *t = ssi_pcast (ssi_event_map_t, _event.ptr);	
	for (ssi_size_t i = 0; i < stream_in[0].dim; i++) {
		for (ssi_size_t j = 0; j < dim / stream_in[0].dim; j++) {
			(t++)->id = Factory::AddString (functionals->getName (j));
		}
	}
}

void FunctionalsEventSender::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {
	
	_event.time = ssi_cast (ssi_size_t, 1000 * consume_info.time + 0.5);
	_event.dur = ssi_cast (ssi_size_t, 1000 * consume_info.dur + 0.5);		

	ssi_real_t *ptr = ssi_pcast (ssi_real_t, stream_in[0].ptr);

	ITransformer::info info;
	info.delta_num = 0;
	info.frame_num = stream_in[0].num;
	info.time = consume_info.time;
	_functionals->transform (info, stream_in[0], _fstream);

	ssi_real_t *fptr = ssi_pcast (ssi_real_t, _fstream.ptr);
	ssi_event_map_t *t = ssi_pcast (ssi_event_map_t, _event.ptr);
	for (ssi_size_t i = 0; i < _fstream.dim; i++) {
		(t++)->value = *fptr++;
	}

	if (_listener) {
		_listener->update (_event);
	}
}

void FunctionalsEventSender::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	_functionals->transform_flush (stream_in[0], _fstream);
	ssi_stream_destroy (_fstream);
	ssi_event_reset (_event);
}



}
