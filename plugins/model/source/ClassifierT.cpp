// ClassifierT.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/03/04
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

#include "ClassifierT.h"
#include "ssiml/include/Trainer.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t ClassifierT::ssi_log_name[] = "classifier";

ClassifierT::ClassifierT (const ssi_char_t *file) 
	: _trainer (0),	
	_n_classes (0),
	_borrowed(false),
	_merged_sample_dimension (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

ClassifierT::~ClassifierT () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void ClassifierT::setTrainer (Trainer *trainer) { 
	_trainer = trainer; 
	_n_classes = _trainer->getClassSize();
	_borrowed = true;
};

void ClassifierT::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	// in case of several streams
	// test if sample rates are equal
	_merged_sample_dimension = stream_in.dim;
	if (xtra_stream_in_num > 0) {
		for (ssi_size_t i = 0; i < xtra_stream_in_num; i++) {
			if (xtra_stream_in[i].sr != stream_in.sr) {
				ssi_err ("sample rates must not differ");
			}
			_merged_sample_dimension += xtra_stream_in[i].dim;
		}
	}
}

void ClassifierT::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_real_t *class_probs = ssi_pcast (ssi_real_t, stream_out.ptr);

	if (xtra_stream_in_num > 0) {
		ssi_stream_t tmp;
		ssi_stream_init (tmp, stream_in.num, _merged_sample_dimension, stream_in.byte, stream_in.type, stream_in.sr);
		ssi_byte_t *tmp_ptr = tmp.ptr;

		memcpy (tmp_ptr, stream_in.ptr, stream_in.tot);
		tmp_ptr += stream_in.tot;

		for (ssi_size_t i = 0; i < xtra_stream_in_num; i++) {
			memcpy (tmp_ptr, xtra_stream_in[i].ptr, xtra_stream_in[i].tot);
			tmp_ptr += xtra_stream_in[i].tot;
		}
		_trainer->forward_probs (tmp, _n_classes, class_probs);
		ssi_stream_destroy (tmp);
	} else {
		ssi_stream_t stream = stream_in;
		if (_options.flat)
		{
			stream.dim = stream.num * stream.dim;
			stream.num = 1;
		}
		_trainer->forward_probs (stream, _n_classes, class_probs);
	}	
}

void ClassifierT::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_merged_sample_dimension = 0;
	_n_classes = 0;

	if (!_borrowed)
	{
		delete _trainer;
		_trainer = 0;
	}
}

void ClassifierT::loadTrainer () {

	// load trainer
	if (_options.trainer[0] != '\0') 
	{
		_trainer = new Trainer ();
		if (!Trainer::Load (*_trainer, _options.trainer)) {
			ssi_err ("could not load trainer '%s'", _options.trainer);
		}
		_borrowed = false;
	} else if (!_trainer) 
	{
		ssi_err ("trainer not set");
	}
	_n_classes = _trainer->getClassSize ();

	ssi_msg (SSI_LOG_LEVEL_BASIC, "trainer loaded '%s'", _options.trainer);
}

ssi_size_t ClassifierT::getSampleDimensionOut (ssi_size_t sample_dimension_in) {

	if (!_trainer) {
		loadTrainer();
	}

	return _n_classes;
}

ssi_size_t ClassifierT::getSampleBytesOut (ssi_size_t sample_bytes_in) {
	return sizeof (ssi_real_t);
}

ssi_type_t ClassifierT::getSampleTypeOut (ssi_type_t sample_type_in) {
	return SSI_REAL;
}

}
