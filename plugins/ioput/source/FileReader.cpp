// FileReader.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/01/16
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

#include "FileReader.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *FileReader::ssi_log_name = "filereader";

FileReader::FileReader (const ssi_char_t *file) 
: _provider (0),
	_sample_number_per_step (0),
	_sample_number_total (0),
	_step_counter (0),
	_max_steps (0),
	_timer (0),
	_event (false, true),
	_is_providing (false),
	_offset_in_bytes (0),
	_offset_in_samples (0),
	_cutoff_in_samples (0),
	_file (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

FileReader::~FileReader () {
	
	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

bool FileReader::setProvider (const ssi_char_t *name, IProvider *provider) {

	if (strcmp (name, SSI_FILEREADER_PROVIDER_NAME) == 0) {
		setProvider (provider);
		return true;
	}

	ssi_wrn ("unkown provider name '%s'", name);

	return false;
}

void FileReader::setProvider (IProvider *provider) {

	if (_provider) {
		ssi_wrn ("provider already set");
		return;
	}

	if (!_file_stream_in.open(_stream, _options.path, _n_meta, (void **)&_meta)) 
	{
		ssi_err("could not open stream '%s'", _options.path);
	}

	_sample_number_total = _file_stream_in.getTotalSampleSize();
	if (_offset_in_samples > _sample_number_total) 
	{
		ssi_err("offset exceeds #samples (%u > %u)", _offset_in_samples, _sample_number_total);
	}
	
	_provider = provider;
	_file_channel.stream = _stream;
	_provider->setMetaData (_n_meta, _meta);
	_provider->init (&_file_channel);

	if (!_file_stream_in.close())
	{
		ssi_err("could not close stream '%s'", _options.path);
	}
}

bool FileReader::prepare_file () {

	if (_options.offsetInSamples > 0) {
		_offset_in_samples = _options.offsetInSamples;
	} else {
		_offset_in_samples = ssi_cast (ssi_size_t, _stream.sr * _options.offset);
	}
	_offset_in_bytes = _offset_in_samples * _stream.dim * _stream.byte;

	if (_offset_in_samples > _sample_number_total) {
		ssi_wrn ("offset exceeds stream number");
		return false;
	}

	if (_options.cutoffInSamples > 0) {
		_cutoff_in_samples = _options.cutoffInSamples;
	} else {
		_cutoff_in_samples = ssi_cast (ssi_size_t, _stream.sr * _options.cutoff);
	}
	
	if (_options.blockInSamples > 0) {
		_sample_number_per_step = _options.blockInSamples;
	} else {
		_sample_number_per_step = ssi_cast (ssi_size_t, _stream.sr * _options.block);
	}

	_step_counter = 0;
	if (_cutoff_in_samples == 0) {
		_max_steps = (_sample_number_total - _offset_in_samples)  / _sample_number_per_step;
	} else {

		if (_cutoff_in_samples > _sample_number_total) {
			ssi_wrn ("cutoff exceeds stream number");
			return false;
		}

		if (_offset_in_samples > _cutoff_in_samples) {
			ssi_wrn ("offset exceeds cutoff");
			return false;
		}

		_max_steps = (_cutoff_in_samples - _offset_in_samples)  / _sample_number_per_step;
	}

	if (_max_steps == 0) {
		ssi_wrn ("file too short");
		return false;
	}
	_options.block = _sample_number_per_step / _stream.sr;
	
	ssi_stream_adjust (_stream, _sample_number_per_step);
	_file_stream_in.getDataFile ()->seek (_offset_in_bytes);

	return true;
}

bool FileReader::connect () {

	if (!_provider) 
	{
		ssi_wrn ("provider not set");
		return false;
	}	

	if (!_file_stream_in.open(_stream, _options.path))
	{
		ssi_err("could not open stream '%s'", _options.path);
	}

	if (!prepare_file ()) 
	{
		ssi_err ("could not prepare stream (%s)", _file_stream_in.getDataFile ()->getPath ());
	}

	_stopped = false;
	// set providing=true to read first chunk
	_is_providing = true;

	ssi_msg (SSI_LOG_LEVEL_BASIC, "start 'path=%s'", _options.path);
	if (ssi_log_level >= SSI_LOG_LEVEL_BASIC) {
		ssi_print ("             sample rate\t= %.2lf Hz\n\
             dim\t\t= %u\n\
             bytes\t= %u\n\
             number\t= %u\n\
             length\t= %.2fs\n",
		_stream.sr, 
		_stream.dim, 
		_stream.byte,
		_sample_number_total,
		_sample_number_total/_stream.sr);
	}

	// set thread name
	ssi_char_t *thread_name = ssi_strcat (getName(), "@", _options.path);
	Thread::setName (thread_name);
	delete[] thread_name;

	ssi_msg(SSI_LOG_LEVEL_BASIC, "start reading from '%s'", _options.path);

	return true;
}

void FileReader::run () {
	
	if (_stopped) {
		::Sleep (100);
		return;
	}

	if (_is_providing) {
		if (_file_stream_in.read (_stream) == FileStreamIn::READ_ERROR) {
			ssi_err ("an error occured while reading file (%s)", _file_stream_in.getDataFile ()->getPath ());
		}
	}

	_is_providing = _provider->provide (_stream.ptr, _stream.num);
	if (!_is_providing) {
		::Sleep (100);
		return;
	}
	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "read %u samples", _stream.num);

	if (!_timer) {
		_timer = new Timer (_options.block);
	}

	if (++_step_counter >= _max_steps)
	{
		if (_options.loop) 
		{
			if (!prepare_file ()) {
				ssi_err ("an error occured while reading file (%s)", _file_stream_in.getDataFile ()->getPath ());
			}
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "loop 'path=%s'", _options.path);
		}
		else
		{
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "release 'path=%s'", _options.path);
			_stopped = true;
			_event.release ();
		}
	
	}
	
	_timer->wait ();
}

bool FileReader::disconnect () {

	ssi_msg(SSI_LOG_LEVEL_BASIC, "stop reading from '%s'", _options.path);

	delete _timer; _timer = 0;
	ssi_stream_destroy(_stream);

	if (!_file_stream_in.close())
	{
		ssi_err("could not close stream '%s'", _options.path);
	}

	return true;
}

}
