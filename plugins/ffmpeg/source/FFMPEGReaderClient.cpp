// FFMPEGReaderClient.cpp
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

#include "FFMPEGReaderClient.h"

namespace ssi {

ssi_char_t *FFMPEGReaderClient::ssi_log_name = "ffread____";

void FFMPEGReaderClient::PrintErrorMsg (const char *func, int msg) {
	static char errbuf[SSI_MAX_CHAR];
	av_strerror (msg, errbuf, SSI_MAX_CHAR);	
	ssi_wrn ("%s() failed: %s", func, errbuf);
}

FFMPEGReaderClient::FFMPEGReaderClient (FFMPEGReader *reader) 
: _mode (FFMPEGReader::MODE::UNDEFINED),
	_state (STATE::IDLE),
	_fmt_ctx (0),
	_video_codec (0),
	_audio_codec (0),
	_video_codec_context (0),
	_audio_codec_context (0),
	_video_stream (0),
	_audio_stream (0),
	_video_stream_idx (-1),
	_audio_stream_idx (-1),
	_video_frame (0),
	_audio_frame (0),
	_video_frame_rgb (0),
	_audio_convert_data (0),
	_audio_convert_size (0),
	_video_convert_ctx (0),
	_audio_convert_ctx (0),
	_reader (reader) {

	ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

	av_register_all();
	avcodec_register_all();
	avdevice_register_all();
	avformat_network_init();
	av_log_set_level (AV_LOG_FATAL);		
};

FFMPEGReaderClient::~FFMPEGReaderClient () {

};

int FFMPEGReaderClient::InterruptCB (void *ctx) {

	FFMPEGReaderClient *me = ssi_pcast (FFMPEGReaderClient, ctx);
	if (me->_state == FFMPEGReaderClient::STATE::TERMINATE) {
		return 1;
	}
    return 0;
} 

bool FFMPEGReaderClient::open () {

	close ();

	_mode = _reader->_mode;

	_fmt_ctx=0;
	_audio_codec_context=0;
	_video_codec_context=0;
	_audio_stream=0;
	_video_stream=0;
	_video_frame=0;
	_audio_frame=0;
	_video_frame_rgb=0;
	int ret = 0;
	
	// fast streaming
	if (_reader->getOptions ()->stream) {
		_fmt_ctx=avformat_alloc_context();
		_fmt_ctx->max_delay=1;
		_fmt_ctx->flags|=AVFMT_FLAG_NOBUFFER;
		_fmt_ctx->max_delay=0;
		_fmt_ctx->max_picture_buffer=0;
		_fmt_ctx->avoid_negative_ts=1;
		_fmt_ctx->fps_probe_size=0;
		_interrupt_cb.callback = InterruptCB;
		_interrupt_cb.opaque = this;
		_fmt_ctx->interrupt_callback = _interrupt_cb; 
	}

	ssi_msg (SSI_LOG_LEVEL_BASIC, "open url '%s'", _reader->_options.url);
	

	int result = avformat_open_input (&_fmt_ctx, _reader->_options.url, NULL, NULL);
	if(result != 0){
		PrintErrorMsg ("avformat_open_input", result);
		_fmt_ctx=0;
		return false;
	}

	av_read_play(_fmt_ctx); // do we have to call this?

	result = avformat_find_stream_info (_fmt_ctx, NULL);
	if (result < 0){
		PrintErrorMsg ("avformat_find_stream_info", result);
		return false;
	}

	if (_mode != FFMPEGReader::MODE::AUDIO) {

		ssi_msg (SSI_LOG_LEVEL_DETAIL, "find best video stream");
		ret = av_find_best_stream (_fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
		if (ret < 0) {
			PrintErrorMsg ("av_find_best_stream", ret);
			return false;
		} 

		_video_stream_idx = ret;
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "opening video stream");
		_video_stream = _fmt_ctx->streams[_video_stream_idx];
		if (!_video_stream){
			ssi_wrn ("no video stream was found");
			return false;
		}
		
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "find video decoder");
		_video_codec_context = _video_stream->codec;
		_video_codec = avcodec_find_decoder (_video_codec_context->codec_id);
		if (!_video_codec) {
			ssi_err ("failed to find %s codec", av_get_media_type_string (AVMEDIA_TYPE_VIDEO));			
			return false;
		}

		_video_codec_context->thread_count = 0;
		_video_codec_context->thread_type = FF_THREAD_FRAME;

		// fast streaming
		if (_reader->getOptions ()->stream) {
			if (_video_codec->capabilities & CODEC_CAP_TRUNCATED) {
				_video_codec_context->flags |= CODEC_FLAG_TRUNCATED;
			}
			_video_codec_context->thread_type  = FF_THREAD_SLICE;
			_video_codec_context->thread_count = 0;
		}		

		ssi_msg (SSI_LOG_LEVEL_DETAIL, "open video decoder");
		if ((ret = avcodec_open2 (_video_codec_context, _video_codec, NULL)) < 0) {				
			ssi_wrn ("failed to open %s codec", av_get_media_type_string (AVMEDIA_TYPE_VIDEO));
			return false;
		}	

		ssi_msg (SSI_LOG_LEVEL_DETAIL, "allocate temporary video frames");
        _video_frame = av_frame_alloc();
        _video_frame_rgb = av_frame_alloc();
        avpicture_alloc ((AVPicture *) _video_frame_rgb, AV_PIX_FMT_BGR24, _video_codec_context->width, _video_codec_context->height);
		if (!_video_frame || !_video_frame_rgb) {
			ssi_wrn("could not allocate frames");
			return false;
		}

		int w = _video_codec_context->width;
		int h = _video_codec_context->height;
        _video_convert_ctx = sws_getCachedContext (NULL,w,h,_video_codec_context->pix_fmt,w,h,AV_PIX_FMT_BGR24,SWS_BILINEAR,NULL,NULL,NULL);
						
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "demuxing video");				
	}

	if (_mode != FFMPEGReader::MODE::VIDEO){

		ssi_msg (SSI_LOG_LEVEL_DETAIL, "find best audio stream");
		ret = av_find_best_stream(_fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
		if (ret < 0) {
			PrintErrorMsg ("av_find_best_stream", ret);
			return false;
		} 

		_audio_stream_idx = ret;
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "opening audio stream");
		_audio_stream = _fmt_ctx->streams[_audio_stream_idx];
		if (!_audio_stream){
			ssi_wrn ("no audio stream was found");
			return false;
		}
		
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "find audio decoder");
		_audio_codec_context = _audio_stream->codec;
		_audio_codec = avcodec_find_decoder (_audio_codec_context->codec_id);
		if (!_audio_codec) {
			ssi_err ("failed to find %s codec", av_get_media_type_string(AVMEDIA_TYPE_AUDIO));			
			return false;
		}
		if ((ret = avcodec_open2 (_audio_codec_context, _audio_codec, NULL)) < 0) {				
			ssi_wrn ("failed to open %s codec", av_get_media_type_string(AVMEDIA_TYPE_AUDIO));
			return false;
		}
		
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "allocate temporary audio frames");
        _audio_frame = av_frame_alloc();
		if (_audio_codec_context->channels != 1 || _audio_codec_context->sample_fmt != AV_SAMPLE_FMT_FLT) {				

			_audio_convert_ctx = swr_alloc();
			if (!_audio_convert_ctx) {
				ssi_wrn ("could not allocate resampler context");
				return false;
			}

			swr_alloc_set_opts (_audio_convert_ctx, 1, AV_SAMPLE_FMT_FLT, _audio_codec_context->sample_rate,
				_audio_codec_context->channels, _audio_codec_context->sample_fmt, _audio_codec_context->sample_rate, 0, NULL);

			if ((ret = swr_init (_audio_convert_ctx)) < 0) {
				ssi_wrn ("failed to initialize the resampling context");
				return false;
			}				
		}
			
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "demuxing audio");
			
	}

	if (!_audio_stream && !_video_stream) {
		SSI_DBG (SSI_LOG_LEVEL_ERROR,"could not find audio or video stream in the input, aborting");
		return false;

	}

	if (!_reader->_options.stream && _reader->_options.offset > 0) {
		ssi_size_t frameIndex = 0;
		if (_mode == FFMPEGReader::MODE::AUDIO) {
			frameIndex = ssi_cast (ssi_size_t, _reader->_options.offset * _reader->_audio_sr + 0.5);
		} else {
			frameIndex = ssi_cast (ssi_size_t, _reader->_options.offset * _reader->_video_format.framesPerSecond + 0.5);
		}
		if (!seek (frameIndex)) {
			ssi_wrn ("could not seek desired location, playing from start instead");
		}
	}

	return true;
};

