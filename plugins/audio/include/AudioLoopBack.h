// AudioLoopBack.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 15/6/2018
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_AUDIO_AUDIOLOOPBACK_H
#define SSI_AUDIO_AUDIOLOOPBACK_H

#include "base/ISensor.h"
#include "ioput/option/OptionList.h"
#include "thread/Thread.h"
#include "AudioLoopBackInterface.h"

#define SSI_AUDIOLOOPBACK_PROVIDER_NAME "audio"

namespace ssi {

	class AudioLoopBackHelper;

	class AudioLoopBack : public ISensor, public AudioLoopBackInterface, public Thread {

	public:

		class AudioChannel : public IChannel {

			friend class AudioLoopBack;

		public:

			AudioChannel() {
				ssi_stream_init(stream, 0, 0, 0, SSI_UNDEF, 0);
			}
			~AudioChannel() {
				ssi_stream_destroy(stream);
			}

			const ssi_char_t *getName() { return SSI_AUDIOLOOPBACK_PROVIDER_NAME; };
			const ssi_char_t *getInfo() { return "A mono or stereo waveform either as a stream of short values in range [-32768 32767] or float values in range [-1.0 1.0]"; };
			ssi_stream_t getStream() { return stream; };

		protected:

			ssi_stream_t stream;
		};

		class Options : public OptionList {

		public:

			Options() : scale(true) {				
				addOption("scale", &scale, 1, SSI_BOOL, "scale to interval [-1..1]");
			}
			
			bool scale;
		};

	public:

		static const ssi_char_t *GetCreateName() { return "AudioLoopBack"; };
		static IObject *Create(const ssi_char_t *file) { return new AudioLoopBack(file); };
		~AudioLoopBack();

		Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "Captures audio coming to a audio playback device"; };

		ssi_size_t getChannelSize() { return 1; };
		IChannel *getChannel(ssi_size_t index) { return &_audio_channel; };
		bool setProvider(const ssi_char_t *name, IProvider *provider);
		bool connect();
		bool start() { return Thread::start(); };
		void run();
		bool stop() { return Thread::stop(); };
		bool disconnect();

		WAVEFORMATEX getFormat() { return _format; };

	protected:

		AudioLoopBack(const ssi_char_t *file = 0);
		ssi_char_t *_file;
		Options _options;
		static char ssi_log_name[];

		AudioChannel _audio_channel;
		WAVEFORMATEX _format;

		int CopyData(const BYTE* Data, const int NumFramesAvailable);
		AudioLoopBackHelper *_listener;

		void setProvider(IProvider *provider);
		IProvider *_provider;
	};

}

#endif
