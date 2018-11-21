// XmlToJson.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 24/10/2017
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

#include "XmlToJson.h"
#include "base/Factory.h"
#include "xml2json.hpp"

namespace ssi {

	char XmlToJson::ssi_log_name[] = "xmltojson_";

	XmlToJson::XmlToJson(const ssi_char_t *file)
		: _listener(0),
		_file(0) {

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}

		ssi_event_init(_event, SSI_ETYPE_STRING);
	}

	XmlToJson::~XmlToJson() {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	bool XmlToJson::setEventListener(IEventListener *listener) {

		_listener = listener;
	
		SSI_OPTIONLIST_SET_ADDRESS(_options.address, _event_address, _event);

		return true;
	}

	bool XmlToJson::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {

		if (n_new_events > 0 && _listener)
		{
			ssi_event_t *e = 0;

			for (ssi_size_t i = 0; i < n_new_events; i++)
			{
				e = events.next();

				switch (e->type)
				{
				case SSI_ETYPE_STRING:
				{
					ssi_char_t *xml = ssi_strcpy(ssi_pcast(ssi_char_t, e->ptr));

					xml2json_params params;
					{
						_options.lock();
						params.prettify = _options.prettify;
						params.text_prefix = _options.text_prefix;
						params.attribute_prefix = _options.attribute_prefix;
						params.numeric_support = _options.numeric_support;
						_options.unlock();
					}

					try
					{
						std::string json = xml2json(xml, params);
						ssi_size_t n_json = (ssi_size_t)json.length();

						_event.glue_id = e->glue_id;
						_event.state = e->state;
						_event.time = e->time;
						_event.dur = e->dur;
						ssi_event_adjust(_event, n_json + 1);
						ssi_strcpy(_event.ptr, json.c_str());
						_listener->update(_event);
					}
					catch (std::exception ex)
					{
						ssi_wrn("%s", ex.what());
					}

					delete[] xml;

					break;
				}
				}
			}
		}


		return true;
	}

}
