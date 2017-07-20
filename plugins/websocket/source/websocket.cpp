// websocket.cpp
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

#include "websocket.h"
#include <thread/RunAsThread.h>


namespace ssi {

	char Websocket::ssi_log_name[] = "websocket_";



	Websocket::Websocket(const ssi_char_t *file)
		: _file(0), _listener(0), server(0) {

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}

		ssi_event_init(_event, SSI_ETYPE_STRING);

		_frame = Factory::GetFramework();
	}

	Websocket::~Websocket() {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}

		ssi_event_destroy(_event);
	}

	void Websocket::listen_enter() {
		if (!server) {
			server = new Websockserver(this);
			server->setName("websockThread");
			server->start();
		}
		
	}

	bool Websocket::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {


		for (int i = 0; i < n_new_events; i++) {
			ssi_event_t *e = events.next();



			
			if (server)  {

				bool sendEvent = false;

				//filter out own messages?
				if (e->sender_id == _event.sender_id)
				{
					if (!_options.send_own_events)
						sendEvent = false;
					else
						sendEvent = true;
				}
				else
				{
					sendEvent = true;
				}



				if (sendEvent) {
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

					server->sendEvent(s.GetString());
				}
			}
		}
		
		return true;
	}

	void  Websocket::listen_flush() {

		if (server) {
			delete server;
			server = nullptr;
		}

	}

	void Websocket::sendSSIEvent(char* msg, int len, char* ip, int ip_len, int id)
	{

		rapidjson::StringBuffer s;
		rapidjson::Writer<rapidjson::StringBuffer> writer(s);

		writer.StartObject();

		writer.String("ip");
		writer.String(ip, ip_len);

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


	bool Websocket::setEventListener(IEventListener *listener) {

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


	void Websocket::consume_enter(ssi_size_t stream_in_num, ssi_stream_t stream_in[])
	{

	}


	void Websocket::consume(IConsumer::info consume_info, ssi_size_t stream_in_num, ssi_stream_t stream_in[])
	{

		if (_options.send_info && server->stream_info_JSON.size() == 0) {


			rapidjson::StringBuffer s;
			rapidjson::Writer<rapidjson::StringBuffer> writer(s);

			writer.StartObject();


			writer.String("stream_number");
			writer.Int(stream_in_num);


			writer.String("streams");
			writer.StartArray();

			for (int i = 0; i < stream_in_num; i++)
			{
				writer.StartObject();

				writer.String("dim");
				writer.Int(stream_in[i].dim);

				writer.String("byte");
				writer.Int(stream_in[i].byte);

				writer.String("sr");
				writer.Double(stream_in[i].sr);

				writer.String("tot");
				writer.Int(stream_in[i].tot);

				writer.String("type");
				writer.String(SSI_TYPE_NAMES[stream_in[i].type]);

				writer.EndObject();

			}

			writer.EndArray();


			writer.EndObject();

			server->stream_info_JSON = s.GetString();

		}



		//combine all streams
		if (stream_in_num > 1)
		{
			//calculate total size of buffer
			long total_size = 0;
			for (int i = 0; i < stream_in_num; i++)
			{
				total_size += stream_in[i].tot;
			}

			//allocate buffer
			char *buffer = new char[total_size];

			//copy data of streams into buffer
			long offset = 0;
			for (int i = 0; i < stream_in_num; i++)
			{
				memcpy(&buffer[offset], stream_in[i].ptr, stream_in[i].tot);
				offset += stream_in[i].tot;
			}

			server->sendRawData(buffer, total_size);

			delete[] buffer;

		}
		else {
			server->sendRawData(stream_in[0].ptr, stream_in[0].tot);
		}
	}


	void Websocket::consume_flush(ssi_size_t stream_in_num, ssi_stream_t stream_in[])
	{
	}
}
