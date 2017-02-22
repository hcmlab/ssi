// TransformedEventSender.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2012/05/23
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

#include "TransformedEventSender.h"
#include "signal/Functionals.h"
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

ssi_char_t *TransformedEventSender::ssi_log_name = "evsend____";
int TransformedEventSender::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

TransformedEventSender::TransformedEventSender (const ssi_char_t *file)
	: _file (0),
	_listener (0),
	_transformer (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}

	_board = Factory::GetEventBoard ();
	ssi_event_init (_event, SSI_ETYPE_MAP);
}

TransformedEventSender::~TransformedEventSender () {

	ssi_event_destroy (_event);

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

bool TransformedEventSender::setEventListener (IEventListener *listener) {

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

void TransformedEventSender::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (_transformer == 0) {
		ssi_wrn ("transformer not set");
		return;
	}

	ssi_size_t dim = _transformer->getSampleDimensionOut (stream_in[0].dim);
	ssi_size_t byte = _transformer->getSampleBytesOut (stream_in[0].byte);
	ssi_type_t type = _transformer->getSampleTypeOut (stream_in[0].type);
	ssi_stream_init (_fstream, 1, dim, byte, type, 0);
	_transformer->transform_enter (stream_in[0], _fstream);
	
	/*ssi_event_adjust (_event, 10 * sizeof (ssi_event_map_t));*/
	ssi_event_adjust (_event, dim * sizeof (ssi_event_map_t));

}

void TransformedEventSender::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (_transformer == 0) {
		ssi_wrn ("transformer not set");
		return;
	}
	
	_event.time = ssi_cast (ssi_size_t, 1000 * consume_info.time + 0.5);
	_event.dur = ssi_cast (ssi_size_t, 1000 * consume_info.dur + 0.5);
	
	ITransformer::info info;
	info.delta_num = 0;
	info.frame_num = stream_in[0].num;
	info.time = consume_info.time;

	_transformer->transform (info, stream_in[0], _fstream);
	ssi_real_t *fptr = ssi_pcast (ssi_real_t, _fstream.ptr);
	
	ssi_event_map_t *e = ssi_pcast (ssi_event_map_t, _event.ptr);

	for (ssi_size_t i = 0; i < _fstream.dim; i++) {
	/*for (ssi_size_t i = 0; i < 10; i++) {*/
		e[i].id = Factory::GetStringId("Dimension"); // TODO: einmalig in consume_enter registrieren und id als variable speichern 
		e[i].value = *fptr++;
	}

	
	if (_listener) {
		_listener->update (_event);
	}
}

void TransformedEventSender::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (_transformer == 0) {
		ssi_wrn ("transformer not set");
		return;
	}

	_transformer->transform_flush (stream_in[0], _fstream);
	ssi_stream_destroy (_fstream);
	ssi_event_reset (_event);
}



}
