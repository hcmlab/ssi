// MicrosoftKinectSmileDetector.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2013/04/14
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

#include "MicrosoftKinect2SmileDetector.h"
#include "MicrosoftKinect2.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

MicrosoftKinect2SmileDetector::MicrosoftKinect2SmileDetector (const ssi_char_t *file) 
	: _file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	_factors[0] = 1.0f;
	_factors[1] = 1.0f;
	_factors[2] = 1.0f;
}

MicrosoftKinect2SmileDetector::~MicrosoftKinect2SmileDetector () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void MicrosoftKinect2SmileDetector::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
	

	if (_options.factors[0] != '\0') {
		ssi_size_t found = ssi_string2array_count (_options.factors, ',');
		if (found < stream_in.dim) {
			ssi_err ("#factors (%u) < #dimensions (%u)", found, stream_in.dim);
		}
		ssi_real_t *tmp = new ssi_real_t[found];
		ssi_string2array (found, tmp, _options.factors, ',');
		for (ssi_size_t i = 0; i < stream_in.dim; i++) {
			_factors[i] = tmp[i];
		}
		delete[] tmp;
	}
}

void MicrosoftKinect2SmileDetector::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {	

	ssi_real_t *srcptr = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);

	ssi_real_t jaw_lowerer, lip_stretcher, lip_corner_depressor;
	for (ssi_size_t i = 0; i < stream_in.num; i++) {				
		jaw_lowerer = srcptr[0]; //JawOpen
		lip_stretcher = (srcptr[3] + srcptr[4]) / 2; //LipStretcherLeft, LipStretcherRight
		lip_corner_depressor = 1.0f - ((srcptr[7] + srcptr[8]) / 2); //LipCornerDepressorLeft, LipCornerDepressorRight
		*dstptr++ = (_factors[0] * jaw_lowerer) * (_factors[1] * lip_stretcher) * (_factors[2] * lip_corner_depressor);
		srcptr += stream_in.dim;
	}				
}

void MicrosoftKinect2SmileDetector::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num ,
	ssi_stream_t xtra_stream_in[]) {
}

ssi_size_t MicrosoftKinect2SmileDetector::getSampleDimensionOut(ssi_size_t sample_dimension_in) {
	if (sample_dimension_in != MicrosoftKinect2::ACTIONUNIT::NUM) {
		ssi_err("input dimension %u != %u", sample_dimension_in, MicrosoftKinect2::ACTIONUNIT::NUM);
	}
	return 1;
}
ssi_size_t MicrosoftKinect2SmileDetector::getSampleBytesOut(ssi_size_t sample_bytes_in) {
	return sample_bytes_in;
}
ssi_type_t MicrosoftKinect2SmileDetector::getSampleTypeOut(ssi_type_t sample_type_in) {
	if (sample_type_in != SSI_REAL) {
		ssi_err("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
	}
	return SSI_REAL;
}

}
