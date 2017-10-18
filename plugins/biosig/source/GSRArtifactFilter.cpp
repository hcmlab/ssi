// GSRArtifactFilter.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2013/05/08
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
#include "../include/GSRArtifactFilter.h"
//#include <float.h>
//#include <math.h>

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

GSRArtifactFilter::GSRArtifactFilter (const ssi_char_t *file) :
	_file (0){

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

}

GSRArtifactFilter::~GSRArtifactFilter () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void GSRArtifactFilter::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

		_samples_to_interpolate = 0;
		_last_valid_value = 0.0f;

		_offset = 0.0f;

		interpolating = false;
		firstrun = true;
}

void GSRArtifactFilter::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t sample_dimension = stream_in.dim;
	SSI_ASSERT(sample_dimension == 1);
	ssi_size_t sample_number = stream_in.num;
	ssi_size_t sample_frames = info.frame_num;
	ssi_size_t sample_deltas = info.delta_num;
	ssi_real_t sample_rate = ssi_real_t(stream_in.sr);
	ssi_size_t winsize_in_samp = ssi_size_t( (sample_frames - 0.5f) * _options.winsize  );
	
	ssi_real_t *ptr_in = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *ptr_out = ssi_pcast (ssi_real_t, stream_out.ptr);

	ssi_real_t* sample_buffer = new ssi_real_t[sample_number];

	if(firstrun){

		/* EDA = (EDA-EDA_range(1))/range(EDA_range); %Normalize
		EDA = max(0,EDA); EDA = min(1,EDA); %limit to values within 0 1 */
		for(ssi_size_t nsamp = 0; nsamp < sample_number; nsamp++){

			ssi_real_t value = 0.0f;

			value = (*ptr_in - _options.lower_range) / (_options.upper_range - _options.lower_range);
			if(value < 0.0f){ value = 0.0f; }
			if(value > 1.0f){ value = 1.0f; }

			sample_buffer[nsamp] = value;
			ptr_in++;

		}

		_last_valid_value = sample_buffer[sample_frames - 1];

		firstrun = false;

	}else{

		/* EDA = (EDA-EDA_range(1))/range(EDA_range); %Normalize
		EDA = max(0,EDA); EDA = min(1,EDA); %limit to values within 0 1 */
		for(ssi_size_t nsamp = 0; nsamp < sample_number; nsamp++){

			ssi_real_t value = 0.0f;

			ssi_real_t lower_range = _options.lower_range;
			ssi_real_t upper_range = _options.upper_range;

			value = (*ptr_in - lower_range) / (upper_range - lower_range);
			if(value < 0.0f){ value = 0.0f; }
			if(value > 1.0f){ value = 1.0f; }

			sample_buffer[nsamp] = value;
			ptr_in++;

		}

		/* Find Artifacts */
		for(ssi_size_t nsamp = 0; nsamp < sample_frames; nsamp++){

			ssi_real_t value = 0.0f;

            if(std::isfinite(sample_buffer[nsamp])  && std::isfinite(sample_buffer[nsamp + winsize_in_samp] )){
				value = abs(sample_buffer[nsamp] - sample_buffer[nsamp + winsize_in_samp]) * 100.0f;
			}

			if(value >= _options.thres){
				value = FLT_MAX;
			}else{
				value = sample_buffer[nsamp];
			}
				
			sample_buffer[nsamp] = value;

		}

		/* Mark Samples */
		for(ssi_size_t nsamp = 0; nsamp < sample_frames; nsamp++){

			ssi_real_t value = sample_buffer[nsamp];

			if (value == FLT_MAX){
				mark_samples(info, nsamp, sample_buffer);
			}

			if(_samples_to_interpolate != 0){
				sample_buffer[nsamp] = FLT_MAX;
				_samples_to_interpolate--;
			}

		}

		/* Interpolate  */
		interpolate(info, sample_buffer);


	}//firstrun

	/* Write to Stream */
	for(ssi_size_t nsamp = 0; nsamp < sample_frames; nsamp++){

        if(std::isfinite(sample_buffer[nsamp] )){
			*ptr_out = sample_buffer[nsamp];
		}else{
			*ptr_out = 0.0f;
		}
		ptr_out++;

	}

	
	delete [] sample_buffer;
	
}

void GSRArtifactFilter::interpolate(ITransformer::info &info, ssi_real_t* sample_buffer){
		
	ssi_size_t sample_frames = info.frame_num;
	ssi_size_t sample_deltas = info.delta_num;
	ssi_size_t sample_number = sample_frames + sample_deltas;

	

	for(ssi_size_t nsamp = 0; nsamp < sample_frames; nsamp++){
        if(!std::isfinite(sample_buffer[nsamp])){
			sample_buffer[nsamp] = _last_valid_value;
			interpolating = true;
		}else{
			if(interpolating){
				_offset = _last_valid_value - sample_buffer[nsamp];
			}
			_last_valid_value = sample_buffer[nsamp] + _offset;
			sample_buffer[nsamp] = _last_valid_value;
			interpolating = false;
		}
	}

}

void GSRArtifactFilter::mark_samples(ITransformer::info &info, ssi_size_t n_sample, ssi_real_t* sample_buffer){
	
	ssi_size_t lower_bound = 0;
	ssi_size_t upper_bound = 0;
	
	ssi_size_t sample_frames = info.frame_num;
	ssi_size_t sample_deltas = info.delta_num;
	ssi_size_t sample_number = sample_frames + sample_deltas;
	ssi_size_t winsize_in_samp = ssi_size_t( (sample_frames - 0.5f) * _options.replacement  );

	/* lower bound */
	if(n_sample - winsize_in_samp <= 0){
		lower_bound = 0;
	}else{
		lower_bound = n_sample - winsize_in_samp;
	}
	/* upper bound */
	if(n_sample + winsize_in_samp >= sample_frames){
		upper_bound = sample_frames;
		_samples_to_interpolate = (n_sample + winsize_in_samp) - sample_frames;
	}else{
		upper_bound = n_sample + winsize_in_samp;
		_samples_to_interpolate = 0;
	}

	for(ssi_size_t nsamp = lower_bound; nsamp < upper_bound; nsamp++){
		sample_buffer[nsamp] = FLT_MAX;
	}
	if(_samples_to_interpolate > 0){
		ssi_size_t interpolation_counter = 0;
		for(ssi_size_t nsamp = sample_frames; nsamp < sample_number; nsamp++){
			if(interpolation_counter < _samples_to_interpolate){
				sample_buffer[nsamp] = FLT_MAX;
				interpolation_counter++;
			}
		}
	}
}

void GSRArtifactFilter::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num ,
	ssi_stream_t xtra_stream_in[]) {

		
}

}
