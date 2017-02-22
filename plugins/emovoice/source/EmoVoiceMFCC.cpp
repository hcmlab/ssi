// EmoVoiceMFCC.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
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

#include "EmoVoiceMFCC.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

extern "C" {
#include "ev_dsp.h"
#include "ev_fextract.h"
}

namespace ssi {

EmoVoiceMFCC::EmoVoiceMFCC () {
	
	int version = DSP_MK_VERSION(1,4);
	fex = dsp_fextract_create (dsp_fextype_MFCC, version, NULL);
	steps = 0;
}

EmoVoiceMFCC::~EmoVoiceMFCC () {

	if (fex) {
		dsp_fextract_destroy((dsp_fextract_t  *)fex);
	}
}

ssi_size_t EmoVoiceMFCC::getSampleDimensionOut (ssi_size_t sample_dimension_in) {

	SSI_ASSERT (sample_dimension_in == 1);
	
	return EMOVOICEMFCC_FEATURE_SIZE;
}

ssi_size_t EmoVoiceMFCC::getSampleNumberOut (ssi_size_t sample_number_in) {

	return (sample_number_in - (EMOVOICEMFCC_FRAME_SIZE - EMOVOICEMFCC_FRAME_STEP)) / EMOVOICEMFCC_FRAME_STEP;
}

ssi_size_t EmoVoiceMFCC::getSampleBytesOut (ssi_size_t sample_bytes_in) {

    SSI_ASSERT (sample_bytes_in == sizeof (int16_t));

	return sizeof (mx_real_t);
}

ssi_type_t EmoVoiceMFCC::getSampleTypeOut (ssi_type_t sample_type_in) {

	SSI_ASSERT (sample_type_in == SSI_SHORT);

	return SSI_FLOAT;
}

void EmoVoiceMFCC::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t sample_number = stream_in.num;

    int16_t *inblock = ssi_pcast (int16_t, stream_in.ptr);
	float *outblock = ssi_pcast (mx_real_t, stream_out.ptr);
	
	ssi_size_t steps = (sample_number - (EMOVOICEMFCC_FRAME_SIZE - EMOVOICEMFCC_FRAME_STEP)) / EMOVOICEMFCC_FRAME_STEP;

	if (steps <= 0) {
		ssi_err ("Input vector too short (< %d)", EMOVOICEMFCC_FRAME_SIZE);
	}

	// for the first 4 frames 'dsp_fextract_calc' does not calculate the derivative
	// hence we set the first 4 frames of outblock to 0
	for (ssi_size_t i = 0; i < 4*39; i++) {
		outblock[i] = 0;
	}

	// now we do the feature calculation
	for (ssi_size_t i = 0; i < steps; i++) {
		dsp_fextract_calc((dsp_fextract_t  *) fex, (mx_real_t *)outblock, (dsp_sample_t *)(inblock + i*EMOVOICEMFCC_FRAME_STEP));
		outblock += EMOVOICEMFCC_FEATURE_SIZE;
	}
}

}
