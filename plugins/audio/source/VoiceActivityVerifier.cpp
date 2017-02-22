// VoiceActivityVerifier.cpp
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

#include "VoiceActivityVerifier.h"
#include "base/Factory.h"
#include "signal/SignalTools.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *VoiceActivityVerifier::ssi_log_name = "vadverify_";

VoiceActivityVerifier::VoiceActivityVerifier (const ssi_char_t *file)
	: _file (0),
	_listener (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	_lpc = ssi_pcast(AudioLpc, Factory::Create(AudioLpc::GetCreateName(), 0, false));	
	if (!_lpc) {
		ssi_err("could not create 'AudioLpc'");
	}
	for (ssi_size_t i = 0; i < _lpc->getOptions()->getSize(); i++) {
		ssi_option_t *o = _lpc->getOptions()->getOption(i);
		_options.addOption(o->name, o->ptr, o->num, o->type, o->help);
	}

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init (_event, SSI_ETYPE_EMPTY);
	
}

VoiceActivityVerifier::~VoiceActivityVerifier () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}

	ssi_event_destroy(_event);
	delete _lpc;
}

bool VoiceActivityVerifier::setEventListener (IEventListener *listener) {

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

void VoiceActivityVerifier::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (stream_in[0].type != SSI_REAL) {
		ssi_err ("type '%s' not supported", SSI_TYPE_NAMES[stream_in[0].type]);
	}
}

void VoiceActivityVerifier::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	ssi_stream_t lpc_s;
	SignalTools::Transform(stream_in[0], lpc_s, *_lpc, "0.01s", "0.05s");

	ssi_real_t thres = _options.threshold;
	ssi_size_t from = _options.from;
	ssi_size_t to = _options.to;
	ssi_size_t num = lpc_s.num;
	ssi_size_t dim = lpc_s.dim;
	ssi_real_t *lpc = ssi_pcast(ssi_real_t, lpc_s.ptr);

	if (dim <= to) {
		ssi_wrn("requested index '%u' out of range, set to '%u'", to, num - 1);
		to = _options.to = num - 1;
	}
	
	ssi_real_t x = 0;
	for (ssi_size_t i = 0; i < num; i++) {		
		for (ssi_size_t j = from; j <= to; j++) {
			x += lpc[j] * lpc[j];
		}
		lpc += dim;
	}
	x /= num;

	bool verified = x >= thres;
	ssi_msg(SSI_LOG_LEVEL_DETAIL, "%s (%.2f)", verified ? "keep" : "skip", x);

	if (verified && _listener) {
		_event.time = consume_info.event->time;
		_event.dur = consume_info.event->dur;
		_event.state = consume_info.event->state;
		_listener->update(_event);
	}

	ssi_stream_destroy(lpc_s);
}

void VoiceActivityVerifier::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	ssi_event_reset (_event);
}



}
