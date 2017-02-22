// EventMonitor.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/10/14
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

#ifndef SSI_EVENT_EVENTMONITOR_H
#define SSI_EVENT_EVENTMONITOR_H

#include "base/IObject.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class Monitor;
class Window;

class EventMonitor : public IObject {

public:

	class Options : public OptionList {

	public:

		Options()
			: all(true), console(false), detail(true), chars(10000), update_ms(0), relative(false) {

			screen[0] = 0;
			screen[1] = 0;

			setPos(0, 0, 100, 100);
			setTitle ("EventBoard");

			addOption("title", title, SSI_MAX_CHAR, SSI_CHAR, "window caption");
			addOption("pos", &pos, 4, SSI_INT, "window position (top, left, width, height) [deprecated use title]");			
			addOption("all", &all, 1, SSI_BOOL, "output all events, otherwise only new events will be plotted");
			addOption("console", &console, 1, SSI_BOOL, "output on console instead of window");
			addOption("chars", &chars, 1, SSI_SIZE, "maximum number of chars displayed");
			addOption("detail", &detail, 1, SSI_BOOL, "output detailed event content");
			addOption("update", &update_ms, 1, SSI_SIZE, "minimum update rate in ms");

		};

		void setPos(int top, int left, int width, int height) {
			pos[0] = top;
			pos[1] = left;
			pos[2] = width;
			pos[3] = height;
		}

		void setTitle (const ssi_char_t *name) {
			ssi_strcpy(title, name);
		}

		bool all;
		bool console;
		int pos[4];
		ssi_real_t screen[2];
		ssi_char_t title[SSI_MAX_CHAR];
		bool relative;
		bool detail;
		ssi_size_t chars;
		ssi_size_t update_ms;
	};

public: 	

	static const ssi_char_t *GetCreateName () { return "EventMonitor"; };
	static IObject *Create (const ssi_char_t *file) { return new EventMonitor (file); };
	~EventMonitor ();

	EventMonitor::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Generic visualization component to monitor events."; };

	void listen_enter ();
	bool update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
	void listen_flush ();

	bool notify(INotify::COMMAND::List command, const ssi_char_t *message);

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	EventMonitor (const ssi_char_t *file = 0);
	EventMonitor::Options _options;
	ssi_char_t *_file;

	ssi_size_t _update_counter;

	Window *_window;
	Monitor *_monitor;
	ssi_char_t _string[SSI_MAX_CHAR];

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;
};

}

#endif

