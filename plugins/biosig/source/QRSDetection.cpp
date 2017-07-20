// QRSDetection.cpp
// author: Florian Lingenfelser <lingenfelser@hcm-lab.de>
// created: 2013/04/08 
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

#include "../include/QRSDetection.h"
#include "../../signal/include/Butfilt.h"
#include "../../signal/include/Derivative.h"
#include "../include/QRSPreProcess.h"
#include "../include/QRSDetect.h"
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

QRSDetection::QRSDetection (const ssi_char_t *file) 
:	_file (0),
	_bandpass (0),
	_diff (0),
	_pre (0),
	_pre_low (0),
	_listener (0),
	_frame_count (0),
	_signal_level (0),
	_noise_level (0),
	_thres1 (0),
	_thres2 (0),
	_pulsed (false),
	_n_R (0),
	_samples_since_last_R (0),
	_head_RR (0),
	_history_RR (0),
	_sum_RR (0),
	_average_RR (0),
	_low_limit_RR (0),
	_high_limit_RR (0),
	_last_R (0),
	_rising (true),
	_send_etuple (false),
	_first_call (true)
	{

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

QRSDetection::~QRSDetection () {

	if (_listener) {
		ssi_event_destroy (_r_event);
	}

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool QRSDetection::setEventListener (IEventListener *listener) {
	_listener = listener;	

	_send_etuple = _options.tuple;
	if(_send_etuple) {
		ssi_event_init (_r_event, SSI_ETYPE_MAP);
		ssi_event_adjust (_r_event, 1 * sizeof (ssi_event_map_t));
		ssi_event_map_t *ptr = ssi_pcast (ssi_event_map_t, _r_event.ptr);
		ptr[0].id = Factory::AddString ("delta");
	} else {
		ssi_event_init (_r_event, SSI_ETYPE_TUPLE);
		ssi_event_adjust (_r_event, 1 * sizeof (ssi_real_t));
	}

	_r_event.sender_id = Factory::AddString (_options.sname);
	if (_r_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	_r_event.event_id = Factory::AddString (_options.ename);
	if (_r_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	
	_event_address.setSender (_options.sname);
	_event_address.setEvents (_options.ename);

	return true;
}

void QRSDetection::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	//bandpass
	Butfilt *bandpass = ssi_pcast (Butfilt, Factory::Create (Butfilt::GetCreateName (), 0, false));
	bandpass->getOptions()->type = Butfilt::BAND;
	bandpass->getOptions()->norm = false;
	bandpass->getOptions()->high = 15;
	bandpass->getOptions()->low = 5;
	bandpass->getOptions()->order = 13;
	_bandpass = bandpass;
	ssi_stream_init (_bandpass_stream, 0, _bandpass->getSampleDimensionOut (stream_in.dim), _bandpass->getSampleBytesOut (stream_in.byte), _bandpass->getSampleTypeOut (stream_in.type), stream_in.sr);
	_bandpass->transform_enter (stream_in, _bandpass_stream);

	//diff
	Derivative *diff = ssi_pcast(Derivative, Factory::Create (Derivative::GetCreateName (), 0, false));
	ssi_strcpy (diff->getOptions()->names, "1st");
	_diff = diff;
	ssi_stream_init (_diff_stream, 0, _diff->getSampleDimensionOut (_bandpass_stream.dim), _diff->getSampleBytesOut (_bandpass_stream.byte), _diff->getSampleTypeOut (_bandpass_stream.type), _bandpass_stream.sr);
	_diff->transform_enter (_bandpass_stream, _diff_stream);

	//qrs-pre-process
	QRSPreProcess *pre = ssi_pcast(QRSPreProcess, Factory::Create (QRSPreProcess::GetCreateName (), 0, false));
	_pre = pre;
	ssi_stream_init (_pre_stream, 0, _pre->getSampleDimensionOut (_diff_stream.dim), _pre->getSampleBytesOut (_diff_stream.byte), _pre->getSampleTypeOut (_diff_stream.type), _diff_stream.sr);
	_pre->transform_enter(_diff_stream, _pre_stream);

	Butfilt *pre_low = ssi_create (Butfilt, 0, false);
	pre_low->getOptions()->zero = true;
	pre_low->getOptions()->norm = false;
	pre_low->getOptions ()->low = 6.4;
	pre_low->getOptions ()->order = 3;
	pre_low->getOptions ()->type = Butfilt::LOW;	
	_pre_low = pre_low;
	ssi_stream_init (_pre_low_stream, 0, _pre_low->getSampleDimensionOut (_pre_stream.dim), _pre_low->getSampleBytesOut (_pre_stream.byte), _pre_low->getSampleTypeOut (_pre_stream.type), _pre_stream.sr);
	_pre_low->transform_enter(_pre_stream, _pre_low_stream);

	//qrs-detect
	ssi_size_t sample_dimension = _pre_low_stream.dim;
	_pulsed = false;
	_n_R = 0;
	_samples_since_last_R = 0;
	_sum_RR = 0;
	_average_RR = 0.0f;
	_history_RR = new ssi_size_t[_options.depthRR];
	_last_R = 0;
	for (ssi_size_t k = 0; k < _options.depthRR; k++) {
		_history_RR[k] = 0;
	}

	_first_call = true;
		
}

void QRSDetection::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t n = stream_in.num;
	ssi_time_t sr = stream_in.sr;

	ssi_stream_adjust (_bandpass_stream, n);
	ssi_stream_adjust (_diff_stream, n);
	ssi_stream_adjust (_pre_stream, n);
	ssi_stream_adjust (_pre_low_stream, n);
				
	_bandpass->transform (info, stream_in, _bandpass_stream);
	_diff->transform (info, _bandpass_stream, _diff_stream);
	_pre->transform (info, _diff_stream, _pre_stream);
	_pre_low->transform (info, _pre_stream, _pre_low_stream);

	//qrs-detect
	ssi_size_t sample_dimension = _pre_low_stream.dim;
	ssi_size_t sample_number = _pre_low_stream.num;
	
	SSI_ASSERT(sample_dimension == 1);

	ssi_real_t *ptr_in = ssi_pcast (ssi_real_t, _pre_low_stream.ptr);
	ssi_real_t *ptr_out = ssi_pcast (ssi_real_t, stream_out.ptr);

	if (_first_call) {

		_frame_count = 0;

		//init
		ssi_real_t average = 0.0f;
		ssi_real_t avg_tmp = 0.0f;
		ssi_size_t n_avg = sample_number;

		for (ssi_size_t nsamples = 0; nsamples < sample_number; nsamples++) {
			if(*ptr_in != 0.0f){
				avg_tmp += *ptr_in++;
			}else{
				n_avg--;
				ptr_in++;
			}
			*ptr_out++ = 0;
		}
		if(n_avg != 0){
			average = avg_tmp / ssi_cast(ssi_real_t, n_avg);
			_rising = true;
			_first_call = false;
		}else{
			average = 0.0f;
		}

		_noise_level = average - average * 0.1f;
		_signal_level = average + average * 0.1f;

		_thres1 = _noise_level + 0.25f*(_signal_level - _noise_level);
		_thres2 = 0.5f * _thres1;

	}else{

		_frame_count++;

		for (ssi_size_t nsamples = 0; nsamples < sample_number-1; nsamples++) {

			if(*ptr_in <= *(ptr_in+1)){
				*ptr_out = 0.0f;
				_rising = true;
			}

			//found peak?
			if(_rising && (*ptr_in > *(ptr_in+1))){
				//found R?
				if(*ptr_in >= _thres1  || _samples_since_last_R > 2*_pre_low_stream.sr){
					//signal peak with t1
					//skip #_options.depthRR first R's and init history_RR and sum_RR
					if(_n_R < _options.depthRR){
						_history_RR[_n_R] = _samples_since_last_R;
						_n_R++;
						_sum_RR += _samples_since_last_R;
						_samples_since_last_R = 0;
					}else{
						_pulsed = true;
					}


					if(_pulsed){

						//pulsed
						_average_RR = _sum_RR / ssi_cast(ssi_real_t, _options.depthRR);
						_low_limit_RR  = _options.lowerLimitRR * _average_RR;
						_high_limit_RR = _options.upperLimitRR * _average_RR;

						/*ssi_print ("\nRR-history\n");
						for (ssi_size_t k = 0; k < _options.depthRR; k++) {
							ssi_print ("%d ", _history_RR[k]);
						}ssi_print ("\n");
						ssi_print ("RR-sum\n");
						ssi_print ("%d ", _sum_RR);				
						ssi_print("\nAverage over %d R's:\t%.1f\nCurrent:\t\t%d\n", _options.depthRR, _average_RR, _samples_since_last_R);
						ssi_print("low_limit:\t%.1f\nhigh_limit:\t%.1f\n", _low_limit_RR, _high_limit_RR);*/

						if( _low_limit_RR <= ssi_cast(ssi_real_t, _samples_since_last_R)){
							//R accepted and published
							_sum_RR = _sum_RR + _samples_since_last_R - _history_RR[_head_RR];
							_history_RR[_head_RR] = _samples_since_last_R;
							_head_RR = (_head_RR + 1) % _options.depthRR;
							*ptr_out = 1.0f;
							_samples_since_last_R = 0;
							if(_options.sendEvent){
								sendEvent_h(info, _pre_low_stream.sr, _frame_count, nsamples);
							}
						}else{
							//T-wave
							*ptr_out = 0.0f;
							_samples_since_last_R++;
						}

					}else{
						//not pulsed
						*ptr_out = 0.0f;
						_samples_since_last_R++;
					}

					_signal_level = 0.125f*(*ptr_in) + 0.875f*_signal_level;
					

				}else{

					if(_pulsed){
						//t2 necessary?
						if(ssi_cast(ssi_real_t, _samples_since_last_R) >= _high_limit_RR){
							if(*ptr_in >= _thres2){
								//signal peak with t2
								_noise_level = 0.25f*(*ptr_in) + 0.75f*_noise_level;
								*ptr_out = 1.0f;
								_samples_since_last_R = 0;
								if(_options.sendEvent){
									sendEvent_h(info, _pre_low_stream.sr, _frame_count, nsamples);
								}

							}else{
								//noise peak
								_noise_level = 0.125f*(*ptr_in) + 0.875f*_noise_level;
								*ptr_out = 0.0f;
								_samples_since_last_R++;
							}

						}else{
							//noise peak
							_noise_level = 0.125f*(*ptr_in) + 0.875f*_noise_level;
							*ptr_out = 0.0f;
							_samples_since_last_R++;
						}

					}else{
						//not pulsed
						//noise peak
						_noise_level = 0.125f*(*ptr_in) + 0.875f*_noise_level;
						*ptr_out = 0.0f;
						_samples_since_last_R++;	
					}
				}

				_rising = false;

			}else{
				//found no peak
				*ptr_out = 0.0f;
				_samples_since_last_R++;
			}
						
			ptr_in++;
			ptr_out++;

			_thres1 = _noise_level + 0.25f*(_signal_level - _noise_level);
			_thres2 = 0.5f * _thres1;

		}

		//last sample
		*ptr_out = 0.0f;
		_samples_since_last_R++;

	}
				
	/*ssi_real_t *ptr_result = ssi_pcast (ssi_real_t, _pre_low_stream.ptr);
	ssi_real_t *ptr_out = ssi_pcast(ssi_real_t, stream_out.ptr);
	for(ssi_size_t nsamp = 0; nsamp < stream_in.num; nsamp++){
		*ptr_out = *ptr_result;
		ptr_result++;
		ptr_out++;
	}*/
}

void QRSDetection::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num ,
	ssi_stream_t xtra_stream_in[] ) {

	_pre_low->transform_flush (_pre_stream, _pre_low_stream);
	ssi_stream_reset (_pre_low_stream);
	delete _pre_low; _pre_low = 0;

	_pre->transform_flush (_diff_stream, _pre_stream);
	ssi_stream_reset (_pre_stream);
	delete _pre; _pre = 0;

	_diff->transform_flush (_bandpass_stream, _diff_stream);
	ssi_stream_reset (_diff_stream);
	delete _diff; _diff = 0;

	_bandpass->transform_flush (stream_in, _bandpass_stream);
	ssi_stream_reset (_bandpass_stream);
	delete _bandpass; _bandpass = 0;

	delete[] _history_RR;
	_history_RR = 0;

}

void QRSDetection::sendEvent_h(ITransformer::info &info, ssi_time_t sample_rate, ssi_size_t frame_count, ssi_size_t sample_number){

	ssi_size_t samples_per_frame = info.frame_num;
	ssi_size_t frame_time = ssi_sec2ms (info.time);
	ssi_real_t time_per_sample = 1000.0f / (ssi_real_t) sample_rate;

	ssi_size_t current_sample = frame_count * samples_per_frame + sample_number;
	ssi_size_t current_sample_time = ssi_cast (ssi_size_t, current_sample * time_per_sample + 0.5);

	if (_listener) {
		_r_event.time = current_sample_time;
		_r_event.dur = ssi_cast (ssi_size_t, time_per_sample + 0.5);

		if(_send_etuple) {
			ssi_event_map_t *ptr = ssi_pcast (ssi_event_map_t, _r_event.ptr);
			ptr[0].value = ssi_real_t (current_sample_time - _last_R);
		} else {
			ssi_real_t *ptr = ssi_pcast (ssi_real_t, _r_event.ptr);
			ptr[0] = ssi_real_t(current_sample_time - _last_R);
		}

		_listener->update (_r_event);
	} else {
		ssi_print("\nR-spike detected in \n\tframe number %u with starting time %u ms", frame_count, frame_time);
		ssi_print("\n\tat sample number %u at time %u ms", current_sample, current_sample_time);
	}

	_last_R = current_sample_time;
}

}
