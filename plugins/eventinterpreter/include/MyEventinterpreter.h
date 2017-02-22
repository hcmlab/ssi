// MyEventinterpreter.h
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 4/11/2015
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_EVENTINTERPRETER_MYEVENTINTERPRETER_H
#define SSI_EVENTINTERPRETER_MYEVENTINTERPRETER_H


#include "base/IObject.h"
#include "ioput/option/OptionList.h"
#include <event/EventAddress.h>
#include <base/ITheFramework.h>


extern "C" {
#include "../build/libs/lua/lua.h"
#include "../build/libs/lua/lualib.h"
#include "../build/libs/lua/lauxlib.h"
}

#include <iostream>
#include <sstream>

#include <string>
#include <fstream>
#include <streambuf>
#include <iomanip>

namespace ssi {

	class MyEventinterpreter : public IObject {

	public:

		class Options : public OptionList {

		public:

			Options() {
				setSenderName("eventinterpreter");
				setEventName("interpreted");
				setLuaScriptName("base.lua");

				//sender options
				addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender for events received by incoming messages");
				addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event for events received by incoming messages");
				addOption("luascript", luascript, SSI_MAX_CHAR, SSI_CHAR, "name of main lua script");
		}


		void setSenderName(const ssi_char_t *sname) {
			if (sname) {
				ssi_strcpy(this->sname, sname);
			}
		}

		void setEventName(const ssi_char_t *ename) {
			if (ename) {
				ssi_strcpy(this->ename, ename);
			}
		}

		void setLuaScriptName(const ssi_char_t *luascript) {
			if (luascript) {
				ssi_strcpy(this->luascript, luascript);
			}
		}


		ssi_char_t sname[SSI_MAX_CHAR];
		ssi_char_t ename[SSI_MAX_CHAR];
		ssi_char_t luascript[SSI_MAX_CHAR];
		};

	public:

		static const ssi_char_t *GetCreateName() { return "Eventinterpreter"; };
		static IObject *Create(const ssi_char_t *file) { return new MyEventinterpreter(file); };
		~MyEventinterpreter();

		Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "lua based interpreter for events"; };

		bool setEventListener(IEventListener *listener);

		const ssi_char_t *getEventAddress() {
			return _event_address.getAddress();
		}

		//event listener
		void listen_enter();
		bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
		void listen_flush();

	protected:

		MyEventinterpreter(const ssi_char_t *file = 0);
		ssi_char_t *_file;
		Options _options;
		static char ssi_log_name[];


		//event sender
		IEventListener *_listener;
		EventAddress _event_address;
		ssi_event_t _event;


		ITheFramework *_frame;

		lua_State *L;
	};

}

#endif