bool FFMPEGReaderClient::seek (ssi_size_t frameIndex) {

    if(!_fmt_ctx) {
        return false;
	}

	int64_t timeBase = 0;
	if (_mode == FFMPEGReader::MODE::AUDIO) {
		timeBase = (int64_t (_audio_codec_context->time_base.num) * AV_TIME_BASE) / int64_t (_audio_codec_context->time_base.den);
	} else {
		timeBase = (int64_t (_video_codec_context->time_base.num) * AV_TIME_BASE) / int64_t (_video_codec_context->time_base.den);
	}
    int64_t seekTarget = int64_t(frameIndex) * timeBase;

	int ret = av_seek_frame (_fmt_ctx, -1, seekTarget, AVSEEK_FLAG_ANY);
    if (ret < 0) {
		PrintErrorMsg ("av_seek_frame", ret);
		return false;
	}

	return true;
}

void FFMPEGReaderClient::enter () {	

	ssi_char_t str[SSI_MAX_CHAR];
	ssi_sprint (str, "FFMPEGReaderClient@%s", _reader->_options.url);
	Thread::setName (str);

	_state = STATE::CONNECT;
}

void FFMPEGReaderClient::terminate () {	
	_state = STATE::TERMINATE;
}

void FFMPEGReaderClient::flush () {	
	close();
	_state = STATE::IDLE;
}

