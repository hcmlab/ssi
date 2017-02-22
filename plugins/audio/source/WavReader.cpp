// WavReader.cpp
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

#include "WavReader.h"
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

const ssi_char_t *WavReader::ssi_log_name = "wavreader_";

WavReader::WavReader (const ssi_char_t *file) 
	: _provider (0),
	_frame_counter (0),
	_frame_timer (0),
	_total_size (0),
	_frame_size (0),
	_offset_in_bytes (0),
	_offset_in_samples (0),
	_wait_event (false, false),
	_is_providing (false),
	_wav_path (0),
	_wav_file (0),	
	_loop_pos (0),
	_file (0) {		

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

bool WavReader::setProvider (const ssi_char_t *name, IProvider *provider) {

	if (strcmp (name, SSI_WAVREADER_PROVIDER_NAME) == 0) {
		setProvider (provider);
		return true;
	}

	ssi_wrn ("unkown provider name '%s'", name);

	return false;
}

void WavReader::setProvider (IProvider *provider) {

	if (!provider) {
		return;
	}

	if (_provider) {
		ssi_wrn ("provider already set");
		return;
	}

	_provider = provider;

	// open file	
	FilePath fp(_options.path);
	if (strcmp(fp.getExtension(), SSI_FILE_TYPE_WAV) != 0) 
	{
		_wav_path = ssi_strcat(_options.path, SSI_FILE_TYPE_WAV);
	}
	else 
	{
		_wav_path = ssi_strcpy(_options.path);
	}

	ssi_msg(SSI_LOG_LEVEL_BASIC, "open file '%s'", _wav_path);

	_wav_file = File::CreateAndOpen (File::BINARY, File::READ, _wav_path);
	if (!_wav_file)
	{
		ssi_err("could open stream '%s'", _wav_path);
	}

	// read wav header
	WavTools::ReadWavHeader (*_wav_file, _header);
	WavTools::ReadWavChunk (*_wav_file, _chunk);
	_format.wFormatTag = WAVE_FORMAT_PCM;
	_format.nChannels = _header.nChannels;
	_format.nAvgBytesPerSec = _header.nAvgBytesPerSec;
	_format.nBlockAlign = _header.nBlockAlign;
	_format.nSamplesPerSec = _header.nSamplesPerSec;

	ssi_time_t _sample_rate = _header.nSamplesPerSec;
	ssi_size_t _sample_dimension = _header.nChannels;
	ssi_size_t _sample_bytes = _header.nBitsPerSample / 8;

	// skip offset
	if (_options.offsetInSamples > 0) {
		_offset_in_samples = _options.offsetInSamples;
	} else {
		_offset_in_samples = ssi_cast (ssi_size_t, _sample_rate * _options.offset);
	}
	_offset_in_bytes = _offset_in_samples * _sample_dimension * _sample_bytes;
	if (_offset_in_bytes > ssi_cast (ssi_size_t, _chunk.chunkLen)) {
		ssi_err ("offset exceeds #bytes in file (%u > %d)", _offset_in_bytes, _chunk.chunkLen);
	}
	_loop_pos = _wav_file->tell () + _offset_in_bytes;

	// calculate frame size
	_total_size = ssi_cast (ssi_size_t, (_chunk.chunkLen - _offset_in_bytes) / (_sample_bytes * _sample_dimension));
	if (_options.blockInSamples > 0) {
		_frame_size = _options.blockInSamples;
	} else {
		_frame_size = ssi_cast (ssi_size_t, _options.block * _sample_rate + 0.5);
	}
	_options.block = ssi_cast (ssi_time_t, _frame_size) / _sample_rate;

	// init buffer and provider
	ssi_stream_init (_stream, 0, _sample_dimension, _sample_bytes, SSI_SHORT, _sample_rate);
	_scale = _options.scale;
	if (_scale) {
		ssi_stream_init (_stream_scale, 0, _sample_dimension, sizeof (ssi_real_t), SSI_FLOAT, _sample_rate);
		_audio_channel.stream = _stream_scale;
	} else {
		_audio_channel.stream = _stream;
	}
	_provider->init (&_audio_channel);	
	ssi_stream_adjust (_stream, _frame_size);		
	if (_scale) {
		ssi_stream_adjust (_stream_scale, _frame_size);		
	}

}

WavReader::~WavReader() {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "close file '%s'", _wav_path);

	// close wav file
	delete _wav_file; _wav_file = 0;
	delete _wav_path; _wav_path = 0;

	// release audio buffer
	if(_provider) {
		ssi_stream_destroy (_stream);
		if (_options.scale) {
			ssi_stream_destroy (_stream_scale);
		}
	}

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

bool WavReader::connect () {

	if (!_provider) {
		ssi_wrn ("provider not set");
		return false;
	}

	// reset file stream
	_frame_counter = _total_size / _frame_size;
	_wav_file->seek(_loop_pos, File::BEGIN);

	// block wait event
	_wait_event.block ();

	// set providing=true to read first chunk
	_is_providing = true;

	// set thread name
	ssi_char_t *thread_name = ssi_strcat (getName(), ":", Factory::GetObjectId(this));
	Thread::setName (thread_name);
	delete[] thread_name;

	ssi_msg(SSI_LOG_LEVEL_BASIC, "start reading from '%s'", _wav_path);

	return true;
}

void WavReader::run () {

	if (_frame_counter > 0) {

		// read next frame
		if (_is_providing) {
			_wav_file->read (_stream.ptr, _stream.byte * _stream.dim, _stream.num);
		}

		if (_scale) {
            int16_t *src = ssi_pcast (int16_t, _stream.ptr);
			float *dst = ssi_pcast (float, _stream_scale.ptr);
			for (ssi_size_t i = 0; i < _stream.num * _stream.dim; i++) {
				*dst++ = *src++ / 32768.0f;
			}
			_is_providing = _provider->provide (_stream_scale.ptr, _stream_scale.num);
		} else {
			_is_providing = _provider->provide (_stream.ptr, _stream.num);
		}
		
		if (!_is_providing) {
			::Sleep (10);
			return;
		}

		// decrement frame counter
		--_frame_counter;

	} else {

		// end of file has been reached
		if (_options.loop) {

			ssi_msg (SSI_LOG_LEVEL_BASIC, "loop '%s'", _wav_path);

			_wav_file->seek (_loop_pos, File::BEGIN);
			_frame_counter = _total_size / _frame_size;
		} else {		
			_wait_event.release ();			
		}
	}

	// sleep until it's time to catch the next frame
	if (!_options.best_effort_delivery) {

		// init timer
		if (!_frame_timer) {
			_frame_timer = new Timer (_options.block);
		}
		_frame_timer->wait (); 
	}
}

bool WavReader::disconnect () {

	// delete timer
	delete _frame_timer; _frame_timer = 0;

	ssi_msg(SSI_LOG_LEVEL_BASIC, "stop reading from '%s'", _wav_path);

	return true;
}

}
