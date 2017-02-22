// VoiceActivitySender.h
// author: Johannes Wager <wagner@hcm-lab.de>
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

#ifndef SSI_AUDIO_VOICEACTIVITYSENDER_H
#define SSI_AUDIO_VOICEACTIVITYSENDER_H

#include "AudioActivity.h"
#include "event/include/ZeroEventSender.h"

namespace ssi {

class VoiceActivitySender : public IFeature {

public:

	class Options : public OptionList {	};

public:

	static const ssi_char_t *GetCreateName () { return "VoiceActivitySender"; };
	static IObject *Create (const ssi_char_t *file) { return new VoiceActivitySender (file); };
	Options *getOptions () { return &_options; };
	AudioActivity::Options *getAudioActivityOptions() { return _audio_activity->getOptions(); };
	ZeroEventSender::Options *getZeroEventSenderOptions() { return _zero_event_sender->getOptions(); };
	~VoiceActivitySender ();
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Determines voice activity in a raw audio signal"; };

	ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) {
		if (sample_dimension_in != 1) {
			ssi_err ("dimension != 1 not supported");
		}
		return sample_dimension_in;
	}
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sizeof (ssi_real_t);
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL && sample_type_in != SSI_SHORT) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return SSI_REAL;
	}

	void transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform_flush (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	bool setEventListener(IEventListener *listener);
	const ssi_char_t *getEventAddress() {
		return _zero_event_sender->getEventAddress();
	}

protected:

	VoiceActivitySender (const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;

	AudioActivity *_audio_activity;
	ZeroEventSender *_zero_event_sender;
};

}

#endif
