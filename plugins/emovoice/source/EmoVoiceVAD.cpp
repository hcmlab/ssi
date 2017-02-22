// EmoVoiceVAD.h
// author: Johannes Wagner <wagner@hcm-lab.de>
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

#include "EmoVoiceVAD.h"

extern "C" {
#include "ev_dsp.h"
#include "ev_vad.h"
}


#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

EmoVoiceVAD::EmoVoiceVAD () {
	
	int version = DSP_MK_VERSION(1,0);
	vad = dsp_vad_create (version, EMOVOICEVAD_FRAME_SIZE);
	voice = new dsp_sample_t[EMOVOICEVAD_FRAME_SIZE];
	steps = 0;
	no_va_counter = 0;
	in_va_segment = false;
}

EmoVoiceVAD::~EmoVoiceVAD () {

	if (vad)
		dsp_vad_destroy ((dsp_vad_t *) vad);
	if ((dsp_sample_t *)voice)
		delete (dsp_sample_t *)voice;
}

ssi_size_t EmoVoiceVAD::getSampleDimensionOut (ssi_size_t sample_dimension_in) {

	SSI_ASSERT (sample_dimension_in == 1);

	return 1;
}

ssi_size_t EmoVoiceVAD::getSampleNumberOut (ssi_size_t sample_number_in) {

	return (sample_number_in - (EMOVOICEVAD_FRAME_SIZE - EMOVOICEVAD_FRAME_STEP)) / EMOVOICEVAD_FRAME_STEP;
}

ssi_size_t EmoVoiceVAD::getSampleBytesOut (ssi_size_t sample_bytes_in) {

    SSI_ASSERT (sample_bytes_in == sizeof (int16_t));

	return sizeof (int);
}

ssi_type_t EmoVoiceVAD::getSampleTypeOut (ssi_type_t sample_type_in) {

	SSI_ASSERT (sample_type_in == SSI_SHORT);

	return SSI_FLOAT;
}

void EmoVoiceVAD::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t sample_number = stream_in.num;

    int16_t *inblock = ssi_pcast (int16_t, stream_in.ptr);
	int *outblock = ssi_pcast (int, stream_out.ptr);

	ssi_size_t steps = (sample_number - (EMOVOICEVAD_FRAME_SIZE - EMOVOICEVAD_FRAME_STEP)) / EMOVOICEVAD_FRAME_STEP;

	if (steps <= 0) {
		ssi_err ("Input vector too short (%d)", EMOVOICEVAD_FRAME_SIZE);
	}

	for (ssi_size_t i = 0; i < steps; i++) {
		*outblock++ = dsp_vad_calc((dsp_sample_t *)voice, (dsp_vad_t *)vad, (dsp_sample_t *)(inblock + i*EMOVOICEVAD_FRAME_STEP));
	}
}

}
