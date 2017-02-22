// DecisionSmoother.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/06/24
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

#ifndef SSI_MODEL_DECISIONSMOOTHER_H
#define SSI_MODEL_DECISIONSMOOTHER_H

#include "base/IObject.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"
#include "thread/Lock.h"

namespace ssi {

class ITheFramework;
class RunAsThread;
class Timer;

class DecisionSmoother : public IObject {

	class Options : public OptionList {

	public:

		Options ()
			: update_ms(0), decay(0.0f), speed(0.0f), average(false), window(0.0) {

			setAddress("");
			setSenderName ("decision");
			setEventName ("smoothed");	

			SSI_OPTIONLIST_ADD_ADDRESS(address);
			
			addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender (if sent to event board) [deprecated, see address]");
			addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event (if sent to event board) [deprecated, see address]");
			addOption("update", &update_ms, 1, SSI_SIZE, "update rate in ms (if 0 an event is only sent after a new event was received)");
			addOption("decay", &decay, 1, SSI_REAL, "factor by which the target decision will be decreased if no new decision arrives", false);
			addOption("speed", &speed, 1, SSI_REAL, "factor by which the smoothed decision will move towards the target decision (if <= 0 it will be immediatelly reached)", false);
			addOption("average", &average, 1, SSI_BOOL, "target decision is replaced with cumulative moving average of previous decisions", false);
			addOption("window", &window, 1, SSI_TIME, "window size in seconds over which previous decisions will be considered (0 -> infinite)", false);
			
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

		ssi_size_t update_ms;
		ssi_char_t address[SSI_MAX_CHAR];
		ssi_char_t sname[SSI_MAX_CHAR];
		ssi_char_t ename[SSI_MAX_CHAR];
		ssi_real_t decay;
		ssi_real_t speed;
		bool average;
		ssi_time_t window;
	};

public:

	static const ssi_char_t *GetCreateName () { return "DecisionSmoother"; };
	static IObject *Create (const ssi_char_t *file) { return new DecisionSmoother (file); };
	~DecisionSmoother ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Returns smoothed decisions at a regular update rate"; };

	void listen_enter ();
	bool update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
	void listen_flush ();

	bool notify(INotify::COMMAND::List command, const ssi_char_t *message);

	bool setEventListener (IEventListener *listener);
	const ssi_char_t *getEventAddress () {
		return _event_address.getAddress ();
	}

protected:

	DecisionSmoother (const ssi_char_t *file = 0);
	Options _options;
	static char ssi_log_name[];
	ssi_char_t *_file;
	int ssi_log_level;

	EventAddress _event_address;
	IEventListener *_listener;
	ssi_event_t _event;
	ssi_size_t _dim;
	ssi_real_t *_target;
	ssi_real_t *_decision;
	ssi_real_t _decay;
	ssi_real_t _speed;
	bool _average;
	ssi_size_t _last_decision_time;
	ssi_time_t _window;
	ssi_size_t _counter;

	void init(ssi_event_t e);
	static void send(void *ptr);	
	void readOptions();

	ITheFramework *_frame;
	RunAsThread *_thread;
	Timer *_timer;
	Mutex _mutex;
};

}

#endif
