// IEvent.h
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

#ifndef SSI_EVENT_ITHEEVENTBOARD_H
#define SSI_EVENT_ITHEEVENTBOARD_H

#include "SSI_Cons.h"
#include "base/IObject.h"
#include "base/IEvents.h"

namespace ssi {

class ITheEventBoard : public IObject {

public:

	virtual ~ITheEventBoard () {};

	virtual void Start () = 0;
	virtual void Stop () = 0;
	virtual bool IsRunning () = 0;

	virtual bool RegisterSender(IObject &sender) = 0; // register sender sending one or more events to the board
	virtual bool RegisterListener(IObject &listener,
		const ssi_char_t *address = 0,
		ssi_size_t time_span_ms = 0,
		IEvents::EVENT_STATE_FILTER::List state_filter = IEvents::EVENT_STATE_FILTER::ALL) = 0; // registers listener to receive certain event(s) (separated by ,) fired by one or more certain sender (separated by ,)

	virtual void Clear () = 0; // reset board

	virtual void Print (FILE *file = ssiout) = 0;
	virtual void Print (IEvents &events, FILE *file = ssiout) = 0;

};

}

#endif
