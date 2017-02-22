// XMPP.h
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 13/3/2015
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

#ifndef SSI_XMPP_XMPP_H
#define SSI_XMPP_XMPP_H

#include "base/IObject.h"
#include "ioput/option/OptionList.h"
#include "base/Factory.h"

#include "event/EventAddress.h"


#include "../../../libs/build/rapidjson/writer.h"
#include "../../../libs/build/rapidjson/stringbuffer.h"

#include "XMPPclient.h"

#include <iostream>
#include <sstream>

#include <string>
#include <fstream>
#include <streambuf>
#include <iomanip>

namespace ssi {

	class XMPP : public IObject {

	public:

		enum MessageBodyFormat {
			MESSAGEBODYFORMAT_CSV,
			MESSAGEBODYFORMAT_JSON,
			MESSAGEBODYFORMAT_TEMPLATE,

		};

		class Options : public OptionList {

		public:

			Options() : msgFormat(0), csvChr(';'), pubNode(false), subNode(false), useSaslMechPlain(false) {

				jid[0] = pw[0] = recip[0] = pubNodeName[0] = pubNodeService[0] = sname[0] = ename[0] = sname_sub[0] = ename_sub[0] = subNodeName[0] = subNodeService[0] ='\0';
				setTemplatesDir("msg_templates");

				setSenderName("XMPP");
				setSenderNodeEventsName("XMPP");

				setEventName("message");
				setEventNodeEventsName("node_event");

				addOption("JID", jid, SSI_MAX_CHAR, SSI_CHAR, "JID of user account");
				addOption("password", pw, SSI_MAX_CHAR, SSI_CHAR, "password of user account");

				addOption("useSaslMechPlain", &useSaslMechPlain, 1, SSI_BOOL, "use SaslMechPlain to connect to server (see gloox documentation)");

				addOption("recipient", recip, SSI_MAX_CHAR, SSI_CHAR, "JID of recipient of messages");

				addOption("msgFormat", &msgFormat, 1, SSI_INT, "message format (0: csv, 1: json, 2: template)");
				addOption("csvChr", &csvChr, 1, SSI_CHAR, "split char for csv message format");

				addOption("templatesDir", &templatesDir, SSI_MAX_CHAR, SSI_CHAR, "path to templates used if \"msgFormat\" set to \"template\"");

				addOption("publishToNode", &pubNode, 1, SSI_BOOL, "use publish node instead of recipient");
				addOption("pubNodeName", pubNodeName, SSI_MAX_CHAR, SSI_CHAR, "name of publish node");
				addOption("pubNodeService", pubNodeService, SSI_MAX_CHAR, SSI_CHAR, "name of service for publish node (e.g. pubsub.servername)");

				//sender options
				addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender for events received by incoming messages");
				addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event for events received by incoming messages");

				addOption("sname_sub", sname_sub, SSI_MAX_CHAR, SSI_CHAR, "name of sender for events received by incoming node events");
				addOption("ename_sub", ename_sub, SSI_MAX_CHAR, SSI_CHAR, "name of event for events received by incoming node events");

				addOption("subscribeToNode", &subNode, 1, SSI_BOOL, "subscribe to node");
				addOption("subNodeName", subNodeName, SSI_MAX_CHAR, SSI_CHAR, "name of subscription node");
				addOption("subNodeService", subNodeService, SSI_MAX_CHAR, SSI_CHAR, "name of service for subscription node (e.g. pubsub.servername)");

				//TODO
				//test float tuples -> xmpp

				//get data from subscribed node
			}

			void setJID(const ssi_char_t *string) {
				this->jid[0] = '\0';
				if (string) {
					ssi_strcpy(this->jid, string);
				}
			}

			void setPw(const ssi_char_t *string) {
				this->pw[0] = '\0';
				if (string) {
					ssi_strcpy(this->pw, string);
				}
			}

			void setRecip(const ssi_char_t *string) {
				this->recip[0] = '\0';
				if (string) {
					ssi_strcpy(this->recip, string);
				}
			}

			void setTemplatesDir(const ssi_char_t *string) {
				this->templatesDir[0] = '\0';
				if (string) {
					ssi_strcpy(this->templatesDir, string);
				}
			}

			void setPubNodeName(const ssi_char_t *string) {
				this->pubNodeName[0] = '\0';
				if (string) {
					ssi_strcpy(this->pubNodeName, string);
				}
			}

			void setPubNodeService(const ssi_char_t *string) {
				this->pubNodeService[0] = '\0';
				if (string) {
					ssi_strcpy(this->pubNodeService, string);
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

			void setSenderNodeEventsName(const ssi_char_t *sname_sub) {
				if (sname_sub) {
					ssi_strcpy(this->sname_sub, sname_sub);
				}
			}

			void setEventNodeEventsName(const ssi_char_t *ename_sub) {
				if (ename_sub) {
					ssi_strcpy(this->ename_sub, ename_sub);
				}
			}

			void setSubNodeName(const ssi_char_t *string) {
				this->subNodeName[0] = '\0';
				if (string) {
					ssi_strcpy(this->subNodeName, string);
				}
			}

			void setSubNodeService(const ssi_char_t *string) {
				this->subNodeService[0] = '\0';
				if (string) {
					ssi_strcpy(this->subNodeService, string);
				}
			}

			ssi_char_t jid[SSI_MAX_CHAR];
			ssi_char_t pw[SSI_MAX_CHAR];
			ssi_char_t recip[SSI_MAX_CHAR];
			ssi_char_t csvChr;

			ssi_char_t templatesDir[SSI_MAX_CHAR];

			ssi_size_t msgFormat;

			ssi_char_t pubNodeName[SSI_MAX_CHAR];
			ssi_char_t pubNodeService[SSI_MAX_CHAR];
			bool pubNode;	//publish to node instead of sending message to recipient

			ssi_char_t sname[SSI_MAX_CHAR];
			ssi_char_t ename[SSI_MAX_CHAR];

			ssi_char_t sname_sub[SSI_MAX_CHAR];
			ssi_char_t ename_sub[SSI_MAX_CHAR];

			ssi_char_t subNodeName[SSI_MAX_CHAR];
			ssi_char_t subNodeService[SSI_MAX_CHAR];
			bool subNode;	//subscribe to node

			bool useSaslMechPlain;
		};

	public:

		static const ssi_char_t *GetCreateName() { return "XMPP"; };
		static IObject *Create(const ssi_char_t *file) { return new XMPP(file); };
		~XMPP();

		Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "sends SSI events as XMPP messages"; };

		bool setEventListener(IEventListener *listener);
		
		const ssi_char_t *getEventAddress() {
			return _event_address.getAddress();
		}


		//event listener
		void listen_enter();
		bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
		void listen_flush();

	protected:

		XMPP(const ssi_char_t *file = 0);
		ssi_char_t *_file;
		Options _options;
		static char ssi_log_name[];


		XMPPclient *r;

		//event sender
		IEventListener *_listener;
		EventAddress _event_address;
		ssi_event_t _event, _event_sub;


		ITheFramework *_frame;

	};

}

#endif
