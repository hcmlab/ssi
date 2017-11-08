// FFMPEGWriterClient.cpp
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

#include "FFMPEGWriterClient.h"

namespace ssi {

ssi_char_t *FFMPEGWriterClient::ssi_log_name = "ffwrite___";

void FFMPEGWriterClient::PrintErrorMsg (const char *func, int msg) {
	static char errbuf[SSI_MAX_CHAR];
	av_strerror (msg, errbuf, SSI_MAX_CHAR);	
	ssi_wrn ("%s() failed: %s", func, errbuf);	
}

FFMPEGWriterClient::FFMPEGWriterClient (FFMPEGWriter *writer)
: _output_context (0),
	_output_format (0),
	_video_stream (0),
	_audio_stream (0),
	_video_codec (0),
	_audio_codec (0),
    _video_codec_context (0),
	_audio_codec_context (0),	
    _video_frame_yuv (0),
	_video_frame_rgb (0),
	_audio_frame (0),
	_video_convert_ctx (0),
	_audio_convert_ctx (0),
	_audio_convert_data (0),
	_audio_convert_size (0),
	_video_got_output (0),
	_audio_got_output (0),
	_audio_sample_count (0),
	_video_frame_count (0),
	_audio_chunk_count (0),
	_n_audio_chunk (0),
	_audio_buffer (0),	
	_video_buffer (0),
	_writer (writer),
	_mode (FFMPEGWriter::MODE::UNDEFINED),
	_ready (false)
{ 

	ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

    av_register_all();
	avcodec_register_all();
	avformat_network_init();
	av_log_set_level(AV_LOG_FATAL);			
};

FFMPEGWriterClient::~FFMPEGWriterClient () {
}

bool FFMPEGWriterClient::open () {

	int ret;
	int width = _writer->_video_format.widthInPixels;
	int height = _writer->_video_format.heightInPixels;	

	_mode = _writer->_mode;

	// create output media context
	ret = avformat_alloc_output_context2 (&_output_context, 0, 0, _writer->_path_or_url);
	if (!_output_context) {		 			
		ret = avformat_alloc_output_context2(&_output_context, NULL, _writer->_options.format, _writer->_path_or_url);
		if (!_output_context) {		 			
			PrintErrorMsg ("avformat_alloc_output_context2", ret);
			return false;
		}
	}
	_output_format = _output_context->oformat;

	// create video codec
	if (_mode != FFMPEGWriter::MODE::AUDIO) {
		if (_output_format->video_codec == AV_CODEC_ID_NONE || !openVideo ()) {
			ssi_wrn ("could not create video codec");
			return false;
		}
	}

	// create audio codec
	if (_mode != FFMPEGWriter::MODE::VIDEO) {
		if (_output_format->audio_codec == AV_CODEC_ID_NONE || !openAudio ()) {
			ssi_wrn ("could not create audio codec");
		}
	}	

	av_dump_format (_output_context, 0, _writer->_path_or_url, 1);

	// open the output file, if needed
	if (!(_output_format->flags & AVFMT_NOFILE)) {
        ret = avio_open (&_output_context->pb, _writer->_path_or_url, AVIO_FLAG_WRITE);
        if (ret < 0) {
            PrintErrorMsg ("avio_open", ret);
			return false;
        }
    }

	// Write the stream header, if any
	ret = avformat_write_header (_output_context, NULL);
    if (ret < 0) {
        PrintErrorMsg ("avformat_write_header", ret);
		return false;
    }

	// output sdp file
	if (_writer->_options.sdp[0] != '\0') {
		char buf[1000];
		av_sdp_create (&_output_context, 1, buf, 1000);
		FILE *fid = fopen (_writer->_options.sdp, "w");
		if (fid) {
			fprintf (fid, "%s", buf);
			fclose (fid);
		} else {
			ssi_err ("could not create sdp file '%s'", _writer->_options.sdp);
		}
	}

	// create buffer	
	if (_mode == FFMPEGWriter::MODE::AUDIO) {
		if (_n_audio_chunk > 0 && _writer->_options.abuffer > 0) {
			_audio_buffer = new FFMPEGAudioBuffer (_writer->_options.abuffer, _writer->_audio_format->nSamplesPerSec, 0);
		}
	} else if (_mode == FFMPEGWriter::MODE::AUDIOVIDEO) {
		if (_writer->_options.vbuffer > 0) {
			_video_buffer = new FFMPEGVideoBuffer (_writer->_options.vbuffer, _writer->_video_format.framesPerSecond, ssi_video_size (_writer->_video_format));
		}
		if (_writer->_options.abuffer > 0) {
			_audio_buffer = new FFMPEGAudioBuffer (_writer->_options.abuffer, _writer->_audio_format->nSamplesPerSec, 0);
		}
	}

	_ready = true;

	return true;
}

bool FFMPEGWriterClient::openAudio () {

    int ret;
	
	// find codec
	if (_writer->_options.stream) {
		//_audio_codec = avcodec_find_encoder (AV_CODEC_ID_AAC);
		_audio_codec = avcodec_find_encoder (_output_format->audio_codec);
	} else if (_writer->_options.acodec[0] != '\0') {
		_audio_codec = avcodec_find_encoder_by_name (_writer->_options.acodec);
		if (!_video_codec) {
			ssi_wrn ("audio codec not found (%s)", _writer->_options.acodec);
			return false;
		}
	} else {
		_audio_codec = avcodec_find_encoder (_output_format->audio_codec);
		if (!_audio_codec) {
			ssi_wrn ("audio codec not found (%u)", _output_format->audio_codec);
			return false;
		}
	}

	// create stream
	_audio_stream = avformat_new_stream (_output_context, _audio_codec);
    if (!_audio_stream) {
        ssi_wrn ("could not allocate stream");
        return false;
    }
    _audio_stream->id = _output_context->nb_streams-1;    	

	// config codec context
	_audio_codec_context = _audio_stream->codec;
	_audio_codec_context->sample_fmt = _audio_codec->sample_fmts ? _audio_codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
	_audio_codec_context->channels = _writer->_audio_format->nChannels;		
	_audio_codec_context->sample_rate = _writer->_audio_format->nSamplesPerSec;

	if (_writer->_options.audio_bit_rate_kb != 0) {
		_audio_codec_context->bit_rate = _writer->_options.audio_bit_rate_kb * 1000;
	}

	// Some formats want stream headers to be separate
    if (_output_context->oformat->flags & AVFMT_GLOBALHEADER) {
		_audio_codec_context->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}

    // allocate and init a re-usable frame
    _audio_frame = av_frame_alloc();
    if (!_audio_stream) {
		ssi_wrn ("could not allocate audio frame");
        return false;
    }

    // open codec
    ret = avcodec_open2 (_audio_codec_context, _audio_codec, NULL);
    if (ret < 0) {
		PrintErrorMsg ("avcodec_open2", ret);
        return false;
    }    

	// determine chunk size
	if (_audio_codec_context->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE) {
		if (_mode == FFMPEGWriter::MODE::AUDIO) {
			_n_audio_chunk = 0;
		} else {
			_n_audio_chunk = ssi_cast (ssi_size_t, _writer->_audio_format->nSamplesPerSec / _writer->_video_format.framesPerSecond + 0.5);
		}
	} else {	
		_n_audio_chunk = _audio_codec_context->frame_size;	
	}

	// determine incoming sample format
	AVSampleFormat sample_fmt_in = _writer->_audio_format->wBitsPerSample == 16 ? AV_SAMPLE_FMT_S16 : AV_SAMPLE_FMT_FLT;

	// if necessary create converting context
	if (_audio_codec_context->sample_fmt != sample_fmt_in) {				

		_audio_convert_ctx = swr_alloc();
		if (!_audio_convert_ctx) {
			ssi_wrn ("could not allocate resampler context");
			return false;
		}

		swr_alloc_set_opts (_audio_convert_ctx, _audio_codec_context->channels, _audio_codec_context->sample_fmt, _audio_codec_context->sample_rate, _audio_codec_context->channels, sample_fmt_in, _audio_codec_context->sample_rate, 0, NULL);

		if ((ret = swr_init (_audio_convert_ctx)) < 0) {
			ssi_wrn ("failed to initialize the resampling context");
			return false;
		}				
	}

	return true;
}

bool FFMPEGWriterClient::openVideo () {

	int ret;
	int width = _writer->_video_format.widthInPixels;
	int height = _writer->_video_format.heightInPixels;
	int channels = _writer->_video_format.numOfChannels;
	
	if (_writer->_options.stream) {
		_video_codec = avcodec_find_encoder (AV_CODEC_ID_H264);
		if (!_video_codec) {
			ssi_wrn ("video codec not found (%u)", AV_CODEC_ID_H264);
			return false;
		}
	} else if (_writer->_options.vcodec[0] != '\0') {
		_video_codec = avcodec_find_encoder_by_name (_writer->_options.vcodec);
		if (!_video_codec) {
			ssi_wrn ("video codec not found (%s)", _writer->_options.vcodec);
			return false;
		}
	} else {
		_video_codec = avcodec_find_encoder (_output_format->video_codec);
		if (!_video_codec) {
			ssi_wrn ("video codec not found (%u)", _output_format->video_codec);
			return false;
		}
	}		

	_video_stream = avformat_new_stream (_output_context, _video_codec);
    if (!_video_stream) {
        ssi_wrn ("could not allocate stream");
        return false;
    }
    _video_stream->id = _output_context->nb_streams-1;
    _video_codec_context = _video_stream->codec;

	// config video context
	_video_codec_context->width = width;
	_video_codec_context->height = height;	
	_video_codec_context->time_base.den = (int) _writer->_video_format.framesPerSecond;
	_video_codec_context->time_base.num = 1;
	_video_codec_context->pix_fmt = AV_PIX_FMT_YUV420P; 
	_video_codec_context->thread_count = 0;
	_video_codec_context->thread_type = FF_THREAD_FRAME;
	
	if (_writer->_options.video_bit_rate_kb != 0) {
		_video_codec_context->bit_rate = _writer->_options.video_bit_rate_kb;
	}
	// emit one intra frame every ten frames
	_video_codec_context->gop_size = 12;
	// add b frames
	_video_codec_context->max_b_frames = 1;

	AVDictionary *video_codec_opt = NULL;

	// low latency
	if (_writer->_options.stream) {
		_video_codec_context->refs = 3;	
		av_opt_set(_video_codec_context->priv_data, "crf", "23", 0);	
		_video_codec_context->flags2 |= CODEC_FLAG2_FAST;
		_video_codec_context->flags2 |= CODEC_FLAG_LOW_DELAY;	 
		//_video_codec_context->coder_type = FF_CODER_TYPE_VLC;
		//_video_codec_context->me_method = 7;
		_video_codec_context->me_subpel_quality = 4;
		_video_codec_context->trellis = 0;    
		_video_codec_context->thread_count = 0;
		_video_codec_context->thread_type = FF_THREAD_SLICE;
		_video_codec_context->delay = 0;
		_video_codec_context->gop_size = 0;
		_video_codec_context->max_b_frames = 0;
		_video_codec_context->keyint_min = 0;
		av_opt_set(_video_codec_context->priv_data, "tune", "zerolatency", 0);
		av_opt_set(_video_codec_context->priv_data, "preset", "ultrafast", 0);
		av_opt_set(_video_codec_context->priv_data, "x264opts", "bframes=0:force-cfr:no-mbtree:sliced-threads:sync-lookahead=0:rc-lookahead=0:intra-refresh=1:keyint=1", 0);	
	}

	if (_video_codec->id == AV_CODEC_ID_MPEG1VIDEO) {	
        // Needed to avoid using macroblocks in which some coeffs overflow            
        _video_codec_context->mb_decision = 2;
    }

	// Some formats want stream headers to be separate
	if (_output_context->oformat->flags & AVFMT_GLOBALHEADER) {
        _video_codec_context->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}

	// open codec
	ret = avcodec_open2 (_video_codec_context, _video_codec, &video_codec_opt);
	if (ret < 0) {
		PrintErrorMsg ("avcodec_open2", ret);
		return false;
	}	

	// create rgb frame
	_video_frame_rgb = av_frame_alloc();
	if (!_video_frame_rgb) {
		ssi_wrn ("could not allocate video frame");
		return false;
	}
	switch (channels) {
		case 1:
			_video_frame_rgb->format = AV_PIX_FMT_GRAY8;
			break;
		case 3:
			_video_frame_rgb->format = AV_PIX_FMT_BGR24;
			break;
		case 4:
			_video_frame_rgb->format = AV_PIX_FMT_RGB32;
			break;
		default:
			ssi_wrn ("#channels %u not supported", channels);
			return false;
	};
	_video_frame_rgb->width  = width;
	_video_frame_rgb->height = height;
	ret = av_image_alloc (_video_frame_rgb->data, _video_frame_rgb->linesize, width, height, (AVPixelFormat) _video_frame_rgb->format, 1);
	if (ret < 0) {
		PrintErrorMsg ("av_image_alloc", ret);
		return false;
	}

	// create yuv frame
	_video_frame_yuv = av_frame_alloc();
	if (!_video_frame_yuv) {
		ssi_wrn ("could not allocate video frame");
		return false;
	}
	_video_frame_yuv->format = _video_codec_context->pix_fmt;
	_video_frame_yuv->width  = width;
	_video_frame_yuv->height = height;
	_video_frame_yuv->pts = 0;
	ret = av_image_alloc (_video_frame_yuv->data, _video_frame_yuv->linesize, width, height, _video_codec_context->pix_fmt, 1);
	if (ret < 0) {
		PrintErrorMsg ("av_image_alloc", ret);
		return false;
	}

	// rgb->yuv convert context
	_video_convert_ctx = sws_getCachedContext (NULL, width, height, (AVPixelFormat) _video_frame_rgb->format, width, height, _video_codec_context->pix_fmt, SWS_BILINEAR, NULL, NULL ,NULL);	

	return true;
}

bool FFMPEGWriterClient::writeAudio (ssi_size_t n_samples, ssi_size_t n_bytes, void *chunk) {

	if (!_ready) {
		return false;
	}

	if (_audio_buffer) {
		return _audio_buffer->push (n_samples, ssi_pcast (ssi_real_t, chunk));		
	} else {
		return writeAudioChunk (n_samples, n_bytes, (uint8_t *) chunk);
	}	
}

bool FFMPEGWriterClient::writeAudioChunk (ssi_size_t n_samples, ssi_size_t n_bytes, uint8_t *samples) {

	int ret;

	av_init_packet (&_audio_pkt);
	_audio_pkt.data = NULL;    // packet data will be allocated by the encoder
    _audio_pkt.size = 0;
    
    // convert samples from native format to destination codec format, using the resampler
    if (_audio_convert_ctx) {

        if (_audio_convert_size < ssi_cast (int, n_samples)) {

			if (_audio_convert_data) {
				av_free (_audio_convert_data[0]);
				av_free (_audio_convert_data);
			}

			ret = av_samples_alloc_array_and_samples (&_audio_convert_data, &_audio_convert_size, _audio_codec_context->channels, n_samples, _audio_codec_context->sample_fmt, 0);
			if (ret < 0) {
				ssi_wrn ("could not allocate destination samples");
				return false;
			}

			_audio_convert_size = n_samples;
		}

		ret = swr_convert (_audio_convert_ctx, _audio_convert_data, _audio_convert_size, (const uint8_t **) &samples, n_samples);
        if (ret < 0) {
            ssi_wrn ("error while converting audio chunk");
            return false;
        }
		samples = _audio_convert_data[0];
		n_samples = ret;
		n_bytes = av_samples_get_buffer_size (NULL, _audio_codec_context->channels, n_samples, _audio_codec_context->sample_fmt, 0);
    }

    _audio_frame->nb_samples = n_samples;
	AVRational rat;
	rat.den = _audio_codec_context->sample_rate;
	rat.num = 1;
    _audio_frame->pts = av_rescale_q (_audio_sample_count, rat, _audio_codec_context->time_base);
    avcodec_fill_audio_frame (_audio_frame, _audio_codec_context->channels, _audio_codec_context->sample_fmt, samples, n_bytes, 0);    

    ret = avcodec_encode_audio2 (_audio_codec_context, &_audio_pkt, _audio_frame, &_audio_got_output);
    if (ret < 0) {
		PrintErrorMsg ("avcodec_encode_audio2", ret);               
		return false;
    }

	//static FILE *fid = fopen ("writeAudioChunk", "wb");
	//fwrite (samples, n_samples, sizeof (short), fid);
	//fflush (fid);

	if (_audio_got_output) {
		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "write audio chunk %u (size=%5d)", _audio_chunk_count, _audio_pkt.size);
		ret = writeFrame (_output_context, &_audio_codec_context->time_base, _audio_stream, &_audio_pkt);

		if (ret < 0) {
			PrintErrorMsg ("writeFrame", ret);
			return false;
		}
		av_packet_unref (&_audio_pkt);
	}