bool FFMPEGReaderClient::close () {	

	if(_fmt_ctx){

		av_read_pause(_fmt_ctx);		

		// close video codec and frames
		if (_video_codec_context) {
			avcodec_close(_video_codec_context);
			_video_codec_context = 0;
			_video_codec = 0;
		}
		if (_video_convert_ctx) {
			sws_freeContext(_video_convert_ctx);
			_video_convert_ctx = 0;
		}
		if(_video_frame){
            av_frame_free(&_video_frame);
			_video_frame=0;
		}
		if (_video_frame_rgb) {
			avpicture_free((AVPicture *)_video_frame_rgb);
			av_frame_free(&_video_frame_rgb);
			_video_frame_rgb = 0;
		}

		// close audio codec and buffer
		if (_audio_codec_context) {
			avcodec_close(_audio_codec_context);
			_audio_codec_context = 0;
			_audio_codec = 0;
		}
		if (_audio_convert_ctx) {
			swr_free(&_audio_convert_ctx);
			_audio_convert_ctx = 0;
		}
		if (_audio_convert_data) {
			av_free (_audio_convert_data[0]);
			av_free (_audio_convert_data);
			_audio_convert_data = 0;
			_audio_convert_size = 0;	
		}
		if (_audio_frame) {
			av_frame_free(&_audio_frame);
			_audio_frame = 0;
		}
	
		avformat_close_input(&_fmt_ctx);
		_fmt_ctx=0;
	}

	return true;
};

void FFMPEGReaderClient::run () {

	while (_state == STATE::CONNECT) {
		if (!open ()) {			
			ssi_wrn("could not open stream '%s', retry..", _reader->_options.url);
			sleep_ms(100);			
		} else {
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "start demuxing '%s'", _reader->_options.url);
			_state = STATE::READ;

			av_init_packet (&_packet);
			_packet.data = NULL;
			_packet.size = 0;
		}
	}

	if (_state == STATE::READ) {
		read ();
	}

	if (_state == STATE::TERMINATE) {
		if (!close ()) {
			ssi_wrn ("could not close stream '%s'", _reader->_options.url);
		} else {
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "stop demuxing '%s'", _reader->_options.url);
		}
	}
};

int FFMPEGReaderClient::read () {

	int ret = 0;	

	/*if (ret != 0) {
		PrintErrorMsg ("av_read_frame", ret);
	}*/
	
	if (av_read_frame(_fmt_ctx, &_packet) >= 0) {
		AVPacket orig_pkt = _packet;
		do {
			ret = decodePacket();
			if (ret < 0) {
				break;
			}
			_packet.size -= ret;
			_packet.data += ret;
		} while (_packet.size > 0);
		av_packet_unref(&orig_pkt);
	}
	else
	{
		//end of file ...hopefully
		_state = STATE::IDLE;
	}


	/*
	_packet.data = NULL;
    _packet.size = 0;
    do {
        decode_packet(&got_frame, 1);
    } while (got_frame); 
	*/

	return ret;
}

int FFMPEGReaderClient::decodePacket () {

	int gotVideoFrame = 0;
	int gotAudioFrame = 0;
	int ret = -1;

	if (_mode != FFMPEGReader::MODE::AUDIO && _packet.stream_index == _video_stream_idx) {
		ret = avcodec_decode_video2 (_video_codec_context, _video_frame, &gotVideoFrame, &_packet);
		if (ret > 0 && gotVideoFrame) {
			provideVideoFrame ();
		}
	} 
	
	if (_mode != FFMPEGReader::MODE::VIDEO && _packet.stream_index == _audio_stream_idx) {
        av_frame_unref (_audio_frame);
		ret = avcodec_decode_audio4 (_audio_codec_context,_audio_frame, &gotAudioFrame, &_packet);
		if (gotAudioFrame) {
			provideAudioFrame ();				
		}
	}

	return ret;
};

