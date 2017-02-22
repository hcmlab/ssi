// AudioOpenSL.h
// author: Simon Flutura <simon.flutura@informatik.uni-augsburg.de>
// created: 2010/06/14
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

#ifndef SSI_AUDIO_AUDIOOPENSL_H
#define SSI_AUDIO_AUDIOOPENSL_H

#include "thread/Timer.h"
#include "thread/Mutex.h"
#include "thread/Thread.h"
#include "base/ISensor.h"
#include "ioput/option/OptionList.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#define SSI_AUDIOOPENSL_PROVIDER_NAME "audio"

namespace ssi
{

class AudioOpenSL;
typedef struct tagAudioOpenSLData
{
        AudioOpenSL*	Instance;
        short*		Data;
} AudioOpenSLData, *PAudioOpenSLData;

class StringList;
class IProvider;
class AudioOpenSL : public ISensor
{

public:

	class AudioChannel : public IChannel {

                friend class AudioOpenSL;

		public:

                        AudioChannel (bool _scale=false) :scale(_scale)
			{
                            if(scale)
                                ssi_stream_init (stream, 0, 0, sizeof(float), SSI_FLOAT, 0);
                            else
                                ssi_stream_init (stream, 0, 0, sizeof(short), SSI_SHORT, 0);
			}
			~AudioChannel () {
				ssi_stream_destroy (stream);
			}

                        const ssi_char_t *getName () { return SSI_AUDIOOPENSL_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Multiple waveforms as float values in range [-1.0 1.0]"; };
			ssi_stream_t getStream () { return stream; };
                        void set_scaled(bool scale)
                        {
                            this->scale=scale;
                            ssi_stream_destroy (stream);
                            if(scale)
                                ssi_stream_init (stream, 0, 0, sizeof(float), SSI_FLOAT, 0);
                            else
                                ssi_stream_init (stream, 0, 0, sizeof(short), SSI_SHORT, 0);
                        }

		protected:
			bool scale;
			ssi_stream_t stream;
	};

public:

	class Options : public OptionList
	{
		public:
                        Options() : sr(16000), block(256), remember(true), api(-1)
			{
				strcpy (channels, "0");
				device[0] = '\0';

				addOption ("device",	device,	    SSI_MAX_CHAR, SSI_CHAR, "audio device name (empty for default)");
				addOption ("api",		&api,		1, SSI_INT, "preferred audio api (all=-1, DS=1, MME=2, ASIO=3, WASAPI=13 ...)");
				addOption ("remember",	&remember,	1, SSI_BOOL, "remember selected audio device");
				addOption ("sr",		&sr,		1, SSI_DOUBLE,	"sample rate in Hz");
				addOption ("size",		&block,		1, SSI_UINT,	"frames per buffer (min 128, max 4096) (deprecated)");
				addOption ("block",		&block,		1, SSI_UINT,	"frames per buffer (min 128, max 4096)");
                                addOption ("channels",	channels,	SSI_MAX_CHAR, SSI_CHAR,	"number of channels (1=mono, 2=stereo)");
                                addOption ("scale", &scale, 1, SSI_BOOL, "scale to interval [-1..1]");
			}

			void setDevice (const ssi_char_t *name) {
				ssi_strcpy (device, name);
			}

			void setChannels (ssi_size_t n_inds, ssi_size_t *inds) {
				channels[0] = '\0';
				if (n_inds > 0) {
					ssi_char_t s[SSI_MAX_CHAR];
					ssi_sprint (s, "%u", inds[0]);
					strcat (channels, s);
					for (ssi_size_t i = 1; i < n_inds; i++) {
						ssi_sprint (s, ",%u", inds[i]);
						strcat (channels, s);
					}
				}
			}

			ssi_time_t sr;
			ssi_size_t block;
			int api;
			ssi_char_t channels[SSI_MAX_CHAR];
			ssi_char_t device[SSI_MAX_CHAR];
                        bool scale;
			bool remember;
	};
	
        static const ssi_char_t *GetCreateName () { return "Audio"; }
	const ssi_char_t *getInfo () { return "Opens multiple audio inputs using ASIO interface for recording and streaming."; }
        static IObject* Create(const ssi_char_t* file) { return new AudioOpenSL(file); }

	ssi_size_t getChannelSize () { return 1; };
	IChannel *getChannel (ssi_size_t index) { return &_audio_channel; };
	bool setProvider(const ssi_char_t *name, IProvider* provider);
	bool connect();
	bool start ();
	bool stop ();
	bool disconnect();
	bool supportsReconnect();

        bool createEngine();
        bool createAudioRecorder();

        Options *getOptions () { return &_options; }
	const ssi_char_t *getName () { return GetCreateName(); }
	ssi_object_t getType () { return SSI_SENSOR; };

protected:

        AudioOpenSL(const ssi_char_t* file);
        ~AudioOpenSL();

        static void SlStreamCallback(SLAndroidSimpleBufferQueueItf bq, void *context);
        virtual void VSlStreamCallback(SLAndroidSimpleBufferQueueItf bq, void *context);


	ssi_char_t			*_file;
	IProvider*			_dataProvider;

	ssi_size_t			_frame_size;
	Options	_options;

        SLObjectItf recorderObject;
        SLRecordItf recorderRecord;
        SLAndroidSimpleBufferQueueItf recorderBufferQueue;
        SLmilliHertz recorderSR;
        short int *recorderBuffer1, *recorderBuffer2;
        short int *recorderBufferProcessing;
        float * scaledBuffer;
        bool bufferOneFilled;
        SLEngineItf engineEngine;
        SLObjectItf engineObject;

	Mutex				_mutex;

	bool Parse (const ssi_char_t *indices);



        AudioChannel _audio_channel;

	static ssi_char_t *ssi_log_name;
};

}

#endif
