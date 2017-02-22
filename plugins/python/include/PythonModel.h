// PythonModel.h
// author: Dominik Schiller <schiller@hcm-lab.de>
// created: 2016/03/02
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

#ifndef SSI_PYTHON_MODEL_H
#define SSI_PYTHON_MODEL_H

#include "base/IModel.h"
#include "PythonOptions.h"

namespace ssi {

class PythonHelper;

class PythonModel : public IModel {

public:

	class Options : public PythonOptions {
	};

public:

	static const ssi_char_t *GetCreateName () { return "PythonModel"; };
	static IObject *Create (const ssi_char_t *file) { return new PythonModel (file); };
	~PythonModel ();

	PythonModel::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Python model wrapper."; };

	bool train(ISamples &samples,
		ssi_size_t stream_index);
	bool isTrained() { return _isTrained; };

	bool forward(ssi_stream_t &stream,
		ssi_size_t n_probs,
		ssi_real_t *probs);
	void release();
	bool save(const ssi_char_t *filepath);
	bool load(const ssi_char_t *filepath);

	ssi_size_t getClassSize() { return _n_classes; };
	ssi_size_t getStreamDim() { return _n_features; };
	ssi_size_t getStreamByte() { return sizeof (ssi_real_t); };
	ssi_type_t getStreamType() { return SSI_REAL; };

protected:

	PythonModel (const ssi_char_t *file = 0);
	PythonModel::Options _options;
	ssi_char_t *_file;
	static ssi_char_t *ssi_log_name;

	ssi_size_t _n_classes;
	ssi_size_t _n_features;
	ssi_size_t _n_samples;
	ssi_char_t **_class_names;

	void initHelper();
	PythonHelper *_helper;

	bool _isTrained;
};

}

#endif
