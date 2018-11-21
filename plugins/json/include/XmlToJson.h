// XmlToJson.h
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

#pragma once

#ifndef SSI_JSON_XMLTOJSON_H
#define SSI_JSON_XMLTOJSON_H

#include "base/IObject.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"

namespace ssi {

	class XmlToJson : public IObject {

	public:

		class Options : public OptionList {

		public:

			Options()
				: prettify(true),
				attribute_prefix(true),
				text_prefix(true),
				numeric_support(true)
			{
				setAddress("string@json");

				SSI_OPTIONLIST_ADD_ADDRESS(address);

				addOption("prettify", &prettify, 1, SSI_BOOL, "Output json with line breaks and indent", false);
				addOption("attributePrefix", &attribute_prefix, 1, SSI_BOOL, "Add '@' prefix to attributes", false);
				addOption("textPrefix", &text_prefix, 1, SSI_BOOL, "Add '#text' prefix to terminal text", false);
				addOption("numericSupport", &numeric_support, 1, SSI_BOOL, "Convert numbers if possible (otherwise numbers will be represented as strings)", false);
			}

			void setAddress(const ssi_char_t *address) {
				if (address) {
					ssi_strcpy(this->address, address);
				}
			}

			bool prettify;
			bool attribute_prefix;
			bool text_prefix;
			bool numeric_support;

			ssi_char_t address[SSI_MAX_CHAR];
		};

	public:

		static const ssi_char_t *GetCreateName() { return "XmlToJson"; };
		static IObject *Create(const ssi_char_t *file) { return new XmlToJson(file); };
		~XmlToJson();

		Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "Converts string events with xml content to json."; };

		bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
		
		bool setEventListener(IEventListener *listener);
		const ssi_char_t *getEventAddress() {
			return _event_address.getAddress();
		}


	protected:

		XmlToJson(const ssi_char_t *file = 0);
		ssi_char_t *_file;
		Options _options;
		static char ssi_log_name[];

		IEventListener *_listener;
		EventAddress _event_address;
		ssi_event_t _event;
	};

}

#endif
