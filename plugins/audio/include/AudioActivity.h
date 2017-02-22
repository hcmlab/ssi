// AudioActivity.h
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

#ifndef SSI_AUDIO_AUDIOACTIVITY_H
#define SSI_AUDIO_AUDIOACTIVITY_H

#include "base/IFeature.h"
#include "base/IFilter.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class AudioActivity : public IFeature {

public:

	enum METHOD {
		LOUDNESS = 0,
		INTENSITY = 1,
		SNRATIO = 2
	};

public:

	class Options : public OptionList {

		public:

			Options ()
				: threshold (0.0f), method (LOUDNESS) {
				
					addOption ("method", &method, 1, SSI_INT, "Activity method (0=loudness, 1=intensity, 2=signal-to-noise ratio");
					addOption ("threshold", &threshold, 1, SSI_REAL, "Threshold to determine activity", false);
			};		
					
			ssi_real_t threshold;
			METHOD method;
		};


public:

	static const ssi_char_t *GetCreateName () { return "AudioActivity"; };
	static IObject *Create (const ssi_char_t *file) { return new AudioActivity (file); };
	AudioActivity::Options *getOptions () { return &_options; };
	~AudioActivity ();
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Determines activity in a raw audio signal"; };

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

protected:

	AudioActivity (const ssi_char_t *file = 0);
	AudioActivity::Options _options;
	ssi_char_t *_file;

	void readOptions();
	ssi_real_t _threshold;

	bool _convert_input;
	IFilter *_convert;
	ssi_stream_t _stream_convert;

	IFeature *_feature;
	ssi_stream_t _stream_feat;
};

}

#endif
