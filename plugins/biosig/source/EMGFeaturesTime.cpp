// EMGFeaturesTime.cpp
// author: Daniel Schork
// created: 2016
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

#include "EMGFeaturesTime.h"
#include "base/Factory.h"
#include "..\..\signal\include\Spectrogram.h"
#include "frame\include\Selector.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

#define FLT_BIG 1000000

#define NOISE_NONE 0
#define NOISE_DIM 1
#define NOISE_XTRA 2

ssi_size_t noise_source = NOISE_NONE;

namespace ssi {

ssi_char_t *EMGFeaturesTime::ssi_log_name = "EMGFeaturesTime__";

ssi_real_t tf_mav_prev = 0;

EMGFeaturesTime::EMGFeaturesTime (const ssi_char_t *file)
	: _file (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}

	tf_mav_prev = 0;
}

EMGFeaturesTime::~EMGFeaturesTime () {


	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void EMGFeaturesTime::transform_enter(ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num,	ssi_stream_t xtra_stream_in[]) {

	if (stream_in.type != SSI_REAL) {
		ssi_err ("type '%s' not supported", SSI_TYPE_NAMES[stream_in.type]);
	}
		
	if (_options.print_features){
		printFeatures();
		_options.print_features = false;
	}

	noise_source = NOISE_NONE;
	if (stream_in.dim == 2){
		noise_source = NOISE_DIM;
	}
	else if (xtra_stream_in_num == 1){
		if (xtra_stream_in[0].dim == 1){
			noise_source = NOISE_XTRA;
		}
	}


	if (noise_source == NOISE_NONE) ssi_wrn("[Time Features] No noise level received! (dim: %i, x num: %i, x dim: %i)\n", stream_in.dim, xtra_stream_in_num, xtra_stream_in[0].dim);

	if (_options.hemg_segments > 0){

		Selector *sel = ssi_pcast(Selector, Factory::Create(Selector::GetCreateName(), 0, false));
		sel->getOptions()->set(0);
		_sel = sel;
		ssi_stream_init(_sel_stream, 0, _sel->getSampleDimensionOut(stream_in.dim), _sel->getSampleBytesOut(stream_in.byte), _sel->getSampleTypeOut(stream_in.type), stream_in.sr);
		_sel->transform_enter(stream_in, _sel_stream);

		Spectrogram *spec = ssi_pcast(Spectrogram, Factory::Create(Spectrogram::GetCreateName(), 0, false));
		spec->getOptions()->nbanks = _options.hemg_segments;
		_spec = spec;
		ssi_stream_init(_spec_stream, 0, _spec->getSampleDimensionOut(_sel_stream.dim), _spec->getSampleBytesOut(_sel_stream.byte), _spec->getSampleTypeOut(_sel_stream.type), stream_in.sr);
		_spec->transform_enter(stream_in, _spec_stream);
	}
	
	
}


//based on https://www.researchgate.net/publication/268363764_Stages_for_Developing_Control_Systems_using_EMG_and_EEG_Signals_A_survey
void EMGFeaturesTime::transform(ITransformer::info info, ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num, ssi_stream_t xtra_stream_in[]) {

	ssi_real_t weight1 = 0; //for mmav1
	ssi_real_t weight2 = 0; //for mmav2
	ssi_real_t mean = 0; //for var
	ssi_real_t ptr_in_prev = 0; //for WL
	ssi_real_t ptr_in_prev_prev = 0; //for WL
	ssi_real_t noise_level = 0; //for ZC, SSC

	ssi_real_t *xtra_ptr_in = nullptr;
	if (noise_source == NOISE_XTRA){
		if (xtra_stream_in_num == 1){
			if (xtra_stream_in[0].dim == 1){
				xtra_ptr_in = ssi_pcast(ssi_real_t, xtra_stream_in[0].ptr);
			}
		}
	}

	ssi_real_t tf_iemg = 0;
	ssi_real_t tf_mav = 0;
	ssi_real_t tf_mmav1 = 0;
	ssi_real_t tf_mmav2 = 0;
	ssi_real_t tf_mavs = 0;
	ssi_real_t tf_rms = 0;
	ssi_real_t tf_var = 0;
	ssi_real_t tf_wl = 0;
	ssi_real_t tf_zc = 0;
	ssi_real_t tf_ssc = 0;
	ssi_real_t tf_wamp = 0;
	ssi_real_t tf_ssi = 0;

	ssi_real_t sin = (float)stream_in.num;
	ssi_real_t *ptr_in = ssi_pcast(ssi_real_t, stream_in.ptr);
	for (int nsamp = 0; nsamp < stream_in.num; nsamp++){
				
		tf_iemg += fabsf(*ptr_in);

		if (nsamp < (sin * 0.25)){
			weight1 = 0.5;
			weight2 = (float)(4 * nsamp) / sin;
		}
		else if (nsamp <= sin * 0.75){
			weight1 = 1.0;
			weight2 = 1.0;
		}
		else {
			weight1 = 0.5;
			weight2 = 4.0f * (float)(nsamp - stream_in.num) / sin;
		}

		tf_mmav1 += weight1 * fabsf(*ptr_in);
		tf_mmav2 += weight2 * fabsf(*ptr_in);
		tf_rms += powf(*ptr_in, 2);
		mean += *ptr_in;

		tf_wl += fabsf(*ptr_in - ptr_in_prev);

		if (noise_source == NOISE_XTRA){
			noise_level = *xtra_ptr_in;
			xtra_ptr_in++;
		}
		else if (noise_source == NOISE_DIM){
			noise_level = *(ptr_in + 1);
		}

		if ((ptr_in_prev > 0 && *ptr_in < 0) || (ptr_in_prev < 0 && *ptr_in > 0) ){
			if (fabsf(ptr_in_prev - *ptr_in) >= noise_level){
				tf_zc += 1;
			}
		}

		if ((ptr_in_prev > ptr_in_prev_prev && ptr_in_prev > *ptr_in) || (ptr_in_prev < ptr_in_prev_prev && ptr_in_prev < *ptr_in)){
			if ((fabsf(ptr_in_prev - *ptr_in) >= noise_level) || (fabsf(ptr_in_prev - ptr_in_prev_prev) >= noise_level)){
				tf_ssc += 1;
			}
		}

		if (fabsf(ptr_in_prev - *ptr_in) > noise_level){
			tf_wamp += 1;
		}

		tf_ssi += powf(*ptr_in, 2);

		ptr_in_prev_prev = ptr_in_prev;
		ptr_in_prev = *ptr_in;
		ptr_in += stream_in.dim;
	}

	tf_mav = tf_iemg / (float)stream_in.num;
	tf_mmav1 /= sin;
	tf_mmav2 /= sin;

	tf_mavs = tf_mav_prev - tf_mav;
	tf_mav_prev = tf_mav;

	tf_rms /= sin;
	tf_rms = sqrtf(tf_rms);

	mean /= sin;

	ptr_in = ssi_pcast(ssi_real_t, stream_in.ptr);
	for (int nsamp = 0; nsamp < stream_in.num; nsamp++){
		tf_var += powf(*ptr_in - mean, 2);
		ptr_in += stream_in.dim;
	}

	tf_var /= sin;

	ssi_real_t *ptr_sel;
	ssi_real_t *ptr_spec;
	if (_options.hemg_segments > 0){

		//histogram
		ITransformer::info tinfo;
		tinfo.delta_num = stream_in.num;
		tinfo.time = info.time;

		ssi_stream_adjust(_sel_stream, stream_in.num);
		_sel->transform(tinfo, stream_in, _sel_stream);

		ssi_stream_adjust(_spec_stream, _sel_stream.num);
		_spec->transform(tinfo, _sel_stream, _spec_stream);
		ptr_spec = ssi_pcast(ssi_real_t, _spec_stream.ptr);

		if (_options.hemg_segments != _spec_stream.dim){
			ssi_wrn("[EMG time features] expected %i hemg segments, spectogram dim is %i\n", _options.hemg_segments, _spec_stream.dim);
			getchar();
		}
	}

	ssi_real_t *ptr_out = ssi_pcast(ssi_real_t, stream_out.ptr);
	*ptr_out = tf_iemg; //IEMG: Integrated EMG (sum of absolutes)
	ptr_out++;
	*ptr_out = tf_mav; //MAV: Mean Absolute Value
	ptr_out++;
	*ptr_out = tf_mmav1; //MMAV1: Modified Mean Absolute Value 1
	ptr_out++;
	*ptr_out = tf_mmav2; //MMAV2: Modified Mean Absolute Value 2
	ptr_out++;
	*ptr_out = tf_mavs; //MAVS: Modified Mean Absolute Value Slope
	ptr_out++;
	*ptr_out = tf_rms; //RMS: Root Mean Square
	ptr_out++;
	*ptr_out = tf_var; //VAR: Variance
	ptr_out++;
	*ptr_out = tf_wl; //WL: Waveform Length
	ptr_out++;
	*ptr_out = tf_zc; //ZC: Zero Crossings
	ptr_out++;
	*ptr_out = tf_ssc; //SSC: Slope Sign Changes
	ptr_out++;
	*ptr_out = tf_wamp; //WAMP: Willison Amplitude
	ptr_out++;
	*ptr_out = tf_ssi; //SSI: Simple Square Integral (energy)
	ptr_out++;

	for (int i = 0; i < _options.hemg_segments; i++){
		*ptr_out = *ptr_spec; //HEMG: Histogram of EMG
		ptr_spec++;
		ptr_out++;
	}


}

void EMGFeaturesTime::transform_flush(ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	delete _spec;
}

void EMGFeaturesTime::printFeatures(){

	ssi_print("\n[EMG Features Time] Activated Features:\n");
	ssi_print(" 0: iemg\n");
	ssi_print(" 1: mav\n");
	ssi_print(" 2: mmav1\n");
	ssi_print(" 3: mmav2\n");
	ssi_print(" 4: mavs\n");
	ssi_print(" 5: rms\n");
	ssi_print(" 6: var\n");
	ssi_print(" 7: wl\n");
	ssi_print(" 8: zc\n");
	ssi_print(" 9: ssc\n");
	ssi_print("10: wamp\n");
	ssi_print("11: ssi\n");

	for (int i = 0; i < _options.hemg_segments; i++){
		ssi_print("%i: hemg%i\n", i + 12, i + 1);
	}

	ssi_print("\n");

}

}
