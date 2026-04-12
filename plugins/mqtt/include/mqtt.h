// mqtt.h
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 23/3/2016
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

#ifndef SSI_MQTT_H
#define SSI_MQTT_H

#include "mqttclient.h"
#include "mqttserver.h"

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

	class Mqtt : public IObject {

	public:

		class Options : public OptionList {

		public:

			Options() {
				setSenderName("mqtt");
				setEventName("message");

				setMqttClientServerName("127.0.0.1");
				setMqttSubTopic("/subtopic");
				setMqttPubTopic("/pubtopic");

				setMqttServerBind("0.0.0.0");

				mqtt_client_port = 1883;
				mqtt_server_port = 1883;


				mqtt_server_enable = false;
				just_data = false;

				//sender options
				addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender for events received by incoming messages");
				addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event for events received by incoming messages");

				addOption("just_data", &just_data, 1, SSI_BOOL, "just send the data of an event");
			
				addOption("mqtt_client_port", &mqtt_client_port, 1, SSI_UINT, "port of mqtt client");
				addOption("mqtt_client_server", mqtt_client_server, SSI_MAX_CHAR, SSI_CHAR, "mqtt client address");

				addOption("mqtt_server_enable", &mqtt_server_enable, 1, SSI_BOOL, "enable integrated mqtt server / broker");
				addOption("mqtt_server_port", &mqtt_server_port, 1, SSI_UINT, "port of mqtt server");
				addOption("mqtt_server_bind", mqtt_server_bind, SSI_MAX_CHAR, SSI_CHAR, "mqtt server address");

				addOption("mqtt_sub_topic", mqtt_sub_topic, SSI_MAX_CHAR, SSI_CHAR, "mqtt subscription topic");
				addOption("mqtt_pub_topic", mqtt_pub_topic, SSI_MAX_CHAR, SSI_CHAR, "mqtt publish topic");
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

			void setMqttClientServerName(const ssi_char_t *mqtt_server) {
				if (mqtt_server) {
					ssi_strcpy(this->mqtt_client_server, mqtt_server);
				}
			}

			void setMqttServerBind(const ssi_char_t *mqtt_server_bind) {
				if (mqtt_server_bind) {
					ssi_strcpy(this->mqtt_server_bind, mqtt_server_bind);
				}
			}

			void setMqttSubTopic(const ssi_char_t *mqtt_sub_topic) {
				if (mqtt_sub_topic) {
					ssi_strcpy(this->mqtt_sub_topic, mqtt_sub_topic);
				}
			}

			void setMqttPubTopic(const ssi_char_t *mqtt_pub_topic) {
				if (mqtt_pub_topic) {
					ssi_strcpy(this->mqtt_pub_topic, mqtt_pub_topic);
				}
			}



			ssi_char_t sname[SSI_MAX_CHAR];
			ssi_char_t ename[SSI_MAX_CHAR];

			ssi_size_t mqtt_client_port;
			ssi_char_t mqtt_client_server[SSI_MAX_CHAR];
			ssi_char_t mqtt_sub_topic[SSI_MAX_CHAR];
			ssi_char_t mqtt_pub_topic[SSI_MAX_CHAR];

			ssi_size_t mqtt_server_port;
			ssi_char_t mqtt_server_bind[SSI_MAX_CHAR];

			bool mqtt_server_enable;

			bool just_data;
		};

	public:

		static const ssi_char_t *GetCreateName() { return "mqtt"; };
		static IObject *Create(const ssi_char_t *file) { return new Mqtt(file); };
		~Mqtt();

		Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "sends SSI events as mqtt messages in JSON format"; };

		bool setEventListener(IEventListener *listener);
		
		const ssi_char_t *getEventAddress() {
			return _event_address.getAddress();
		}

		//event listener
		void listen_enter();
		bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
		void listen_flush();

		void sendSSIEvent(char* msg, int len, char* ip, int ip_len, int id);

	protected:

		Mqtt(const ssi_char_t *file = 0);
		ssi_char_t *_file;
		Options _options;
		static char ssi_log_name[];



		//event sender
		IEventListener *_listener;
		EventAddress _event_address;
		ssi_event_t _event;


		ITheFramework *_frame;

		MQTTclient *client;

		MQTTserver *server;
	};

}

#endif
