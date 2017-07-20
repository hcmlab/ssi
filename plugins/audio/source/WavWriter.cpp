// WavWriter.cpp
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

#include "WavWriter.h"
#include "ioput/file/FilePath.h"

namespace ssi {

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

const ssi_char_t *WavWriter::ssi_log_name = "wavwriter_";

WavWriter::WavWriter(const char *file)
	: _wav_file (0),
	_counter (0),
	_chunks(false),
	_chunk_counter(0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

WavWriter::~WavWriter() {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void WavWriter::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (stream_in[0].type != SSI_FLOAT && stream_in[0].type != SSI_SHORT) {
		ssi_err ("format not supported (%s)", SSI_TYPE_NAMES[stream_in[0].type]);
	}

	if (stream_in[0].type == SSI_FLOAT) {
		ssi_stream_init (_stream_short, 0, stream_in[0].dim, sizeof (short), SSI_SHORT, stream_in[0].sr);
	}

	_format = ssi_create_wfx (stream_in[0].sr, stream_in[0].dim, sizeof (short));
	_chunks = _options.chunks;

	if (_chunks) { 
		_chunk_counter = _options.chunksCountFrom;
	}
	else {

		// open file
		FilePath fp(_options.path);
		ssi_char_t *path;
		if (ssi_strcmp(fp.getExtension(), SSI_FILE_TYPE_WAV)) {
			path = ssi_strcpy(_options.path);
		} else {
			path = ssi_strcat(_options.path, SSI_FILE_TYPE_WAV);
		}
		_wav_file = File::CreateAndOpen(File::BINARY, File::WRITE, path);
		delete[] path;

		// write empty header
		WavTools::WriteWavHeader(*_wav_file, _format, 0);
		WavTools::WriteWavChunk(*_wav_file, _format, 0);
		_counter = 0;

		ssi_msg(SSI_LOG_LEVEL_BASIC, "start writing '%s'", _options.path);
	}
}

void WavWriter::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (_chunks) {

		FilePath fp(_options.path);
	
		char number[10];
		ssi_sprint(number, "_%03u", _chunk_counter++);
		ssi_char_t *path = ssi_strcat(fp.getPath(), number, SSI_FILE_TYPE_WAV);
	
		WavTools::WriteWavFile(path, stream_in[0]);

		delete[] path;

	} else {

		if (stream_in[0].type == SSI_FLOAT) {
			ssi_stream_adjust(_stream_short, stream_in[0].num);
			short *dstptr = ssi_pcast(short, _stream_short.ptr);
			float *srcptr = ssi_pcast(float, stream_in[0].ptr);
			for (ssi_size_t i = 0; i < stream_in[0].num * stream_in[0].dim; i++) {
				*dstptr++ = ssi_cast(short, *srcptr++ * 32768.0f);
			}
			_wav_file->write(_stream_short.ptr, 1, _stream_short.tot);
		}
		else {
			_wav_file->write(stream_in[0].ptr, 1, stream_in[0].tot);
		}
	}

	_counter += stream_in[0].num;
}

void WavWriter::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (_chunks) {
		_chunk_counter = 0;
	} else {

		ssi_msg(SSI_LOG_LEVEL_BASIC, "stop writing '%s'", _options.path);

		// re-write header
		_wav_file->seek(0, File::BEGIN);
		WavTools::WriteWavHeader(*_wav_file, _format, _counter);
		WavTools::WriteWavChunk(*_wav_file, _format, _counter);

		// close file
		delete _wav_file; _wav_file = 0;

		if (stream_in[0].type == SSI_FLOAT) {
			ssi_stream_destroy(_stream_short);
		}
	}
}

}
