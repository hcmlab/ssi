// EventList.h
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

#pragma once

#ifndef SSI_EVENT_EVENTLIST_H
#define SSI_EVENT_EVENTLIST_H

#include "base/IEvents.h"

namespace ssi {

class EventList : public IEvents {

public:

	EventList (ssi_size_t n_events);
	virtual ~EventList ();

	void reset ();
	ssi_event_t *get (ssi_size_t index);
	ssi_event_t *next (); // returns next element and moves iterator one back
	ssi_size_t getSize ();

	virtual void push (ssi_event_t &e);
	virtual void clear ();	

protected:

	ssi_event_t *_events;
	ssi_size_t _n_events;
	ssi_size_t _events_count;
	ssi_size_t _next_count;
	ssi_size_t _head_pos;
	ssi_event_t *_next;
	ssi_size_t _next_pos;
};

}

#endif