	_audio_sample_count += n_samples;
	_audio_chunk_count++;

	return true;
}

bool FFMPEGWriterClient::writeVideo (ssi_size_t nbytes, ssi_byte_t *image) {

	if (!_ready) {
		return false;
	}
	
	if (_video_buffer) {
		return _video_buffer->push (image);
	} else {
		return writeVideoFrame (nbytes, image);
	}
}

bool FFMPEGWriterClient::writeVideoFrame (ssi_size_t nbytes, ssi_byte_t *image) {

	av_init_packet (&_video_pkt);
    _video_pkt.data = NULL;    // packet data will be allocated by the encoder
    _video_pkt.size = 0;

	memcpy (_video_frame_rgb->data[0], image, nbytes);
	int result = sws_scale (_video_convert_ctx, _video_frame_rgb->data, _video_frame_rgb->linesize, 0, _video_codec_context->height, _video_frame_yuv->data, _video_frame_yuv->linesize);
	if (result != _video_codec_context->height) {
		ssi_wrn ("could not convert rgb to yuv");
		return false;
	}

	// encode the image
	int ret = avcodec_encode_video2 (_video_codec_context, &_video_pkt, _video_frame_yuv, &_video_got_output);
	if (ret < 0) {
		PrintErrorMsg ("avcodec_encode_video2", ret);
		return false;
	}
	
	// write packet
	if (_video_got_output) {
		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "write video frame %u (size=%5d)", _video_frame_count, _video_pkt.size);
		ret = writeFrame (_output_context, &_video_codec_context->time_base, _video_stream, &_video_pkt);
		if (ret < 0) {
			PrintErrorMsg ("writeFrame", ret);
			return false;
		}
		av_packet_unref(&_video_pkt);
	}

	_video_frame_yuv->pts++;
	_video_frame_count++;

	return true;
}


