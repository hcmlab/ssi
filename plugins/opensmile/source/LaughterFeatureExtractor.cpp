// OSIntensity.cpp
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

#include "LaughterFeatureExtractor.h"
#include "LaughterPreProcessor.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

LaughterFeatureExtractor::LaughterFeatureExtractor ()
 : _transformers_dim_initialized (false) {

	_mfcc_func = ssi_pcast (OSFunctionals, OSFunctionals::Create (0));
	_intensity_func = ssi_pcast (OSFunctionals, OSFunctionals::Create (0));

	setOptions();
	
	_transformers[0] = _mfcc_func;
	_transformers[1] = _intensity_func;
}

LaughterFeatureExtractor::~LaughterFeatureExtractor() {

	delete _mfcc_func;
	delete _intensity_func;
}

ssi_size_t LaughterFeatureExtractor::getSampleDimensionOut (ssi_size_t sample_dimension_in) {
		
	if (!_transformers_dim_initialized) {
		LaughterPreProcessor* pre = ssi_factory_create(LaughterPreProcessor, 0, false);
		pre->setDimensionArray(_transformers_dim);
		delete pre;
		_transformers_dim_initialized = true;
	}

	ssi_size_t num = SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_FEATURE_NUM;
	ssi_size_t dim = 0;

	for (ssi_size_t i=0; i<num; i++) {
		dim += _transformers[i]->getSampleDimensionOut(_transformers_dim[i]);
	}
		
	return dim;
}

void LaughterFeatureExtractor::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	if (!_transformers_dim_initialized) {
		LaughterPreProcessor* pre = ssi_factory_create(LaughterPreProcessor, 0, false);
		pre->setDimensionArray(_transformers_dim);
		delete pre;
		_transformers_dim_initialized = true;
	}

	_stream_tmp = new ssi_stream_t *[SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_FEATURE_NUM];
	_pre_stream_tmp = new ssi_stream_t *[SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_FEATURE_NUM];

	for (ssi_size_t i = 0; i < SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_FEATURE_NUM; i++) {
		_stream_tmp[i] = new ssi_stream_t;	
	}

	for (ssi_size_t i = 0; i < SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_FEATURE_NUM; i++) {
		_pre_stream_tmp[i] = new ssi_stream_t;
		ssi_stream_init(*_pre_stream_tmp[i], 0, _transformers_dim[i], stream_in.byte, stream_in.type, stream_in.sr);
	}

	for(int i=0; i<SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_FEATURE_NUM; i++) {
		ssi_stream_init(*_stream_tmp[i], getSampleNumberOut(0), _transformers[i]->getSampleDimensionOut(_transformers_dim[i]),
		_transformers[i]->getSampleBytesOut(stream_in.byte), _transformers[i]->getSampleTypeOut(stream_in.type), stream_in.sr);
		_transformers[i]->transform_enter (*_pre_stream_tmp[i], *_stream_tmp[i]);
		
	}			
	
}

void LaughterFeatureExtractor::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	for (ssi_size_t i = 0; i < SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_FEATURE_NUM; i++) {		
		ssi_stream_adjust (*_pre_stream_tmp[i], stream_in.num);
	}

	//made for 2 streams only , can be used for more but the intermediate stream has to be initialized
	split_streams_real(stream_in, *_pre_stream_tmp[0], *_pre_stream_tmp[1]);

	for(int i=0; i<SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_FEATURE_NUM; i++)
		_transformers[i]->transform(info, *_pre_stream_tmp[i], *_stream_tmp[i]);

	//made for 2 streams only , can be used for more but the intermediate stream has to be initialized
	merge_streams_real(*_stream_tmp[0], *_stream_tmp[1], stream_out);
	
}

void LaughterFeatureExtractor::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	for (int i=0; i<SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_FEATURE_NUM; i++) {
		_transformers[i]->transform_flush(*_pre_stream_tmp[i], *_stream_tmp[i]);
	}
	
	for (ssi_size_t i = 0; i < SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_FEATURE_NUM; i++) {		
		ssi_stream_destroy (*_stream_tmp[i]);
		ssi_stream_destroy (*_pre_stream_tmp[i]);
		delete[] _stream_tmp[i];
		delete[] _pre_stream_tmp[i];	
	}

	delete[] _stream_tmp; _stream_tmp = 0;
	delete[] _pre_stream_tmp; _pre_stream_tmp = 0;
}

bool LaughterFeatureExtractor::merge_streams_real(ssi_stream_t &a, ssi_stream_t &b, ssi_stream_t &to) {

	if(a.type != b.type || a.num != b.num)
		return false;

	int num = a.num;
	int d1 = a.dim;
	int d2 = b.dim;
	int N = 0;

	ssi_real_t *dst = ssi_pcast (ssi_real_t, to.ptr);
	ssi_real_t *src_a = ssi_pcast (ssi_real_t, a.ptr);
	ssi_real_t *src_b = ssi_pcast (ssi_real_t, b.ptr);

	for(int i=0; i<num; i++) {

        for(int j=0; j<d1; j++){

            dst[N] = src_a[(i*d1)+j];
            N++;
        }

        for(int j=0; j<d2; j++){

            dst[N] = src_b[(i*d2)+j];
            N++;
        }
		
	}

	return true;
	
}

bool LaughterFeatureExtractor::split_streams_real(ssi_stream_t &from, ssi_stream_t &a, ssi_stream_t &b) {

		if(a.type != b.type || a.num != b.num)
			return false;

		int num = a.num;
		int d1 = a.dim;
		int d2 = b.dim;
		int N,N1,N2;
		
        N =0;
        N1 =0;
        N2 = 0;

		ssi_real_t *src = ssi_pcast (ssi_real_t, from.ptr);
		ssi_real_t *dst_a = ssi_pcast (ssi_real_t, a.ptr);
		ssi_real_t *dst_b = ssi_pcast (ssi_real_t, b.ptr);

		for(int i=0; i<num; i++) {

            for(int j=0; j<d1; j++)
            {
                dst_a[N1] = src[N];
                N1++;
                N++;
            }

            for(int j=0; j<d2; j++)
            {
                dst_b[N2] = src[N];
                N2++;
                N++;
            }
		
		}

		return true;
	
	}


}

