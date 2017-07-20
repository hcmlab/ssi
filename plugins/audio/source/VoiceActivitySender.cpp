// VoiceActivitySender.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/11/19
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

#include "VoiceActivitySender.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi
{

VoiceActivitySender::VoiceActivitySender (const ssi_char_t *file)
: _file (0),
	_audio_activity(0),
	_zero_event_sender(0) {

	_audio_activity = ssi_pcast(AudioActivity, Factory::Create(AudioActivity::GetCreateName(), 0, false));
	_audio_activity->getOptions()->threshold = 0.05f;
	_audio_activity->getOptions()->method = AudioActivity::LOUDNESS;
	if (!_audio_activity) {
		ssi_err("could not create 'AudioActivity'");
	}
	for (ssi_size_t i = 0; i < _audio_activity->getOptions()->getSize(); i++) {
		ssi_option_t *o = _audio_activity->getOptions()->getOption(i);
		_options.addOption(o->name, o->ptr, o->num, o->type, o->help);
	}

	_zero_event_sender = ssi_pcast(ZeroEventSender, Factory::Create(ZeroEventSender::GetCreateName(), 0, false));
	_zero_event_sender->getOptions()->mindur = 0.5;
	_zero_event_sender->getOptions()->maxdur = 5.0;
	_zero_event_sender->getOptions()->hangin = 3;
	_zero_event_sender->getOptions()->hangout = 10;
	_zero_event_sender->getOptions()->setEvent("vad");
	_zero_event_sender->getOptions()->setSender("audio");
	if (!_zero_event_sender) {
		ssi_err("could not create 'ZeroEventSender'");
	}	
	for (ssi_size_t i = 0; i < _zero_event_sender->getOptions()->getSize(); i++) {
		ssi_option_t *o = _zero_event_sender->getOptions()->getOption(i);
		_options.addOption(o->name, o->ptr, o->num, o->type, o->help);
	}

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

VoiceActivitySender::~VoiceActivitySender () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}

	delete _audio_activity;
	delete _zero_event_sender;
}

bool VoiceActivitySender::setEventListener(IEventListener *listener) {

	return _zero_event_sender->setEventListener(listener);
}

void VoiceActivitySender::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_audio_activity->transform_enter(stream_in, stream_out, xtra_stream_in_num, xtra_stream_in);
	_zero_event_sender->consume_enter(1, &stream_out);
}

void VoiceActivitySender::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_audio_activity->transform(info, stream_in, stream_out, xtra_stream_in_num, xtra_stream_in);
	IConsumer::info cinfo;
	cinfo.dur = (info.frame_num + info.delta_num) / stream_in.sr;
	cinfo.time = info.time;
	cinfo.status = IConsumer::CONTINUED;
	cinfo.event = 0;
	_zero_event_sender->consume(cinfo, 1, &stream_out);
}

void VoiceActivitySender::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_audio_activity->transform_flush(stream_in, stream_out, xtra_stream_in_num, xtra_stream_in);
	_zero_event_sender->consume_flush(1, &stream_out);
}

}
