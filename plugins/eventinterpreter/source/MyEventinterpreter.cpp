// MyEventinterpreter.cpp
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

#include "MyEventinterpreter.h"
#include <base/Factory.h>

namespace ssi {

	char MyEventinterpreter::ssi_log_name[] = "eventinter";

	MyEventinterpreter::MyEventinterpreter(const ssi_char_t *file)
		: _file(0), _listener(0), L(0) {

		if (file) {
			if (!OptionList::LoadXML(file, _options)) {
				OptionList::SaveXML(file, _options);
			}
			_file = ssi_strcpy(file);
		}


		ssi_event_init(_event, SSI_ETYPE_STRING);

		_frame = Factory::GetFramework();
	}

	std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
		std::stringstream ss(s);
		std::string item;
		while (std::getline(ss, item, delim)) {
			elems.push_back(item);
		}
		return elems;
	}


	std::vector<std::string> split(const std::string &s, char delim) {
		std::vector<std::string> elems;
		split(s, delim, elems);
		return elems;
	}



	bool MyEventinterpreter::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {

		if (L) {

			for (unsigned int i = 0; i < n_new_events; i++) {
				ssi_event_t *e = events.next();

				//don't handle own events
				if (strcmp(Factory::GetString(e->sender_id), _options.sname) == 0)
					continue;

				std::string data = "";

				if (e->type == SSI_ETYPE_STRING)
				{
					data = std::string(e->ptr);

				}
				else if (e->type == SSI_ETYPE_TUPLE) {
					std::stringstream strf;

					if (e->tot / sizeof(ssi_real_t) > 1) {
						//found several values
						strf << "[";

						float *bufPos = (float*)e->ptr;

						for (int i = 0; i < e->tot / sizeof(ssi_real_t); i++) {
							strf << *(bufPos++);
							if (i != (e->tot / sizeof(ssi_real_t)) - 1)
								strf << ',';
						}


						strf << "]";
					}
					else {
						//found just one float value
						float *bufPos = (float*)e->ptr;
						strf << *bufPos;
					}

					data = strf.str();

					strf.clear();
				}

				{

					/* push functions and arguments */
					lua_getglobal(L, "interpret");  /* function to be called */

					if (!lua_isfunction(L, -1))
					{
						lua_pop(L, 1);
						printf("lua: error not a function!\n");
					}
					else {


						lua_pushstring(L, data.c_str());
						lua_pushstring(L, Factory::GetString(e->sender_id));
						lua_pushstring(L, Factory::GetString(e->event_id));

						std::stringstream eventState;
						eventState << e->state;
						lua_pushstring(L, eventState.str().c_str());
						eventState.clear();


						//ISODATE
						std::string isoDateStr = "";

#if defined( _WIN32 )
						//http://www.technical-recipes.com/2014/converting-a-systemtime-to-a-stdstring-in-c/
						SYSTEMTIME st;
						GetSystemTime(&st);

						std::ostringstream ossMessage;

						ossMessage << st.wYear << "-"
							<< std::setw(2) << std::setfill('0') << st.wMonth << "-"
							<< std::setw(2) << std::setfill('0') << st.wDay << "T"
							<< std::setw(2) << std::setfill('0') << st.wHour << ":"
							<< std::setw(2) << std::setfill('0') << st.wMinute << ":"
							<< std::setw(2) << std::setfill('0') << st.wSecond << "."
							<< std::setw(3) << std::setfill('0') << st.wMilliseconds << "Z";

						isoDateStr = ossMessage.str();

#else
						//TODO add linux code


#endif

						lua_pushstring(L, isoDateStr.c_str());

						//4 arguments
						if (lua_pcall(L, 5, 1, 0) != 0) {
							printf("lua: error running function: %s\n", lua_tostring(L, -1));

						}
						else {
							const char* result = lua_tostring(L, -1);
							lua_pop(L, 1);  /* pop returned value */

											//std::cout << result;

											//lua should return data!
							if (result[0] != 0) {

								int len = strlen(result);

								ssi_event_adjust(_event, len + 1);
								memcpy(_event.ptr, result, len + 1);

								_event.time = e->time;	//_frame->GetElapsedTimeMs()
								_event.dur = e->dur;
								_event.state = e->state;

								_listener->update(_event);
							}
						}
					}

				}
			}

		}


		return true;
	}



	void MyEventinterpreter::listen_enter() {
		if (!L) {
			L = luaL_newstate();
			luaL_openlibs(L);


			const char* luafile = _options.luascript;

			int s = luaL_loadfile(L, luafile);
			lua_pcall(L, 0, 0, 0);

			if (s != 0) {
				ssi_err("could not load Lua script: %s", luafile);
			}
		}

	}

	void  MyEventinterpreter::listen_flush() {

		if (L) {
			lua_close(L);
			L = nullptr;
		}

	}


	bool  MyEventinterpreter::setEventListener(IEventListener *listener) {

		_listener = listener;
		_event.sender_id = Factory::AddString(_options.sname);
		if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}
		_event.event_id = Factory::AddString(_options.ename);
		if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}

		_event_address.setSender(_options.sname);
		_event_address.setEvents(_options.ename);

		return true;
	}


	MyEventinterpreter::~MyEventinterpreter() {

		if (_file) {
			OptionList::SaveXML(_file, _options);
			delete[] _file;
		}
	}



}
