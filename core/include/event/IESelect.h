// IESelect.h
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

#ifndef SSI_EVENT_IESelect_H
#define SSI_EVENT_IESelect_H

#include "base/IEvents.h"
#include "base/ITheEventBoard.h"

namespace ssi {

class IESelect : public IEvents {

public:

	IESelect (IEvents *events);
	virtual ~IESelect ();

	void set (ssi_size_t n_sender_ids, 
		ssi_size_t *sender_ids = 0, 
		ssi_size_t n_event_ids = 0, 
		ssi_size_t *event_ids = 0,
		ssi_size_t time_span_ms = 0,
		EVENT_STATE_FILTER::List state_filter = EVENT_STATE_FILTER::ALL);
	bool check (ssi_event_t &e, bool check_time_span);

	void reset () {
		_events.reset (); 
	}
	void setTime (ssi_size_t time_ms) {
		_time_ref = time_ms;
	}
	ssi_event_t *get (ssi_size_t index);
	ssi_event_t *next ();
	ssi_size_t getSize ();

protected:

	void release ();

	IEvents &_events;
	ssi_size_t _n_event_ids;
	ssi_size_t *_event_ids;
	ssi_size_t _n_sender_ids;
	ssi_size_t *_sender_ids;
	ssi_size_t _time_span;
	ssi_size_t _time_ref;
	EVENT_STATE_FILTER::List _state_filter;
};

}

#endif
