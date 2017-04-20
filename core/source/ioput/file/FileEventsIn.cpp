// FileEventsIn.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/10/21
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "ioput/file/FileEventsIn.h"
#include "ioput/xml/tinyxml.h"
#include "ioput/file/FilePath.h"
#include "base/Factory.h"

namespace ssi {

File::VERSION FileEventsIn::DEFAULT_VERSION = File::V2;
int FileEventsIn::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
ssi_char_t FileEventsIn::ssi_log_name[] = "fsampin___";

FileEventsIn::FileEventsIn ()
	: _file (0),
	_path (0),	
	_n_events(0),
	_event_count(0),
	_first(0),
	_current(0),
	_doc(0),
	_version (File::V2) {	

	ssi_event_init(_event);
}

FileEventsIn::~FileEventsIn () {

	if (_file) {
		close ();
	}
}

bool FileEventsIn::open (const ssi_char_t *path) {

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "open event file '%s'", path);	

	if (_file) {
		ssi_wrn ("event file already open");
		return 0;
	}

	if (path == 0 || path[0] == '\0') {
		ssi_wrn ("'%s' is not a valid path", path);
		return 0;
	}

	FilePath fp (path);
	if (strcmp (fp.getExtension (), SSI_FILE_TYPE_EVENTS) != 0) {
		_path = ssi_strcat(path, SSI_FILE_TYPE_EVENTS);
	} else {
		_path = ssi_strcpy(path);
	}	

	_file = File::CreateAndOpen(File::ASCII, File::READ, _path);
	if (!_file) {
		ssi_wrn("could not open info file '%s'", _path);
		return 0;
	}

	_doc = new TiXmlDocument();
	if (!_doc->LoadFile (_file->getFile (), false)) {
		ssi_wrn("failed loading samples from file '%s'", _path);
		return 0;
	}

	TiXmlElement *body = _doc->FirstChildElement();	
	if (!body || strcmp (body->Value (), "events") != 0) {
		ssi_wrn ("tag <events> missing");
		return 0;	
	}

	int v = 0;
	if (body->QueryIntAttribute ("ssi-v", &v) != TIXML_SUCCESS) {
		ssi_wrn ("attribute <ssi-v> in tag <events> missing");
		return 0;	
	}
	_version = ssi_cast (File::VERSION, v);

	if (_version < File::V2) {
		ssi_wrn ("version < V2 not supported");
		return 0;
	}

	_current = body->FirstChildElement("event");
	_n_events = 0;
	if (_current) {
		_n_events++;
		while (_current = _current->NextSiblingElement("event")) {
			_n_events++;
		}
	}
	_first = _current = body->FirstChildElement("event");

	_event_count = 0;

	return true;
};

ssi_event_t *FileEventsIn::get (ssi_size_t index) {

	reset ();
	ssi_event_t *e = 0;
	for (ssi_size_t i = 0; i < index; i++) {
		e = next ();
	}
	return e;
}

