// EventQueue.cpp
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

#include "EventQueue.h"
#include "thread/Lock.h"

namespace ssi {

EventQueue::EventQueue (ssi_size_t n)
: _queue(n) {
}

EventQueue::~EventQueue () {
	
	clear ();
}

bool EventQueue::push (ssi_event_t &e) {

	bool result = false;

	{
		Lock lock (_mutex);
		if (!_queue.full ()) {
			ssi_event_t *e_copy = new ssi_event_t;
			ssi_event_init (*e_copy);
			ssi_event_clone (e, *e_copy);
			_queue.enqueue (e_copy);			
			result = true;
		} else {
			ssi_wrn ("event queue is full");
		}
	}

	return result;
}

ssi_size_t EventQueue::fetch (EventList &list) {
	
	ssi_size_t n = 0;

	{
		Lock lock (_mutex);

		ssi_event_t *e;
		while (_queue.dequeue (ssi_pcast (void*, &e))) {
			list.push (*e);
			ssi_event_destroy (*e);
			delete e;
			n++;
		}
	}

	return n;
};

void EventQueue::clear () {

	void *e;
	while (_queue.dequeue (&e)) {		
		ssi_event_t *ep = ssi_pcast(ssi_event_t, e);
		ssi_event_destroy(*ep);
		delete ep;		
	}
}

}
