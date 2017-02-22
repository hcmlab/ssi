// VoiceActivityVerifier.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/01/12
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_AUDIO_VOICEACTIVITYVERIFIER_H
#define SSI_AUDIO_VOICEACTIVITYVERIFIER_H

#include "base/IConsumer.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"
#include "AudioLpc.h"

namespace ssi {

class VoiceActivityVerifier : public IConsumer {

public:

	class Options : public OptionList {

	public:

		Options() : from(1u), to(6u), threshold(2.0f) {

			setSenderName ("audio");
			setEventName ("vadok");			

			addOption ("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender");
			addOption ("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event");
			addOption("threshold", &threshold, 1, SSI_SIZE, "event is verified if threshold is exceeded (normalized squared sum of lpc coefficients)");
			addOption ("from", &from, 1, SSI_SIZE, "first lpc index to consider");
			addOption ("to", &to, 1, SSI_SIZE, "last lpc index to consider");
		};

		void setSenderName (const ssi_char_t *sname) {			
			if (sname) {
				ssi_strcpy (this->sname, sname);
			}
		}
		void setEventName (const ssi_char_t *ename) {
			if (ename) {
				ssi_strcpy (this->ename, ename);
			}
		}
		
		ssi_char_t sname[SSI_MAX_CHAR];
		ssi_char_t ename[SSI_MAX_CHAR];			
		ssi_real_t threshold;
		ssi_size_t from, to;
	};

public: 	

	static const ssi_char_t *GetCreateName () { return "VoiceActivityVerifier"; };
	static IObject *Create (const ssi_char_t *file) { return new VoiceActivityVerifier (file); };
	~VoiceActivityVerifier ();

	Options *getOptions () { return &_options; };
	AudioLpc::Options *getAudioLpcOptions() { return _lpc->getOptions(); };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Verifies if an activity event is voiced."; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	bool setEventListener (IEventListener *listener);
	const ssi_char_t *getEventAddress () {
		return _event_address.getAddress ();
	}

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	VoiceActivityVerifier (const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	IEventListener *_listener;
	EventAddress _event_address;
	ssi_event_t _event;
	
	ssi_stream_t _lpc_s;
	AudioLpc *_lpc;
};

}

#endif
