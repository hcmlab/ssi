// GSRBaselineMean.cpp
// author: Florian Lingenfelser <lingenfelser@hcm-lab.de>
// created: 2013/02/13
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

#include "GSRBaselineMean.h"
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

ssi_char_t *GSRBaselineMean::ssi_log_name = "gsrmean__";

GSRBaselineMean::GSRBaselineMean (const ssi_char_t *file)
	: _file (0),
	_listener (0),
	_src_sr (0),
	_sample_counter (0),
	_win_samples (0),
	_sample_sum (0),
	_last_mean (0),
	firstrun (true),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init (_event, SSI_ETYPE_TUPLE);
}

GSRBaselineMean::~GSRBaselineMean () {

	ssi_event_destroy (_event);

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

bool GSRBaselineMean::setEventListener (IEventListener *listener) {

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

void GSRBaselineMean::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (stream_in[0].type != SSI_REAL) {
		ssi_err ("type '%s' not supported", SSI_TYPE_NAMES[stream_in[0].type]);
	}

	if (stream_in[0].dim != 1) {
		ssi_err ("dimension > 1 not supported");
	}

	_src_sr = stream_in[0].sr;
	
	ssi_event_adjust (_event, 1 * sizeof (ssi_real_t));

	_win_samples = ssi_real_t(_src_sr * ssi_real_t(_options.winsize));

	firstrun = true;
	
}

void GSRBaselineMean::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {
	
	ssi_size_t n_samples = stream_in[0].num;
	ssi_real_t *ptr_in = ssi_pcast (ssi_real_t, stream_in[0].ptr);
	
	for(ssi_size_t nsamp = 0; nsamp < n_samples; nsamp++){

		_sample_sum += *ptr_in++;
		_sample_counter++;

		ssi_real_t mean = 0.0f;
		ssi_real_t p_change = 0.0f;

		if(_sample_counter >= _win_samples){

			if(firstrun){

				firstrun = false;
				mean = _sample_sum / _sample_counter;

			}else{

				ssi_size_t frame_time = ssi_sec2ms (consume_info.time);
				ssi_real_t time_per_sample = 1000.0f / (ssi_real_t) _src_sr;
				ssi_size_t current_sample_time = ssi_cast (ssi_size_t, _sample_counter * time_per_sample + 0.5);

				
				mean = _sample_sum / _sample_counter;
				p_change = mean / _last_mean - 1.0f;
				
				/*ssi_print("\nLastMean:\t\t\t%.2f\n", _last_mean);
				ssi_print("Mean:\t\t\t\t%.2f\n", mean);*/

				if(_options.cap){
					if(p_change > 1.0f){
						p_change = 1.0f;
					}else if(p_change < -1.0f){
						p_change = -1.0f;
					}
				}
				

				if (_listener) {
				
					_event.time = frame_time;
					_event.dur = current_sample_time;
					ssi_real_t *ptr = ssi_pcast (ssi_real_t, _event.ptr);
					ptr[0] = p_change;

					_listener->update (_event);

					/*printf ("change from %.0f to %.0f:\t%.2f\n", frame_time, frame_time + current_sample_time, p_change);*/
			
				} else {
					printf ("change from %u to %u:\t%.2f\n", frame_time, frame_time + current_sample_time, p_change);
				}

			}

			_last_mean = mean;
			_sample_counter = 0;
			_sample_sum = 0.0f;

		}

	}

	n_samples = 0;
	ptr_in = 0;	
	
}

void GSRBaselineMean::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	ssi_event_reset (_event);
}

}
