// AudioOpenSLPlayer.h
// author: Simon Flutura <simon.flutura@informatik.uni-augsburg.de>
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

#ifndef SSI_AUDIOOPENSLPLAYER_H
#define SSI_AUDIOOPENSLPLAYER_H

#include "base/IConsumer.h"
#include "ioput/file/FileTools.h"
#include "ioput/file/FileBinary.h"
#include "ioput/option/OptionList.h"
#include "ioput/wav/WavTools.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>


namespace ssi {

class AudioOpenSLPlayer : public IConsumer {

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

        static const ssi_char_t *GetCreateName () { return "AudioOpenSLPlayer"; };
        static IObject *Create (const ssi_char_t *file) { return new AudioOpenSLPlayer (file); };
        ~AudioOpenSLPlayer ();

        AudioOpenSLPlayer::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Opens an audio output device for playback of a mono or stereo waveform."; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);


       static void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context);
       virtual void VbqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context);

       SLObjectItf playerObject;
       SLPlayItf playerPlay;
       SLAndroidSimpleBufferQueueItf playerBufferQueue;
       SLmilliHertz playerSR;
       int nextSize, nextCount;
       short int *buffer1, *buffer2;
       short int *bufferProcessing;
       bool bufferOneFilled;
       SLEngineItf engineEngine;
       SLObjectItf engineObject;

protected:

        AudioOpenSLPlayer (const ssi_char_t *file = 0);
        AudioOpenSLPlayer::Options _options;
        ssi_char_t *_file;



	WAVEFORMATEX _format;
	bool _scaled;	

	
	static ssi_char_t *ssi_log_name;
};

}

#endif
