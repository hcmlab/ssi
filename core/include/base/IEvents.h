// EventBase.h
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

#ifndef SSI_IEVENTS_H
#define SSI_IEVENTS_H

#include "SSI_Cons.h"

namespace ssi {

class IEvents {

public:

	struct EVENT_STATE_FILTER {
		enum List {
			ALL = 0,
			COMPLETED,
			CONTINUED,
			ZERODUR,
			NONZERODUR
		};
	};

	virtual ~IEvents () {};

	virtual void reset () = 0;
	virtual ssi_event_t *get (ssi_size_t index) = 0;
	virtual ssi_event_t *next () = 0; 	
	virtual ssi_size_t getSize () = 0;

};

class IEventListener {

public:

	virtual ~IEventListener () {};
	virtual void listen_enter () {};
	virtual bool update (ssi_event_t &e) { return false; };
	virtual bool update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) { return false; };
	virtual void listen_flush () {};
};	

class IEventSender {

public:

	virtual ~IEventSender () {};	
	virtual void send_enter () {};
	virtual bool setEventListener (IEventListener *listener) { return false; };
	virtual void send_flush () {};
	virtual const ssi_char_t *getEventAddress () { return 0; };

};


}

#endif
