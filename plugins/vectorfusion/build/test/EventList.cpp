// EventList.cpp
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

#include "event/EventList.h"

namespace ssi {

EventList::EventList (ssi_size_t n_events)
: _n_events (n_events),
	_events_count (0),
	_next_count (0),
	_head_pos (0),
	_next_pos (0),
	_next (0) {

	_events = new ssi_event_t[_n_events];
	for (ssi_size_t i = 0; i < _n_events; i++) {
		ssi_event_init (_events[i]);
	}	

	reset ();
}

EventList::~EventList () {

	clear ();
	delete[] _events;
}

void EventList::reset () {
	
	_next_pos = _head_pos == 0 ? _n_events - 1 : _head_pos - 1;
	_next = _events + _next_pos;
	_next_count = _events_count;
}

ssi_event_t *EventList::get (ssi_size_t index) {

	if (index >= _n_events) {
		ssi_wrn ("index '%u' exceeds #events '%u'", index, _n_events);
		return 0;
	}

	if (index <= _head_pos) {		
		return _events  + (_head_pos - index);
	} else {
		return _events + (_n_events - (index - _head_pos));
	}
}

void EventList::push (ssi_event_t &e) {

	ssi_event_copy (e, _events[_head_pos]);	
	if (_events_count < _n_events) {
		++_events_count;
	}
	++_head_pos %= _n_events;
}

ssi_event_t *EventList::next () {

	if (_next_count-- == 0) {
		return 0;
	}

	ssi_event_t *next = _next;

	if (_next_pos == 0) {
		_next_pos = _n_events - 1;
		_next = _events + _next_pos;
	} else {
		--_next_pos;
		--_next;
	}

	return next;
}

ssi_size_t EventList::getSize () {

	return _events_count;
}

void EventList::clear () {
	for (ssi_size_t i = 0; i < _n_events; i++) {
		ssi_event_destroy (_events[i]);
	}	
	_head_pos = 0;
	_events_count = 0;
	reset ();
}

}
