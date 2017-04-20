// MSSpeechRecognizer.h
// author: Johannes Wagner <wagner@hcm-lab.de>, Kathrin Janowski <janowski@hcm-lab.de>
// created: 2012/10/10
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

#ifndef SSI_MS_SPEECH_RECOGNIZER_H
#define SSI_MS_SPEECH_RECOGNIZER_H

#include "base/IConsumer.h"
#include "thread/Thread.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"
#include "MSSpeechPhraseEvent.h"

class MSSpeechAudioStream;
class MSSpeechWrapper;

namespace ssi {

class MSSpeechRecognizer : public IConsumer, public MSSpeechPhraseEvent {

protected:

	class RunAsThread : public Thread {
	public:
		RunAsThread (MSSpeechWrapper *sr);
		virtual ~RunAsThread ();
		void run ();
		MSSpeechWrapper *_sr;
	};


public:

	class Options : public OptionList {

	public:

		Options () : confidence (.2f), kinect (false) {

			grammar[0] = '\0';	
			setSenderName ("speechrec");
			setEventName ("keyword");
			setLanguage("409");
			addOption ("grammar", grammar, SSI_MAX_CHAR, SSI_CHAR, "grammar path");		
			addOption ("language", language, SSI_MAX_CHAR, SSI_CHAR, "language code (409=English US,809=English UK,040c=French,407=German,410=Italian,040a=Spanish)");		
			addOption ("kinect", &kinect, 1, SSI_BOOL, "turn on if audio is captured from kinect");
			addOption ("confidence", &confidence, 1, SSI_FLOAT, "confidence threshold");
			addOption ("outputFormat", &outputFormat, SSI_MAX_CHAR, SSI_CHAR, "output format (keyword, structure)");		
			addOption ("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender");
			addOption ("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event");					
		};

		void setGrammar (const ssi_char_t *grammar) {
			this->grammar[0] = '\0';
			if (grammar) {
				ssi_strcpy (this->grammar, grammar);
			}
		}

		void setLanguage (const ssi_char_t *language) {
			this->language[0] = '\0';
			if (language) {
				ssi_strcpy (this->language, language);
			}
		}

		void setOutputFormat (const ssi_char_t *format) {
			if (format) {
				ssi_strcpy (this->outputFormat, format);
			}
		}

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
		float confidence;
		ssi_char_t grammar[SSI_MAX_CHAR];				
		ssi_char_t language[SSI_MAX_CHAR];				
		bool kinect;
		ssi_char_t outputFormat[SSI_MAX_CHAR];
	};

public: 	

	static const ssi_char_t *GetCreateName () { return "MSSpeechRecognizer"; };
	static IObject *Create (const ssi_char_t *file) { return new MSSpeechRecognizer (file); };
	~MSSpeechRecognizer ();

	MSSpeechRecognizer::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Speech recognition based on Microsoft Speech Platform 11."; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	void update (const ssi_char_t *keyword, ULONGLONG start_ms, ULONGLONG dur_ms, float confidence);

	// Update With Event From Extern Source
	bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);

	bool setEventListener (IEventListener *listener);
	const ssi_char_t *getEventAddress () {
		return _event_address.getAddress ();
	}

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	MSSpeechRecognizer (const ssi_char_t *file = 0);
	MSSpeechRecognizer::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	MSSpeechAudioStream *_stream;
	MSSpeechWrapper *_speechrec;
	RunAsThread *_speechrecthread;

	IEventListener *_listener;
	EventAddress _event_address;
	ssi_event_t _event;

	ssi_stream_t _stream_short;
};

}

#endif
