// LDA.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/12/23
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

#ifndef SSI_MODEL_LDA_H
#define SSI_MODEL_LDA_H

#include "base/IModel.h"
#include "ioput/option/OptionList.h"

namespace alglib_impl {
	struct ae_matrix;
	struct ae_vector;
}

namespace ssi {

class LDA : public IModel {

public:

	class Options : public OptionList {

	public:

		Options ()
			: norm (true), scale (true) {

			addOption ("norm", &norm, 1, SSI_BOOL, "normalize probabilities");
			addOption ("scale", &scale, 1, SSI_BOOL, "scale feature values");
		};

		bool norm;
		bool scale;
	};

public:

	static const ssi_char_t *GetCreateName () { return "LDA"; };
	static IObject *Create (const ssi_char_t *file) { return new LDA (file); };	
	LDA::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Linear discriminant analysis classifier."; };

	ssi_size_t getClassSize () { return _n_classes; };
	ssi_size_t getStreamDim () { return _n_features; };
	ssi_size_t getStreamByte () { return sizeof (ssi_real_t); };
	ssi_type_t getStreamType () { return SSI_REAL; };

	bool train (ISamples &samples,
		ssi_size_t stream_index);	
	bool isTrained () { return _covinv != 0; };
	bool forward (ssi_stream_t &stream,
		ssi_size_t n_probs,
		ssi_real_t *probs);	
	void release ();
	bool save (const ssi_char_t *filepath);
	bool load (const ssi_char_t *filepath);

	virtual void print (FILE *file = stdout);

protected:

	LDA (const ssi_char_t *file = 0);
	virtual ~LDA ();
	LDA::Options _options;
	ssi_char_t *_file;

	alglib_impl::ae_vector **_meanc;
	alglib_impl::ae_matrix *_covinv;
	ssi_size_t _n_classes;
	ssi_size_t _n_features;

	double *_min, *_max;	
};

}

#endif
