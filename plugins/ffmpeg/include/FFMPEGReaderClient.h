// FFMPEGReaderClient.h
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

#ifndef SSI_FFMPEGREADERCLIENT_INCLUDE_H
#define SSI_FFMPEGREADERCLIENT_INCLUDE_H

#include "FFMPEGReader.h"
#include <sstream>
#include "base/String.h"
#include "thread/Thread.h"
#include "thread/Lock.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <istream>
#include <libswresample/swresample.h>
#include <libavutil/avutil.h>
#include <libavdevice/avdevice.h>
#include <libavutil/pixfmt.h>
}

namespace ssi {

class FFMPEGReaderClient : public Thread {

	friend class FFMPEGReader;

public:

	struct STATE {
		enum item {
			IDLE,
			CONNECT,
			READ,
			TERMINATE
		};
	};

public:

	FFMPEGReaderClient(FFMPEGReader *reader);
	~FFMPEGReaderClient();

	bool peekVideoFormat (ssi_video_params_t &video_params);
	bool peekAudioFormat (ssi_time_t &audio_sr, ssi_size_t &samples);
	
	void enter ();
	void run ();	
	void terminate ();
	void flush ();		

	void setLogLevel (int level) {
		ssi_log_level = level;
	};

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	STATE::item getState() { return _state; }

protected:

	void PrintErrorMsg (const char *func, int msg);
	
	bool open ();		
	int read ();
	bool close ();
	int decodePacket ();
	void provideVideoFrame ();
	void provideAudioFrame ();
	bool seek (ssi_size_t frameIndex);

	static int InterruptCB (void *ctx);
	
	FFMPEGReader::MODE::item _mode;
	volatile STATE::item _state;
	AVFormatContext *_fmt_ctx;
	AVCodec *_video_codec, *_audio_codec;
	AVCodecContext *_video_codec_context, *_audio_codec_context;
	AVStream *_video_stream, *_audio_stream;
	int _video_stream_idx, _audio_stream_idx;
	AVFrame *_video_frame, *_audio_frame;
	AVFrame *_video_frame_rgb;
	uint8_t **_audio_convert_data;
	int _audio_convert_size;
	AVPacket _packet;
	SwsContext *_video_convert_ctx;
	SwrContext *_audio_convert_ctx;
	AVIOInterruptCB _interrupt_cb;

	FFMPEGReader *_reader;
};

};

#endif
