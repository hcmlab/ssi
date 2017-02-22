// Stimuli.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 17/11/2014
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
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "Stimuli.h"

#include "CWebBrowser.h"
#include "ioput/file/FileTools.h"
#include "base/Factory.h"
#include "ioput/file/FilePath.h"
#include "ioput/file/FileAnnotationWriter.h"

namespace ssi {

char Stimuli::ssi_log_name[] = "stimuli___";

Stimuli::Stimuli(const ssi_char_t *file)
	: _elistener (0),
	_next_name(0),
	_n_names(0),
	_finished(false),
	_anno_writer(0),
	_file(0) {

	if (file) {
		if (!OptionList::LoadXML(file, _options)) {
			OptionList::SaveXML(file, _options);
		}
		_file = ssi_strcpy(file);
	}	

	ssi_event_init(_ename, SSI_ETYPE_STRING);
}

Stimuli::~Stimuli() {

	if (_file) {
		OptionList::SaveXML(_file, _options);
		delete[] _file;
	}

	ssi_event_destroy(_ename);
}

void Stimuli::listen_enter() {
	
	_next_name = 0;
	_finished = true;

	if (_options.randomize) {
		ssi_random_seed();
	}

	if (loadFromFolder()) {	
		_finished = false;
	} else if (loadFromNames()) {
		_finished = false;		
	}

	if (!_finished) {
		ssi_msg(SSI_LOG_LEVEL_BASIC, "found %u names", _n_names);
	}  else {
		ssi_wrn("no names found");
	}
	
	if (_options.annopath[0] != '\0') {
		_anno_writer = new FileAnnotationWriter(_options.annopath);
	}

	if (_options.startName[0] != '\0') {
		sendName(_options.startName, 0);
	} else if (!_finished) {
		const ssi_char_t *url = nextName();
		sendName(url, 0);
	}
}

void Stimuli::loadHelp(ssi_size_t n, ssi_char_t **names, ssi_char_t **labels, bool randomize, bool insertBlank) {

	_names.clear();
	_labels.clear();

	ssi_size_t *indices = new ssi_size_t[n];
	for (ssi_size_t i = 0; i < n; i++) {
		indices[i] = i;
	}

	if (randomize) {
		ssi_random_shuffle(n, indices);
	}

	for (ssi_size_t i = 0; i < n; i++) {
		if (insertBlank) {
			_names.add("");
			if (labels) {
				_labels.add("");
			}
		}
		_names.add(names[indices[i]]);
		if (labels) {
			_labels.add(labels[indices[i]]);
		}
	}
	if (insertBlank) {
		_names.add("");
		if (labels) {
			_labels.add("");
		}
	}

	delete[] indices;
	_n_names = _names.size();
}

bool Stimuli::loadFromFolder() {
	
	_n_names = 0;

	if (_options.folder[0] != '\0') {

		ssi_size_t n = ssi_split_string_count(_options.folder, ';');
		ssi_msg(SSI_LOG_LEVEL_DETAIL, "found '%u' folders", n);

		if (n > 0) {

			ssi_char_t **folders = new ssi_char_t *[n];
			ssi_split_string(n, folders, _options.folder, ';');
			StringList names_list;
			for (ssi_size_t i = 0; i < n; i++) {
				if (_options.randomize) {
					StringList names_list_tmp;
					FileTools::ReadFilesFromDir(names_list_tmp, folders[i], _options.extension);
					ssi_size_t n_indices = names_list_tmp.size();
					if (n_indices > 0) {
						ssi_size_t *indices = new ssi_size_t[n_indices];
						for (ssi_size_t j = 0; j < n_indices; j++) {
							indices[j] = j;
						}
						ssi_random_shuffle(n_indices, indices);
						for (ssi_size_t j = 0; j < n_indices; j++) {
							names_list.add(names_list_tmp.get(indices[j]));
						}
						delete indices;
					}

				} else {
					FileTools::ReadFilesFromDir(names_list, folders[i], _options.extension);
				}
				delete folders[i];
			}			
			delete[] folders;
			ssi_size_t n = names_list.size();
			ssi_char_t **names = new ssi_char_t *[n];
			for (ssi_size_t i = 0; i < n; i++) {
				names[i] = ssi_strcpy(names_list.get(i));
			}

			ssi_char_t **labels = 0;
			if (_options.labelFromFile && n > 0) {
				labels = new ssi_char_t *[n];
				for (ssi_size_t i = 0; i < n; i++) {
					labels[i] = labelFromFile(names_list.get(i));
				}				
			}

			loadHelp(n, names, labels, false, _options.insertBlank);

			for (ssi_size_t i = 0; i < n; i++) {
				delete[] names[i];
			}
			if (labels) {
				for (ssi_size_t i = 0; i < n; i++) {
					delete[] labels[i];
				}
			}
			delete[] names;
			delete[] labels;
		}
	}

	return _n_names != 0;
}

bool Stimuli::loadFromNames() {

	_names.clear();
	_n_names = 0;

	if (_options.names[0] != '\0') {

		ssi_size_t n = ssi_split_string_count(_options.names, ';');
		ssi_msg(SSI_LOG_LEVEL_DETAIL, "found '%u' names", n);

		if (n > 0) {

			ssi_char_t **names = new ssi_char_t *[n];
			ssi_split_string(n, names, _options.names, ';');

			ssi_char_t **labels = 0;
			if (_options.labels[0] != '\0') {
				ssi_size_t n_labels = ssi_split_string_count(_options.labels, ';');
				if (n_labels != n) {
					ssi_wrn("#names ('%u') != #labels ('%u')", _names.size(), n)
				} else {
					labels = new ssi_char_t *[n];
					ssi_split_string(n, labels, _options.labels, ';');
				}
			}

			loadHelp(n, names, labels, _options.randomize, _options.insertBlank);

			for (ssi_size_t i = 0; i < n; i++) {
				delete[] names[i];				
			}
			if (labels) {
				for (ssi_size_t i = 0; i < n; i++) {
					delete[] labels[i];
				}
			}
			delete[] names;
			delete[] labels;			
		}		
	}

	return _n_names != 0;
}

const ssi_char_t *Stimuli::nextName() {
	
	if (_next_name == _n_names) {
		return 0;
	}

	const ssi_char_t *name = _names.get(_next_name);
	ssi_msg(SSI_LOG_LEVEL_BASIC, "next name '%s'", name);
	_next_name++;

	return name;
}

void Stimuli::sendName(const ssi_char_t *name, ssi_size_t time) {

	if (_elistener && name) {
		ssi_event_adjust(_ename, ssi_strlen(name) + 1);
		ssi_strcpy(_ename.ptr, name);
		_ename.time = time;
		_elistener->update(_ename);
	}
}

ssi_char_t *Stimuli::labelFromFile(const ssi_char_t *file) {

	if (file && file[0] != '\0') {

		FilePath fp(file);
		const ssi_char_t *name = fp.getName();

		ssi_size_t n = ssi_strlen(name);
		ssi_char_t *result = new ssi_char_t[n+1];

		ssi_char_t *ptr = result;
		for (ssi_size_t i = 0; i < n; i++) {
			if (isalpha(name[i])) {
				*ptr++ = name[i];
			}
		}
		*ptr = '\0';

		return result;
	}

	return 0;
}

bool Stimuli::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms)
{
	if (_finished) {
		return true;
	}

	if (n_new_events > 0) {

		if (n_new_events > 1) {
			ssi_wrn("received %u events, only latest will be processed", n_new_events)
		}
		
		ssi_event_t *e = events.next();		
		if (_next_name > 0) {
			if (_options.applyLabelToEvent && _labels.size() > 0 && e->type == SSI_ETYPE_STRING) {
				ssi_event_adjust(*e, ssi_strlen(_labels.get(_next_name - 1)) + 1);
				ssi_strcpy(e->ptr, _labels.get(_next_name - 1));
				ssi_msg(SSI_LOG_LEVEL_BASIC, "set label '%s'", _labels.get(_next_name - 1));
			}
			if (_anno_writer) {
				if (_labels.size() > 0) {
					_anno_writer->setLabel(_labels.get(_next_name - 1));
				} else {
					_anno_writer->setLabel(_names.get(_next_name - 1));
				}
				_anno_writer->update(*e);
			}
		}

		const ssi_char_t *url = nextName();

		if (url == 0) {
			_finished = true; 
			if (_options.endName[0] != '\0') {
				sendName(_options.endName, e->time);
			}
		} else {
			sendName(url, e->time);
		}
	}

	return true;	
}

void Stimuli::listen_flush() {

	delete _anno_writer;	
	_next_name = 0;
	_finished = false;
}

bool Stimuli::setEventListener(IEventListener *listener) {

	_elistener = listener;

	if (_options.address[0] != '\0') 
	{
		SSI_OPTIONLIST_SET_ADDRESS(_options.address, _eaname, _ename);
	}
	else
	{
		ssi_wrn("ename and sname are deprecated, use address");

		_ename.sender_id = Factory::AddString(_options.sname);
		if (_ename.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}
		_ename.event_id = Factory::AddString(_options.ename);
		if (_ename.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}

		_eaname.setSender(_options.sname);
		_eaname.setEvents(_options.ename);
	}

	return true;
}

}


