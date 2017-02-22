
// ConsumerBase.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/28
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "ConsumerBase.h"
#include "TheFramework.h"
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

int ConsumerBase::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
ssi_char_t *ConsumerBase::ssi_log_name = "consume___";

ConsumerBase::ConsumerBase (int buffer_id,
		IConsumer *consumer, 
		ssi_size_t frame_size,
		ssi_size_t delta_size,
		ITransformer *transformer,
		int trigger_id)
	: _consumer (consumer), 
	_transformer (0),
	_streams_trans (0),
	_frame_size (frame_size),
	_delta_size (delta_size),
	_frame_size_in_sec (0),
	_delta_size_in_sec (0),
	_streams_raw (0),
	_frame (0),
	_trigger_id (trigger_id) {

	_stream_number = 1;
	int buffer_id_tmp[1] = {buffer_id};
	if (transformer) {
		ITransformer *transformer_tmp[1] = {transformer};
		init (buffer_id_tmp, transformer_tmp);
	} else {
		init (buffer_id_tmp, 0);
	}
}

ConsumerBase::ConsumerBase (ssi_size_t stream_number, 
		int *buffer_id,
		IConsumer *consumer, 
		ssi_size_t frame_size,
		ssi_size_t delta_size,		
		ITransformer **transformer,
		int trigger_id)
	: _stream_number (stream_number),
	_consumer (consumer), 
	_transformer (0), 
	_streams_trans (0),
	_frame_size (frame_size),
	_delta_size (delta_size),
	_frame_size_in_sec (0),
	_delta_size_in_sec (0),
	_streams_raw (0),
	_streams (0),
	_frame (0),
	_trigger_id (trigger_id) {

	init (buffer_id, transformer);
}

