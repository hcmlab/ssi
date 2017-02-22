// EmoVoicePitch.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/05/26
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

/**

Provides fast pitch estimation.

*/

#pragma once

#ifndef SSI_SIGNAL_EMOVOICEPITCH_H
#define SSI_SIGNAL_EMOVOICEPITCH_H

#include "base/ITransformer.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class EmoVoicePitch : public ITransformer {

public:

	class Options : public OptionList {

	public:

		Options () 
			: method (1), minfreq (50), maxfreq (500) {

			addOption ("method", &method, 1, SSI_INT, "Pitch calculation method (0=AC_HANNING, 1=AC_GAUSS, 2=FCC_NORMAL, 3=FCC_ACCURATE");
			addOption ("minfreq", &minfreq, 1, SSI_DOUBLE, "minimum frequency");
			addOption ("maxfreq", &maxfreq, 1, SSI_DOUBLE, "maximum frequency");
		};

		int method;
		double minfreq;
		double maxfreq;
	};


public:

	static const ssi_char_t *GetCreateName () { return "EmoVoicePitch"; };
	static IObject *Create (const ssi_char_t *file) { return new EmoVoicePitch (file); };
	~EmoVoicePitch ();
	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Pitch calculation"; };

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in);
	ssi_size_t getSampleNumberOut (ssi_size_t sample_number_in);
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in);
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in);

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

	EmoVoicePitch (const ssi_char_t *file = 0);
	Options _options;	
	ssi_char_t *_file;

	void *_cfg;
	ssi_size_t _sample_number_in, _sample_number_out;

};

}

#endif