bool FFMPEGWriterClient::flushBuffer () {

	bool result = true;

	if (_mode == FFMPEGWriter::MODE::AUDIO && _audio_buffer) {

		bool is_empty;

		while (_audio_buffer->getPushed () >= _n_audio_chunk) {
			ssi_size_t n_samples = _n_audio_chunk;			
			ssi_real_t *chunk = _audio_buffer->pop (n_samples, is_empty);
			if (!is_empty) {								
				result = writeAudioChunk (n_samples, n_samples * sizeof (ssi_real_t), ssi_pcast (uint8_t, chunk)) && result;				
				_audio_buffer->pop_done (n_samples);
			}
		}

	} else if (_mode == FFMPEGWriter::MODE::AUDIOVIDEO && _audio_buffer && _video_buffer) {

		bool is_empty;
		
		while (_audio_buffer->getPushed () >= _n_audio_chunk || _video_buffer->getPushed () > 0) {

			int64_t audio_pts = av_stream_get_end_pts(_audio_stream);
			double audio_time = audio_pts * av_q2d(_audio_stream->time_base);

			int64_t video_pts = av_stream_get_end_pts(_video_stream);
			double video_time = video_pts * av_q2d(_video_stream->time_base);

			//double audio_time = _audio_stream->pts.val * av_q2d (_audio_stream->time_base);
			//double video_time = _video_stream->pts.val * av_q2d (_video_stream->time_base);			
		
			if (audio_time <= video_time) {		
				if (_audio_buffer->getPushed () < _n_audio_chunk) {
					break;
				}
				ssi_size_t n_samples = _n_audio_chunk;
				ssi_real_t *chunk = _audio_buffer->pop (n_samples, is_empty);
				if (!is_empty) {					
					//printf ("%.2lf %.2lf write audio\n", audio_time, video_time);					
					result = writeAudioChunk (n_samples, n_samples * sizeof (ssi_real_t), ssi_pcast (uint8_t, chunk)) && result;				
					_audio_buffer->pop_done (n_samples);
				}
			} else {	
				if (_video_buffer->getPushed () == 0) {
					break;
				}
				ssi_byte_t *image = 0;
				ssi_size_t n_bytes = 0;
				image = _video_buffer->pop (n_bytes, is_empty);
				if (!is_empty) {
					//printf ("%.2lf %.2lf write video\n", audio_time, video_time);
					result = writeVideoFrame (n_bytes, image) && result;
					_video_buffer->pop_done ();			
				}
			}
		}
	}

	return result;
}

