// FileProvider.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/07/23
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

#include "ioput/file/FileProvider.h"
#include "base/Factory.h"

namespace ssi {

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

FileProvider::FileProvider (IConsumer *writer,
	ITransformer *transformer)
	: _transformer (transformer),
	_writer (writer)
{

}

FileProvider::~FileProvider () {	

	if (_writer)
	{
		_writer->consume_flush(1, &_stream);
	}

	if (_transformer) {
		_transformer->transform_flush (_stream, _stream_t);
		ssi_stream_destroy (_stream_t);
	}
}

void FileProvider::init (IChannel *channel) {

	ssi_size_t sample_dimension = channel->getStream ().dim;
	ssi_size_t sample_bytes = channel->getStream ().byte;
	ssi_type_t sample_type = channel->getStream ().type;
	ssi_time_t sample_rate = channel->getStream ().sr;

	ssi_stream_init (_stream, 0, sample_dimension, sample_bytes, sample_type, sample_rate);
	
	_info.status = IConsumer::NO_TRIGGER;
	_info.time = 0;

	if (_transformer) {

		ssi_size_t sample_dimension_out = _transformer->getSampleDimensionOut(sample_dimension);
		ssi_size_t sample_bytes_out = _transformer->getSampleBytesOut(sample_bytes);
		ssi_type_t sample_type_out = _transformer->getSampleTypeOut(sample_type);
		ssi_time_t sample_rate_out = sample_rate;

		/*ssi_time_t sample_rate_cast = ssi_cast(ssi_time_t, ssi_cast(ssi_size_t, sample_rate));
		ssi_size_t sample_number_out = _transformer->getSampleNumberOut(ssi_cast(ssi_size_t, sample_rate));

		ssi_time_t sample_rate_out = 
			sample_rate_cast
			* (sample_rate_cast
				/ ssi_cast(ssi_time_t, sample_number_out));*/

		ssi_stream_init (_stream_t, 0, sample_dimension_out, sample_bytes_out, sample_type_out, sample_rate_out);
		_transformer->transform_enter (_stream, _stream_t);
		if (_writer)
		{
			_writer->consume_enter(1, &_stream_t);
		}
	} else {
		if (_writer)
		{
			_writer->consume_enter(1, &_stream);
		}
	}
}

bool FileProvider::provide (ssi_byte_t *data, 
	ssi_size_t sample_number) {

	_stream.num_real = _stream.num = sample_number;
	_stream.tot_real = _stream.tot = sample_number * _stream.dim * _stream.byte;
	_stream.ptr = data;
	_info.dur = sample_number * (1.0 / _stream.sr);	

	if (_transformer) {
		ssi_size_t num_out = _transformer->getSampleNumberOut (_stream.num);
		ssi_stream_adjust (_stream_t, num_out);
		ITransformer::info info_t;
		info_t.delta_num = 0;
		info_t.frame_num = sample_number;
		info_t.time = _info.time;
		_transformer->transform (info_t, _stream, _stream_t);
		if (_writer)
		{
			_writer->consume(_info, 1, &_stream_t);
		}
	} else {
		if (_writer)
		{
			_writer->consume(_info, 1, &_stream);
		}
	}

	_info.time += _info.dur;

	return true;
}

}
