// AudioPortaudioPlayer.h
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

#ifndef SSI_ASIO_AUDIOPORTAUDIOPLAYER_H
#define SSI_ASIO_AUDIOPORTAUDIOPLAYER_H

#include "base/IConsumer.h"
#include "ioput/file/FileTools.h"
#include "ioput/file/FileBinary.h"
#include "ioput/option/OptionList.h"
#include "ioput/wav/WavTools.h"

typedef void PaStream;

namespace ssi {

class AudioPortaudioPlayer : public IConsumer {

public:

	class Options : public OptionList {

	public:

		Options ()
			: device (-1), defdevice (false), remember (true), api(-1) {

			addOption ("device", &device, 1, SSI_INT, "audio device id");
			addOption ("api", &api, 1, SSI_INT, "preferred audio api (all=-1, DS=1, MME=2, ASIO=3, WASAPI=13 ...)");
			addOption ("default", &defdevice, 1, SSI_BOOL, "select default device (overwrites device option)");
			addOption ("remember",	&remember,	1, SSI_BOOL, "remember selected audio device");		
		};	

		int api;
		int device;
		bool defdevice;
		bool remember;
	};

public:

	static const ssi_char_t *GetCreateName () { return "AudioPortaudioPlayer"; };
	static IObject *Create (const ssi_char_t *file) { return new AudioPortaudioPlayer (file); };
	~AudioPortaudioPlayer ();

	AudioPortaudioPlayer::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Opens an audio output device for playback of a mono or stereo waveform."; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

protected:

	AudioPortaudioPlayer (const ssi_char_t *file = 0);
	AudioPortaudioPlayer::Options _options;
	ssi_char_t *_file;	

	WAVEFORMATEX _format;
	PaStream *_stream;
	bool _scaled;	

	int selectDevice ();			
	int getDevice (const ssi_char_t *name);

	static int LetUserSelectDevice (StringList &list);
	static int LetUserSelectDeviceOnConsole(StringList &list);

	unsigned int numDevicesForApi(int api_type);
	
	static ssi_char_t *ssi_log_name;
};

}

#endif
