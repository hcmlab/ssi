// EmoVoiceVAD.h
// author: Johannes Wager <wagner@hcm-lab.de>
// created: 2007/10/26
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

//! \brief VAD calculation using EmoVoice.

#ifndef SSI_SIGNAL_EMOVOICEVAD_H
#define SSI_SIGNAL_EMOVOICEVAD_H

#include "base/ITransformer.h"

#define EMOVOICEVAD_FRAME_SIZE  256
#define EMOVOICEVAD_FRAME_STEP  160
#define EMOVOICEVAD_WITHIN_PAUSE_LENGTH 5

namespace ssi {

class EmoVoiceVAD : public ITransformer {

public:

	EmoVoiceVAD ();
	virtual ~EmoVoiceVAD ();

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in);
	ssi_size_t getSampleNumberOut (ssi_size_t sample_number_in);
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in);
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in);

	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

protected:

	int steps, in_cols;
	void *vad;
	void *voice;
	int no_va_counter;
	bool in_va_segment;

};

}

#endif