ssi_event_t *FileEventsIn::next() {

	if (!_file) {
		ssi_wrn ("files not open");
		return 0;
	}
	if (_event_count++ >= _n_events || _current == 0) {
		return 0;
	}

	ssi_event_destroy(_event);	
	ssi_event_init(_event);

	const char *type_name = _current->Attribute("type");
	for (ssi_size_t i = 0; i < SSI_ETYPE_NAME_SIZE; i++) {
		if (ssi_strcmp(type_name, SSI_ETYPE_NAMES[i], false)) {
			_event.type = ssi_cast(ssi_etype_t, i);
			break;
		}
	}

	const char *state_name = _current->Attribute("state");
	for (ssi_size_t i = 0; i < SSI_ESTATE_NAME_SIZE; i++) {
		if (ssi_strcmp(type_name, SSI_ESTATE_NAMES[i], false)) {
			_event.state = ssi_cast(ssi_estate_t, i);
			break;
		}
	}

	const char *sender_name = _current->Attribute("sender");
	if (!sender_name) {
		ssi_wrn("attribute 'sender' is missing in row %d (%s)", _current->Row(), _path);
	} else {
		_event.sender_id = Factory::AddString(sender_name);
	}

	const char *event_name = _current->Attribute("event");
	if (!event_name) {
		ssi_wrn("attribute 'event' is missing in row %d (%s)", _current->Row(), _path);
	} else {
		_event.event_id = Factory::AddString(event_name);
	}

	int from = 0;
	if (_current->QueryIntAttribute("from", &from) != TIXML_SUCCESS) {
		ssi_wrn("attribute 'from' is missing in row %d (%s)", _current->Row(), _path);
	} else {
		_event.time = ssi_cast(ssi_size_t, from);
	}

	int dur = 0;
	if (_current->QueryIntAttribute("dur", &dur) != TIXML_SUCCESS) {
		ssi_wrn("attribute 'dur' is missing in row %d (%s)", _current->Row(), _path);
	}
	else {
		_event.dur = ssi_cast(ssi_size_t, dur);
	}

	float prob = 0;
	if (_current->QueryFloatAttribute("prob", &prob) != TIXML_SUCCESS) {
		ssi_wrn("attribute 'prob' is missing in row %d (%s)", _current->Row(), _path);
	}
	else {
		_event.prob = ssi_cast(ssi_real_t, prob);
	}

	int glue = 0;
	if (_current->QueryIntAttribute("glue", &glue) != TIXML_SUCCESS) {
		ssi_wrn("attribute 'glue' is missing in row %d (%s)", _current->Row(), _path);
	} else {
		_event.glue_id = ssi_cast(ssi_size_t, glue);
	}

	switch (_event.type) {
	case SSI_ETYPE_STRING: {

		const ssi_char_t *string = _current->GetText();
		if (string) {
			ssi_event_adjust(_event, ssi_strlen(string) + 1);
			ssi_strcpy(_event.ptr, string);
		} else {
			ssi_event_adjust(_event, 1);
			_event.ptr[0] = '\0';
		}

		break;
	}

	case SSI_ETYPE_TUPLE: {

		const ssi_char_t *floats = _current->GetText();
		ssi_size_t n_floats = ssi_string2array_count(floats);
		if (n_floats > 0) {
			ssi_event_adjust(_event, n_floats * sizeof(ssi_event_tuple_t));
			ssi_string2array(n_floats, ssi_pcast(ssi_event_tuple_t, _event.ptr), floats);
		}

		break;
	}

	case SSI_ETYPE_MAP: {

		TiXmlElement *tuple = _current->FirstChildElement("tuple");
		if (tuple) {
			ssi_size_t n_tuples = 0;
			do {			
				n_tuples++;
			} while (tuple = tuple->NextSiblingElement("tuple"));
			ssi_event_adjust(_event, n_tuples * sizeof(ssi_event_map_t));
			ssi_event_map_t *t = ssi_pcast(ssi_event_map_t, _event.ptr);
			tuple = _current->FirstChildElement("tuple");
			do {
				const ssi_char_t *string = tuple->Attribute("string");
				if (!string) {
					ssi_wrn("attribute 'string' is missing in row %d (%s)", tuple->Row(), _path);
					t->id = SSI_FACTORY_STRINGS_INVALID_ID;
				} else {
					t->id = Factory::AddString(string);
				}
				float value;
				if (tuple->QueryFloatAttribute("value", &value) != TIXML_SUCCESS) {
					ssi_wrn("attribute 'value' is missing in row %d (%s)", tuple->Row(), _path);
					t->value = 0;
				} else {
					t->value = ssi_cast(ssi_real_t, value);
				}
				t++;
			} while (tuple = tuple->NextSiblingElement("tuple"));
		}

		break;
	}
	}

	_current = _current->NextSiblingElement("event");

	return &_event;
}

bool FileEventsIn::close () { 

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "close event file '%s'", _path);

	if (!_file || !_file->close ()) {
		ssi_wrn ("could not close file '%s'", _path);
		return false;
	}

	delete _doc; _doc = 0;
	delete _file; _file = 0;
	delete _path; _path = 0;	
	ssi_event_destroy (_event);
	_n_events = 0;
	_first = 0;
	_current = 0;
	_event_count = 0;

	return true;
};

}
