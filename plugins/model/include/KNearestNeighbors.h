// KNearestNeighbors.h
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

#ifndef SSI_MODEL_KNEARESTNEIGHBORS_H
#define SSI_MODEL_KNEARESTNEIGHBORS_H

#include "base/IModel.h"
#include "model/SampleList.h"
#include "model/ModelTools.h"
#include "ioput/file/FileBinary.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class KNearestNeighbors : public IModel {

public:

	class Options : public OptionList {

	public:

		Options ()
			: k (3), distsum (false) {
			addOption ("k", &k, 1, SSI_UINT, "k neighbours");
			addOption ("distsum", &distsum, 1, SSI_BOOL, "instead of counting neighbors use average distance");
		};

		ssi_size_t k;
		bool distsum;
	};

public:

	static const ssi_char_t *GetCreateName () { return "KNearestNeighbors"; };
	static IObject *Create (const ssi_char_t *file) { return new KNearestNeighbors (file); };
	
	KNearestNeighbors::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "K-nearest neighbors classifier."; };

	bool train (ISamples &samples,
		ssi_size_t stream_index);	
	bool isTrained () { return _data != 0; };
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

	KNearestNeighbors (const ssi_char_t *file = 0);
	virtual ~KNearestNeighbors ();
	KNearestNeighbors::Options _options;
	ssi_char_t *_file;

	ssi_size_t _n_classes;
	ssi_size_t _n_samples;
	ssi_size_t _n_features;
	ssi_real_t **_data;
	ssi_size_t *_classes;
};

}

#endif
