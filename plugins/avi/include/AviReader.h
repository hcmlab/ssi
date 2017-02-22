// AviReader.h
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

#ifndef SSI_AVI_AVIREADER_H
#define SSI_AVI_AVIREADER_H

#include "base/ISensor.h"
#include "AviLibCons.h"
#include "AviTools.h"
#include "thread/Thread.h"
#include "thread/Timer.h"
#include "thread/Event.h"
#include "ioput/file/FileProvider.h"
#include "ioput/option/OptionList.h"

#define SSI_AVIREADER_VIDEO_PROVIDER_NAME "video"
#define SSI_AVIREADER_AUDIO_PROVIDER_NAME "audio"

namespace ssi {

class AviReader : public ISensor, public Thread {

	class VideoChannel : public IChannel {

		friend class AviReader;

		public:

			VideoChannel () {
				ssi_stream_init (stream, 0, 0, 0, SSI_IMAGE, 0);
			}
			~VideoChannel () {
				ssi_stream_destroy (stream);
			}

			const ssi_char_t *getName () { return SSI_AVIREADER_AUDIO_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Delivers a stream of uncompressed images in the Device Independent Bitmap (DIB) file format of the desired sub type."; };
			ssi_stream_t getStream () { return stream; };

		protected:

			ssi_stream_t stream;
	};

class AudioChannel : public IChannel {

	friend class AviReader;

	public:

		const ssi_char_t *getName () { return SSI_AVIREADER_VIDEO_PROVIDER_NAME; };
		const ssi_char_t *getInfo () { return "A mono or stereo waveform either as a stream of short values in range [-32768 32767] or float values in range [-1.0 1.0]"; };
		ssi_stream_t getStream () { return stream; };

	protected:

		ssi_stream_t stream;
};

public:

	class Options : public OptionList {

	public:

		Options () 
			: best_effort_delivery (false),
			block (0.1),
			video_stream_index (0),
			audio_stream_index (0), 
			offset (0), 
			flip (false), 
			mirror (false)
		{

			path[0] = '\0';
			addOption ("path", path, SSI_MAX_CHAR, SSI_CHAR, "filepath");
			addOption ("block", &block, 1, SSI_DOUBLE, "audio block size in seconds (if audio only)");			
			addOption ("offset", &offset, 1, SSI_DOUBLE, "offset in seconds");			
			addOption ("flip", &flip, 1, SSI_BOOL, "flip video image");
			addOption ("mirror", &mirror, 1, SSI_BOOL, "mirror video image");
			addOption ("run", &best_effort_delivery, 1, SSI_BOOL, "best effort delivery");
			addOption ("#video", &video_stream_index, 1, SSI_INT, "video stream index");
			addOption ("#audio", &audio_stream_index, 1, SSI_INT, "audio stream index");
		};

		void setPath (const ssi_char_t *path) {
			ssi_strcpy (this->path, path);
		}

		ssi_char_t path[SSI_MAX_CHAR];
		ssi_time_t block;
		ssi_time_t offset;
		bool best_effort_delivery;
		int video_stream_index;
		int audio_stream_index;
		bool flip, mirror;
	};

public:

	static const ssi_char_t *GetCreateName () { return "AviReader"; };
	static IObject *Create (const ssi_char_t *file) { return new AviReader (file); };
	~AviReader ();
	
	AviReader::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Reads a video and/or audio stream from an avi file."; };
	
	ssi_size_t getChannelSize () { return 2; };
	IChannel *getChannel (ssi_size_t index) { if (index == 0) return &_video_channel; if (index == 1) return &_audio_channel; return 0; };
	bool setProvider (const ssi_char_t *name, IProvider *provider);
	bool connect ();
	bool start () { return Thread::start (); };
	bool stop () { return Thread::stop (); };
	void run ();
	bool disconnect ();

	virtual void wait ();

	WAVEFORMAT getAudioFormat () { return _audio_format; };
	ssi_video_params_t getVideoFormat () { 
		return _video_format; 
	};	
	const void *getMetaData (ssi_size_t &size) { size = sizeof (_video_format); return &_video_format; };

protected:

	AviReader (const ssi_char_t *file = 0);
	AviReader::Options _options;
	ssi_char_t *_file;

	AudioChannel _audio_channel;
	VideoChannel _video_channel;
	void setVideoProvider (IProvider *provider);
	void setAudioProvider (IProvider *provider);
	
	IProvider *_video_provider;
	IProvider *_audio_provider;

	AVIFILEINFO _avi_info;
	PAVIFILE _avi;
	
	ssi_video_params_t _video_format;
	PAVISTREAM _video_stream;
	AVISTREAMINFO _video_stream_info;	
	BITMAPINFO _video_format_info_in, _video_format_info_out;
	long _video_frame_first, _video_frame_last;	
	PGETFRAME _video_frame;

	ssi_byte_t *_picData;
	ssi_byte_t *_picDataTmp;

	WAVEFORMAT _audio_format;
	PAVISTREAM _audio_stream;
	AVISTREAMINFO _audio_stream_info;			
	PGETFRAME _audio_frame;	
	long _audio_frame_first, _audio_sample_last;	
	ssi_stream_t _audio_buffer;

	ssi_time_t _frame_time;
	long _frame_counter;
	ssi_size_t _offset_in_frames, _offset_in_samples;
	Timer *_frame_timer;
	Event *_wait_event;

	bool _is_providing;

	const static ssi_char_t *ssi_log_name;
};

}


#endif
