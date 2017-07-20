// QRSHRVtime.cpp
// author: Florian Lingenfelser <lingenfelser@hcm-lab.de>
// created: 2013/03/08
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
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

#include "QRSHRVtime.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

QRSHRVtime::QRSHRVtime(const ssi_char_t *file)
	:	calibrated (false),
		_time_per_sample (0),
		_first_R (0),
		_samples_since_last_R (0),
		_file (0) {		

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

QRSHRVtime::~QRSHRVtime() {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void QRSHRVtime::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

		_time_per_sample = ssi_real_t(1000.0 / stream_in.sr);

		_first_R = 0;
		_samples_since_last_R = 0;

		calibrated = false;

}

void QRSHRVtime::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

		ssi_size_t n_samples = stream_in.num;
		ssi_real_t *ptr_in = ssi_pcast (ssi_real_t, stream_in.ptr);
		ssi_real_t *ptr_out = ssi_pcast (ssi_real_t, stream_out.ptr);

		ssi_real_t RR = 0.0f;
		ssi_real_t sumRR = 0.0f;
		
		ssi_size_t nR = 0;
		ssi_real_t mRR = 0.0f;
		ssi_real_t mHR = 0.0f;
		ssi_real_t SDRR = 0.0f;
		ssi_real_t SDHR = 0.0f;
		ssi_real_t CVRR = 0.0f;
		ssi_real_t RMSSD = 0.0f;
		ssi_real_t pRR20 = 0.0f;
		ssi_real_t pRR50 = 0.0f;
		
		for(ssi_size_t nsamp = 0; nsamp < n_samples; nsamp++){

			if (_first_R == 0){

				if (*ptr_in == 1) {
					_first_R = _samples_since_last_R;
					nR++;
					_samples_since_last_R = 0;
				}else{
					_samples_since_last_R++;
				}
				
			}else{
				
				if (*ptr_in == 1) {
					RR = ssi_real_t(_samples_since_last_R) * _time_per_sample;
					_RR_intervals.push_back(RR);
					nR++;
					_samples_since_last_R = 0;
				}else{
					_samples_since_last_R++;
				}

			}//first R

			ptr_in++;

		}//for nsamp ..

		if (_first_R != 0){
			calibrated = true;
		}

		if(calibrated && _RR_intervals.size() > 1){

			//calc features
			ssi_size_t n_rrs = ssi_size_t(_RR_intervals.size());
			for(ssi_size_t n_rr = 0; n_rr < n_rrs; n_rr++){
				sumRR += _RR_intervals.at(n_rr);
				mHR += (60000.0f / _RR_intervals.at(n_rr));
			}
			mRR = sumRR / ssi_real_t(n_rrs);
			mHR = mHR / ssi_real_t(n_rrs);
			for(ssi_size_t n_rr = 0; n_rr < n_rrs; n_rr++){
				SDRR += pow(_RR_intervals.at(n_rr) - mRR, 2);
				SDHR += pow((60000.0f / _RR_intervals.at(n_rr)) - mHR, 2);
			}
			SDRR = SDRR / (ssi_real_t(n_rrs) - 1.0f);
			SDRR = sqrt(SDRR);
			SDHR = SDHR / (ssi_real_t(n_rrs) - 1.0f);
			SDHR = sqrt(SDHR);
			CVRR = (SDRR * 100.0f) / mRR;
			for(ssi_size_t n_rr = 0; n_rr < n_rrs - 1; n_rr++){
				RMSSD += pow(_RR_intervals.at(n_rr + 1) - _RR_intervals.at(n_rr), 2);
				if(abs(_RR_intervals.at(n_rr + 1) - _RR_intervals.at(n_rr)) > 20.0f){
					pRR20++;
					if(abs(_RR_intervals.at(n_rr + 1) - _RR_intervals.at(n_rr)) > 50.0f){
						pRR50++;	
					}
				}
			}
			RMSSD = RMSSD / (ssi_real_t(n_rrs) - 1.0f);
			RMSSD = sqrt(RMSSD);
			pRR20 = (pRR20 * 100.0f) / (ssi_real_t(n_rrs) - 1.0f);
			pRR50 = (pRR50 * 100.0f) / (ssi_real_t(n_rrs) - 1.0f);

			if(_options.print){
				ssi_print("\nTIME DOMAIN FEATURES");
				ssi_print("\n-------------------------");
				//ssi_print("\nnR:\t%u\t(# of R-spikes)", nR);
				ssi_print("\nmRR:\t%.2f\t(mean RR interval in ms)", mRR);
				ssi_print("\nmHR:\t%.2f\t(mean heart rate in bpm)", mHR);
				ssi_print("\nSDRR:\t%.2f\t(stdev of RR intervals in ms)", SDRR);
				ssi_print("\nSDHR:\t%.2f\t(stdev of heart rate in bpm)", SDHR);
				ssi_print("\nCVRR:\t%.2f\t(coefficient of variance of RR's)", CVRR);
				ssi_print("\nRMSSD:\t%.2f\t(root mean square successive difference in ms)", RMSSD);
				ssi_print("\npRR20:\t%.2f\t(# of pairs of RR's differing by >20ms in %%)", pRR20);
				ssi_print("\npRR50:\t%.2f\t(# of pairs of RR's differing by >50ms in %%)", pRR50);				
				ssi_print("\n");

				/*ssi_print("\n\nDEBUG:");
				for(ssi_size_t i = 0; i < _RR_intervals.size(); i++){
					ssi_print("\nRR:\t%.2f", _RR_intervals.at(i));
				}
				for(ssi_size_t i = 0; i < _RR_intervals.size() - 1; i++){
					ssi_print("\ndiff:\t%.2f", abs(_RR_intervals.at(i + 1) - _RR_intervals.at(i)));
				}ssi_print("\n");*/

			}


			if (std::isnan(mRR) || std::isinf(mRR)) mRR = 0;
			if (std::isnan(mHR) || std::isinf(mHR)) mHR = 0;
			if (std::isnan(SDRR) || std::isinf(SDRR)) SDRR = 0;
			if (std::isnan(SDHR) || std::isinf(SDHR)) SDHR = 0;

			*ptr_out = mRR;
			ptr_out++;
			*ptr_out = mHR;
			ptr_out++;
			*ptr_out = SDRR;
			ptr_out++;
			*ptr_out = SDHR;
			ptr_out++;
			*ptr_out = CVRR;
			ptr_out++;
			*ptr_out = RMSSD;
			ptr_out++;
			*ptr_out = pRR20;
			ptr_out++;
			*ptr_out = pRR50;

		}else{

			if(_options.print){
				ssi_print("\nTIME DOMAIN FEATURES");
				ssi_print("\n-------------------------");
				ssi_print("\ncalibrating ..");
				ssi_print("\n");
			}

			*ptr_out = 0.0f;
			ptr_out++;
			*ptr_out = 0.0f;
			ptr_out++;
			*ptr_out = 0.0f;
			ptr_out++;
			*ptr_out = 0.0f;
			ptr_out++;
			*ptr_out = 0.0f;
			ptr_out++;
			*ptr_out = 0.0f;
			ptr_out++;
			*ptr_out = 0.0f;
			ptr_out++;
			*ptr_out = 0.0f;

		}//calibrated
		
		_RR_intervals.clear();

}

void QRSHRVtime::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num ,
	ssi_stream_t xtra_stream_in[]) {

		calibrated = false;
		_RR_intervals.clear();

}

}