void ConsumerBase::init (int *buffer_id,
	ITransformer **transformer) {

	// get framework
	_frame = ssi_pcast (TheFramework, Factory::GetFramework ());

	// store buffer ids and make sure all buffer exist
	_buffer_id = new int[_stream_number];
	for (ssi_size_t i = 0; i < _stream_number; i++) {
		_buffer_id[i] = buffer_id[i];
		if (!_frame->IsBufferInUse (_buffer_id[i])) {
			ssi_err ("buffer '%d' is not in use", _buffer_id[i]);
		}
	}

	// check trigger buffer
	if (_trigger_id >= 0) {
		if (!_frame->IsBufferInUse (_trigger_id)) {
			ssi_err ("buffer '%d' is not in use", _trigger_id);
		}
	}

	// store transformer
	if (transformer) {
		_transformer = new ITransformer *[_stream_number];
		for (ssi_size_t i = 0; i < _stream_number; i++) {
			_transformer[i] = transformer[i];
		}
	}

	// get all necessary buffer information from framework
	ssi_time_t *sample_rate_in = new ssi_time_t[_stream_number];
	ssi_size_t *sample_bytes_in = new ssi_size_t[_stream_number];
	ssi_size_t *sample_dimension_in = new ssi_size_t[_stream_number];
	ssi_type_t *sample_type_in = new ssi_type_t[_stream_number];
	for (ssi_size_t i = 0; i < _stream_number; i++) {
		_frame->GetSampleRate (_buffer_id[i], sample_rate_in[i]);
		_frame->GetSampleBytes (_buffer_id[i], sample_bytes_in[i]);
		_frame->GetSampleDimension (_buffer_id[i], sample_dimension_in[i]);
		_frame->GetSampleType (_buffer_id[i], sample_type_in[i]);
	}	

	// store frame/delta size in seconds
	if (_frame_size) {
		_frame_size_in_sec = _frame_size / sample_rate_in[0];
	}
	if (_delta_size) {
		_delta_size_in_sec = _delta_size / sample_rate_in[0];		
	}

	// get meta data of each buffer
	ssi_size_t * meta_size = new ssi_size_t[_stream_number];
	typedef const void* meta_t;
	meta_t * meta = new meta_t[_stream_number];
	for(ssi_size_t i = 0; i < _stream_number; i++)
	{
		meta[i] = _frame->GetMetaData (_buffer_id[i], meta_size[i]);	
	}

	// initialize transformer arrays and adjust stream information
	ssi_time_t *sample_rate = 0;
	ssi_size_t *sample_bytes = 0, *sample_dimension = 0;
	ssi_type_t *sample_type = 0;
	if (_transformer) {
		_streams_trans = new ssi_stream_t[_stream_number];
		sample_rate = new ssi_time_t[_stream_number];
		sample_dimension = new ssi_size_t[_stream_number];
		sample_bytes = new ssi_size_t[_stream_number];
		sample_type = new ssi_type_t[_stream_number];

		for (ssi_size_t i = 0; i < _stream_number; i++) {	
			if (_transformer[i]) {

				// set meta data		
				if (meta[i]) {	
					_transformer[i]->setMetaData (meta_size[i], meta[i]);
				}

				// try to guess sample rate
				// find out if transformer is a filter, otherwise set sample rate to 0
				ssi_size_t num_in = 100000;
				ssi_size_t num_out = _transformer[i]->getSampleNumberOut (num_in);
				if (num_in == num_out) { // filter
					sample_rate[i] = sample_rate_in[i];
				} else if (num_in == 1) { // feature
					sample_rate[i] = _frame_size > 0 ? sample_rate[i] = 1.0 / _frame_size : 0;			
				} else { // otherwise
					sample_rate[i] = sample_rate_in[i] * (ssi_cast (ssi_time_t, num_out) / ssi_cast (ssi_time_t, num_in));
				}
				sample_dimension[i] = _transformer[i]->getSampleDimensionOut (sample_dimension_in[i]);
				sample_bytes[i] = _transformer[i]->getSampleBytesOut (sample_bytes_in[i]);
				sample_type[i] = _transformer[i]->getSampleTypeOut (sample_type_in[i]);

				// get meta data
				meta[i] = _transformer[i]->getMetaData (meta_size[i]);

			} else {
				sample_rate[i] = sample_dimension_in[i];
				sample_dimension[i] = sample_dimension_in[i];
				sample_bytes[i] = sample_bytes_in[i];
				sample_type[i] = sample_type_in[i];
			}
		}

	} else {
		sample_rate = sample_rate_in;
		sample_dimension = sample_dimension_in;
		sample_bytes = sample_bytes_in;
		sample_type = sample_type_in;
	}

	// set meta data to consumer
	for(ssi_size_t i = 0; i < _stream_number; i++) {
		if (meta[i]) {
			_consumer->setMetaData (meta_size[i], meta[i]);
		}
	}

	// initialize streams
	_streams = new ssi_stream_t[_stream_number];
	_streams_raw = new ssi_stream_t[_stream_number];
	for (ssi_size_t i = 0; i < _stream_number; i++) {
		ssi_stream_init (_streams_raw[i], 0, sample_dimension_in[i], sample_bytes_in[i], sample_type_in[i], sample_rate_in[i]);
		_streams[i] = _streams_raw[i];
	}
	if (_transformer) {
		for (ssi_size_t i = 0; i < _stream_number; i++) {
			if (_transformer[i]) {				
				ssi_stream_init (_streams_trans[i], 0, sample_dimension[i], sample_bytes[i], sample_type[i], sample_rate[i]);				
				_streams[i] = _streams_trans[i];
			}
		}
	}

	// initialize status
	_consume_status = new int[_stream_number];

	// initialize trigger stream
	if (_trigger_id >= 0) {
		ssi_time_t sample_rate_trigger;
		_frame->GetSampleRate (_trigger_id, sample_rate_trigger);
		ssi_size_t sample_bytes_trigger;
		_frame->GetSampleBytes (_trigger_id, sample_bytes_trigger);
		ssi_size_t sample_dimension_trigger;
		_frame->GetSampleDimension (_trigger_id, sample_dimension_trigger);
		ssi_type_t sample_type_trigger;
		_frame->GetSampleType (_trigger_id, sample_type_trigger);
		ssi_stream_init (_stream_trigger, 0, sample_dimension_trigger, sample_bytes_trigger, sample_type_trigger, sample_rate_trigger);
		if (sample_dimension_trigger > 1) {
			ssi_wrn ("found trigger stream with more than one dimension");
		}
	}

	// clean up
	delete[] sample_rate_in;
	delete[] sample_dimension_in;
	delete[] sample_bytes_in;	
	delete[] sample_type_in;
	delete[] meta;
	delete[] meta_size;
	if (_transformer) {	
		delete[] sample_rate;
		delete[] sample_dimension;
		delete[] sample_bytes;
		delete[] sample_type;
	}
}

