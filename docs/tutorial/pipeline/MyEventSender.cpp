// MyEventSender.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/10/02
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

#include "MyEventSender.h"
#include "base/Factory.h"

namespace ssi {

char MyEventSender::ssi_log_name[] = "myesender_";

MyEventSender::MyEventSender(const ssi_char_t *file) {

	ssi_event_init(_event, SSI_ETYPE_TUPLE);
}

MyEventSender::~MyEventSender() {
}

void MyEventSender::consume_enter(ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	ssi_event_adjust(_event, stream_in[0].dim * sizeof(ssi_real_t));

	ssi_msg(SSI_LOG_LEVEL_BASIC, "enter()..ok");
}

void MyEventSender::consume(IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	ssi_real_t *in = ssi_pcast(ssi_real_t, stream_in[0].ptr);
	ssi_real_t *out = ssi_pcast(ssi_real_t, _event.ptr);

	ssi_mean(stream_in[0].num, stream_in[0].dim, in, out);

	_event.time = ssi_cast(ssi_size_t, consume_info.time * 1000);
	_event.dur = ssi_cast(ssi_size_t, consume_info.dur * 1000);
	_elistener->update(_event);
}

void MyEventSender::consume_flush(ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	ssi_event_destroy(_event);

	ssi_msg(SSI_LOG_LEVEL_BASIC, "flush()..ok");
}


bool MyEventSender::setEventListener(IEventListener *listener) {

	_elistener = listener;
	_event.sender_id = Factory::AddString("myevent");
	if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	_event.event_id = Factory::AddString("mysender");
	if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}

	_event_address.setSender("myevent");
	_event_address.setEvents("mysender");

	return true;
}

}
