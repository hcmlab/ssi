// GSRResponseEventSender.h
// author: Fabian Hertwig <fabian.hertwig@student.uni-augsburg.de>,
//         Florian Obermayer <florian.obermayer@student.uni-augsburg.de>
// created: 2015/01/16
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

#include "GSRResponseEventSender.h"
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

	char GSRResponseEventSender::ssi_log_name[] = "gsrresponseeventsender__";

	GSRResponseEventSender::GSRResponseEventSender(const ssi_char_t *file):
		_file(0),
		_elistener(0),
		_send_as_tuple(false),
		_etuple_min_id(0),
		_etuple_max_id(0),
		_etuple_amplitude_id(0),
		_etuple_energy_id(0),
		_etuple_power_id(0),
		_etuple_dur_id(0)
		{

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}
	}

	GSRResponseEventSender::~GSRResponseEventSender() {
		if (_elistener){
			ssi_event_destroy(_event);
		}

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file; _file = 0;
		}
	}

	void GSRResponseEventSender::consume_enter(ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) {

		_response = new GSRResponse(
			_options.minAllowedRegression,
			_options.minAmplitude,
			_options.minRisingTime, 
			_options.print);
	}

	void GSRResponseEventSender::consume(IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) {

		for (ssi_size_t i = 0; i< stream_in[0].num; i++){
			_response->findResponse(stream_in[0].ptr, stream_in[0].sr, i, consume_info.time, this);
		}
	}

	void GSRResponseEventSender::consume_flush(ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) {

		delete _response; _response = 0;
	}


	bool GSRResponseEventSender::setEventListener(IEventListener *listener) {

		_elistener = listener;

		_send_as_tuple = _options.tuple;
		ssi_size_t data_ct = 6;

		if (_send_as_tuple){
			_etuple_min_id = Factory::AddString("minimum");
			_etuple_max_id = Factory::AddString("maximum");
			_etuple_amplitude_id = Factory::AddString("amplitude");
			_etuple_energy_id = Factory::AddString("energy");
			_etuple_power_id = Factory::AddString("power");
			_etuple_dur_id = Factory::AddString("rising time");

			ssi_event_init(_event, SSI_ETYPE_MAP);

			ssi_event_adjust(_event, data_ct * sizeof(ssi_event_map_t));
			ssi_event_map_t *ptr = ssi_pcast(ssi_event_map_t, _event.ptr);
			ptr[SSI_GSR_EVENT_FEATURE_MIN_VAL_INDEX].id = _etuple_min_id;
			ptr[SSI_GSR_EVENT_FEATURE_MAX_VAL_INDEX].id = _etuple_max_id;
			ptr[SSI_GSR_EVENT_FEATURE_AMP_INDEX].id = _etuple_amplitude_id;
			ptr[SSI_GSR_EVENT_FEATURE_ENERGY_INDEX].id = _etuple_energy_id;
			ptr[SSI_GSR_EVENT_FEATURE_POWER_INDEX].id = _etuple_power_id;
			ptr[SSI_GSR_EVENT_FEATURE_RISING_TIME_INDEX].id = _etuple_dur_id;
		}
		else{
			ssi_event_init(_event, SSI_ETYPE_TUPLE);
			ssi_event_adjust(_event, data_ct * sizeof(ssi_real_t));

		}

		_event.sender_id = Factory::AddString(_options.s_name);
		_event.event_id = Factory::AddString(_options.e_name);

		if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}
		if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}

		_event_address.setSender(_options.s_name);
		_event_address.setEvents(_options.e_name);

		return true;
	}

	void GSRResponseEventSender::handleResponse(gsr_response_t response){

		if (_elistener){

			_event.dur = ssi_cast(ssi_size_t, response.duration * 1000);
			_event.time = ssi_cast(ssi_size_t,response.start_time * 1000);
			_event.state = SSI_ESTATE_COMPLETED;
			
			if (_send_as_tuple){
				ssi_event_map_t * ptr = ssi_pcast(ssi_event_map_t, _event.ptr);
				ptr[SSI_GSR_EVENT_FEATURE_MIN_VAL_INDEX].value = response.min_value;
				ptr[SSI_GSR_EVENT_FEATURE_MAX_VAL_INDEX].value = response.max_value;
				ptr[SSI_GSR_EVENT_FEATURE_AMP_INDEX].value = response.max_value - response.min_value;
				ptr[SSI_GSR_EVENT_FEATURE_ENERGY_INDEX].value = response.energy;
				ptr[SSI_GSR_EVENT_FEATURE_POWER_INDEX].value = response.power;
				ptr[SSI_GSR_EVENT_FEATURE_RISING_TIME_INDEX].value = ssi_cast(ssi_real_t,response.duration);
			}
			else{

				ssi_real_t * ptr = ssi_pcast(ssi_real_t, _event.ptr);
				ptr[SSI_GSR_EVENT_FEATURE_MIN_VAL_INDEX] = response.min_value;
				ptr[SSI_GSR_EVENT_FEATURE_MAX_VAL_INDEX] = response.max_value;
				ptr[SSI_GSR_EVENT_FEATURE_AMP_INDEX] = response.max_value - response.min_value;
				ptr[SSI_GSR_EVENT_FEATURE_ENERGY_INDEX] = response.energy;
				ptr[SSI_GSR_EVENT_FEATURE_POWER_INDEX] = response.power;
				ptr[SSI_GSR_EVENT_FEATURE_RISING_TIME_INDEX] = ssi_cast(ssi_real_t, response.duration);
			}
			_elistener->update(_event);

		}
	}
}
