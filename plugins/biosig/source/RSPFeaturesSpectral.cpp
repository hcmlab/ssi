// RSPFeaturesSpectral.cpp
// author: Florian Lingenfelser <lingenfelser@hcm-lab.de>
// created: 2013/03/08
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
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

#include "RSPFeaturesSpectral.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

RSPFeaturesSpectral::RSPFeaturesSpectral(const ssi_char_t *file)
	: _file (0) {		

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

RSPFeaturesSpectral::~RSPFeaturesSpectral() {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void RSPFeaturesSpectral::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	
}

void RSPFeaturesSpectral::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

		ssi_real_t *ptr_in = ssi_pcast (ssi_real_t, stream_in.ptr);
		ssi_real_t *ptr_out = ssi_pcast (ssi_real_t, stream_out.ptr);

		ssi_real_t VLF = 0.0f;
		ssi_real_t LF = 0.0f;
		ssi_real_t HF = 0.0f;
		ssi_real_t nVLF = 0.0f;
		ssi_real_t nLF = 0.0f;
		ssi_real_t nHF = 0.0f;

		if (stream_in.num == 1){
	
			VLF = *ptr_in;
			ptr_in++;
			LF = *ptr_in;
			ptr_in++;
			HF = *ptr_in;
			ptr_in++;

			nVLF = (VLF * 100.0f) / (VLF + LF + HF);
			nLF = (LF * 100.0f) / (VLF + LF + HF);
			nHF = (HF * 100.0f) / (VLF + LF + HF);

			if (std::isnan(VLF) || std::isinf(VLF)) VLF = 0;
			if (std::isnan(LF) || std::isinf(LF)) LF = 0;
			if (std::isnan(HF) || std::isinf(HF)) VLF = 0;

			if (std::isnan(nVLF) || std::isinf(nVLF)) nVLF = 0;
			if (std::isnan(nLF) || std::isinf(nLF)) nLF = 0;
			if (std::isnan(nHF) || std::isinf(nHF)) nHF = 0;


			*ptr_out = VLF;
			ptr_out++;
			*ptr_out = LF;
			ptr_out++;
			*ptr_out = HF;
			ptr_out++;
			*ptr_out = nVLF;
			ptr_out++;
			*ptr_out = nLF;
			ptr_out++;
			*ptr_out = nHF;
			ptr_out++;
							

			if(_options.print){
				ssi_print("\nRSP FREQUENCY DOMAIN FEATURES");
				ssi_print("\n-------------------------");
				ssi_print("\nVLF:\t%.2f\t(power spectrum 0.001 to 0.05 Hz)", VLF);
				ssi_print("\nLF:\t%.2f\t(power spectrum 0.05 to 0.15 Hz)", LF);
				ssi_print("\nHF:\t%.2f\t(power spectrum 0.15 to 0.5 Hz)", HF);
				ssi_print("\nnVLF:\t%.2f\t(fraction in %%)", nVLF);
				ssi_print("\nnLF:\t%.2f\t(fraction in %%)", nLF);
				ssi_print("\nnHF:\t%.2f\t(fraction in %%)", nHF);
				ssi_print("\n");
			}
		}else{
			ssi_err("ambiguous call: more than one sample gathered from spectogram");
		}
}

void RSPFeaturesSpectral::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num ,
	ssi_stream_t xtra_stream_in[]) {
}

}
