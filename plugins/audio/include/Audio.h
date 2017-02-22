// Audio.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/03/06
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

#ifndef SSI_SENSOR_AUDIOSENSOR_H
#define SSI_SENSOR_AUDIOSENSOR_H

#include "base/ISensor.h"
#include "AudioCons.h"
#include "AudioIn.h"
#include "AudioHeader.h"
#include "thread/Thread.h"
#include "thread/Event.h"
#include "thread/Lock.h"
#include "ioput/option/OptionList.h"
#include "ioput/file/StringList.h"

namespace ssi {

class Audio : public ISensor, public Thread {

public:

	class AudioChannel : public IChannel {

		friend class Audio;

		public:

			AudioChannel () {
				ssi_stream_init (stream, 0, 0, 0, SSI_UNDEF, 0);
			}
			~AudioChannel () {
				ssi_stream_destroy (stream);
			}

			const ssi_char_t *getName () { return SSI_AUDIO_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "A mono or stereo waveform either as a stream of short values in range [-32768 32767] or float values in range [-1.0 1.0]"; };
			ssi_stream_t getStream () { return stream; };

		protected:

			ssi_stream_t stream;
	};

	class Options : public OptionList {

	public:

		Options ()
			: remember (true), sr (16000.0), block (0.1), blockInSamples (0), scale (true), channels (1), bytes (2), volume (-1.0f) {

			device[0] = '\0';

			addOption ("device", device, SSI_MAX_CHAR, SSI_CHAR, "audio device name (if empty a dialog will be shown)");
			addOption ("remember",	&remember,	1, SSI_BOOL, "remember selected audio device");		
			addOption ("sr", &sr, 1, SSI_DOUBLE, "sample rate in Hz");
			addOption ("size", &block, 1, SSI_DOUBLE, "block size in seconds (deprecated)");		
			addOption ("block", &block, 1, SSI_DOUBLE, "block size in seconds");		
			addOption ("blockInSamples", &blockInSamples, 1, SSI_SIZE, "block size in samples (overwrites size if > 0)");		
			addOption ("channels", &channels, 1, SSI_UINT, "number of channels (1=mono, 2=stereo)");		
			addOption ("bytes", &bytes, 1, SSI_UINT, "bytes per sample");	
			addOption ("scale", &scale, 1, SSI_BOOL, "scale to interval [-1..1]");
			addOption ("volume", &volume, 1, SSI_FLOAT, "recording volume in range [0..1] (only applied if > 0)");
		};		

		void setDevice (const ssi_char_t *name) {
			ssi_strcpy (device, name);
		}

		ssi_time_t sr;
		ssi_size_t channels;
		ssi_size_t bytes;
		ssi_time_t block;
		ssi_size_t blockInSamples;
		bool scale;		
		ssi_char_t device[SSI_MAX_CHAR];
		bool remember;
		float volume;
	};

public:

	static const ssi_char_t *GetCreateName () { return "Audio"; };
	static IObject *Create (const ssi_char_t *file) { return new Audio (file); };
	~Audio ();
	Audio::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Opens an audio input device for recording and streams a mono or stereo waveform."; };

	ssi_size_t getChannelSize () { return 1; };
	IChannel *getChannel (ssi_size_t index) { return &_audio_channel; };
	bool setProvider (const ssi_char_t *name, IProvider *provider);
	bool connect ();
	bool start () { return Thread::start (); };
	void run ();
	bool stop () { return Thread::stop (); };
	bool disconnect ();	

	WAVEFORMATEX getFormat () { return _format; };

	void setLogLevel (int level) {
		ssi_log_level = level;
	}
	
protected:

	Audio (const ssi_char_t *file = 0);
	Audio::Options _options;
	ssi_char_t *_file;

	AudioChannel _audio_channel;
	void setProvider (IProvider *provider);

	WAVEFORMATEX _format;

	int ssi_log_level;
	static ssi_char_t *ssi_log_name;
	bool _first_call;

	bool Start ();
    void Stop ();
    bool BufferDone ();
    bool IsBufferDone () const {
        return _header_pool [_current_buffer].IsDone ();
    }
    char *GetData () const { 
		return _header_pool [_current_buffer].lpData; 
	}
    bool _isStarted;

	unsigned int _device_id;
    AudioIn _waveInDevice;
    ssi_size_t _samples_per_buffer, _bytes_per_buffer;	
    int _current_buffer;
    char *_buffer_pool;
	float *_scale_buffer;
    AudioHeader _header_pool [SSI_AUDIO_BLOCK_COUNT];

	Mutex		_mutex;
    Event       _event;

	IProvider *_provider;

	int selectDevice ();			
	int getDevice (const ssi_char_t *name);

	static int LetUserSelectDevice (StringList &list);
	static int LetUserSelectDeviceOnConsole (StringList &list);


};

}

#endif
