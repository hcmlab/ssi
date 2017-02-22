// EmoVoiceMFCC.h
// author: Johannes Wager <wagner@hcm-lab.de>
// created: 2007/10/23
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

//! \brief MFCC calculation using EmoVoice.

#ifndef SSI_SIGNAL_EMOVOICEMFCC_H
#define SSI_SIGNAL_EMOVOICEMFCC_H

#include "base/ITransformer.h"

#define EMOVOICEMFCC_FEATURE_SIZE 39
#define EMOVOICEMFCC_FRAME_SIZE  256
#define EMOVOICEMFCC_FRAME_STEP  160

namespace ssi {

	class EmoVoiceMFCC : public ITransformer {

	public:

		static const ssi_char_t *GetCreateName () { return "EmoVoiceMFCC"; };
		static IObject *Create (const ssi_char_t *file) { return new EmoVoiceMFCC (); };
		IOptions *getOptions () { return 0; };
		~EmoVoiceMFCC ();
		const ssi_char_t *getName () { return GetCreateName (); };
		const ssi_char_t *getInfo () { return "Mel Frequency Ceptral Components (MFCCs)"; };

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

		EmoVoiceMFCC ();

		int steps, in_cols;
		void *fex;
	};

}

#endif
