// EmoVoiceBayes.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/19
// Copyright (C) 2007-14 University of Augsburg, Lab for Human Centered Multimedia
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

#ifndef SSI_EV_EMOVOICEBAYES_H
#define SSI_EV_EMOVOICEBAYES_H

#include "base/IModel.h"
#include "ioput/option/OptionList.h"

extern "C" {
#include "ev_class.h"
}

namespace ssi {

class EmoVoiceBayes : public IModel {

public:

	static const ssi_char_t *GetCreateName () { return "ssi_model_EmoVoiceBayes"; };
	static IObject *Create (const ssi_char_t *file) { return new EmoVoiceBayes (file); };
	IOptions *getOptions () { return 0; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Naive Bayes Classifier"; };

	bool train (ISamples &samples,
		ssi_size_t stream_index);	
	bool isTrained () { return _model != 0; };
	bool forward (ssi_stream_t &stream,
		ssi_size_t n_probs,
		ssi_real_t *probs);	
	void release ();
	bool save (const ssi_char_t *filepath);
	bool load (const ssi_char_t *filepath);

	ssi_size_t getClassSize () { return _n_classes; };
	ssi_size_t getStreamDim () { return _n_features; };
	ssi_size_t getStreamByte () { return sizeof (ssi_real_t); };
	ssi_type_t getStreamType () { return SSI_REAL; };	

	//static Trainer *LoadOldFormat (const ssi_char_t *filepath); 

protected:	

	EmoVoiceBayes (const ssi_char_t *file = 0);
	virtual ~EmoVoiceBayes ();
	

	classifier_t *_model;
	cType _cl_type;
	ssi_char_t *_cl_param;

	ssi_size_t _n_classes;
	ssi_size_t _n_features;

	static IModel *LoadOldFormatH (FILE *file, 
		ssi_size_t n_classes, 
		ssi_char_t **class_names);
};

}

#endif
