// MyMouseConsumer.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2012/04/24
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

#include "MyMouseConsumer.h"
#include "ioput/file/FileTools.h"
#include "base/ITheFramework.h"
#include "base/Factory.h"

namespace ssi {

char MyMouseConsumer::ssi_log_name[] = "mymouse_c_";

MyMouseConsumer::MyMouseConsumer (const ssi_char_t *file)
	:	_file (0),
		_listener (0)
		{

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init (_event, SSI_ETYPE_MAP);	

}

MyMouseConsumer::~MyMouseConsumer () {
	
	ssi_event_destroy (_event);

}

bool MyMouseConsumer::setEventListener (IEventListener *listener) {

	_listener = listener;
	_event.sender_id = Factory::AddString (_options.sname);
	if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	_event.event_id = Factory::AddString (_options.ename);
	if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}

	_event_address.setSender (_options.sname);
	_event_address.setEvents (_options.ename);

	return true;

}

void MyMouseConsumer::listen_enter (){

	if (_listener) {
		ssi_event_adjust (_event, sizeof (ssi_event_map_t));
		ssi_event_map_t *e = ssi_pcast (ssi_event_map_t, _event.ptr);
		e[0].id = Factory::AddString("ButtonDuration");
	}

}

bool MyMouseConsumer::update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {

	ssi_event_t *e = 0;
	events.reset ();
	for(ssi_size_t nevent = 0; nevent < n_new_events; nevent++){
		e = events.next ();
		ssi_size_t dur = e->dur;
		e = 0;

		if (_listener) {
			_event.dur = time_ms - _event.time;
			_event.time = time_ms;
			
			ssi_event_map_t *e = ssi_pcast (ssi_event_map_t, _event.ptr);
			e[0].value = ssi_real_t(dur);

			_listener->update (_event);
		}
	}

	return true;
}

void MyMouseConsumer::listen_flush (){

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}

	if (_listener) {
		ssi_event_reset (_event);
	}

}

}
