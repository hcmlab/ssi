// pythonbridgeentry.cpp
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

#include "ioput/file/FileTools.h"
#include "base/ITheFramework.h"
#include "base/Factory.h"

#include "../include/pythonbridgeentry.h"

namespace ssi {

	PythonBridgeEntry::PythonBridgeEntry(const ssi_char_t *file)
		: _file(0) {

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}

		ssi_event_init(_event, SSI_ETYPE_UNDEF);
		// ssi_event_init(_event, SSI_ETYPE_MAP);
	}

	bool PythonBridgeEntry::setEventListener(IEventListener *listener) {

		_listener = listener;

		if (_options.address[0] != '\0') {

			_event_address.setAddress(_options.address);
			_event.sender_id = Factory::AddString(_event_address.getSender(0));
			_event.event_id = Factory::AddString(_event_address.getEvent(0));

		}
		else {

			ssi_wrn("use of deprecated option 'sname' and 'ename', use 'address' instead")

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
		}

		return true;

	}

	PythonBridgeEntry::~PythonBridgeEntry() {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}

		ssi_event_destroy(_event);
	}

	void PythonBridgeEntry::consume_enter(ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) {
		
	}

	void PythonBridgeEntry::consume(IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) {

		if (stream_in_num != 1) {
			ssi_wrn("PythonBridgeEntry is currently only usable for single stream.");
			return;
		}

		if (_listener) {
			
			// ssi_print("\ntime:\t%.2f\ndur:\t%.2f\nstream_in.tot:\t%d", consume_info.time, consume_info.dur, stream_in[0].tot);

			_event.time = consume_info.time * 1000;
			_event.dur = consume_info.dur * 1000;
			// ssi_print("\nconsume(e.ptr):\t%d", _event.ptr);

			if (stream_in[0].tot_real > 0) {

				ssi_event_adjust(_event, stream_in[0].tot);
				// ssi_print("\nconsume (stream.tot):\t%d", stream_in[0].tot);
				// ssi_event_adjust(_event, 1 * sizeof(ssi_event_map_t));

				ssi_byte_t *in_ptr = stream_in[0].ptr;
				ssi_byte_t *out_ptr = _event.ptr;
				
				//ssi_real_t* in_ptr = ssi_pcast(ssi_real_t, stream_in[0].ptr);
				//if (_listener) {
				//	ssi_char_t string[SSI_MAX_CHAR];
				//	ssi_event_map_t* e = ssi_pcast(ssi_event_map_t, _event.ptr);
				//	ssi_sprint(string, "pyentry");
				//	e[0].id = Factory::AddString(string); // TODO: einmalig in listen_enter registrieren und id als variable speichern
				//	e[0].value = in_ptr[0];
				//	_listener->update(_event);
				//}

				for (ssi_size_t n_bytes = 0; n_bytes < stream_in[0].tot_real; n_bytes++)
				{
					*out_ptr = *in_ptr;
					out_ptr++;
					in_ptr++;
				}

				_listener->update(_event);
			}
		}
	}

	void PythonBridgeEntry::consume_flush(ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}

		if (_listener) {
			ssi_event_reset(_event);
		}
	}

}