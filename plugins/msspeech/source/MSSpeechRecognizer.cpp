// MSSpeechSpeechRecognizer.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
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

#include "MSSpeechRecognizer.h"
#include "MSSpeechAudioStream.h"
#include "MSSpeechWrapper.h"

#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *MSSpeechRecognizer::ssi_log_name = "sapirec___";

MSSpeechRecognizer::MSSpeechRecognizer (const ssi_char_t *file)
	: _file (0),
	_stream (0),
	_speechrec (0),
	_speechrecthread (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init (_event, SSI_ETYPE_STRING);
}

MSSpeechRecognizer::~MSSpeechRecognizer () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool MSSpeechRecognizer::setEventListener (IEventListener *listener) {

	_listener = listener;
	_event.sender_id = Factory::AddString (_options.sname);
	if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	_event.event_id = Factory::AddString (_options.ename);
	if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	
	_event_address.setSender (_options.sname);
	_event_address.setEvents (_options.ename);

	return true;
}

void MSSpeechRecognizer::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (stream_in[0].type != SSI_SHORT && stream_in[0].type != SSI_FLOAT) {
		ssi_err ("format %s not supported", SSI_TYPE_NAMES[stream_in[0].type]);
	}

	if (stream_in[0].type == SSI_FLOAT) {
		ssi_stream_init (_stream_short, 0, stream_in[0].dim, sizeof (short), SSI_SHORT, stream_in[0].sr);
	}

	if (stream_in[0].dim != 1) {
		ssi_err ("dimension %u not supported", stream_in[0].dim);
	}

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "create audio buffer");
	_stream = new MSSpeechAudioStream (ssi_cast (ULONG, stream_in[0].sr * 5.0));
	ssi_msg (SSI_LOG_LEVEL_DETAIL, "create speech recognition");
	if (!ssi_exists (_options.grammar)) {
		ssi_err ("invalid grammar path '%s'", _options.grammar);
	}

	ssi_char_t option[SSI_MAX_CHAR];
	ssi_sprint (option, "Language=%s%s", _options.language, _options.kinect ? ";Kinect=True" : "");	
	_speechrec = new MSSpeechWrapper (this, _options.grammar, option, _stream, _options.outputFormat);
	ssi_msg (SSI_LOG_LEVEL_BASIC, "init speech recognition");
	if (!_speechrec->Init ()) {
		ssi_err ("could not init speech recognition");
	}
}

void MSSpeechRecognizer::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	short *inptr = 0;
	if (stream_in[0].type == SSI_FLOAT) {
		ssi_stream_adjust (_stream_short, stream_in[0].num);
		inptr = ssi_pcast (short, _stream_short.ptr);
		float *srcptr = ssi_pcast (float, stream_in[0].ptr);
		for (ssi_size_t i = 0; i < stream_in[0].num * stream_in[0].dim; i++) {
			*inptr++ = ssi_cast (short, *srcptr++ * 32768.0f);
		}
		inptr = ssi_pcast (short, _stream_short.ptr);
	} else {
		inptr = reinterpret_cast<short *> (stream_in[0].ptr);
	}

	_stream->Write (ssi_pcast (BYTE, inptr), stream_in[0].num * stream_in[0].dim * sizeof (short));

	if (!_speechrecthread) {
		ssi_msg (SSI_LOG_LEVEL_BASIC, "start speech recognition");
		_speechrecthread = new RunAsThread (_speechrec);
		_speechrecthread->start ();
	}
}

void MSSpeechRecognizer::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "stop speech recognition");
	_stream->Stop ();
	if (!_speechrec->Stop ()) {
		ssi_err ("could not stop speech recognition");
	}
	_speechrecthread->stop ();	

	delete _speechrec; _speechrec = 0;
	delete _stream; _stream = 0;
	delete _speechrecthread; _speechrecthread = 0;

	if (stream_in[0].type == SSI_FLOAT) {
		ssi_stream_destroy (_stream_short);
	}
}

void MSSpeechRecognizer::update (const ssi_char_t *keyword, ULONGLONG start_ms, ULONGLONG dur_ms, float confidence) {
	
	if (_listener) {
		if (confidence > _options.confidence) {
			ssi_size_t n_keyword = (ssi_size_t)strlen(keyword);
			ssi_event_adjust (_event, n_keyword + 1);
			ssi_strcpy (_event.ptr, keyword);
			_event.time = (ssi_size_t)start_ms;
			_event.dur = (ssi_size_t)dur_ms;
			_event.prob = confidence;
			_listener->update (_event);
		} else {
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "rejected keyword '%s'[%.2f < %.2f]", keyword, confidence, _options.confidence);
		}
	}
}

bool MSSpeechRecognizer::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {
	ssi_event_t *e = 0;
	for (ssi_size_t nevent = 0; nevent < n_new_events; nevent++){
		e = events.next();
		// Print Event Content
		ssi_print(e->ptr);
		// Reset Speech Grammar
		_speechrec->ResetSpeechGrammar(e->ptr);		
	}
	return true;
}

MSSpeechRecognizer::RunAsThread::RunAsThread (MSSpeechWrapper *sr) 
	: Thread (true), _sr (sr) {
	setName ("speechrec");
}

MSSpeechRecognizer::RunAsThread::~RunAsThread () {
};
		
void MSSpeechRecognizer::RunAsThread::run () {
	if (!_sr->Start ()) {
		ssi_err ("could not start speech recognition");
	}
}

}
