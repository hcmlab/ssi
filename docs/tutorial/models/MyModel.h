// MyModel.h
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2009/03/23
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

#ifndef _SIMPLEMODEL_H
#define _SIMPLEMODEL_H

#include "base/IModel.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class MyModel : public IModel {

public:

	static const ssi_char_t *GetCreateName () { return "mymodel"; };
	static IObject *Create (const ssi_char_t *file) { return new MyModel (); };
	virtual ~MyModel ();

	IOptions *getOptions () { return 0; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "A simple distance based model"; };

	bool train (ISamples &samples,
		ssi_size_t stream_index);	
	bool isTrained () { return _centers != 0; };
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

protected:	

	MyModel ();
	static ssi_real_t dist (ssi_real_t *x1, ssi_real_t *x2, ssi_size_t n_dim);

	ssi_size_t _n_classes;
	ssi_size_t _n_samples;
	ssi_size_t _n_features;
	ssi_real_t **_centers;
};

}

#endif
