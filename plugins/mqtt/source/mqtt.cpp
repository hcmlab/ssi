// mqtt.cpp
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

#include "mqtt.h"
#include <thread/RunAsThread.h>



namespace ssi {

	char Mqtt::ssi_log_name[] = "mqtt_";



	Mqtt::Mqtt(const ssi_char_t *file)
		: _file(0), _listener(0), client(0), server(0) {

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}

		ssi_event_init(_event, SSI_ETYPE_STRING);

		_frame = Factory::GetFramework();
	}

	Mqtt::~Mqtt() {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}

		ssi_event_destroy(_event);
	}

	void Mqtt::listen_enter() {
		if (!server && getOptions()->mqtt_server_enable) {
			server = new MQTTserver(this);
			server->setName("mqttServerThread");
			server->start();
		}
		if (!client) {
			client = new MQTTclient(this);
			client->setName("mqttClientThread");
			client->start();
		}

		
	}

	bool Mqtt::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {


		for (int i = 0; i < n_new_events; i++) {
			ssi_event_t *e = events.next();

			if (client) {

				if (getOptions()->just_data)
				{
					if (e->type == SSI_ETYPE_STRING)
					{
						client->sendEvent(e->ptr);
					}
				}
				else {

					rapidjson::StringBuffer s;
					rapidjson::Writer<rapidjson::StringBuffer> writer(s);

					writer.StartObject();


					writer.String("type");
					writer.String(SSI_ETYPE_NAMES[e->type]);

					writer.String("sender");
					writer.String(Factory::GetString(e->sender_id));

					writer.String("name");
					writer.String(Factory::GetString(e->event_id));

					writer.String("time");
					writer.Int(e->time);

					writer.String("dur");
					writer.Int(e->dur);

					writer.String("tot");
					writer.Int(e->tot);

					writer.String("state");
					writer.String(SSI_ESTATE_NAMES[e->state]);

					writer.String("glue");
					writer.Int(e->glue_id);

					writer.String("prob");
					writer.Double(e->prob);


					writer.String("value");

					if (e->type == SSI_ETYPE_STRING)
					{
						writer.String(e->ptr);
					}
					else if (e->type == SSI_ETYPE_TUPLE)
					{
						writer.StartArray();

						ssi_real_t *floatptr = ssi_pcast(ssi_real_t, e->ptr);

						for (int i = 0; i < e->tot / sizeof(ssi_real_t); i++)
							writer.Double(floatptr[i]);

						writer.EndArray();
					}
					else if (e->type == SSI_ETYPE_EMPTY)
					{
						writer.Null();
					}
					else if (e->type == SSI_ETYPE_MAP)
					{
						writer.StartArray();

						ssi_event_map_t *ptr = ssi_pcast(ssi_event_map_t, e->ptr);

						for (int i = 0; i < e->tot / sizeof(ssi_event_map_t); i++) {

							writer.StartObject();
							writer.String(Factory::GetString(ptr[i].id));
							writer.Double(ptr[i].value);
							writer.EndObject();

						}

						writer.EndArray();
					}
					else if (e->type == SSI_ETYPE_UNDEF)
					{
						unsigned char *convbuf = reinterpret_cast<unsigned char*>(e->ptr);
						char *base64_buf = new char[4 * (int)ceil(e->tot / 3.0)];				//http://stackoverflow.com/questions/9668863/predict-the-byte-size-of-a-base64-encoded-byte

						mg_base64_encode(convbuf, e->tot, base64_buf);

						writer.String(e->ptr, strlen(base64_buf));
					}
					else // unknown types!
					{
						ssi_wrn("unknown event type!");
						writer.Null();
					}


					writer.EndObject();

					client->sendEvent(s.GetString());
				}
			}
			
		}
		
		return true;
	}

	void  Mqtt::listen_flush() {

		if (client) {
			delete client;
			client = nullptr;
		}

		if (server) {
			delete server;
			server = nullptr;
		}

	}

	void Mqtt::sendSSIEvent(char* msg, int len, char* topic, int topic_len, int id)
	{

		rapidjson::StringBuffer s;
		rapidjson::Writer<rapidjson::StringBuffer> writer(s);

		writer.StartObject();

		writer.String("topic");
		writer.String(topic, topic_len);

		writer.String("id");
		writer.Int(id);

		writer.String("data");
		writer.String(msg, len);

		writer.EndObject();


		std::string jsonStr = s.GetString();

		int strlength = jsonStr.length();
		const char *str = jsonStr.c_str();

		ssi_event_adjust(_event, strlength + 1 );
		memcpy(_event.ptr, str, strlength + 1);
		_event.time = _frame->GetElapsedTimeMs();

		_listener->update(_event);
	}


	bool Mqtt::setEventListener(IEventListener *listener) {

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

}
