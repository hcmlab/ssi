// SocketEventWriter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/12/07
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

#include "SocketEventWriter.h"
#include "ioput/xml/tinyxml.h"
#include "ioput/file/File.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


namespace ssi {

int SocketEventWriter::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
ssi_char_t *SocketEventWriter::ssi_log_name = "esockwrite";

SocketEventWriter::SocketEventWriter (const ssi_char_t *file)
	: _file (0),
	_socket (0),
	_socket_osc (0),
	_frame (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	_frame = Factory::GetFramework ();
};

SocketEventWriter::~SocketEventWriter () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void SocketEventWriter::listen_enter () {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "start streaming events (%s:%u)", _options.host, _options.port);

	_socket = Socket::CreateAndConnect (_options.type, Socket::CLIENT, _options.port, _options.host);	
	if (_options.osc) {
		_socket_osc = new SocketOsc (*_socket);
	}
}

bool SocketEventWriter::update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {

	for (ssi_size_t i = 0; i < n_new_events; i++) {

		ssi_event_t *e = events.next ();		

		int result = 0;
		if (_options.osc) {

			switch (e->type) {

				case SSI_ETYPE_EMPTY: {

					ssi_size_t time = e->time;
					if (_options.reltime) {
						time = _frame->GetElapsedTimeMs() - time;
					}

					result = _socket_osc->send_event(Factory::GetString(e->sender_id),
						Factory::GetString(e->event_id),
						time,
						e->dur,
						e->state,
						0,
						0,
						0
					);

					break;
				}

				case SSI_ETYPE_MAP: {			

					ssi_size_t n = e->tot / sizeof (ssi_event_map_t);
					ssi_event_map_t *t = ssi_pcast (ssi_event_map_t, e->ptr);
					ssi_char_t **names = new ssi_char_t *[n];
					ssi_real_t *values = new ssi_real_t[n];
					
					for (ssi_size_t i = 0; i < n; i++) {
						names[i] = ssi_ccast (ssi_char_t *, Factory::GetString (t[i].id));
						values[i] = t[i].value;
					}

					ssi_size_t time = e->time;
					if (_options.reltime) {
						time = _frame->GetElapsedTimeMs () - time;
					}

					result = _socket_osc->send_event (Factory::GetString (e->sender_id),
						Factory::GetString(e->event_id),
						time,
						e->dur,
						e->state,
						n,						
						names, 
						values
					);	

					delete[] names;
					delete[] values;

					break;
				}

				case SSI_ETYPE_STRING: {

					ssi_size_t time = e->time;
					if (_options.reltime) {
						time = _frame->GetElapsedTimeMs () - time;
					}

					result = _socket_osc->send_message (Factory::GetString (e->sender_id),
						Factory::GetString(e->event_id),
						time,
						e->dur,
						e->ptr);

					break;

				}

				default:
					ssi_wrn ("type %s not supported", SSI_ETYPE_NAMES[e->type]);
					return false;
			}			
		} else {
			if (_options.xml) {

				TiXmlDocument doc;
				TiXmlDeclaration head ("1.0", "", "");
				doc.InsertEndChild (head);
				TiXmlElement body ("events");				
				body.SetAttribute ("ssi-v", File::VERSION_NAMES[File::DEFAULT_VERSION]);
				TiXmlElement sample ("event"); 
				sample.SetAttribute ("sender", Factory::GetString (e->sender_id));
				sample.SetAttribute ("event", Factory::GetString (e->event_id));

				ssi_size_t time = e->time;
				if (_options.reltime) {
					time = _frame->GetElapsedTimeMs () - time;
				}

				sample.SetAttribute ("from", time);
				sample.SetAttribute ("dur", e->dur);
				sample.SetDoubleAttribute ("prob", e->prob);
				sample.SetAttribute("type", SSI_ETYPE_NAMES[e->type]);
				sample.SetAttribute("state", SSI_ESTATE_NAMES[e->state]);
				sample.SetAttribute("glue", e->glue_id);

				switch (e->type) {
					case SSI_ETYPE_UNDEF:
					case SSI_ETYPE_EMPTY:
						break;
					case SSI_ETYPE_TUPLE: {
						ssi_size_t num = e->tot / sizeof (ssi_event_tuple_t);
						ssi_char_t string[SSI_MAX_CHAR];
						if (!ssi_array2string (num, ssi_pcast (ssi_event_tuple_t, e->ptr), SSI_MAX_CHAR, string)) {
							return false;
						}
						TiXmlText text (string);
						sample.InsertEndChild (text);
						break;
					}
					case SSI_ETYPE_STRING: {	
						TiXmlText text (ssi_pcast (ssi_char_t, e->ptr));
						text.SetCDATA(_options.cdata);
						sample.InsertEndChild (text);				
						break;
					}
					case SSI_ETYPE_MAP: {
						ssi_size_t num = e->tot / sizeof (ssi_event_map_t);
						ssi_event_map_t *ts = ssi_pcast (ssi_event_map_t, e->ptr);
						for (ssi_size_t n = 0; n < num; n++) {
							TiXmlElement tuple ("tuple");
							tuple.SetAttribute ("string", Factory::GetString (ts[n].id));
							tuple.SetDoubleAttribute ("value", ts[n].value);
							sample.InsertEndChild (tuple);
						}
						break;
					}
				}

				body.InsertEndChild (sample);
				doc.InsertEndChild (body);

				TiXmlPrinter printer;
				printer.SetIndent("");
				doc.Accept (&printer);
				
				_socket->send (printer.CStr (), ssi_cast (ssi_size_t, printer.Size ()));
					
			} else {
				result = _socket->send (e->ptr, e->tot);
			}
		}
		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "sent %d bytes", result);

	}

	return true;
}

void SocketEventWriter::listen_flush () {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "stop streaming events (%s:%u)", _options.host, _options.port);

	delete _socket_osc; _socket_osc = 0;
	delete _socket; _socket = 0;
}

}
