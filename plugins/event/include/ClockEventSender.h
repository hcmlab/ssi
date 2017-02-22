// ClockEventSender.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/02/01
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

#ifndef SSI_EVENT_CLOCKEVENTSENDER_H
#define SSI_EVENT_CLOCKEVENTSENDER_H

#include "base/IObject.h"
#include "base/ITheFramework.h"
#include "thread/Thread.h"
#include "thread/Timer.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"

namespace ssi {

class ClockEventSender : public IObject, public Thread {

public:

	class Options : public OptionList {

	public:

		Options () : clock (true), empty(true), init(false), silence(false) {

			setSenderName ("clock");
			setEventName ("tick");	
			clocks[0] = '\0';
			string[0] = '\0';

			SSI_OPTIONLIST_ADD_ADDRESS(address);
			
			addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender (if sent to event board) [deprecated, see address]");
			addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event (if sent to event board) [deprecated, see address]");
			addOption("clock", &clock, 1, SSI_SIZE, "clock interval in milliseconds");
			addOption("clocks", &clocks, SSI_MAX_CHAR, SSI_CHAR, "a set of clock intervals in milliseconds (separted by ;)");
			addOption("init", &init, 1, SSI_BOOL, "send an event in the beginning");
			addOption("empty", &empty, 1, SSI_BOOL, "send as empty event");
			addOption("string", string, SSI_MAX_CHAR, SSI_CHAR, "default string (if empty == false)");
			addOption("silence", &silence, 1, SSI_BOOL, "do not send events");
		};

		void setAddress(const ssi_char_t *address) {
			if (address) {
				ssi_strcpy(this->address, address);
			}
		}

		void setSenderName (const ssi_char_t *sname) {			
			if (sname) {
				ssi_strcpy (this->sname, sname);
			}
		}
		void setEventName (const ssi_char_t *ename) {
			if (ename) {
				ssi_strcpy (this->ename, ename);
			}
		}
		void setClocks(ssi_size_t n, ssi_size_t *nclocks) {
			ssi_array2string(n, nclocks, SSI_MAX_CHAR, clocks, ';');
		}
		void setString(const ssi_char_t *label) {
			if (label) {
				ssi_strcpy(this->string, label);
			}
		}

		ssi_char_t address[SSI_MAX_CHAR];
		ssi_char_t sname[SSI_MAX_CHAR];
		ssi_char_t ename[SSI_MAX_CHAR];		
		ssi_size_t clock;
		ssi_char_t clocks[SSI_MAX_CHAR];
		bool empty;
        bool silence;
		bool init;
		ssi_char_t string[SSI_MAX_CHAR];
	};

public: 	

	static const ssi_char_t *GetCreateName () { return "ClockEventSender"; };
	static IObject *Create (const ssi_char_t *file) { return new ClockEventSender (file); };
	~ClockEventSender ();

	ClockEventSender::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Sends an event in regular intervals."; };

	bool setEventListener (IEventListener *listener);
	const ssi_char_t *getEventAddress () {
		return _event_address.getAddress ();
	}

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

	void enter();
	void run();
	void flush();

protected:

	ClockEventSender (const ssi_char_t *file = 0);
	ClockEventSender::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	void initClocks();

	IEventListener *_listener;
	EventAddress _event_address;
	ssi_event_t _event;
	
	Timer _timer;
	ssi_size_t _n_clocks;
	ssi_size_t *_clocks;
	ssi_size_t _next;
	bool _init;

	ITheFramework *_frame;
};

}

#endif
