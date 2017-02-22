// PraatScriptBase.h
// author: Andreas Seiderer
// created: 2013/09/16
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

#pragma once

#ifndef SSI_CONSUMER_PRAATSCRIPT_H
#define SSI_CONSUMER_PRAATSCRIPT_H

#include "base/IConsumer.h"
#include "PraatScriptIParser.h"
#include "PraatScriptOptions.h"
#include "event/EventAddress.h"

namespace ssi {

class PraatScriptBase : public IConsumer {

public:

	~PraatScriptBase ();
	PraatScriptOptions *getOptions () { return &_options; };

	//Consumer
	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	// abstract methods
	virtual PraatScriptIParser *getParser () = 0;

	//event sender
	bool setEventListener (IEventListener *listener);
	const ssi_char_t *getEventAddress () {
		return _event_address.getAddress ();
	}

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	PraatScriptBase (const ssi_char_t *file = 0);
	bool handleEvent (bool condition, ssi_time_t min_dur, ssi_time_t time, const char* ev_value, ssi_estate_t ev_state);

	PraatScriptOptions _options;
	ssi_char_t *_file;

	static char *ssi_log_name;
	int ssi_log_level;

	ssi_size_t _n_values;
	IEventListener *_elistener;
	EventAddress _event_address;
	ssi_event_t _event;

	PraatScriptIParser *_parser;
	bool _ready;

};

}

#endif
