// Collector.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/03/04
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

#include "Collector.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

Collector::Collector (const ssi_char_t *file) 
	: _sample_list (0),
	_n_classes (0),
	_user_index (0),
	_class_index (0),
	_event (false, false),	
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}	
}

Collector::~Collector () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void Collector::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (!_sample_list)
	{
		ssi_err("no sample list");
	}

	_user_index = _sample_list->addUserName (_options.user);

	size_t len = strlen (_options.classes);
	_n_classes = 0;

	ssi_char_t string[SSI_MAX_CHAR];
	
	char *pch;
	printf ("classes: ");
	strcpy (string, _options.classes);
	pch = strtok (string," ");
	while (pch != NULL) {
		printf ("%s ", pch);
		_n_classes++;
		pch = strtok (NULL, " ");
	}

	if (_n_classes == 0) {
		ssi_err ("no classes");
	}

	_class_index = new ssi_size_t[_n_classes];
	strcpy (string, _options.classes);
	pch = strtok (string," ");
	ssi_size_t nclass = 0;
	while (pch != NULL) {
		_class_index[nclass++] = _sample_list->addClassName (pch);
		pch = strtok (NULL, " ");
	}

	_iter_count = 0;
	_class_count = 0;	

	_event.block ();

	ssi_print_off ("ready to record sample (%u/%u)", _iter_count+1, _options.iter);
	if (_n_classes > 1) {
		ssi_print (" to model %s (%u/%u)...", _sample_list->getClassName (_class_index[_class_count]), _class_count+1, _n_classes); 
	} else {
		ssi_print (" to model %s", _sample_list->getClassName (_class_index[_class_count]));
	}
	ssi_print ("\n");
}

void Collector::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {
	
	if (_class_count < _n_classes) {

		bool add_sample = true;
		if (_options.ask) {
			ssi_print_off("add training sample? (j/n)\n");
			ssi_print_off("> ");
			scanf ("%s", _input);
			if (_input[strlen(_input)-1] == 'n') {
				add_sample = false;
				ssi_print ("sample skipped!\n");
			}
		}
		if (add_sample) {
			ssi_sample_t *sample = new ssi_sample_t;
			sample->class_id = _class_index[_class_count];
			sample->user_id = _user_index;
			sample->score = 0.0f;
			sample->num = stream_in_num;
			sample->time = consume_info.time;
			ssi_stream_t **streams = new ssi_stream_t *[stream_in_num];
			for (unsigned int i = 0; i < stream_in_num; i++) {
				ssi_stream_t *stream = new ssi_stream_t;
				ssi_stream_init (*stream, stream_in[i].num, stream_in[i].dim, stream_in[i].byte, stream_in[i].type, stream_in[i].sr);
				memcpy (stream->ptr, stream_in[i].ptr, stream->tot);								
				streams[i] = stream;								
			}	
			sample->streams = streams;
			_sample_list->addSample (sample);
			_iter_count++;
			ssi_print_off("sample stored!\n");
		}

		if (_iter_count == _options.iter) {
			_class_count++;
			if (_class_count == _n_classes) {
				_event.release ();
				return;
			} else {
				_iter_count = 0;
			}
		}
		ssi_print_off("ready to record sample (%u/%u)", _iter_count + 1, _options.iter);
		if (_n_classes > 1) {
			ssi_print (" to model %s (%u/%u)...", _sample_list->getClassName (_class_index[_class_count]), _class_count+1, _n_classes); 
		} else {
			ssi_print (" to model %s", _sample_list->getClassName (_class_index[_class_count]));
		}
		ssi_print ("\n");
	}
}

void Collector::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {
	
	delete[] _class_index;
	_sample_list->reset ();
}

}
