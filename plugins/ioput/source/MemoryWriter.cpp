// MemoryWriter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2017/11/28
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

#include "MemoryWriter.h"
#include "ioput/file/FileTools.h"
#include "base/Factory.h"
#include "ioput/file/FilePath.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *MemoryWriter::ssi_log_name = "filewriter";

MemoryWriter::MemoryWriter (const ssi_char_t *file)
	: _file (0),
	_stream_ptr(0),
	_stream_num(0),
	_ready(true),
	_borrowed(false),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) 
{

	ssi_stream_init(_stream, 0, 0, 0, SSI_UNDEF, 0);

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	_frame = Factory::GetFramework ();
}

MemoryWriter::~MemoryWriter () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}

	if (!_borrowed)
	{
		ssi_stream_destroy(_stream);
	}
}

void MemoryWriter::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	_ready = false;

	if (_borrowed)
	{
		if (_stream.dim != stream_in[0].dim
			|| _stream.byte != stream_in[0].byte
			|| _stream.type != stream_in[0].type)
		{
			ssi_wrn("borrowed stream differs from input stream");
			return;
		}
	}
	else
	{
		ssi_size_t n_samples = 0;
		if (!ssi_parse_samples(_options.size, n_samples, stream_in[0].sr))
		{
			ssi_wrn("could not parse buffer size '%s'", _options.size);
			return;
		}

		ssi_stream_destroy(_stream);
		ssi_stream_init(_stream, n_samples, stream_in[0].dim, stream_in[0].byte, stream_in[0].type, stream_in[0].sr);
		ssi_stream_zero(_stream);
	}

	_stream_ptr = _stream.ptr;
	_stream_num = 0;
	_ready = true;
}

void MemoryWriter::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (!_ready)
	{
		return;
	}

	ssi_size_t n_samples = stream_in[0].num;
	
	if (_stream_num + n_samples >= _stream.num)
	{
		ssi_wrn("reached end of stream buffer, stop writing samples");
		_ready = false;
		n_samples = _stream.num - _stream_num;
	}

	ssi_size_t n_bytes = n_samples * _stream.dim * _stream.byte;
	memcpy(_stream_ptr, stream_in[0].ptr, n_bytes);
	_stream_ptr += n_bytes;
	
	_stream_num += n_samples;
	
	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "%u samples written", n_samples);
}

void MemoryWriter::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) 
{	
	_stream.num = _stream_num;
}

void MemoryWriter::setStream(ssi_stream_t &stream)
{
	_borrowed = true;
	_stream = stream;
}

ssi_stream_t &MemoryWriter::getStream()
{
	return _stream;
}

}
