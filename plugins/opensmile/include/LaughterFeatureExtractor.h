// Laughter_Feature_Extractor.h
// author: Andrew Sadek <andrew.sadek.se@gmail.com>
// created: 2012/04/27 
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

#pragma once

#ifndef SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_H
#define SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_H

#include "base/IFeature.h"
#include "OSFunctionals.h"

#ifdef _MSC_VER 
#ifdef _DEBUG
#pragma comment(lib, "ssid.lib")
#else
#pragma comment(lib, "ssi.lib")
#endif
#endif

namespace ssi {

#define SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_FEATURE_NUM 2

class LaughterFeatureExtractor : public IFeature {

public:

	static const ssi_char_t *GetCreateName () { return "LaughterFeatureExtractor"; };
	static IObject *Create (const ssi_char_t *file) { return new LaughterFeatureExtractor (); };
	~LaughterFeatureExtractor ();

	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "This component extracts laughter features."; };
	IOptions *getOptions () { return 0; };
	

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

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in);
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sample_bytes_in;
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {

		if (sample_type_in != SSI_REAL) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
		}

		return SSI_REAL;
	}

	void setOptions() {

		_mfcc_func->getOptions()->enab_funct[FUNCTIONAL_CROSSINGS] = 1;
		_mfcc_func->getOptions()->enab_output[FUNCTIONAL_CROSSINGS][FUNCT_AMEAN] = 0;
		_mfcc_func->getOptions()->enab_funct[FUNCTIONAL_MOMENTS] = 1;

		_intensity_func->getOptions()->enab_funct[FUNCTIONAL_CROSSINGS] = 1;
		_intensity_func->getOptions()->enab_output[FUNCTIONAL_CROSSINGS][FUNCT_AMEAN] = 0;
		_intensity_func->getOptions()->enab_funct[FUNCTIONAL_MOMENTS] = 1;
		_intensity_func->getOptions()->enab_funct[FUNCTIONAL_REGRESSION] = 1;
		_intensity_func->getOptions()->enab_funct[FUNCTIONAL_PEAKS] = 1;
		_intensity_func->getOptions()->enab_funct[FUNCTIONAL_MEANS] = 1;
		_intensity_func->getOptions()->enab_funct[FUNCTIONAL_SEGMENTS] = 1;
	}

protected:

	LaughterFeatureExtractor ();
	OSFunctionals* _mfcc_func;
	OSFunctionals* _intensity_func;
	ITransformer *_transformers[SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_FEATURE_NUM];

	bool _transformers_dim_initialized;
	ssi_size_t _transformers_dim[SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_FEATURE_NUM];

	ssi_stream_t **_stream_tmp;
	ssi_stream_t **_pre_stream_tmp;
	ssi_stream_t* current;
    
	static bool merge_streams_real(ssi_stream_t &a, ssi_stream_t &b, ssi_stream_t &to);
	static bool split_streams_real(ssi_stream_t &from, ssi_stream_t &a, ssi_stream_t &b);
	
};

}

#endif

