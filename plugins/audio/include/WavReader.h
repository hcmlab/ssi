// WavReader.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/07/23
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

#ifndef SSI_WAV_WAVREADER_H
#define SSI_WAV_WAVREADER_H

#include "base/ISensor.h"
#include "thread/Thread.h"
#include "thread/Timer.h"
#include "thread/Event.h"
#include "ioput/file/FileBinary.h"
#include "ioput/option/OptionList.h"
#include "ioput/wav/WavTools.h"

#define SSI_WAVREADER_PROVIDER_NAME "audio"

namespace ssi {

class WavReader : public ISensor, public Thread {

	class AudioChannel : public IChannel {

		friend class WavReader;

		public:

			AudioChannel () {
				ssi_stream_init (stream, 0, 0, 0, SSI_UNDEF, 0);
			}
			~AudioChannel () {
				ssi_stream_destroy (stream);
			}

			const ssi_char_t *getName () { return SSI_WAVREADER_PROVIDER_NAME; };
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
			blockInSamples (0),
			scale (true),
			offset (0), 
			offsetInSamples (0),
			loop (true) {

			path[0] = '\0';
			addOption ("path", path, SSI_MAX_CHAR, SSI_CHAR, "filepath");
			addOption ("block", &block, 1, SSI_DOUBLE, "block size in seconds");
			addOption ("blockInSamples", &blockInSamples, 1, SSI_SIZE, "block size in samples (overwrites block if > 0)");
			addOption ("offset", &offset, 1, SSI_DOUBLE, "offset in seconds");
			addOption ("offsetInSamples", &offsetInSamples, 1, SSI_SIZE, "offset in samples (overwrites offset if > 0)");
			addOption ("run", &best_effort_delivery, 1, SSI_BOOL, "best effort delivery");
			addOption ("scale", &scale, 1, SSI_BOOL, "scale to interval [-1..1]");
			addOption ("loop", &loop, 1, SSI_BOOL, "loop file");
		};

		void setPath (const ssi_char_t *path) {
			ssi_strcpy (this->path, path);
		}

		ssi_char_t path[SSI_MAX_CHAR];
		ssi_time_t block;
		ssi_size_t blockInSamples;
		ssi_time_t offset;
		ssi_size_t offsetInSamples;
		bool best_effort_delivery;
		bool scale;
		bool loop;
	};

public:

	static const ssi_char_t *GetCreateName () { return "WavReader"; };
	static IObject *Create (const ssi_char_t *file) { return new WavReader (file); };
	~WavReader ();
	WavReader::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Reads in a wav file from disk and streams it as a mono or stereo audio waveform."; };

	ssi_size_t getChannelSize () { return 1; };
	IChannel *getChannel (ssi_size_t index) { return &_audio_channel; };
	bool setProvider (const ssi_char_t *name, IProvider *provider);
	bool connect ();
	bool start () { return Thread::start (); };
	bool stop () { return Thread::stop (); };
	void run ();
	bool disconnect ();

	void wait () { _wait_event.wait (); };

	WAVEFORMAT getFormat () { return _format; };

protected:

	WavReader (const ssi_char_t *file = 0);
	WavReader::Options _options;
	ssi_char_t *_file;

	AudioChannel _audio_channel;
	void setProvider (IProvider *provider);

	ssi_char_t *_wav_path;
	File *_wav_file;
	ssi_size_t _wav_start_pos;
	IProvider *_provider;
	bool _scale;
	ssi_stream_t _stream;
	ssi_stream_t _stream_scale;
	bool _is_providing;
	ssi_size_t _loop_pos;

	ssi_time_t _sample_rate;
	ssi_size_t _sample_dimension;
	ssi_size_t _sample_bytes;
	ssi_size_t _total_size, _frame_size, _frame_counter;
	ssi_size_t _offset_in_bytes, _offset_in_samples;
	Timer *_frame_timer;
	Event _wait_event;

	WAVEFORMAT _format;
	WavHeader _header;
	WavChunkHeader _chunk;

	static const ssi_char_t *ssi_log_name;
};

}


#endif
