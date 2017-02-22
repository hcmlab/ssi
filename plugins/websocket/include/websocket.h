// websocket.h
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 06/08/2015
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

#ifndef SSI_WEBSOCKET_H
#define SSI_WEBSOCKET_H

#include "websockserver.h"

#include "base/IConsumer.h"
#include "ioput/option/OptionList.h"
#include "base/Factory.h"

#include "event/EventAddress.h"


#include "../../../libs/build/rapidjson/writer.h"
#include "../../../libs/build/rapidjson/stringbuffer.h"


#include <iostream>
#include <sstream>

#include <string>
#include <fstream>
#include <streambuf>
#include <iomanip>


namespace ssi {

	class Websocket : public IConsumer {

	public:

		class Options : public OptionList {

		public:

			Options() : send_info(false), send_own_events(false) {
				setSenderName("websocket");
				setEventName("message");
				http_port = 8000;
				setHttpRoot(".");

				//sender options
				addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender for events received by incoming messages");
				addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event for events received by incoming messages");
			
				addOption("http_port", &http_port, 1, SSI_UINT, "port of http server");
				addOption("http_root", http_root, SSI_MAX_CHAR, SSI_CHAR, "root path of http server");

				addOption("send_info", &send_info, 1, SSI_BOOL, "send JSON info about streams as first websocket message to browser");
				addOption("send_own_events", &send_own_events, 1, SSI_BOOL, "send own events to all clients?");
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

			void setHttpRoot(const ssi_char_t *root) {
				if (http_root) {
					ssi_strcpy(this->http_root, root);
				}
			}


			ssi_char_t sname[SSI_MAX_CHAR];
			ssi_char_t ename[SSI_MAX_CHAR];

			ssi_size_t http_port;
			ssi_char_t http_root[SSI_MAX_CHAR];

			bool send_info;
			bool send_own_events;

		};

	public:

		static const ssi_char_t *GetCreateName() { return "websocket"; };
		static IObject *Create(const ssi_char_t *file) { return new Websocket(file); };
		~Websocket();

		Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "sends SSI events as websocket messages in JSON format"; };

		bool setEventListener(IEventListener *listener);
		
		const ssi_char_t *getEventAddress() {
			return _event_address.getAddress();
		}

		//consumer
		void consume_enter(ssi_size_t stream_in_num,
			ssi_stream_t stream_in[]);
		void consume(IConsumer::info consume_info,
			ssi_size_t stream_in_num,
			ssi_stream_t stream_in[]);
		void consume_flush(ssi_size_t stream_in_num,
			ssi_stream_t stream_in[]);


		//event listener
		void listen_enter();
		bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
		void listen_flush();

		void sendSSIEvent(char* msg, int len, char* ip, int ip_len, int id);

	protected:

		Websocket(const ssi_char_t *file = 0);
		ssi_char_t *_file;
		Options _options;
		static char ssi_log_name[];



		//event sender
		IEventListener *_listener;
		EventAddress _event_address;
		ssi_event_t _event;


		ITheFramework *_frame;

		Websockserver *server;
	};

}

#endif
