// AviReader.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/07/02
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

#include "AviReader.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

const ssi_char_t *AviReader::ssi_log_name = "avireader_";

AviReader::AviReader (const ssi_char_t *file) 
	: _video_provider (0),
	_audio_provider (0),
	_video_stream (0),
	_audio_stream (0),
	_video_frame_first (0),
	_video_frame_last (0),
	_audio_frame_first (0),
	_audio_sample_last (0),
	_video_frame (0),
	_audio_frame (0),	
	_frame_time (0),
	_frame_counter (0),
	_frame_timer (0),	
	_offset_in_frames (0),
	_offset_in_samples (0),
	_picData (0),
	_picDataTmp (0),
	_is_providing (false),
	_avi (0),
	_file (0) {

	_wait_event = new Event (false, false);

	// init COM
	CoInitializeEx (NULL, COINIT_MULTITHREADED);
	// init avi library
	AVIFileInit ();

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

}

bool AviReader::setProvider (const ssi_char_t *name, IProvider *provider) {

	if (strcmp (name, SSI_AVIREADER_AUDIO_PROVIDER_NAME) == 0) {
		setAudioProvider (provider);
		return true;
	} else if (strcmp (name, SSI_AVIREADER_VIDEO_PROVIDER_NAME) == 0) {
		setVideoProvider (provider);
		return true;
	}

	ssi_wrn ("unkown provider name '%s'", name);

	return false;
}

void AviReader::setVideoProvider (IProvider *provider) {

	if (!provider) {
		return;
	}

	if (_video_provider) {
		ssi_wrn ("video provider already set");
		return;
	}

	int res = 0;

	// open avi file	
	if (!_avi) {
		if (!AviTools::OpenAviFile (_options.path, _avi, _avi_info)) {
			ssi_err ("could not open avi file '%s'", _options.path);
		}
	}
 
	// open video stream
	_video_provider = provider;

	if (!AviTools::OpenAviStream (_avi, _video_stream, _video_stream_info, _options.video_stream_index, streamtypeVIDEO)) {
		ssi_err ("could not open video stream #%d", _options.video_stream_index);
	}
	if (!AviTools::GetAviStreamSize (_avi, _video_stream, _video_frame_first, _video_frame_last)) {
		ssi_err ("could not determine length of video stream #%d", _options.video_stream_index);
	}
	if (!AviTools::GetVideoFormat (_video_stream, _video_format_info_in, _video_format_info_out)) {
		ssi_err ("could not determine format of video stream #%d", _options.video_stream_index);
	}			
	_video_frame = AVIStreamGetFrameOpen (_video_stream, &(_video_format_info_out.bmiHeader));			
	int n_channel = _video_format_info_out.bmiHeader.biBitCount / 8;
	ssi_video_params (_video_format, _video_format_info_in.bmiHeader.biWidth,  
		_video_format_info_out.bmiHeader.biHeight,
		_video_stream_info.dwRate / _video_stream_info.dwScale,
		_video_format_info_out.bmiHeader.biBitCount / n_channel, 
		n_channel, 			
		SSI_GUID_NULL); 

	_video_provider->setMetaData (sizeof (_video_format), &_video_format);
	ssi_stream_init (_video_channel.stream, 0, 1, _video_format_info_out.bmiHeader.biSizeImage, SSI_IMAGE, _video_format.framesPerSecond);
	_video_provider->init (&_video_channel);
	_options.block = 1.0 / _video_format.framesPerSecond;
}


void AviReader::setAudioProvider (IProvider *provider) {

	if (!provider) {
		return;
	}

	if (_audio_provider) {
		ssi_wrn ("audio provider already set");
		return;
	}

	int res = 0;

	// open avi file	
	if (!_avi) {
		if (!AviTools::OpenAviFile (_options.path, _avi, _avi_info)) {
			ssi_err ("could not open avi file '%s'", _options.path);
		}
	}

	// open audio stream
	_audio_provider = provider;
		
	if (!AviTools::OpenAviStream (_avi, _audio_stream,_audio_stream_info, _options.audio_stream_index, streamtypeAUDIO)) {
		ssi_err ("could not open audio stream #%d", _options.audio_stream_index);
	}
	if (!AviTools::GetAviStreamSize (_avi, _audio_stream, _audio_frame_first, _audio_sample_last)) {
		ssi_err ("could not determine size of audio stream #%d", _options.audio_stream_index);
	}
	if (!AviTools::GetAudioFormat (_audio_stream, _audio_format)) {
		ssi_err ("could not determine format of audio stream #%d", _options.audio_stream_index);
	}		

	ssi_stream_init (_audio_buffer, 0, _audio_format.nChannels, _audio_format.nAvgBytesPerSec / (_audio_format.nSamplesPerSec * _audio_format.nChannels), SSI_SHORT, _audio_format.nSamplesPerSec);
	_audio_channel.stream = _audio_buffer;	
	_audio_provider->init (&_audio_channel);	
	
	_audio_frame = AVIStreamGetFrameOpen (_audio_stream, NULL);
}

