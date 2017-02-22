// EventAddress.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2012/04/23
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

#include "event/EventAddress.h"

namespace ssi {

EventAddress::EventAddress () 
	: _n_sender (0),
	_sender (0),
	_n_events (0),
	_events (0),
	_address (0) {
}

EventAddress::~EventAddress () {

	clear ();
}

void EventAddress::setAddress (const ssi_char_t *address) {
	parseAddress (address);
}

void EventAddress::setSender (const ssi_char_t *names) {
	parseNames (names, _n_sender, &_sender);
}

void EventAddress::setEvents (const ssi_char_t *names) {
	parseNames (names, _n_events, &_events);
}

const ssi_char_t *EventAddress::getAddress () {

	delete[] _address; _address = 0;

	size_t n = 1 + (_n_sender > 0 ? _n_sender - 1 : 0 ) + (_n_events > 0 ? _n_events - 1 : 0 );
	for (ssi_size_t i = 0; i < _n_sender; i++) {
		n += strlen (_sender[i]);
	}
	for (ssi_size_t i = 0; i < _n_events; i++) {
		n += strlen (_events[i]);
	}

	_address = new ssi_char_t[n+1];
	_address[n] = '\0';
	ssi_char_t *ptr = _address;

	for (ssi_size_t i = 0; i < _n_events; i++) {
		memcpy (ptr, _events[i], strlen (_events[i]));
		ptr += strlen (_events[i]);
		if (i != _n_events-1) {
			*ptr++ = ',';
		}
	}
	*ptr++ = '@';
	for (ssi_size_t i = 0; i < _n_sender; i++) {
		memcpy (ptr, _sender[i], strlen (_sender[i]));
		ptr += strlen (_sender[i]);
		if (i != _n_sender-1) {
			*ptr++ = ',';
		}
	}

	return _address;
}

const ssi_char_t *EventAddress::getSender (ssi_size_t i) {

	if (i >= _n_sender) {
		ssi_wrn ("requested index '%u' exceeds #sender '%u'", i, _n_sender);
		return 0;
	}

	return _sender[i];
}

const ssi_char_t *EventAddress::getEvent (ssi_size_t i) {

	if (i >= _n_events) {
		ssi_wrn ("requested index '%u' exceeds #events '%u'", i, _n_events);
		return 0;
	}

	return _events[i];
}

void EventAddress::parseNames (const ssi_char_t *names, ssi_size_t &n, ssi_char_t ***target) {

	ssi_size_t n_new = 0;
	if (names && names[0] != '\0') {
		n_new = ssi_split_string_count (names, ',');
	}

	if (n_new == 0) {
		return;
	}
	
	ssi_char_t **target_new = new ssi_char_t *[n + n_new];
	for (ssi_size_t i = 0; i < n; i++) {
		target_new[i] = ssi_strcpy ((*target)[i]);
		delete[] (*target)[i];
	}
	delete[] (*target);

	ssi_split_string (n_new, target_new + n, names, ',');

	*target = target_new;
	n += n_new;
}

void EventAddress::parseAddress (const ssi_char_t *address) {

	if (address == 0 || address[0] == '\0') {
		return;
	}

	size_t len = strlen (address);

	ssi_char_t *chunks[2];

	if (address[0] == '@') {
		chunks[0] = 0;
		chunks[1] = ssi_strcpy (address + 1);
	} else if (address[len-1] == '@') {
		chunks[0] = ssi_strcpy (address);		
		chunks[0][len-1] = '\0';
		chunks[1] = 0;
	} else {
		ssi_size_t n_chunks = ssi_split_string_count (address, '@');
		if (n_chunks != 2) {
			ssi_wrn ("malformed event address '%s'", address);
			return;
		}
		ssi_split_string (2, chunks, address, '@');		
	}

	parseNames (chunks[0], _n_events, &_events);	
	parseNames (chunks[1], _n_sender, &_sender);
	
	delete[] chunks[0];
	delete[] chunks[1];
}

void EventAddress::clear () {

	for (ssi_size_t i = 0; i < _n_sender; i++) {
		delete[] _sender[i]; _sender[i] = 0;
	}
	delete[] _sender;

	for (ssi_size_t i = 0; i < _n_events; i++) {
		delete[] _events[i]; _events[i] = 0;
	}
	delete[] _events;

	delete[] _address; _address = 0;

	_n_sender = 0;
	_sender = 0;
	_n_events = 0;
	_events = 0;
}

}