ConsumerBase::~ConsumerBase () {

	if (_transformer) {
		delete[] _transformer;
		delete[] _streams_trans;
	}
	delete[] _buffer_id;
	delete[] _streams_raw;
	delete[] _streams;
	delete[] _consume_status;
}

void ConsumerBase::enter () {

	_consumer->consume_enter (_stream_number, _streams);
	if (_transformer) {
		for (ssi_size_t i = 0; i < _stream_number; i++) {
			if (_transformer[i]) {
				_transformer[i]->transform_enter (_streams_raw[i], _streams_trans[i]);
			}
		}
	}

	ssi_msg (SSI_LOG_LEVEL_BASIC, "start '%s:%s'", _consumer->getName (), Factory::GetObjectId(_consumer));
	if ( ssi_log_level >= SSI_LOG_LEVEL_BASIC) {
		ssi_print ("\
             frame[s]\t= %.2lf\n\
             delta[s]\t= %.2lf\n",
		_frame_size_in_sec,
		_delta_size_in_sec);
		for (ssi_size_t i = 0; i < _stream_number; i++) {
			ssi_print ("\
             stream#%u\n\
             id\t= %d\n\
             rate[hz]\t= %.2lf\n\
             dim\t= %u\n\
             bytes\t= %u\n\
             type\t= %s\n", 
				i+1, 
				_buffer_id[i],
				_streams[i].sr, 
				_streams[i].dim, 
				_streams[i].byte,
				SSI_TYPE_NAMES[_streams[i].type]);		
		}
	}
}

int ConsumerBase::consume (IConsumer::info info) {

	// get trigger data
	if (_trigger_id >= 0) {
		_trigger_status = _frame->GetData (_trigger_id, _stream_trigger, info.time, info.dur);
		if (_trigger_status != TimeBuffer::SUCCESS) {
			return _trigger_status;
		}
		// in case no error occured, we can now decide whether to consume data or not
		if (!check_trigger_stream (_stream_trigger)) {
			return TimeBuffer::SUCCESS;
		}
	}

	// get data for each stream
	for (ssi_size_t i = 0; i < _stream_number; i++) {
		_consume_status[i] = _frame->GetData (_buffer_id[i], _streams_raw[i], info.time, info.dur);
		_streams[i] = _streams_raw[i];
	}
		
	// check if data is available for all streams
	// otherwise return error code
	for (ssi_size_t i = 0; i < _stream_number; i++) {
		if (_consume_status[i] != TimeBuffer::SUCCESS) {
			return _consume_status[i];
		}
	}

	// in case no error occured, we can now consume the data
	// if necessary push data through transformer
	if (_transformer) {		
		for (ssi_size_t i = 0; i < _stream_number; i++) {
			if (_transformer[i]) {			
				ssi_size_t number_out = _transformer[i]->getSampleNumberOut (_streams_raw[i].num);
				ssi_stream_adjust (_streams_trans[i], number_out);
				ITransformer::info tinfo;
				tinfo.delta_num = 0;
				tinfo.frame_num = _streams_raw[i].num;
				tinfo.time = info.time;
				_transformer[i]->transform (tinfo, _streams_raw[i], _streams_trans[i]);
				_streams_trans[i].sr = _streams_raw[i].sr * ssi_cast (ssi_time_t, number_out) / ssi_cast (ssi_time_t, _streams_raw[i].num);				
				_streams[i] = _streams_trans[i];
			}
		}	
	} 

	for (ssi_size_t i = 0; i < _stream_number; i++) {
		_streams[i].time = info.time;
	}
	
	if (_consumer->isEnabled()) {
		_consumer->consume(info, _stream_number, _streams);
	}

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "consumed %u samples '%s:%s'", _streams[0].num, _consumer->getName (), Factory::GetObjectId(_consumer));
	
	return TimeBuffer::SUCCESS;
}


