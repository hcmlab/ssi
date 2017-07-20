// EmoVoicePitch.cpp
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

#include "EmoVoicePitch.h"

extern "C" {
#include "ev_pitch.h"
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

EmoVoicePitch::EmoVoicePitch (const ssi_char_t *file) 
	: _cfg (0),
	_file (0) {	

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
} 

EmoVoicePitch::~EmoVoicePitch () { 

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

ssi_size_t EmoVoicePitch::getSampleDimensionOut (ssi_size_t sample_dimension_in) {
	
	SSI_ASSERT (sample_dimension_in == 1);

	return 1;
}

ssi_size_t EmoVoicePitch::getSampleNumberOut (ssi_size_t sample_number_in) {

	// check if we already know the result
	if (sample_number_in == _sample_number_in) {
		return _sample_number_out;
	}

	if (!_cfg) {
		_cfg = pitch_create(ssi_cast(pitch_method_t, _options.method));
		((pitch_t *)_cfg)->minimumPitch = ssi_cast(mx_real_t, _options.minfreq);
		((pitch_t *)_cfg)->maximumPitch = ssi_cast(mx_real_t, _options.maxfreq);
	}

	// calculate new sample number
	pitch_method_t method = ((pitch_t *)_cfg)->method;
	mx_real_t minimumPitch = ((pitch_t *)_cfg)->minimumPitch;
	mx_real_t dt = ((pitch_t *)_cfg)->dt;
	int periodsPerWindow = ((pitch_t *)_cfg)->periodsPerWindow;
	if (method == AC_GAUSS) 
		 periodsPerWindow *= 2;
	mx_real_t x1 = (mx_real_t) 0.5/SAMPLERATE;
	mx_real_t dt_window = periodsPerWindow / minimumPitch;

	int nFrames;
	mx_real_t t1;

	int result = fitInFrame (method >= FCC_NORMAL ? 1 / minimumPitch + dt_window : dt_window, dt, &nFrames, &t1, x1, sample_number_in);
	SSI_ASSERT (result);

	if (method >= FCC_NORMAL)
		nFrames = nFrames - 1;

	_sample_number_out = nFrames;

	return nFrames;
}

ssi_size_t EmoVoicePitch::getSampleBytesOut (ssi_size_t sample_bytes_in) {

    SSI_ASSERT (sample_bytes_in == sizeof (int16_t));

	return sizeof (mx_real_t);
}

ssi_type_t EmoVoicePitch::getSampleTypeOut (ssi_type_t sample_type_in) {

	SSI_ASSERT (sample_type_in == SSI_SHORT);

	return SSI_FLOAT;
}

void EmoVoicePitch::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	if (!_cfg) {
		_cfg = pitch_create (ssi_cast (pitch_method_t, _options.method));
		((pitch_t *)_cfg)->minimumPitch = ssi_cast(mx_real_t, _options.minfreq);
		((pitch_t *)_cfg)->maximumPitch = ssi_cast(mx_real_t, _options.maxfreq);
	}
}

void EmoVoicePitch::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t sample_number = stream_in.num;

    int16_t *srcptr = ssi_pcast (int16_t, stream_in.ptr);
	mx_real_t *dstptr = ssi_pcast (mx_real_t, stream_out.ptr);

	mx_real_t *p = pitch_calc((pitch_t *)_cfg, srcptr, sample_number);
	memcpy(dstptr, p, ((pitch_t *)_cfg)->nframes * sizeof(mx_real_t));

	pitch_destroy((pitch_t *)_cfg);
	_cfg = pitch_create (ssi_cast (pitch_method_t, _options.method));
	((pitch_t *)_cfg)->minimumPitch = ssi_cast(mx_real_t, _options.minfreq);
	((pitch_t *)_cfg)->maximumPitch = ssi_cast(mx_real_t, _options.maxfreq);

	free (p);
}

void EmoVoicePitch::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	pitch_destroy((pitch_t *)_cfg);
	_cfg = 0;
}

}
