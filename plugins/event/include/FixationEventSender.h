// FixationEventSender.h
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

#ifndef SSI_EVENT_FIXATIONEVENTSENDER_H
#define SSI_EVENT_FIXATIONEVENTSENDER_H

#include "base/IConsumer.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"

namespace ssi {

class FixationEventSender : public IConsumer {

public:

	class Options : public OptionList {

	public:

		Options () : disp_thres (0.05f), min_dur (0.25), max_dur (0.5) {

			setAddress("");
			setSender ("gaze");
			setEvent ("fixation");	

			addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender");
			addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event");
			addOption ("thres", &disp_thres, 1, SSI_REAL, "fixation threshold");
			addOption ("mindur", &min_dur, 1, SSI_TIME, "minimum duration");
			addOption ("maxdur", &max_dur, 1, SSI_TIME, "minimum duration");			

			SSI_OPTIONLIST_ADD_ADDRESS(address);
		};

		void setAddress(const ssi_char_t *address) {
			if (address) {
				ssi_strcpy(this->address, address);
			}
		}

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

		ssi_real_t disp_thres;
		ssi_time_t min_dur;		
		ssi_time_t max_dur;		
		ssi_char_t address[SSI_MAX_CHAR];
		ssi_char_t sname[SSI_MAX_CHAR];
		ssi_char_t ename[SSI_MAX_CHAR];	
	};

public:

	static const ssi_char_t *GetCreateName () { return "FixationEventSender"; };
	static IObject *Create (const ssi_char_t *file) { return new FixationEventSender (file); };
	~FixationEventSender ();
	FixationEventSender::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Looks for fixations and sends them as an event."; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	bool setEventListener (IEventListener *listener);
	const ssi_char_t *getEventAddress () {
		return _event_address.getAddress ();
	}

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	FixationEventSender (const ssi_char_t *file = 0);
	FixationEventSender::Options _options;
	ssi_char_t *_file;

	static char *ssi_log_name;
	int ssi_log_level;

	IEventListener *_elistener;
	ssi_event_t _event;
	EventAddress _event_address;

	bool _infix;
	ssi_time_t _start, _last;
	ssi_real_t _minx, _miny, _maxx, _maxy;

	bool isfix ();
	void reset (ssi_time_t start);
	void update (ssi_real_t x, ssi_real_t y);
};

}

#endif
