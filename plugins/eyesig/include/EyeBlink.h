// EyeBlink.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/12/20
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

#ifndef SSI_EVENT_EYE_BLINK_H
#define SSI_EVENT_EYE_BLINK_H

#include "base/IConsumer.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"

namespace ssi {

class EyeBlink : public IConsumer {

public:

	class Options : public OptionList {

	public:

		Options () : mindur (0.15), maxdur(1.0), console(false) {
;
			setSender ("EyeTracking");
			setEvent ("Eyeblink");
			addOption ("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender (if sent to event board)");
			addOption ("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event (if sent to event board)");	
			addOption ("console", &console, 1, SSI_BOOL, "Send events to Console instead of Eventboard");
			addOption("mindur", &mindur, 1, SSI_DOUBLE, "minimum duration");
			addOption("maxdur", &maxdur, 1, SSI_DOUBLE, "maximum duration");
		};


		void setSender (const ssi_char_t *sname) {			
			if (sname) {
				ssi_strcpy (this->sname, sname);
			}
		}

		void setEvent (const ssi_char_t *ename) {
			if (ename) {
				ssi_strcpy (this->ename, ename);
			}
		}

		ssi_char_t sname[SSI_MAX_CHAR];
		ssi_char_t ename[SSI_MAX_CHAR];	
		double mindur;
		double maxdur;
		bool console;
	};

public:

	static const ssi_char_t *GetCreateName () { return "EyeBlink"; };
	static IObject *Create (const ssi_char_t *file) { return new EyeBlink (file); };
	~EyeBlink ();
	EyeBlink::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "detects eye blinks and sends them to a socket connection"; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	//event sender
	bool setEventListener (IEventListener *listener);
	const ssi_char_t *getEventAddress () {
		return _event_address.getAddress ();
	}

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	EyeBlink (const ssi_char_t *file = 0);
	EyeBlink::Options _options;
	ssi_char_t *_file;

	IEventListener *_elistener;
	ssi_event_t _event;
	EventAddress _event_address;

	ssi_time_t _start, _last;

	static char *ssi_log_name;
	int ssi_log_level;

	bool _inblink;

	bool isblink (int x, int y);
	void reset (double start);

	void blinkstart (double time);
	void blinkend (double time);

};

}

#endif
