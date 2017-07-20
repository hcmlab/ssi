// QRSHrvEventListener.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2013/01/17
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

#include "../include/QRSHrvEventListener.h"
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

ssi_char_t *QRSHrvEventListener::ssi_log_name = "qrs_hrv___";

QRSHrvEventListener::QRSHrvEventListener (const ssi_char_t *file)
	: _file (0),
	_update_counter (0),
	_update_ms (0),
	_last_event_ms (0),
	_last_hrv (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init (_hrv_event, SSI_ETYPE_TUPLE);	
}


QRSHrvEventListener::~QRSHrvEventListener() {

}

bool QRSHrvEventListener::setEventListener (IEventListener *listener) {

	_listener = listener;
	_hrv_event.sender_id = Factory::AddString (_options.sname);
	if (_hrv_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	_hrv_event.event_id = Factory::AddString (_options.ename);
	if (_hrv_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}

	_event_address.setSender (_options.sname);
	_event_address.setEvents (_options.ename);

	return true;
}

void QRSHrvEventListener::listen_enter (){

	if (_listener) {
		ssi_event_adjust (_hrv_event, 1 * sizeof (ssi_real_t));
	}
	_last_hrv = 0.0f;
}

bool QRSHrvEventListener::update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {

	if (time_ms > _options.span) {

		_update_ms = _options.update_ms;

		ssi_real_t hrv = 0.0f;
		
		ssi_size_t n_events = 0;
		ssi_real_t *events_ms = 0;
		
		ssi_size_t n_rr = 0;
		ssi_real_t* rr_ms = 0;

		ssi_real_t avg_rr = 0.0f;
		
		ssi_real_t *ptr_in = 0;
		ssi_real_t last_ms = 0.0f;

		ssi_event_t *e = 0;

		events.reset ();
		while (e = events.next ()) {
			n_events++;
		}
		n_rr = n_events - 1;
		
		if(n_events > 2){
			
			events_ms = new ssi_real_t [n_events];
			rr_ms = new ssi_real_t [n_rr];
			
			ssi_size_t count = 0;
			events.reset();
			while (e = events.next ()) {
				ptr_in = ssi_pcast(ssi_real_t, e->ptr);
				events_ms[count] = *ptr_in;
				count++;
			}

			for(ssi_size_t i = 0; i < n_rr; i++){
				rr_ms[i] = events_ms[i] - events_ms[i+1];
				avg_rr += rr_ms[i];
			}
			avg_rr /= ssi_cast(ssi_real_t, n_rr);

			ssi_real_t tmp = 0.0f;
			for(ssi_size_t i = 0; i < n_rr; i++){
				hrv += pow((rr_ms[i] - avg_rr), 2.0f);
			}
			hrv /= n_rr;
			hrv = sqrt(hrv);
			
			//clean
			if(events_ms){
				delete [] events_ms;
				events_ms = 0;
			}
			if(rr_ms){
				delete [] rr_ms;
				rr_ms = 0;
			}
			ptr_in = 0;
		
			if (_listener) {
				if( _update_counter * _update_ms <= time_ms && (time_ms - _last_event_ms >= _update_ms)){
					_hrv_event.time = time_ms;
					_hrv_event.dur = 1;
					ssi_real_t *ptr = ssi_pcast (ssi_real_t, _hrv_event.ptr);
					ssi_real_t p_change = 0.0f;

					if(_last_hrv != 0.0f){
						p_change = _options.amplifier * (hrv / _last_hrv - 1.0f);
					}else{
						p_change = 0.0f;
					}

					/*ssi_print("\nLastHRV:\t\t\t%.2f\n", _last_hrv);
					ssi_print("HRV:\t\t\t\t%.2f\n", hrv);*/

					if(p_change > 1.0f){
						p_change = 1.0f;
					}else if(p_change < -1.0f){
						p_change = -1.0f;
					}
				

					ptr[0] = p_change;

					_listener->update (_hrv_event);
					_update_counter++;
					_last_event_ms = time_ms;
					_last_hrv = hrv;
				}
			}

		}
	}
	
	return true;
}

void QRSHrvEventListener::listen_flush (){
	
	if (_listener) {
		ssi_event_reset (_hrv_event);
	}

}

}