AviReader::~AviReader() {

	delete _wait_event;

	// release audio buffer
	if(_audio_provider)
		ssi_stream_destroy (_audio_buffer);

	// close frames
	if (_video_frame) {
		AVIStreamGetFrameClose (_video_frame);
	}
	if (_audio_frame) {
		AVIStreamGetFrameClose (_audio_frame);
	}

	// close streams
	if (_video_stream) {
		AVIStreamRelease (_video_stream);
		ssi_msg (SSI_LOG_LEVEL_BASIC, "closed video stream #%d", _options.video_stream_index);
	}
	if (_audio_stream) {
		AVIStreamRelease (_audio_stream);
		ssi_msg (SSI_LOG_LEVEL_BASIC, "closed audio stream #%d", _options.audio_stream_index);
	}

	// close avi file
	if (_avi) {
 		AVIFileRelease (_avi);
	}		

	// clean avifile library
	AVIFileExit ();

	// uninitialize COM
	CoUninitialize ();

	delete[] _picData; _picData = 0;
	delete[] _picDataTmp; _picDataTmp = 0;

	ssi_msg (SSI_LOG_LEVEL_BASIC, "closed avi file '%s'", _options.path);

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool AviReader::connect () {

	if (!(_audio_provider || _video_provider)) {
		ssi_wrn ("provider not set");
		return false;
	}

	// init timer
	_frame_timer = 0;
	_frame_counter = 0;
	_frame_time = 0;

	// offset
	if (_options.offset > 0) {		
		_offset_in_samples = _audio_provider ? ssi_cast (ssi_size_t, _audio_format.nSamplesPerSec * _options.offset + 0.5) : 0;
		_offset_in_frames = _video_provider ? ssi_cast (ssi_size_t, _video_format.framesPerSecond * _options.offset + 0.5) : 0;
	}	

	// block wait event
	_wait_event->block ();

	// set thread name
	ssi_char_t *thread_name = ssi_strcat ("ssi_sensor_AviReader@", _options.path);
	Thread::setName (thread_name);
	delete[] thread_name;

	// to capture first frame
	_is_providing = false;

	return true;
}

void AviReader::run () {

	bool end_of_file = true;

	// get next video frame
	if (_video_provider) {
		if (ssi_cast (long, _offset_in_frames) + _frame_counter < _video_frame_last) {
			ssi_byte_t *frame = ssi_pcast (ssi_byte_t, AVIStreamGetFrame (_video_frame, _offset_in_frames + _frame_counter - _video_frame_first));	
			if (!frame) {
				ssi_wrn ("could not read video frame #%ld", _frame_counter);
			} else {

				BITMAPINFOHEADER info;
				memcpy (&info, frame, sizeof (BITMAPINFOHEADER));

				if (_options.flip || _options.mirror) {

					if (!_picData) {
						_picData = new ssi_byte_t[_video_format_info_out.bmiHeader.biSizeImage];
						_picDataTmp = new ssi_byte_t[_video_format_info_out.bmiHeader.biSizeImage];
					}

					memcpy (_picData, frame + sizeof (BITMAPINFOHEADER), _video_format_info_out.bmiHeader.biSizeImage);

					if (_options.flip) {

						int stride = ssi_video_stride (_video_format);
						int height = _video_format.heightInPixels;
						int copyLength = _video_format.widthInPixels * 3;
						ssi_byte_t *dstptr = _picDataTmp + (height - 1) * stride;
						ssi_byte_t *srcptr = _picData;
						for(int j = 0; j < height; ++j)
						{
							memcpy(dstptr, srcptr, copyLength);
							dstptr -= stride;
							srcptr += stride;
						}
						ssi_byte_t *tmp = _picDataTmp;
						_picDataTmp = _picData;
						_picData = tmp;
					}

					if (_options.mirror) {
						ssi_byte_t *dstptr = 0;
						const ssi_byte_t *srcptr = 0;
						int height = _video_format.heightInPixels;
						int width = _video_format.widthInPixels;
						int stride = ssi_video_stride (_video_format);
						for(int j = 0; j < height; ++j)
						{
							dstptr = _picDataTmp + j * stride;
							srcptr = _picData + j * stride + (width - 1) * 3;
							for (int i = 0; i < width; i++)
							{
								memcpy(dstptr, srcptr, 3);
								dstptr +=3;
								srcptr -=3;
							}
						}
						ssi_byte_t *tmp = _picData;
						_picData = _picDataTmp;
						_picDataTmp = tmp;
					}

					_is_providing = _video_provider->provide (_picData, 1);		

				} else {
					_is_providing = _video_provider->provide (frame + sizeof (BITMAPINFOHEADER), 1);		
				}
			}
			end_of_file = false;
		}
	}

	// get next audio frame
	if (_audio_provider) {
		long first_sample = _offset_in_samples + AVIStreamTimeToSample (_audio_stream, ssi_cast (long, _frame_time * 1000.0 + 0.5));
		long last_sample = _offset_in_samples + AVIStreamTimeToSample (_audio_stream, ssi_cast (long, (_frame_time + _options.block) * 1000.0 + 0.5));
		long delta_sample = last_sample - first_sample;
		if (last_sample < _audio_sample_last) {
			ssi_stream_adjust (_audio_buffer, delta_sample);
			int status = AVIStreamRead (_audio_stream, first_sample, delta_sample, _audio_buffer.ptr, _audio_buffer.tot, NULL, NULL);
			if (status) {
				ssi_wrn ("could not read audio chunk [%ld..%ld]", first_sample, last_sample);
			} else {
				_is_providing = _audio_provider->provide (_audio_buffer.ptr, delta_sample);
			}
			end_of_file = false;
		}
	}

	if (end_of_file) {
		// release wait event
		_interrupted = false;
		_wait_event->release ();
	}

	// increment frame counter
	if (_is_providing) {
		if (!_frame_timer) {
			_frame_timer = new Timer (_options.block);
		}
		++_frame_counter;
		_frame_time += _options.block;
		// sleep until it's time to catch the next frame
		if (!_options.best_effort_delivery) {
			_frame_timer->wait ();
		}
	}
}

bool AviReader::disconnect () {

	// delete timer
	delete _frame_timer; _frame_timer = 0;
	return true;
}

bool AviReader::wait()
{
	_wait_event->wait();

	return !_interrupted;
}

bool AviReader::cancel()
{
	_interrupted = true;
	_wait_event->release();

	return true;
}


}

 
