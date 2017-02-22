// BufferWriter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/04
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

#include "BufferWriter.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *BufferWriter::ssi_log_name = "bufwrite__";

BufferWriter::BufferWriter (const ssi_char_t *file)
	: _file (0),
	_buffer (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

BufferWriter::~BufferWriter () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void BufferWriter::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	{
		Lock lock (_mutex);
		_buffer = new TimeBuffer (_options.size, stream_in[0].sr, stream_in[0].dim, stream_in[0].byte, stream_in[0].type);
	}

	ssi_msg (SSI_LOG_LEVEL_BASIC, "start 'size=%.2lfs'", _options.size);
}

void BufferWriter::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	{
		Lock lock (_mutex); 
		_buffer->push (stream_in[0].ptr, stream_in[0].num);
	}

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "%u samples written", stream_in[0].num);
}

void BufferWriter::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "stop 'time=%.2lfs'", _buffer->getCurrentSampleTime ());

	{
		Lock lock (_mutex); 
		delete _buffer;
		_buffer = 0;
	}
}

bool BufferWriter::get (ssi_stream_t &stream, IConsumer::info consume_info) {

	if (!_buffer) {
		ssi_wrn ("buffer not initialized");
		return false;
	}

	ssi_size_t num_out;
	TimeBuffer::STATUS status;
	{
		Lock lock (_mutex); 
		status = _buffer->get (&stream.ptr, stream.num_real, num_out, consume_info.time, consume_info.dur);
	}
	if (status != TimeBuffer::SUCCESS) {
		ssi_wrn ("could not get requested (%.2lf@.2lf) '%s'", consume_info.dur, consume_info.time, TimeBuffer::STATUS_NAMES[status]);
		return false;
	}

	stream.num = num_out;
	stream.tot = num_out * stream.byte * stream.dim;
	if (num_out > stream.num_real) {
		stream.num_real = stream.num;
		stream.tot_real = stream.tot;
	}

	return true;
}

}
