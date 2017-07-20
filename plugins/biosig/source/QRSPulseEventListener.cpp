// QRSPulseEventListener.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2013/01/09
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

#include "../include/QRSPulseEventListener.h"
#include "base/ITheEventBoard.h"
#include "base/ITheFramework.h"
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

ssi_char_t *QRSPulseEventListener::ssi_log_name = "qrs_pulse_";

QRSPulseEventListener::QRSPulseEventListener (const ssi_char_t *file)
	: _file (0),
	_listener (0),
	_update_counter (0),
	_update_ms (0),
	_last_event_ms (0),
	_send_etuple (false),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}


QRSPulseEventListener::~QRSPulseEventListener() {

	if (_listener) {
		ssi_event_destroy (_pulse_event);
	}

}

bool QRSPulseEventListener::setEventListener (IEventListener *listener) {	

	_listener = listener;
	_send_etuple = _options.tuple;

	if (_send_etuple) {
		ssi_event_init (_pulse_event, SSI_ETYPE_MAP);
		ssi_event_adjust (_pulse_event, 1 * sizeof (ssi_event_map_t));
		ssi_event_map_t *ptr = ssi_pcast (ssi_event_map_t, _pulse_event.ptr);
		ptr[0].id = Factory::AddString ("pulse");
	} else {
		ssi_event_init (_pulse_event, SSI_ETYPE_TUPLE);	
		ssi_event_adjust (_pulse_event, 1 * sizeof (ssi_real_t));
	}

	_pulse_event.sender_id = Factory::AddString (_options.sname);
	if (_pulse_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	_pulse_event.event_id = Factory::AddString (_options.ename);
	if (_pulse_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}

	_event_address.setSender (_options.sname);
	_event_address.setEvents (_options.ename);

	return true;
}

void QRSPulseEventListener::listen_enter (){
}

bool QRSPulseEventListener::update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {

	if (time_ms > _options.span) {

		_update_ms = _options.update_ms;
		
		ssi_size_t count = 0;
		ssi_event_t *e = 0;
		events.reset ();
		while (e = events.next ()) {
			count++;		
		}

		ssi_real_t pulse = (60000.0f / _options.span) * count;
		if (_listener) {
			if( _update_counter * _update_ms <= time_ms && (time_ms - _last_event_ms >= _update_ms)){
				_pulse_event.time = time_ms;
				_pulse_event.dur = _options.span;

				if (_send_etuple) {
					ssi_event_map_t *ptr = ssi_pcast (ssi_event_map_t, _pulse_event.ptr);
					ptr[0].value = pulse;
				} else {
					ssi_real_t *ptr = ssi_pcast (ssi_real_t, _pulse_event.ptr);
					ptr[0] = pulse;
				}

				_pulse_event.state = SSI_ESTATE_COMPLETED;
				_listener->update (_pulse_event);
				_update_counter++;
				_last_event_ms = time_ms;
			}
		}
	}

	return true;	
}

void QRSPulseEventListener::listen_flush (){


}

}
