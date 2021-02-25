// pythonbridgeexit.h
// author: Florian Lingenfelser <lingenfelser@hcm-lab.de>
// created: 2020/09/08
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

#ifndef PYTHONBRIDGEEXIT_H
#define PYTHONBRIDGEEXIT_H
#endif

#ifndef SSI_USE_SDL

#include "SSI_Define.h"

#include "base/IObject.h"
#include "thread/Thread.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"
#include "ioput/socket/Socket.h"

namespace ssi {

	class PythonBridgeExit : public IObject, public Thread {

	public:

		class Options : public OptionList {

		public:

			Options() {

				setAddress("");
				setSenderName("sender");
				setEventName("event");

				addOption("address", address, SSI_MAX_CHAR, SSI_CHAR, "event address (if sent to event board) (event@sender)");
				addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender");
				addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event");

				port = 1234;
				setHost("localhost");
				addOption("host", host, SSI_MAX_CHAR, SSI_CHAR, "host name (empty for any) [deprecated use 'url']");
				addOption("port", &port, 1, SSI_INT, "port number (-1 for any) [deprecated use 'url']");
			};

			void setAddress(const ssi_char_t *address) {
				if (address) {
					ssi_strcpy(this->address, address);
				}
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
			void setHost(const ssi_char_t* host) {
				this->host[0] = '\0';
				if (host) {
					ssi_strcpy(this->host, host);
				}
			}

			ssi_char_t address[SSI_MAX_CHAR];
			ssi_char_t sname[SSI_MAX_CHAR];
			ssi_char_t ename[SSI_MAX_CHAR];
			int port;
			ssi_char_t host[SSI_MAX_CHAR];
		};

	public:

		static const ssi_char_t *GetCreateName() { return "PythonBridgeExit"; };
		static IObject *Create(const ssi_char_t *file) { return new PythonBridgeExit(file); };
		~PythonBridgeExit();

		static ssi_char_t *ssi_log_name;
		int ssi_log_level;

		Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "..."; };

		bool setEventListener(IEventListener *listener);
		const ssi_char_t *getEventAddress() {
			return _event_address.getAddress();
		}

		void enter();
		void run();
		void flush();

	protected:

		PythonBridgeExit(const ssi_char_t *file = 0);
		PythonBridgeExit::Options _options;
		ssi_char_t *_file;

		EventAddress _event_address;
		IEventListener *_listener;
		ssi_event_t _event;

		std::string _msg_start_sequence = "&&&";
		std::string _msg_end_sequence = "###";

		Socket* _socket;

	};

}

#endif