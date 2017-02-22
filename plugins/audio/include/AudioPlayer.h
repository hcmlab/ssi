// AudioPlayer.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/08/19
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

#ifndef SSI_SENSOR_AUDIOPLAYER_H
#define SSI_SENSOR_AUDIOPLAYER_H

#include "base/IConsumer.h"
#include "AudioOut.h"
#include "ioput/file/FileTools.h"
#include "ioput/file/FileBinary.h"
#include "ioput/option/OptionList.h"
#include "ioput/wav/WavTools.h"

namespace ssi {

class AudioPlayer : public IConsumer {

public:

	class Options : public OptionList {

	public:

		Options ()
			: device (-1), remember (true), numBuffers(8), bufferSize(0.1), bufferSizeSamples(0) {

			addOption ("device", &device, 1, SSI_INT, "audio device id");
			addOption ("remember",	&remember,	1, SSI_BOOL, "remember selected audio device");		
			addOption ("numBuffers", &numBuffers,	1, SSI_SIZE, "the number of audio buffers (blocks) to be used in parallel for audio playback");		
			addOption ("bufferSize", &bufferSize, 1, SSI_DOUBLE, "the size of the audio buffers (blocks) in seconds. Must be >= audio block input");		
			addOption ("bufferSizeSamples", &bufferSizeSamples, 1, SSI_SIZE, "the size of the audio buffers (blocks) in samples (overwrites size if > 0)");		
		};	

		int device;
		bool remember;
		ssi_time_t bufferSize;
		ssi_size_t bufferSizeSamples;
		ssi_size_t numBuffers;
	};

public:

	static const ssi_char_t *GetCreateName () { return "AudioPlayer"; };
	static IObject *Create (const ssi_char_t *file) { return new AudioPlayer (file); };
	~AudioPlayer ();

	AudioPlayer::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Opens an audio output device for playback of a mono or stereo waveform."; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	static void Play (AudioPlayer &player, ssi_stream_t &stream) {

		player.consume_enter (stream.num, &stream);
		IConsumer::info info;
		info.status = IConsumer::NO_TRIGGER;
		info.dur = stream.num / stream.sr;
		info.time = 0;
		info.event = 0;
		player.consume (info, stream.num, &stream);
		player.consume_flush (stream.num, &stream);
	}

	static void PlaySSIFile (AudioPlayer &player, const ssi_char_t *audio_file) {

		ssi_stream_t stream;
		File *file = File::Create (File::BINARY, File::READ, audio_file);
		FileTools::ReadStreamHeader (*file, stream);
		ssi_time_t time;
		FileTools::ReadStreamData (*file, &time, stream);
		Play (player, stream);
		delete file;
	}

	static void PlayWAVFile (AudioPlayer &player, const ssi_char_t *audio_file) {

		ssi_stream_t stream;
		WavTools::ReadWavFile (audio_file, stream);
		Play (player, stream);
		ssi_stream_destroy (stream);
	}

protected:

	AudioPlayer (const ssi_char_t *file = 0);
	AudioPlayer::Options _options;
	ssi_char_t *_file;
	static ssi_char_t *ssi_log_name;

	AudioOut _waveOutDevice;
	WAVEFORMATEX audio_format;
	HWAVEOUT hWaveOut;

	bool _scale;
	ssi_stream_t _stream_scale;

	int selectDevice ();			
	int getDevice (const ssi_char_t *name);
	bool getDevice (ssi_size_t id, ssi_size_t n_name, char *name);

	static int LetUserSelectDevice (StringList &list);
	static int LetUserSelectDeviceOnConsole (StringList &list);
	
};

}

#endif
