// FFMPEGWriterClient.h
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

#pragma once

#ifndef SSI_FFMPEGWRITERCLIENT_INCLUDE_H
#define SSI_FFMPEGWRITERCLIENT_INCLUDE_H

#include "FFMPEGWriter.h"
#include <sstream>
#include "base/String.h"
#include "FFMPEGAVBuffer.h"

extern "C"
{
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libswscale/swscale.h>
#include "libswresample/swresample.h"
#include <libavformat/avformat.h>
}

namespace ssi {

class FFMPEGWriterClient {
		
public:

	FFMPEGWriterClient(FFMPEGWriter *writer);
	~FFMPEGWriterClient();

	void setLogLevel (int level) {
		ssi_log_level = level;
	};

	bool open ();
	bool writeVideo (ssi_size_t nbytes, ssi_byte_t *image);
	bool writeAudio (ssi_size_t nsamples, ssi_size_t nbytes, void *chunk);
	bool flushBuffer ();
	bool close ();

protected:

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	void PrintErrorMsg (const char *func, int msg);

	bool openVideo ();
	bool openAudio ();
	int writeFrame (AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt);
	bool writeVideoFrame (ssi_size_t nbytes, ssi_byte_t *image);
	bool writeAudioChunk (ssi_size_t n_samples, ssi_size_t n_bytes, uint8_t *samples);	

	FFMPEGWriter *_writer;
	bool _ready;
	FFMPEGWriter::MODE::item _mode;
	
	AVOutputFormat *_output_format;	
	AVFormatContext *_output_context;

	AVStream *_video_stream, *_audio_stream;
	AVCodec *_video_codec, *_audio_codec;
    AVCodecContext *_video_codec_context, *_audio_codec_context;        
    AVFrame *_video_frame_rgb, *_video_frame_yuv, *_audio_frame;
	SwsContext *_video_convert_ctx;
	SwrContext *_audio_convert_ctx;
	uint8_t **_audio_convert_data;
	int _audio_convert_size;
	AVPacket _video_pkt, _audio_pkt;
	ssi_size_t _n_audio_chunk;
	ssi_size_t _video_frame_count, _audio_sample_count, _audio_chunk_count;
	int _video_got_output, _audio_got_output;

	FFMPEGVideoBuffer *_video_buffer;
	FFMPEGAudioBuffer *_audio_buffer;	

};

};

#endif