void ConsumerBase::flush () {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "stop '%s:%s'", _consumer->getName (), Factory::GetObjectId(_consumer));

	// reset input stream
	for (ssi_size_t i = 0; i < _stream_number; i++) {
		ssi_stream_reset (_streams_raw[i]);	
	}

	_consumer->consume_flush (_stream_number, _streams);
	if (_transformer) {
		for (ssi_size_t i = 0; i < _stream_number; i++) {
			if (_transformer[i]) {
				_transformer[i]->transform_flush (_streams_raw[i], _streams_trans[i]);
			}
		}
	}

	// flush transformer and reset transformer stream
	if (_transformer) {
		for (ssi_size_t i = 0; i < _stream_number; i++) {
			if (_transformer[i]) {
				ssi_stream_reset (_streams_trans[i]);
			}
		}
	}

	// reset trigger stream
	if (_trigger_id >= 0) {
		ssi_stream_reset (_stream_trigger);
	}
}

bool ConsumerBase::check_trigger_stream (ssi_stream_t &s) {

	ssi_size_t n = s.num * s.dim;
	bool result = false;

	switch (s.type) {
		case SSI_CHAR: {						
			char *ptr = ssi_pcast (char, s.ptr);			
			for (ssi_size_t i = 0; i < n; i++) {
				if (*ptr++ != 0) {
					result = true;
					break;
				}
			}
			break;
		}
		case SSI_UCHAR: {			
			unsigned char *ptr = ssi_pcast (unsigned char, s.ptr);			
			for (ssi_size_t i = 0; i < n; i++) {
				if (*ptr++ != 0) {
					result = true;
					break;
				}
			}
			break;
		}
		case SSI_SHORT: {			
            int16_t *ptr = ssi_pcast (int16_t, s.ptr);
			for (ssi_size_t i = 0; i < n; i++) {
				if (*ptr++ != 0) {
					result = true;
					break;
				}
			}
			break;
		}
		case SSI_USHORT: {			
            uint16_t *ptr = ssi_pcast (uint16_t, s.ptr);
			for (ssi_size_t i = 0; i < n; i++) {
				if (*ptr++ != 0) {
					result = true;
					break;
				}
			}
			break;
		}
		case SSI_INT: {			
            int32_t *ptr = ssi_pcast (int32_t, s.ptr);
			for (ssi_size_t i = 0; i < n; i++) {
				if (*ptr++ != 0) {
					result = true;
					break;
				}
			}
			break;
		}
		case SSI_UINT: {			
			uint64_t *ptr = ssi_pcast (uint64_t, s.ptr);
			for (ssi_size_t i = 0; i < n; i++) {
				if (*ptr++ != 0) {
					result = true;
					break;
				}
			}
			break;
		}
		case SSI_LONG: {			
            int32_t *ptr = ssi_pcast (int32_t, s.ptr);
			for (ssi_size_t i = 0; i < n; i++) {
				if (*ptr++ != 0) {
					result = true;
					break;
				}
			}
			break;
		}
		case SSI_ULONG: {			
			uint64_t *ptr = ssi_pcast (uint64_t, s.ptr);
			for (ssi_size_t i = 0; i < n; i++) {
				if (*ptr++ != 0) {
					result = true;
					break;
				}
			}
			break;
		}
		case SSI_FLOAT: {
			float *ptr = ssi_pcast (float, s.ptr);			
			for (ssi_size_t i = 0; i < n; i++) {
				if (*ptr++ != 0) {
					result = true;
					break;
				}
			}
			break;
		}
		case SSI_DOUBLE: {			
			double *ptr = ssi_pcast (double, s.ptr);			
			for (ssi_size_t i = 0; i < n; i++) {
				if (*ptr++ != 0) {
					result = true;
					break;
				}
			}
			break;
		}
		default:
			ssi_err ("unsupported sample type (%s)", SSI_TYPE_NAMES[s.type]);
	}

	return result;
}

}
