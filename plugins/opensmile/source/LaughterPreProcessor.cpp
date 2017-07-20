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

LaughterPreProcessor::LaughterPreProcessor (const ssi_char_t *file)
	: _file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	initializeTransformers();
}

LaughterPreProcessor::~LaughterPreProcessor() {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}

	delete _mfcc;
	delete _intensity;
}

void LaughterPreProcessor::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_stream_tmp = new ssi_stream_t *[SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_FILTER_NUM];
	for (ssi_size_t i = 0; i < SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_FILTER_NUM; i++) {
		_stream_tmp[i] = new ssi_stream_t;

	}

	for(int i=0; i<SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_FILTER_NUM; i++) {
		ssi_stream_init (*_stream_tmp[i], 
			0, 
			_transformers[i]->getSampleDimensionOut(stream_in.dim), 
			_transformers[i]->getSampleBytesOut(stream_in.byte),
			_transformers[i]->getSampleTypeOut(stream_in.type),
			stream_out.sr);
		_transformers[i]->transform_enter (stream_in, *_stream_tmp[i]);
	}				

}

void LaughterPreProcessor::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t sample_number;
	
	for (ssi_size_t i = 0; i < SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_FILTER_NUM; i++) {		
		//sample_number = _transformers[i]->getSampleNumberOut(info.frame_num);
		sample_number = _transformers[i]->getSampleNumberOut(stream_in.num);
		ssi_stream_adjust (*_stream_tmp[i], sample_number);
		_transformers[i]->transform(info, stream_in, *_stream_tmp[i]);
	}

	ssi_real_t *pmfcc = ssi_pcast (ssi_real_t, _stream_tmp[0]->ptr);

	//made for 2 streams only , can be used for more but the intermediate stream has to be initialized
	merge_streams_real(*_stream_tmp[0], *_stream_tmp[1], stream_out);				
}

void LaughterPreProcessor::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	for(int i=0; i<SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_FILTER_NUM; i++) {
		_transformers[i]->transform_flush(stream_in, *_stream_tmp[i]);
	}
	
	for (ssi_size_t i = 0; i < SSI_OPENSMILE_LAUGTHERFEATUREEXTRACTOR_FILTER_NUM; i++) {
		ssi_stream_destroy (*_stream_tmp[i]);
		delete[] _stream_tmp[i];
	}

	delete[] _stream_tmp; _stream_tmp = 0;
}


}

