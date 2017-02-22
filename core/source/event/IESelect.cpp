// IESelect.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/09/19
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

#include "event/IESelect.h"

namespace ssi {

IESelect::IESelect (IEvents *events)
: _events (*events),
	_n_event_ids (0),
	_event_ids (0),
	_n_sender_ids (0),
	_sender_ids (0),
	_time_span (0),
	_time_ref (0) {
}

IESelect::~IESelect () {

	release ();
}

void IESelect::release () {

	_n_event_ids = 0;
	delete[] _event_ids; _event_ids = 0;
	_n_sender_ids = 0;
	delete[] _sender_ids; _sender_ids = 0;
	_time_span = 0;
	_time_ref = 0;
}

void IESelect::set (ssi_size_t n_sender_ids, 
	ssi_size_t *sender_ids, 
	ssi_size_t n_event_ids, 
	ssi_size_t *event_ids,
	ssi_size_t time_span_ms,
	EVENT_STATE_FILTER::List state_filter) {

	release ();

	_n_event_ids = n_event_ids;
	if (_n_event_ids > 0) {
		_event_ids = new ssi_size_t[_n_event_ids];
		memcpy (_event_ids, event_ids, sizeof (ssi_size_t) * _n_event_ids);
	}
	_n_sender_ids = n_sender_ids;
	if (_n_sender_ids > 0) {
		_sender_ids = new ssi_size_t[_n_sender_ids];
		memcpy (_sender_ids, sender_ids, sizeof (ssi_size_t) * _n_sender_ids);
	}

	_time_span = time_span_ms;
	_state_filter = state_filter;
}

ssi_event_t *IESelect::get (ssi_size_t index) {

	ssi_size_t size = getSize ();
	if (index >= size) {
		ssi_wrn ("index '%u' exceeds #events '%u'", index, size);
		return 0;
	}

	while (index-- > 0) {
		next ();
	}

	return next ();
}

bool IESelect::check (ssi_event_t &e, bool check_time_span) {

	switch (_state_filter) {
		case EVENT_STATE_FILTER::COMPLETED:
			if (e.state != SSI_ESTATE_COMPLETED) {
				return false;
			}
			break;
		case EVENT_STATE_FILTER::CONTINUED:
			if (e.state != SSI_ESTATE_CONTINUED) {
				return false;
			}
			break;
		case EVENT_STATE_FILTER::ZERODUR:
			if (e.dur != 0) {
				return false;
			}
			break;
		case EVENT_STATE_FILTER::NONZERODUR:
			if (e.dur == 0) {
				return false;
			}
			break;
	}

	if (check_time_span && _time_ref - e.time > _time_span) {
		return false;
	}

	bool sender_ok = false;
	bool event_ok = false;

	if (_n_sender_ids > 0) {
		for (ssi_size_t i = 0; i < _n_sender_ids; i++) {
			if (e.sender_id == _sender_ids[i]) {
				sender_ok = true;
				break;
			}
		}
	} else {
		sender_ok = true;
	}
	if (_n_event_ids > 0) {
		for (ssi_size_t i = 0; i < _n_event_ids; i++) {
			if (e.event_id == _event_ids[i]) {
				event_ok = true;
				break;
			}
		}
	} else {
		event_ok = true;
	}

	return sender_ok && event_ok;
}

ssi_event_t *IESelect::next () {

	ssi_event_t *e = 0;
	bool ok = false;	
	do {
		ok = false;
		e = _events.next ();
		if (e) {			
			ok = check (*e, _time_span > 0);		
		}
	} while (e && !ok);

	return e;
}

ssi_size_t IESelect::getSize () {

	_events.reset ();

	ssi_size_t size = 0;
	while (next ()) {
		size++;
	}

	return size;
}


}
