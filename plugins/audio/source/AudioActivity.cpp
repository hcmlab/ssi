// AudioActivity.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/03/18
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

#include "AudioActivity.h"
#include "base/Factory.h"
#include "AudioConvert.h"
#include "AudioIntensity.h"
#include "SNRatio.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi
{

AudioActivity::AudioActivity (const ssi_char_t *file)
: _file (0),
	_convert_input (false),
	_convert (0),
	_feature (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

AudioActivity::~AudioActivity () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void AudioActivity::readOptions() {

	_options.lock();

	_threshold = _options.threshold;

	_options.unlock();
}

void AudioActivity::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	switch (_options.method) {
		case AudioActivity::INTENSITY: {
			AudioIntensity *intensity = ssi_pcast (AudioIntensity, Factory::Create (AudioIntensity::GetCreateName (), 0, false));
			intensity->getOptions ()->intensity = true;
			intensity->getOptions ()->loudness = false;
			_feature = intensity;
			break;				
		}
		case AudioActivity::LOUDNESS: {
			AudioIntensity *loudness = ssi_pcast (AudioIntensity, Factory::Create (AudioIntensity::GetCreateName (), 0, false));
			loudness->getOptions ()->intensity = false;
			loudness->getOptions ()->loudness = true;
			_feature = loudness;
			break;
		}
		case AudioActivity::SNRATIO: {
			SNRatio *snratio = ssi_pcast (SNRatio, Factory::Create (SNRatio::GetCreateName (), 0, false));
			snratio->getOptions ()->thresh = 0;
			_feature = snratio;
			break;
		}
	}

	ssi_stream_init (_stream_feat, 1, 1, sizeof (ssi_real_t), SSI_REAL, stream_out.sr);

	_convert_input = stream_in.type == SSI_SHORT;
	if (_convert_input) {
		_convert = ssi_pcast (AudioConvert, Factory::Create (AudioConvert::GetCreateName (), 0, false));
		ssi_stream_init (_stream_convert, 0, stream_in.dim, sizeof (ssi_real_t), SSI_REAL, stream_in.sr);
		_convert->transform_enter (stream_in, _stream_convert);
		_feature->transform_enter (_stream_convert, _stream_feat);
	} else {
		_feature->transform_enter (stream_in, _stream_feat);
	}
	
}

void AudioActivity::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t sample_dimension = stream_in.dim;
	ssi_size_t sample_number = stream_in.num;
	ssi_real_t threshold = _options.threshold;
	
	if (_convert_input) {
		ssi_stream_adjust (_stream_convert, stream_in.num);
		_convert->transform (info, stream_in, _stream_convert);
		_feature->transform (info, _stream_convert, _stream_feat);
	} else {
		_feature->transform (info, stream_in, _stream_feat);
	}

	ssi_real_t *srcptr = ssi_pcast (ssi_real_t, _stream_feat.ptr);	
	ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);
	
	readOptions();

	*dstptr = *srcptr > _threshold ? *srcptr : 0.0f;
}

void AudioActivity::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	if (_convert_input) {
		_feature->transform_flush (_stream_convert, _stream_feat);
		delete _convert; _convert = 0;
		ssi_stream_destroy (_stream_convert);
	} else {
		_feature->transform_flush (stream_in, _stream_feat);
	}

	delete _feature; _feature = 0;
	ssi_stream_destroy (_stream_feat);
}

}
