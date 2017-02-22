// FFMPEGReader.h
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

#ifndef SSI_FFMPEG_READER_H
#define SSI_FFMPEG_READER_H

#include "base/ISensor.h"
#include "thread/Thread.h"
#include "ioput/socket/SocketOsc.h"
#include "ioput/socket/SocketImage.h"
#include "ioput/option/OptionList.h"
#include "ioput/file/FileMem.h"
#include "thread/Timer.h"
#include "FFMPEGReaderChannel.h"
#include "FFMPEGAVBuffer.h"

namespace ssi {

class FFMPEGReaderClient;

class FFMPEGReader :  public ISensor, public Thread {

friend class FFMPEGReaderClient;

public:

	struct MODE {
		enum item {
			UNDEFINED,
			VIDEO,
			AUDIO,
			AUDIOVIDEO
		};
	};

public:

	class Options : public OptionList {

	public:

		Options() : buffer(1.0), fps(25.0), asr(16000.0), width(640), height(480), ablock(0.01), stream(false), offset(0), bestEffort(false){

			setUrl ("");

			addOption ("url", url, SSI_MAX_CHAR, SSI_CHAR, "url (file path or streaming address, e.g. udp://<ip:port>)");
			addOption ("stream", &stream, 1, SSI_BOOL, "set this flag for very fast decoding in streaming applications (forces h264/aac codec)");
			addOption ("buffer", &buffer, 1, SSI_TIME, "internal buffer size in seconds");			
			addOption ("fps", &fps, 1, SSI_TIME, "default video frame rate in Hz (if not determined from url)");
			addOption ("width", &width, 1, SSI_SIZE, "default video width in pixels (if not determined from url)");
			addOption ("height", &height, 1, SSI_SIZE, "default video height in pixels (if not determined from url)");
			addOption ("asr", &asr, 1, SSI_TIME, "default audio sample rate in Hz (if not determined from url)");
			addOption ("ablock", &ablock, 1, SSI_TIME, "audio block size in seconds if no video is available (otherwise will be set to 1/fps)");			
			addOption ("offset", &offset, 1, SSI_DOUBLE, "offset in seconds");
			addOption ("bestEffort", &bestEffort, 1, SSI_BOOL, "best effort delivery, ignores fps");
		};

		void setUrl (const ssi_char_t *url) {
			this->url[0] = '\0';
			if (url) {
				ssi_strcpy (this->url, url);
			}
		};

		ssi_char_t url[SSI_MAX_CHAR];
		ssi_time_t buffer;
		ssi_time_t fps, asr;
		ssi_size_t width, height;
		ssi_time_t ablock;
		bool stream;
		bool bestEffort;
		ssi_time_t offset;
	};

public:

	static const ssi_char_t *GetCreateName () { return "FFMPEGReader"; };
	static IObject *Create (const ssi_char_t *file) { return new FFMPEGReader (file); };
	~FFMPEGReader();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "ffmpeg wrapper to read audio/video streams"; };

	ssi_size_t getChannelSize () { return 2; };
	IChannel *getChannel (ssi_size_t index) { 
		if(index==0)
			return &_video_channel;
		if(index==1)
			return &_audio_channel;
		return 0;
	};
	bool setProvider (const ssi_char_t *name, IProvider *provider);
	bool connect ();
	void enter ();
	bool start () { return Thread::start (); };
	bool stop () { return Thread::stop (); };
	void flush ();
	void run ();
	bool pushVideoFrame (ssi_size_t n_bytes, ssi_byte_t *frame);
	bool pushAudioChunk (ssi_size_t n_samples, ssi_real_t *chunk);
	bool disconnect();

	virtual void wait();
		
	void setLogLevel (int level) {
		ssi_log_level = level;
	};

protected:

	FFMPEGReader (const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;
	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	MODE::item _mode;

	VideoChannel _video_channel;
	AudioChannel _audio_channel;
	FFMPEGReaderClient *_client;
	void setVideoProvider(IProvider *provider);
	void setAudioProvider(IProvider *provider);
	void setProvider (IProvider *provider);

	IProvider *_video_provider, *_audio_provider;
	ssi_video_params_t _video_format;
	ssi_time_t _audio_sr;
	Timer *_timer;

	FFMPEGVideoBuffer *_video_buffer;
	FFMPEGAudioBuffer *_audio_buffer;

	bool			_is_running;
	Event			_wait_event;
	
};

};

#endif

