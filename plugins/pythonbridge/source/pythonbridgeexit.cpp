// pythonbridgeexit.cpp
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

#include "../include/pythonbridgeexit.h"

namespace ssi {

	ssi_char_t *PythonBridgeExit::ssi_log_name = "___pybexit";

	PythonBridgeExit::PythonBridgeExit(const ssi_char_t *file)
		: _file(0) {

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}

		ssi_event_init(_event, SSI_ETYPE_MAP);
	}

	bool PythonBridgeExit::setEventListener(IEventListener *listener) {

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

	PythonBridgeExit::~PythonBridgeExit() {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}

		ssi_event_destroy(_event);
	}

	void PythonBridgeExit::enter() {
		ssi_print("\nSTART()");

		if (_listener) {
			ssi_event_adjust(_event, 1 * sizeof(ssi_event_map_t));
		}
	}

	void PythonBridgeExit::run() {
		ssi_print("\nRUN()");

		if (_listener) {

			if (true) {
				ssi_char_t string[SSI_MAX_CHAR];
				_event.dur = 500;
				_event.time = getElapsedTime();
				ssi_event_map_t* e = ssi_pcast(ssi_event_map_t, _event.ptr);
				ssi_sprint(string, "pypexit");
				e[0].id = Factory::AddString(string); // TODO: einmalig in listen_enter registrieren und id als variable speichern
				e[0].value = 0.0f;
				_listener->update(_event);
			}
		}

		::Sleep(500);
	}

	void PythonBridgeExit::flush() {
		ssi_print("\nSTOP()");
	}

}