void FFMPEGReaderClient::provideVideoFrame (){
	
	int result = sws_scale (_video_convert_ctx, _video_frame->data, _video_frame->linesize, 0, _video_codec_context->height, _video_frame_rgb->data, _video_frame_rgb->linesize);

	/*static int i = 0;
	static char filename[255];
	sprintf (filename, "writeVideoFrame%03d.bmp", i++);
	ssi_write_bmp (filename, (BYTE *) _video_frame_rgb->data[0], _video_frame_rgb->linesize[0] * _video_codec_context->height, _video_codec_context->width, _video_codec_context->height, 24);
	*/

	if (result ==_video_codec_context->height){
		/*static int counter = 0;			
		ssi_print ("provide #%d %u\n", counter++, ssi_toc ());
		ssi_tic ();*/
		
		while (_state == STATE::READ && !_reader->pushVideoFrame (result * _video_frame_rgb->linesize[0], ssi_pcast (ssi_byte_t, _video_frame_rgb->data[0]))) {
			sleep_ms (10);
		}
	}	
};

void FFMPEGReaderClient::provideAudioFrame () {

	int ret;
	uint8_t *samples = _audio_frame->data[0];
	int n_samples = _audio_frame->nb_samples;

	/*static FILE *fid = fopen ("provideAudioFrame", "wb");
	fwrite (samples, n_samples, sizeof (short), fid);
	fflush (fid);*/

	if (_audio_convert_ctx) {

		if (_audio_convert_size < n_samples) {

			if (_audio_convert_data) {
				av_free (_audio_convert_data[0]);
				av_free (_audio_convert_data);
			}

			ret = av_samples_alloc_array_and_samples (&_audio_convert_data, &_audio_convert_size, 1, n_samples, AV_SAMPLE_FMT_FLT, 0);
			if (ret < 0) {
				ssi_wrn ("could not allocate destination samples");
				return;
			}

			_audio_convert_size = n_samples;
		}

		ret = swr_convert (_audio_convert_ctx, _audio_convert_data, _audio_convert_size, (const uint8_t **) &_audio_frame->data[0], n_samples);
        if (ret < 0) {
            ssi_wrn ("error while converting audio chunk");
            return;
        }
		samples = _audio_convert_data[0];
		n_samples = ret;
	}

	while (_state == STATE::READ && !_reader->pushAudioChunk (n_samples, (float *) samples)) {
		sleep_ms (10);
	}

};

bool FFMPEGReaderClient::peekVideoFormat (ssi_video_params_t &params) {

	int ret;
	AVFormatContext *fmt_ctx = NULL;
	bool found = false;
	
	ssi_msg (SSI_LOG_LEVEL_DETAIL, "try to determine video format '%s'",  _reader->_options.url);
	if ((ret = avformat_open_input(&fmt_ctx, _reader->_options.url, NULL, NULL))) {
		PrintErrorMsg ("avformat_open_input", ret);  
		return false;
	}

	for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {		
		if (fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {

			AVRational sr = fmt_ctx->streams[i]->avg_frame_rate;
			int width = fmt_ctx->streams[i]->codec->width;
			int height = fmt_ctx->streams[i]->codec->height;

			if (sr.den > 0 && sr.num > 0 && width > 0 && height > 0) {
				ssi_video_params (params, width, height, ((double) sr.num) / sr.den, 8, 3);				
				found = true;
				break;
			}
			
		}
	}

	avformat_close_input(&fmt_ctx);

	return found;
}

bool FFMPEGReaderClient::peekAudioFormat (ssi_time_t &audio_sr, ssi_size_t &n_samples) {

	int ret;
	AVFormatContext *fmt_ctx = NULL;
	bool found = false;
	
	ssi_msg (SSI_LOG_LEVEL_DETAIL, "try to determine audio format '%s'",  _reader->_options.url);
	if ((ret = avformat_open_input(&fmt_ctx, _reader->_options.url, NULL, NULL))) {
		PrintErrorMsg ("avformat_open_input", ret);  
		return false;
	}

	if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
		PrintErrorMsg("avformat_find_stream_info", ret);		
		return false;
	}

	for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {		
		if (fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {

			int sr = fmt_ctx->streams[i]->codec->sample_rate;
			
			n_samples = (ssi_size_t) ceil(fmt_ctx->streams[i]->duration * av_q2d(fmt_ctx->streams[i]->time_base) * sr);
			
			if (sr > 0) {
				audio_sr = sr;				
				found = true;
				break;
			}
			
		}
	}

	avformat_close_input(&fmt_ctx);

	return found;
}

};
