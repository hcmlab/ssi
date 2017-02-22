// FFMPEGReader.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/04/15
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

#include "FFMPEGReader.h"
#include "FFMPEGReaderClient.h"

namespace ssi {

ssi_char_t *FFMPEGReader::ssi_log_name = "ffread____";

FFMPEGReader::FFMPEGReader (const ssi_char_t *file)
	: _file (0),
	_video_buffer (0),
	_audio_buffer (0),
	_video_provider (0),
	_audio_provider(0),
	_audio_sr (0),
	_mode (MODE::UNDEFINED),
	_client(0),
	_timer (0)
{

	ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}				
};

FFMPEGReader::~FFMPEGReader () {

	delete _video_buffer; _video_buffer = 0;
	delete _audio_buffer; _audio_buffer = 0;

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
};

bool FFMPEGReader::setProvider (const ssi_char_t *name, IProvider *provider) {
	
	if (!_client)
	{
		_client = new FFMPEGReaderClient(this);
	}
	
	if(strcmp (name,SSI_FFMPEGREADER_VIDEO_PROVIDER_NAME)==0)	{
		setVideoProvider(provider);		
		return true;
	}
	if(strcmp(name,SSI_FFMPEGREADER_AUDIO_PROVIDER_NAME)==0){
		setAudioProvider(provider);		
		return true;
	}
	ssi_wrn ("unkown provider name '%s'", name);
	return false;
};

void FFMPEGReader::setVideoProvider(IProvider *provider){

	if(_video_provider){
		ssi_wrn ("provider already set");
		return;
	}

	_video_provider = provider;	
	_mode = _mode == MODE::UNDEFINED ? MODE::VIDEO : MODE::AUDIOVIDEO;	

	if (_options.stream) {
		ssi_video_params (_video_format, _options.width, _options.height, _options.fps, 8, 3);
	} else {
		if (!_client->peekVideoFormat (_video_format)) {
			ssi_wrn ("could not determine video format, use default options");
			ssi_video_params (_video_format, _options.width, _options.height, _options.fps, 8, 3);
		}
	}
	
	_video_provider->setMetaData (sizeof (ssi_video_params_t), &_video_format);
	ssi_stream_init (_video_channel.stream, 0, 1, ssi_video_size (_video_format), SSI_IMAGE, _video_format.framesPerSecond);
	_video_provider->init (&_video_channel);	
};

void FFMPEGReader::setAudioProvider(IProvider *provider){

	if (_audio_provider){
		ssi_wrn ("provider already set");
		return;
	}
	
	_audio_provider = provider;
	_mode = _mode == MODE::UNDEFINED ? MODE::AUDIO : MODE::AUDIOVIDEO;	

	if (_options.stream) {
		_audio_sr = _options.asr;
	} else {
		if (!_client->peekAudioFormat (_audio_sr)) {
			ssi_wrn ("could not determine audio format, use default options");
			_audio_sr = _options.asr;
		}
	}
	
	ssi_stream_init (_audio_channel.stream, 0, 1, sizeof(ssi_real_t), SSI_FLOAT, _audio_sr);
	_audio_provider->init(&_audio_channel);

};


bool FFMPEGReader::connect () {

	if (!_client)
	{
		_client = new FFMPEGReaderClient(this);
	}

	if (!(_video_provider||_audio_provider)) {
		ssi_wrn ("provider not set");
		return false;
	};

	FFMPEGReaderClient::SetLogLevel (ssi_log_level);

	if (_mode != MODE::AUDIO) {
		_video_buffer = new FFMPEGVideoBuffer (_options.buffer, _video_format.framesPerSecond, ssi_video_size (_video_format));
		_video_buffer->reset ();
	}

	if (_mode != MODE::VIDEO) {
		if (_mode == MODE::AUDIOVIDEO) {
			_audio_buffer = new FFMPEGAudioBuffer (_options.buffer, _audio_channel.stream.sr, 1.0/_video_format.framesPerSecond);
		} else {
			_audio_buffer = new FFMPEGAudioBuffer (_options.buffer, _audio_channel.stream.sr, _options.ablock);
		}
		_audio_buffer->reset ();
	}

	_client->start();
	_wait_event.block();

	return true;
};

void FFMPEGReader::enter () {

	ssi_char_t str[SSI_MAX_CHAR];
	ssi_sprint (str, "FFMPEGReader@%s", _options.url);
	Thread::setName (str);

	if (_mode == MODE::AUDIO) { 
		_timer = new Timer (_options.ablock);	
	} else {
		_timer = new Timer (1.0/_video_format.framesPerSecond);	
	}
}

void FFMPEGReader::run () {

	if (_video_provider) {
		
		ssi_size_t n_bytes;
		bool is_old;
		ssi_byte_t *frame = _video_buffer->pop(n_bytes, is_old);
		
		SSI_ASSERT (n_bytes == ssi_video_size (_video_format));
		
		if (!_options.bestEffort || !is_old) //in bestEffort mode we only provide new frames
		{
			bool result = _video_provider->provide(frame, 1);

			if (!_options.stream) {
				if (result) {
					_video_buffer->pop_done();
				}
			}
			else {
				_video_buffer->pop_done();
			}

			SSI_DBG(SSI_LOG_LEVEL_DEBUG, "video buffer : %.2f%% ", _video_buffer->getFillState() * 100);
		}
	}

	if (_audio_provider) {

		ssi_size_t n_samples;
		bool is_old;
		ssi_real_t *chunk = _audio_buffer->pop(n_samples, is_old);

		if (!_options.bestEffort || !is_old) //in bestEffort mode we only provide new frames
		{
			bool result = _audio_provider->provide(ssi_pcast(ssi_byte_t, chunk), n_samples);
			if (!_options.stream) {
				if (result) {
					_audio_buffer->pop_done(n_samples);
				}
			}
			else {
				_audio_buffer->pop_done(n_samples);
			}

			SSI_DBG(SSI_LOG_LEVEL_DEBUG, "audio buffer : %.2f%% ", _audio_buffer->getFillState() * 100);
		}
	}

	if (_client->getState() == FFMPEGReaderClient::STATE::IDLE || _client->getState() == FFMPEGReaderClient::STATE::TERMINATE)
		_wait_event.release();

	if (!_options.bestEffort)
		_timer->wait ();
}

void FFMPEGReader::flush () {
	
	delete _timer; _timer = 0;
}

bool FFMPEGReader::disconnect () {

	_client->stop ();

	delete _client;_client=0;
	delete _timer; _timer = 0;

	return true;
};

bool FFMPEGReader::pushVideoFrame (ssi_size_t n_bytes, ssi_byte_t *frame) {

	SSI_ASSERT (n_bytes == ssi_video_size (_video_format));

	return _video_buffer->push (frame);	
}

bool FFMPEGReader::pushAudioChunk (ssi_size_t n_samples, ssi_real_t *chunk) {

	return _audio_buffer->push (n_samples, chunk);
}

void FFMPEGReader::wait() {
	_wait_event.wait();
}

}
