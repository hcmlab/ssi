// ISTransform.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/05/01
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

#include "ISTransform.h"
#include "signal/SignalTools.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ISTransform::ISTransform (ISamples *samples)
	: _samples (*samples),
	_n_transformer (samples->getStreamSize ()) {

	_transformers = new ITransformer *[_n_transformer];
	_streams = new ssi_stream_t[_n_transformer];
	_frame_sizes = new ssi_time_t[_n_transformer];
	_frame_sizes_in_samples = new ssi_size_t[_n_transformer];
	_delta_sizes = new ssi_time_t[_n_transformer];
	_delta_sizes_in_samples = new ssi_size_t[_n_transformer];
	for (ssi_size_t i = 0; i < _n_transformer; i++) {
		_transformers[i] = 0;
		_frame_sizes[i] = 0;
		_delta_sizes[i] = 0;
		_frame_sizes_in_samples[i] = 0;
		_delta_sizes_in_samples[i] = 0;
	}
	_sample_out.num = _n_transformer;
	_sample_out.streams = new ssi_stream_t *[_sample_out.num];
	for (ssi_size_t i = 0; i < _n_transformer; i++) {
		_sample_out.streams[i] = 0;
	}
}

ISTransform::~ISTransform () {

	delete[] _sample_out.streams;
	delete[] _transformers;
	delete[] _delta_sizes;
	delete[] _frame_sizes;
	delete[] _delta_sizes_in_samples;
	delete[] _frame_sizes_in_samples;
	delete[] _streams;
}	

bool ISTransform::setTransformer (ssi_size_t index, 
	ITransformer &transformer) {

	if (index >= _n_transformer) {
		ssi_wrn ("index exceeds stream size");
		return false;
	} 
	_transformers[index] = &transformer;
	_frame_sizes[index] = 0;
	_frame_sizes_in_samples[index] = 0;
	_delta_sizes[index] = 0;
	_delta_sizes_in_samples[index] = 0;

	return true;
}

bool ISTransform::setTransformer (ssi_size_t index, 
	ITransformer &transformer, 
	ssi_time_t frame_size, 
	ssi_time_t delta_size) {

	if (index >= _n_transformer) {
		ssi_wrn ("index exceeds stream size");
		return false;
	} 
	_transformers[index] = &transformer;
	_frame_sizes[index] = frame_size;
	_frame_sizes_in_samples[index] = 0;
	_delta_sizes[index] = delta_size;
	_delta_sizes_in_samples[index] = 0;

	return true;
}

bool ISTransform::setTransformer (ssi_size_t index, 
	ITransformer &transformer, 
	ssi_size_t frame_size, 
	ssi_size_t delta_size) {

	if (index >= _n_transformer) {
		ssi_wrn ("index exceeds stream size");
		return false;
	} 
	_transformers[index] = &transformer;
	_frame_sizes[index] = 0;
	_frame_sizes_in_samples[index] = frame_size;
	_delta_sizes[index] = 0;
	_delta_sizes_in_samples[index] = delta_size;

	return true;
}

void ISTransform::callEnter () {

	_sample_in = *(_samples.get (0));
	transform (true, false);
}

void ISTransform::callFlush () {

	_sample_in = *(_samples.get (0));
	transform (false, true);
}

