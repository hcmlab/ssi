// TupleSelect.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2013/10/15
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

#include "../include/TupleSelect.h"
#include "ioput/file/FileTools.h"
#include "base/ITheFramework.h"
#include "base/Factory.h"
#include "SSI_Tools.h"

namespace ssi {

char TupleSelect::ssi_log_name[] = "t_select__";

TupleSelect::TupleSelect (const ssi_char_t *file)
:	_file (0),
	_listener (0),
	_dim(0),
	_max (0),
	_indices (0)
	{

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init (_event, SSI_ETYPE_MAP);	

}

TupleSelect::~TupleSelect () {
	
	ssi_event_destroy (_event);

}

bool TupleSelect::setEventListener (IEventListener *listener) {

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

void TupleSelect::listen_enter () {

	int *indices = ssi_parse_indices (_options.indices, _dim);
	if (_dim == 0) {
		ssi_wrn ("could not parse indices");
		return;
	}

	_indices = new ssi_size_t[_dim];
	_max = 0;
	for (ssi_size_t i = 0; i < _dim; i++) {
		_indices[i] = ssi_cast (ssi_size_t, indices[i]);
		if (_indices[i] > _max) {
			_max = _indices[i];
		}
	}

	if (_listener) {
		ssi_event_adjust (_event, _dim * sizeof (ssi_event_map_t));
	}

}

bool TupleSelect::update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {

	if (_dim == 0) {
		ssi_wrn ("no selection set yet");
		return false;
	}

	ssi_event_t *e = 0;
	events.reset ();

	for(ssi_size_t nevent = 0; nevent < n_new_events; nevent++){
		
		e = events.next();

		if (_listener) {

			if(e->type == SSI_ETYPE_MAP){

				_event.dur = e->dur;
				_event.time = e->time;		
				ssi_event_map_t *ptr = ssi_pcast (ssi_event_map_t, _event.ptr);
				ssi_event_map_t *in_ptr = ssi_pcast (ssi_event_map_t, e->ptr);

				ssi_size_t dim = e->tot / sizeof (ssi_event_map_t);
				if (dim < _max) {
					ssi_wrn ("#event does not match selection");
					return false;
				}
			
				for(ssi_size_t ndim = 0; ndim < _dim; ndim++){
					ptr[ndim].id = in_ptr[_indices[ndim]].id; 
					ptr[ndim].value = in_ptr[_indices[ndim]].value;
				}

				_listener->update (_event);

			}else{

				ssi_wrn ("event type not supported");
				return false;

			}
					
			e = 0;
		}
	}

	return true;
}

void TupleSelect::listen_flush (){

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}

	if (_listener) {
		ssi_event_reset (_event);
	}

	_dim = 0;
	_max = 0;
	delete[] _indices; _indices = 0;

}


}
