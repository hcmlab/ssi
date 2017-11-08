// FFMPEGWriter.h
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

#ifndef SSI_FFMPEGWRITER_H
#define SSI_FFMPEGWRITER_H

#include "base/IConsumer.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class FFMPEGWriterClient;

class FFMPEGWriter : public IConsumer {

friend class FFMPEGWriterClient;

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

		Options ()
			 : overwrite(false) {

			setPath ("");
			setUrl ("");
			setFormat ("mp4");
			setACodec ("");
			setVCodec ("");
			setSdp ("");
			video_bit_rate_kb = 0;
			audio_bit_rate_kb = 0;
			stream = false;
			abuffer = 3.0;
			vbuffer = 1.0;

			addOption("path", path, SSI_MAX_CHAR, SSI_CHAR, "file path (leave empty to use url)");
			addOption("overwrite", &overwrite, 1, SSI_BOOL, "overwrite file if it already exists (otherwise a unique path will be created)");			

			addOption ("url", url, SSI_MAX_CHAR, SSI_CHAR, "streaming address in the format udp://<ip:port>");			
			addOption ("stream", &stream, 1, SSI_BOOL, "set this flag for very fast encoding in streaming applications (forces h264/aac codec)");			

			addOption ("format", format, SSI_MAX_CHAR, SSI_CHAR, "default output format if not determined from url or path (forced to 'mpegts' if in streaming mode)");
			addOption ("acodec", acodec, SSI_MAX_CHAR, SSI_CHAR, "force audio codec name (otherwise leave empty)");
			addOption ("vcodec", vcodec, SSI_MAX_CHAR, SSI_CHAR, "force video codec name (otherwise leave empty)");
			addOption ("video_bitrate", &video_bit_rate_kb, 1, SSI_SIZE, "average bit rate in kB (0 for default)");
			addOption ("audio_bitrate", &audio_bit_rate_kb, 1, SSI_SIZE, "average bit rate in kB (0 for default)");			
			addOption ("abuffer", &abuffer, 1, SSI_TIME, "internal buffer size to cue audio samples for encoder");
			addOption ("vbuffer", &vbuffer, 1, SSI_TIME, "internal buffer size to cue video frames for encoder");
			addOption ("sdp", sdp, SSI_MAX_CHAR, SSI_CHAR, "sdp path (leave empty if you don't want to create a sdp file)");			
		};

		void setPath(const ssi_char_t *path) {
			ssi_strcpy(this->path, path);
		}
		void setUrl (const ssi_char_t *url) {
			ssi_strcpy (this->url, url);
		}
		void setSdp (const ssi_char_t *sdp) {
			ssi_strcpy (this->sdp, sdp);
		}
		void setFormat (const ssi_char_t *format) {
			strcpy (this->format, format);
		}
		void setACodec (const ssi_char_t *acodec) {
			strcpy (this->acodec, acodec);
		}
		void setVCodec (const ssi_char_t *vcodec) {
			strcpy (this->vcodec, vcodec);
		}

		ssi_char_t path[SSI_MAX_CHAR];
		bool overwrite;
		ssi_char_t url[SSI_MAX_CHAR];
		ssi_char_t sdp[SSI_MAX_CHAR];
		ssi_char_t format[SSI_MAX_CHAR];
		ssi_char_t acodec[SSI_MAX_CHAR];
		ssi_char_t vcodec[SSI_MAX_CHAR];
		ssi_size_t video_bit_rate_kb;
		ssi_size_t audio_bit_rate_kb;
		ssi_time_t abuffer;
		ssi_time_t vbuffer;
		bool stream;
	};

public:

	static const ssi_char_t *GetCreateName () { return "FFMPEGWriter"; };
	static IObject *Create (const ssi_char_t *file) { return new FFMPEGWriter (file); };
	~FFMPEGWriter ();

	FFMPEGWriter::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "ffmpeg wrapper to write audio/video streams"; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_fail (ssi_time_t fail_time, 
		ssi_time_t fail_duration,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	bool notify(INotify::COMMAND::List command, const ssi_char_t *message);

	void setVideoFormat (ssi_video_params_t	video_format) { _video_format = video_format; };
	void setMetaData (ssi_size_t size, const void *meta) {
		if (sizeof (_video_format) != size) {
			ssi_err ("invalid meta size");
			return;
		}
		memcpy (&_video_format, meta, size);
	}
	ssi_video_params_t getVideoFormat () { return _video_format; };
	WAVEFORMATEX getAudioFormat () { return *_audio_format; };

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	FFMPEGWriter (const ssi_char_t *file = 0);
	FFMPEGWriter::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	const WAVEFORMATEX *_audio_format;
	ssi_video_params_t	_video_format;

	int _audio_index;
	int _video_index;

	MODE::item _mode;
	FFMPEGWriterClient *_client;

	void open();
	void close();

	bool _ready;
	bool _is_url;
	ssi_char_t *_path_or_url;
	
};

}


#endif
