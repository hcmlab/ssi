// MyEventListener.cpp
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

#include "MyEventListener.h"
#include "event/EventAddress.h"
#include "base/Factory.h"

namespace ssi {

char MyEventListener::ssi_log_name[] = "myelisten_";

MyEventListener::MyEventListener(const ssi_char_t *file) {
}

MyEventListener::~MyEventListener() {
}

void MyEventListener::listen_enter() {

	ssi_msg(SSI_LOG_LEVEL_BASIC, "enter()..ok");
}

bool MyEventListener::update(IEvents &events,
	ssi_size_t n_new_events,
	ssi_size_t time_ms) {

	EventAddress ea;

	ssi_event_t *e = 0;
	for (ssi_size_t i = 0; i < n_new_events; i++) {
		e = events.next();
		ea.clear();
		ea.setSender(Factory::GetString(e->sender_id));
		ea.setEvents(Factory::GetString(e->event_id));
		ssi_print("received event %s of type %s at %ums for %ums\n", ea.getAddress(), SSI_ETYPE_NAMES[e->type], e->time, e->dur);
		if (e->type == SSI_ETYPE_TUPLE) {
			ssi_real_t *ptr = ssi_pcast(ssi_real_t, e->ptr);
			ssi_size_t n = e->tot / sizeof(ssi_real_t);
			for (ssi_size_t j = 0; j < n; j++) {
				ssi_print("%.2f ", *ptr++);
			}
			ssi_print("\n");
		}
	}

	return true;
}

void MyEventListener::listen_flush() {

	ssi_msg(SSI_LOG_LEVEL_BASIC, "flush()..ok");
}

}
