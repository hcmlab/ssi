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

#pragma once

#ifndef SSI_EVENTADDRESS_H
#define SSI_EVENTADDRESS_H

#include "SSI_Cons.h"

namespace ssi {

class EventAddress {

public:
	
	EventAddress ();
	virtual ~EventAddress ();

	void clear ();

	void setAddress (const ssi_char_t *address);
	void setSender (const ssi_char_t *names);	
	void setEvents (const ssi_char_t *names);

	ssi_size_t getSenderSize () { return _n_sender; };
	ssi_size_t getEventsSize () { return _n_events; };

	const ssi_char_t *getAddress ();
	const ssi_char_t *getSender (ssi_size_t i = 0);
	const ssi_char_t *getEvent (ssi_size_t i = 0);	

	void print (FILE *file = stdout) {		
		ssi_fprint (file, "%s\n", getAddress ());		
	}

protected:

	void parseNames (const ssi_char_t *names, ssi_size_t &n, ssi_char_t ***target);
	void parseAddress (const ssi_char_t *address);	

	ssi_size_t _n_sender;
	ssi_size_t _n_events;
	
	ssi_char_t **_sender;
	ssi_char_t **_events;

	ssi_char_t *_address;

};

}

#endif