SSI_INLINE void ISTransform::transform (bool call_enter, bool call_flush) {

	_sample_out.class_id = _sample_in.class_id;
	_sample_out.user_id = _sample_in.user_id;
	_sample_out.score = _sample_in.score;
	_sample_out.time = _sample_in.time;

	for (ssi_size_t i = 0; i < _n_transformer; i++) {

		_sample_out.streams[i] = _sample_in.streams[i];

		if (_transformers[i]) {

			ssi_stream_t &from = *_sample_in.streams[i];
			ssi_stream_t &to = _streams[i];
			ITransformer &transformer = *_transformers[i];

			if (call_enter) {

				ssi_size_t sample_number_in = 0;
				if (_frame_sizes[i] <= 0 && _frame_sizes_in_samples[i] == 0) {
					sample_number_in = from.num;
				} else {
					if (_frame_sizes_in_samples[i] == 0) {
						_frame_sizes_in_samples[i] = ssi_cast (ssi_size_t, _frame_sizes[i] * from.sr + 0.5); 				
					}
					if (_delta_sizes_in_samples[i] == 0) {
						if (_delta_sizes[i] > 0) {
							_delta_sizes_in_samples[i] = ssi_cast (ssi_size_t, _delta_sizes[i] * from.sr + 0.5);		
						}
					}
					sample_number_in = _frame_sizes_in_samples[i] + _delta_sizes_in_samples[i];
				}

				ssi_time_t sample_rate_in = from.sr;
				ssi_size_t sample_dimension_in = from.dim;
				ssi_size_t sample_bytes_in = from.byte;
				ssi_type_t sample_type_in = from.type;
				ssi_size_t sample_number_out = transformer.getSampleNumberOut (sample_number_in);
				ssi_size_t sample_dimension_out = transformer.getSampleDimensionOut (sample_dimension_in);
				ssi_size_t sample_bytes_out = transformer.getSampleBytesOut (sample_bytes_in);
				ssi_type_t sample_type_out = transformer.getSampleTypeOut (sample_type_in);
				ssi_time_t sample_rate_out = (ssi_cast (ssi_time_t, sample_number_out) / ssi_cast (ssi_time_t, sample_number_in)) * sample_rate_in;
				ssi_stream_init (to, 0, sample_dimension_out, sample_bytes_out, sample_type_out, sample_rate_out);

				transformer.transform_enter (from, to, 0, 0);	

			} else if (call_flush) {

				transformer.transform_flush (from, to, 0, 0);	
				ssi_stream_destroy (to);

			} else {

				if (_frame_sizes_in_samples[i] == 0) {

					ssi_size_t sample_number_in = from.num;
					ssi_size_t sample_number_out = transformer.getSampleNumberOut (sample_number_in);
					ssi_stream_adjust (to, sample_number_out);

					ITransformer::info tinfo;
					tinfo.delta_num = 0;
					tinfo.frame_num = from.num;
					tinfo.time = 0;
					transformer.transform (tinfo, from, to, 0, 0);

				} else {

					ssi_time_t sample_rate_in = from.sr;

					ssi_size_t frame_size = _frame_sizes_in_samples[i];
					ssi_size_t delta_size = _delta_sizes_in_samples[i];

					ssi_size_t from_num = from.num;
					ssi_size_t from_tot = from.tot;
					SSI_ASSERT (from_num > frame_size + delta_size);
					ssi_size_t max_shift = (from_num - delta_size) / frame_size;

					ssi_size_t sample_number_in = frame_size + delta_size;
					ssi_size_t sample_number_out = transformer.getSampleNumberOut (frame_size);	
					ssi_size_t sample_dimension_in = from.dim;
					ssi_size_t sample_dimension_out = transformer.getSampleDimensionOut (sample_dimension_in);
					ssi_size_t sample_bytes_in = from.byte;
					ssi_size_t sample_bytes_out = transformer.getSampleBytesOut (sample_bytes_in);
					ssi_type_t sample_type_in = from.type;
					ssi_type_t sample_type_out = transformer.getSampleTypeOut (sample_type_in);
					ssi_time_t sample_rate_out = (ssi_cast (ssi_time_t, sample_number_out) / ssi_cast (ssi_time_t, frame_size)) * sample_rate_in;

					ssi_size_t to_num = max_shift * sample_number_out;
					//ssi_stream_init (to, 0, sample_dimension_out, sample_bytes_out, sample_type_out, sample_rate_out);
					ssi_stream_adjust (to, to_num);
					ssi_size_t to_tot = to.tot;

					ssi_byte_t *from_ptr = from.ptr;
					ssi_byte_t *to_ptr = to.ptr;
					from.num = sample_number_in;
					to.num = sample_number_out;
					ssi_size_t byte_shift_in = sample_bytes_in * sample_dimension_in * frame_size;
					from.tot = byte_shift_in;
					ssi_size_t byte_shift_out = sample_bytes_out * sample_dimension_out * sample_number_out;
					to.tot = byte_shift_out;

					ITransformer::info tinfo;
					tinfo.delta_num = delta_size;
					tinfo.frame_num = frame_size;
					tinfo.time = 0;
					for (ssi_size_t i = 0; i < max_shift; i++) {			
						transformer.transform (tinfo, from, to, 0, 0);					
						tinfo.time += frame_size / sample_rate_in;
						from.ptr += byte_shift_in;
						to.ptr += byte_shift_out;
					}

					from.ptr = from_ptr;
					from.num = from_num;
					from.tot = from_tot;
					to.ptr = to_ptr;
					to.num = to_num;
					to.tot = to_tot;
					
				}

			}
			_sample_out.streams[i] = &_streams[i];
		}
	}
}

ssi_sample_t *ISTransform::get (ssi_size_t index) {

	ssi_sample_t *tmp = _samples.get (index);

	if (tmp == 0) {
		return 0;
	}

	_sample_in = *tmp;
	transform ();
	return &_sample_out;	
}

ssi_sample_t *ISTransform::next () {

	ssi_sample_t *tmp = _samples.next ();
	if (!tmp) {
		return 0;
	}

	_sample_in = *tmp;
	transform ();
	return &_sample_out;
}


}