int FFMPEGWriterClient::writeFrame (AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt) {

    // rescale output packet timestamp values from codec to stream timebase
	pkt->pts = av_rescale_q_rnd(pkt->pts, *time_base, st->time_base, (AVRounding) (AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
    pkt->dts = av_rescale_q_rnd(pkt->dts, *time_base, st->time_base, (AVRounding) (AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
    pkt->duration = (int) av_rescale_q(pkt->duration, *time_base, st->time_base);
    pkt->stream_index = st->index;

    // Write the compressed frame to the media file
    return av_interleaved_write_frame(fmt_ctx, pkt);
}

bool FFMPEGWriterClient::close () {

	int ret;

	if (_ready) {		

		if (_mode != FFMPEGWriter::MODE::AUDIO) {

			// get the delayed frames
			for (_video_got_output = 1; _video_got_output; _video_frame_count++) {
			
				ret = avcodec_encode_video2(_video_codec_context, &_video_pkt, NULL, &_video_got_output);
				if (ret < 0) {
					PrintErrorMsg ("avcodec_encode_video2", ret);
					return false;
				}

				if (_video_got_output) {
					SSI_DBG (SSI_LOG_LEVEL_DEBUG, "write frame %u (size=%5d)", _video_frame_count, _video_pkt.size);
					ret = writeFrame (_output_context, &_video_codec_context->time_base, _video_stream, &_video_pkt);
					if (ret < 0) {
						PrintErrorMsg ("writeFrame", ret);
						return false;
					}
					av_packet_unref(&_video_pkt);
				}
			}
		}

		if (_mode != FFMPEGWriter::MODE::VIDEO) {

			// get the delayed chunks			
			for (_audio_got_output = 1; _audio_got_output; _audio_chunk_count++) {
			
				ret = avcodec_encode_audio2 (_audio_codec_context, &_audio_pkt, NULL, &_audio_got_output);
				if (ret < 0) {
					PrintErrorMsg ("avcodec_encode_video2", ret);
					return false;
				}

				if (_audio_got_output) {
					SSI_DBG (SSI_LOG_LEVEL_DEBUG, "write chunk %u (size=%5d)", _audio_chunk_count, _audio_pkt.size);
					ret = writeFrame (_output_context, &_audio_codec_context->time_base, _audio_stream, &_audio_pkt);
					if (ret < 0) {
						PrintErrorMsg ("writeFrame", ret);
						return false;
					}
					av_packet_unref(&_audio_pkt);
				}
			}
		}

		// write the trailer, if any
		av_write_trailer (_output_context);

		// close video codec and frames
		if (_video_codec_context) {
			avcodec_close (_video_codec_context);		
			_video_codec_context = 0;
		}
		if (_video_convert_ctx) {
			sws_freeContext (_video_convert_ctx);
			_video_convert_ctx = 0;
		}
		if (_video_frame_yuv) {
			av_freep (&_video_frame_yuv->data[0]);
			av_frame_free (&_video_frame_yuv);
			_video_frame_yuv = 0;
		}
		if (_video_frame_rgb) {
			av_freep (&_video_frame_rgb->data[0]);
			av_frame_free (&_video_frame_rgb);
			_video_frame_rgb = 0;
		}

		// close audio codec and buffer
		if (_audio_codec_context) {
			avcodec_close(_audio_codec_context);
			_audio_codec_context = 0;
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
			av_frame_free (&_audio_frame);
			_audio_frame = 0;
		}

		// close media
		if (!(_output_format->flags & AVFMT_NOFILE)) {
			avio_close (_output_context->pb);
		}
		avformat_free_context (_output_context);
		_output_context = 0;

		_audio_sample_count = 0;
		_video_frame_count = 0;
		_audio_chunk_count = 0;

		_video_got_output = 0;
		_audio_got_output = 0;

		delete _audio_buffer; _audio_buffer = 0;
		delete _video_buffer; _video_buffer = 0;

		_ready = false;
	}

	return true;
}

};
