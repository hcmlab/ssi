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

#ifndef SSI_OPENSMILE_LAUGTHERPREPROCESSOR_H
#define SSI_OPENSMILE_LAUGTHERPREPROCESSOR_H

#include "base/IFeature.h"
#include "OSMfccChain.h"
#include "OSIntensity.h"
#include "ioput/option/CmdArgOption.h"

#ifdef _MSC_VER 
#ifdef _DEBUG
#pragma comment(lib, "ssid.lib")
#else
#pragma comment(lib, "ssi.lib")
#endif
#endif

#define SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_FILTER_NUM 2


namespace ssi {

class LaughterPreProcessor : public IFeature {

public:

	class Options : public OptionList {

	public:

		Options ()
		: use_deltas (true) {

			addOption ("useDeltas", &use_deltas, 1, SSI_BOOL, "extract delta coefficients");
		};

		bool use_deltas;
	};


public:

	static const ssi_char_t *GetCreateName () { return "LaughterPreProcessor"; };
	static IObject *Create (const ssi_char_t *file) { return new LaughterPreProcessor (file); };
	~LaughterPreProcessor ();

	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "This component does laughter pre processing (filters)."; };
	Options *getOptions () { return &_options; };
	

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

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) {
		
		ssi_size_t num = SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_FILTER_NUM;
		ssi_size_t dim = 0;

		for(ssi_size_t i=0; i<num; i++)
			dim += (_transformers[i]->getSampleDimensionOut(sample_dimension_in));

		
		return dim;

	}
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sample_bytes_in;
	}
	
	ssi_size_t getSampleNumberOut (ssi_size_t sample_number_in) {
		int result;
		return result = _transformers[0]->getSampleNumberOut(sample_number_in);

	}
	
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {

		if (sample_type_in != SSI_REAL) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
		}

		return SSI_REAL;
	}
	void setOptions() {

		_mfcc->getOptions()->deltas_enable = _options.use_deltas ? 1 : 0;		

		_mfcc->getOSMfcc()->getOptions()->first = 0;
		_mfcc->getOSMfcc()->getOptions()->last = 12;
		_mfcc->getOSMfcc()->getOptions()->cepLifter = 0;

		_intensity->getOptions()->intensity = 1;
		_intensity->getOptions()->loudness = 0;

		
	}

	static bool merge_streams_real(ssi_stream_t &a, ssi_stream_t &b, ssi_stream_t &to) {

		if(a.type != b.type || a.num != b.num) {
			ssi_err ("cannot merge streams");			
		}

		int num = a.num;
		int d1 = a.dim;
		int d2 = b.dim;
		int N = 0;

		ssi_real_t *dst = ssi_pcast (ssi_real_t, to.ptr);
		ssi_real_t *src_a = ssi_pcast (ssi_real_t, a.ptr);
		ssi_real_t *src_b = ssi_pcast (ssi_real_t, b.ptr);

		for(int i=0; i<num; i++) {

			for(int j=0; j<d1; j++)
				dst[N++] = src_a[(i*d1)+j];

			for(int j=0; j<d2; j++)
				dst[N++] = src_b[(i*d2)+j];
		
		}

		return true;
	
	}

	static bool split_streams_real(ssi_stream_t &from, ssi_stream_t &a, ssi_stream_t &b) {

		if(a.type != b.type || a.num != b.num)
			return false;

		int num = a.num;
		int d1 = a.dim;
		int d2 = b.dim;
		int N,N1,N2;
		
		N = N1 = N2 = 0;

		ssi_real_t *src = ssi_pcast (ssi_real_t, from.ptr);
		ssi_real_t *dst_a = ssi_pcast (ssi_real_t, a.ptr);
		ssi_real_t *dst_b = ssi_pcast (ssi_real_t, b.ptr);

		for(int i=0; i<num; i++) {

			for(int j=0; j<d1; j++)
				dst_a[N1++] = src[N++];

			for(int j=0; j<d2; j++)
				dst_b[N2++] = src[N++];
		
		}

		return true;
	
	}

	void initializeTransformers() {

		_mfcc = ssi_pcast (OSMfccChain, OSMfccChain::Create (0));
		_intensity = ssi_pcast (OSIntensity, OSIntensity::Create (0));
			
		setOptions();
		
		_transformers[0] = _mfcc;
		_transformers[1] = _intensity;
	
	}

	void setDimensionArray(ssi_size_t* _array) {
		for (int i=0; i<SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_FILTER_NUM; i++) {
			_array[i] = _transformers[i]->getSampleDimensionOut(1);
		}
	}
	
protected:

	LaughterPreProcessor (const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;

	OSMfccChain* _mfcc;
	OSIntensity* _intensity;
	ITransformer *_transformers[SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_FILTER_NUM];

	ssi_stream_t **_stream_tmp;
	
};

}

#endif
