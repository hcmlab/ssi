// AudioNoiseGate.h
// author: Johannes Wager <wagner@hcm-lab.de>
// created: 2014/03/18
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

#ifndef SSI_AUDIO_AUDIONOISEGATE_H
#define SSI_AUDIO_AUDIONOISEGATE_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class AudioNoiseGate : public IFilter {

public:

	class Options : public OptionList {

	public: 
			
		Options():
			bypass(false), threshold (-70.0f), attack(30), hold(500), decay(1000), range(-90.0f) {

			addOption("bypass", &bypass, 1, SSI_BOOL, "bypass", false);
			addOption("threshold", &threshold, 1, SSI_REAL, "threshold (dB)", false);
			addOption("attack", &attack, 1, SSI_SIZE, "attack (ms)", false);
			addOption("hold", &hold, 1, SSI_SIZE, "hold (ms)", false);
			addOption("decay", &decay, 1, SSI_SIZE, "decay (ms)", false);
			addOption("range", &range, 1, SSI_REAL, "range (dB)", false);

		};

		bool bypass;
		ssi_real_t threshold;
		ssi_size_t attack;
		ssi_size_t decay;
		ssi_size_t hold;
		ssi_real_t range;
	};

public:

	static const ssi_char_t *GetCreateName () { return "AudioNoiseGate"; };
	static IObject *Create (const ssi_char_t *file) { return new AudioNoiseGate (file); };
	~AudioNoiseGate ();

	AudioNoiseGate::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Computes Intensity and/or Loudness (narrow band approximation)"; };

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

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) {

		if (sample_dimension_in != 1) {
			ssi_err ("sample dimension != 1 not supported");
		}

		return sample_dimension_in;
	}

	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sample_bytes_in;
	}

	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {

		if (sample_type_in != SSI_REAL) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
		}

		return SSI_REAL;
	}
	
protected:

	struct STATE
	{
		enum List {
			CLOSED,
			ATTACK,
			OPENED,
			DECAY,
		};
	};

	AudioNoiseGate (const ssi_char_t *file = 0);
	AudioNoiseGate::Options _options;
	ssi_char_t *_file;

	void updateOptions();

	bool _bypass;
	ssi_real_t _threshold;
	ssi_size_t _attack;
	ssi_size_t _decay;
	ssi_size_t _hold;
	ssi_real_t _range;
	STATE::List _state;
	ssi_real_t _gate;
	int _holding;

};

}

#endif
