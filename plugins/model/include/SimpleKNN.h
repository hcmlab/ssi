// SimpleKNN.h
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

#ifndef SSI_MODEL_SIPMLEKNN_H
#define SSI_MODEL_SIMPLEKNN_H

#include "base/IModel.h"
#include "ssiml/include/SampleList.h"
#include "ssiml/include/ModelTools.h"
#include "ioput/file/FileBinary.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class SimpleKNN : public IModel {

public:

	enum DISTANCE_MEASURE_FUNCTION {
		EUCLIDIAN = 0
	};

public:

	class Options : public OptionList {

	public:

		Options ()
			: k (3), dist (EUCLIDIAN) {
			addOption ("k", &k, 1, SSI_UINT, "k neighbours");
			addOption ("dist", &dist, 1, SSI_INT, "distance measure function ( 0 = Eucidian )");
		};

		ssi_size_t k;
		DISTANCE_MEASURE_FUNCTION dist;
	};

public:

	static const ssi_char_t *GetCreateName () { return "SimpleKNN"; };
	static IObject *Create (const ssi_char_t *file) { return new SimpleKNN (file); };
	
	SimpleKNN::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "K-nearest neighbor classifier."; };

	bool train (ISamples &samples,
		ssi_size_t stream_index);	
	bool isTrained () { return _data != 0; };
	bool forward (ssi_stream_t &stream,
		ssi_size_t n_probs,
		ssi_real_t *probs,
		ssi_real_t &confidence);
	void release ();
	bool save (const ssi_char_t *filepath);
	bool load (const ssi_char_t *filepath);

	IModel::TYPE::List getModelType() { return IModel::TYPE::CLASSIFICATION; }
	ssi_size_t getClassSize () { return _n_classes; };
	ssi_size_t getStreamDim () { return _n_features; };
	ssi_size_t getStreamByte () { return sizeof (ssi_real_t); };
	ssi_type_t getStreamType () { return SSI_REAL; };

protected:	

	SimpleKNN (const ssi_char_t *file = 0);
	virtual ~SimpleKNN ();
	SimpleKNN::Options _options;
	ssi_char_t *_file;

	typedef ssi_real_t(*DistanceMeasureFunc)(ssi_real_t*, ssi_real_t*, ssi_size_t);
	static ssi_real_t EuclideanSquaredReal (ssi_real_t *x1, ssi_real_t *x2, ssi_size_t numberDimensions);

	ssi_size_t _n_classes;
	ssi_size_t _n_samples;
	ssi_size_t _n_features;
	ssi_real_t *_data;
	ssi_size_t *_classes;
};

}

#endif
