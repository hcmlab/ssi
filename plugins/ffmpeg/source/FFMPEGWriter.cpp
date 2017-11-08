// FFMPEGWriter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/05/13
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

#include "FFMPEGWriter.h"
#include "FFMPEGWriterClient.h"
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

ssi_char_t *FFMPEGWriter::ssi_log_name = "ffwrite___";

FFMPEGWriter::FFMPEGWriter(const char *file)
	: _client (0),
	_audio_format (0),
	_audio_index (-1),
	_video_index (-1),
	_mode (FFMPEGWriter::MODE::UNDEFINED),
	_ready(false),
	_is_url(false),
	_path_or_url(0),
	_file (0) {

	ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	_client = new FFMPEGWriterClient (this);
}

FFMPEGWriter::~FFMPEGWriter() {

	delete _client; _client = 0;

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void FFMPEGWriter::open()
{
	if (_ready)
	{
		return;
	}

	_ready = false;

	if (!_is_url)
	{
		if (_options.overwrite)
		{
			ssi_mkdir_r(FilePath(_options.path).getDir());
			_path_or_url = ssi_strcpy(_options.path);
		}
		else
		{
			FilePath fp(_options.path);
			ssi_char_t *ext = 0;
			if (fp.hasExtension())
			{
				ext = ssi_strcpy(fp.getExtension());
			}
			else
			{
				ext = ssi_strcat(".", _options.format);
			}

			_path_or_url = FilePath(_options.path, ext).getUnique(true);

			delete[] ext;
		}

		if (ssi_exists(_path_or_url))
		{
			remove(_path_or_url);
		}
	}
	else
	{
		_path_or_url = ssi_strcpy(_options.url);
	}

	ssi_msg(SSI_LOG_LEVEL_BASIC, "open '%s'", _path_or_url);

	_client->setLogLevel(ssi_log_level);

	if (!_client->open())
	{
		ssi_err("could not start encoder");
		return;
	}

	_ready = true;
}

void FFMPEGWriter::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {	

	_ready = false;

	if (_options.path[0] == '\0' && _options.url[0] == '\0')
	{
		ssi_err("path/url are empty");
		return;
	}

	_is_url = _options.path[0] == '\0';

	if (stream_in[0].type == SSI_IMAGE)
	{
		_video_index = 0;
		_mode = FFMPEGWriter::MODE::VIDEO;
	} else 
	{
		_audio_index = 0;
		_mode = FFMPEGWriter::MODE::AUDIO;
	}

	if (stream_in_num > 1)
	{
		if (stream_in[1].type == SSI_IMAGE && _mode != FFMPEGWriter::MODE::VIDEO) 
		{
			_video_index = 1;
			_mode = FFMPEGWriter::MODE::AUDIOVIDEO;
		} else if (_mode != FFMPEGWriter::MODE::AUDIO) {
			_audio_index = 1;
			_mode = FFMPEGWriter::MODE::AUDIOVIDEO;
		}				
	}

	if (_mode != FFMPEGWriter::MODE::VIDEO)
	{
		_audio_format = new WAVEFORMATEX(ssi_create_wfx(stream_in[_audio_index].sr,
			stream_in[_audio_index].dim,
			stream_in[_audio_index].byte));
	}

	if (_mode != FFMPEGWriter::MODE::AUDIO)
	{
		if (!(stream_in[_video_index].sr == _video_format.framesPerSecond
			&& stream_in[_video_index].byte == ssi_video_stride(_video_format) * _video_format.heightInPixels
			&& stream_in[_video_index].dim == 1))
		{
			ssi_err("invalid video format");
			return;
		}
	}

	open();	
}

void FFMPEGWriter::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (!_ready)
	{
		return;
	}

	if (_mode != FFMPEGWriter::MODE::VIDEO) {
		if (!_client->writeAudio(stream_in[_audio_index].num, stream_in[_audio_index].tot, stream_in[_audio_index].ptr)) {
			ssi_wrn("could not write audio chunk");
		}
	}

	if (_mode != FFMPEGWriter::MODE::AUDIO) {
		if (!_client->writeVideo(stream_in[_video_index].byte, stream_in[_video_index].ptr)) {
			ssi_wrn("could not write video frame");
		}
	}

	if (!_client->flushBuffer()) {
		ssi_wrn("could not flush buffer");
	}

}

void FFMPEGWriter::close()
{
	if (!_ready)
	{
		return;
	}

	ssi_msg(SSI_LOG_LEVEL_BASIC, "close '%s'", _path_or_url);

	if (!_client->close())
	{
		ssi_err("could not stop encoder");
	}

	_ready = false;
}


void FFMPEGWriter::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	close();

	delete _audio_format;
	_audio_format = 0;

	_audio_index = _video_index = -1;
	_mode = FFMPEGWriter::MODE::UNDEFINED;

	_ready = false;
}

// add empty frames
void FFMPEGWriter::consume_fail (const ssi_time_t fail_time, 
	const ssi_time_t fail_duration,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (!_ready)
	{
		return;
	}

	if (_mode != FFMPEGWriter::MODE::AUDIO) {

		// add empty frames
		ssi_size_t frames_to_add = ssi_cast (ssi_size_t, fail_duration * _video_format.framesPerSecond + 0.5);
		if (frames_to_add > 0) {
			ssi_byte_t *empty_video_data = new ssi_byte_t[stream_in[_video_index].byte];
			memset (empty_video_data, 0, stream_in[_video_index].byte);
			for (ssi_size_t i = 0; i < frames_to_add; ++i) {
				if (!_client->writeVideo (stream_in[_video_index].byte, empty_video_data)) {
					ssi_wrn ("could not write video frame");
				}
			}
			delete[] empty_video_data;
		}
	}

	if (_mode != FFMPEGWriter::MODE::VIDEO) {

		// create empty audio
		ssi_size_t samples_to_add = ssi_cast (ssi_size_t, fail_duration * stream_in[_audio_index].sr + 0.5);	
		if (samples_to_add > 0) {
			ssi_size_t bytes = samples_to_add * stream_in[_audio_index].byte * stream_in[_audio_index].dim;
			ssi_byte_t *empty_audio_data = new ssi_byte_t[bytes];
			memset (empty_audio_data, 0, bytes);
			if (!_client->writeAudio (samples_to_add, bytes, empty_audio_data)) {
				ssi_wrn ("could not write audio chunk");
			}
			delete[] empty_audio_data;
		}
	}

	if (!_client->flushBuffer ()) {
		ssi_wrn ("could not flush buffer");
	}

	ssi_msg (SSI_LOG_LEVEL_BASIC, "added %.2Lfs empty frames/chunks", fail_duration); 
}

bool FFMPEGWriter::notify(INotify::COMMAND::List command, const ssi_char_t *message)
{

	switch (command)
	{
	case INotify::COMMAND::SLEEP_POST:
	{
		if (!_is_url)
		{
			close();
		}
		return true;
	}
	case INotify::COMMAND::WAKE_PRE:
	{
		if (!_is_url)
		{
			open();
		}
		return true;
	}
	}

	return false;
}


}
