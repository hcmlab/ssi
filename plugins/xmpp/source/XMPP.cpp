// XMPP.cpp
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

#include "XMPP.h"


namespace ssi {

	char XMPP::ssi_log_name[] = "xmpp______";



	bool replace(std::string& str, const std::string& from, const std::string& to) {
		size_t start_pos = str.find(from);
		if (start_pos == std::string::npos)
			return false;
		str.replace(start_pos, from.length(), to);
		return true;
	}



	XMPP::XMPP(const ssi_char_t *file)
		: _file(0), r(0), _listener(0) {

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}

		ssi_event_init(_event, SSI_ETYPE_STRING);
		ssi_event_init(_event_sub, SSI_ETYPE_STRING);

		_frame = Factory::GetFramework();
	}

	XMPP::~XMPP() {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}

		ssi_event_destroy(_event);
		ssi_event_destroy(_event_sub);
	}

	void XMPP::listen_enter() {
		r = new XMPPclient(_options.jid, _options.pw, _options.pubNode, _options.pubNodeName, _options.pubNodeService, _listener, &_event, &_event_sub, _frame, _options.subNode, _options.subNodeName, _options.subNodeService, _options.useSaslMechPlain);
		r->setName("xmppThread");

		r->start();
	}



	//out of fossa library
	void base64_encode(const unsigned char *src, int src_len, char *dst) {
		static const char *b64 =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		int i, j, a, b, c;

		for (i = j = 0; i < src_len; i += 3) {
			a = src[i];
			b = i + 1 >= src_len ? 0 : src[i + 1];
			c = i + 2 >= src_len ? 0 : src[i + 2];

			dst[j++] = b64[a >> 2];
			dst[j++] = b64[((a & 3) << 4) | (b >> 4)];
			if (i + 1 < src_len) {
				dst[j++] = b64[(b & 15) << 2 | (c >> 6)];
			}
			if (i + 2 < src_len) {
				dst[j++] = b64[c & 63];
			}
		}
		while (j % 4 != 0) {
			dst[j++] = '=';
		}
		dst[j++] = '\0';
	}


	void ns_base64_encode(const unsigned char *src, int src_len, char *dst) {
		base64_encode(src, src_len, dst);
	}



	bool XMPP::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {


		for (int i = 0; i < n_new_events; i++) {
			ssi_event_t *e = events.next();

			//don't handle own events
			if (strcmp(Factory::GetString(e->sender_id), _options.sname) == 0)
				return true;

			switch (_options.msgFormat) {
				case MESSAGEBODYFORMAT_CSV:
				{
					//CSV mode
					std::stringstream strstr;
					char ch = _options.csvChr;
					strstr << SSI_ETYPE_NAMES[e->type] << ch << Factory::GetString(e->sender_id) << ch << Factory::GetString(e->event_id) << ch << e->time << ch << e->dur << ch << e->tot << ch << e->state << ch << e->glue_id;
					r->sendMessage(_options.recip, strstr.str());
					//std::cout << strstr.str() << std::endl;
					strstr.clear();

					break;
				}
				case MESSAGEBODYFORMAT_JSON:
				{
					//JSON mode
					rapidjson::StringBuffer s;
					rapidjson::Writer<rapidjson::StringBuffer> writer(s);

					writer.StartObject();
					writer.Key("type");
					writer.String(SSI_ETYPE_NAMES[e->type]);

					writer.Key("sender");
					writer.String(Factory::GetString(e->sender_id));

					writer.Key("event");
					writer.String(Factory::GetString(e->event_id));

					writer.Key("time");
					writer.Uint(e->time);

					writer.Key("dur");
					writer.Uint(e->dur);

					writer.Key("tot");
					writer.Uint(e->tot);

					writer.Key("state");
					writer.Uint(e->state);

					writer.Key("glue_id");
					writer.Uint(e->glue_id);



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

						ns_base64_encode(convbuf, e->tot, base64_buf);

						writer.String(e->ptr, strlen(base64_buf));
					}
					else // unknown types!
					{
						ssi_wrn("unknown event type!");
						writer.Null();
					}



					writer.EndObject();

					r->sendMessage(_options.recip, s.GetString());

					s.Clear();
					break;
				}
				case MESSAGEBODYFORMAT_TEMPLATE:
				{
					//KEYWORDS:
					/*
						<ISODATE>		"2014-11-25T10:20:42.472656Z"
						<VALUE>
						<RELIABILITY>	{-1.0; [0.0, 1.0] }
					*/

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

					//get path of template file with the format "<sender>_<event>.txt"
					std::stringstream path;
					path << _options.templatesDir << '/' << Factory::GetString(e->sender_id) << "_" << Factory::GetString(e->event_id) << ".txt";

					//load the template file
					std::ifstream t(path.str());
					
					//check if template exists
					if (!t.good()) {
						ssi_err("template \"%s\" not found!", path.str().c_str());
					}

					path.clear();

					std::string str;

					t.seekg(0, std::ios::end);
					str.reserve(t.tellg());
					t.seekg(0, std::ios::beg);

					str.assign((std::istreambuf_iterator<char>(t)),
						std::istreambuf_iterator<char>());


					//replace template variables
					replace(str, "<ISODATE>", isoDateStr);
					replace(str, "<RELIABILITY>", "-1");

					std::stringstream eventState;
					eventState << e->state;
					replace(str, "<EVENTSTATE>", eventState.str());


					//for <VALUE> we can use STRING or FLOATS
					if (e->type == SSI_ETYPE_STRING) {

						ssi_char_t buffer[SSI_MAX_CHAR];
						//buffer[0] = '\"';
						//memcpy(&buffer[1], e->ptr, e->tot);
						//buffer[e->tot] = '\"';
						//buffer[e->tot+1] = '\0';

						memcpy(&buffer[0], e->ptr, e->tot);

						replace(str, "<VALUE>", buffer);
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

						replace(str, "<VALUE>", strf.str());

						strf.clear();
					}


					//send to publish node or to recipient?
					if (_options.pubNode)
						r->publishToNode(str);
					else
						r->sendMessage(_options.recip, str);

					str.clear();
					t.close();


					break;
				}
				default: {
					ssi_err("Unknown message format!");
					break;
				}
			}
		}
		
		return true;
	}

	void  XMPP::listen_flush() {
		if (r) {
			r->disconnect();
			delete r;
			r = nullptr;
		}
	}



	bool XMPP::setEventListener(IEventListener *listener) {

		//ssi_event_init(_event, SSI_ETYPE_STRING);
		//ssi_event_adjust(_event, SSI_MAX_CHAR);
		//ssi_strcpy(_event.ptr, "");

		_listener = listener;
		_event.sender_id = Factory::AddString(_options.sname);
		if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}
		_event.event_id = Factory::AddString(_options.ename);
		if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}

		_event_sub.sender_id = Factory::AddString(_options.sname_sub);
		if (_event_sub.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}
		_event_sub.event_id = Factory::AddString(_options.ename_sub);
		if (_event_sub.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}


		std::string snames = _options.sname;
		snames += ",";
		snames.append(_options.sname_sub);

		std::string enames = _options.ename;
		enames += ",";
		enames.append(_options.ename_sub);

		_event_address.setSender(snames.c_str());
		_event_address.setEvents(enames.c_str());


		return true;
	}

}
