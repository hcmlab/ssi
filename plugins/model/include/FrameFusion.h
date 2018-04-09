// FrameFusion.h
// author: Johannes Wagner <johannes.wagner@informatik.uni-augsburg.de>
// created: 2018/02/25
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

#ifndef SSI_MODEL_FRAMEFUSION_H
#define SSI_MODEL_FRAMEFUSION_H

#include "base/IModel.h"
#include "ssiml/include/SampleList.h"
#include "ssiml/include/ModelTools.h"
#include "ioput/file/FileBinary.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class FrameFusion : public IModel {

public:

	enum class METHOD 
	{
		SUM = 0,
		PRODUCT,
		MAX
	};

public:

	class Options : public OptionList {

	public:

		Options ()
			: n_context(0), method(METHOD::SUM) {
			addOption("method", &method, 1, SSI_INT, "fusion method ( 0 = Mean )");
			addOption("context", &n_context, 1, SSI_UINT, "number of context frames");
		};

		ssi_size_t n_context;
		METHOD method;
	};

public:

	static const ssi_char_t *GetCreateName () { return "FrameFusion"; };
	static IObject *Create (const ssi_char_t *file) { return new FrameFusion (file); };
	
	FrameFusion::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Within sample fusion."; };

	bool train (ISamples &samples,
		ssi_size_t stream_index);	
	bool isTrained () { return _model->isTrained(); };
	bool forward (ssi_stream_t &stream,
		ssi_size_t n_probs,
		ssi_real_t *probs,
		ssi_real_t &confidence);
	void release ();
	bool save (const ssi_char_t *filepath);
	bool load (const ssi_char_t *filepath);

	IModel::TYPE::List getModelType() { return _model->getModelType(); }
	ssi_size_t getClassSize () { return _model->getClassSize(); };
	ssi_size_t getStreamDim () { return _model->getStreamDim(); };
	ssi_size_t getStreamByte () { return _model->getStreamByte(); };
	ssi_type_t getStreamType () { return _model->getStreamType(); };

	virtual void setModel(IModel *model);

protected:	

	FrameFusion (const ssi_char_t *file = 0);
	virtual ~FrameFusion ();
	FrameFusion::Options _options;
	ssi_char_t *_file;
	IModel *_model;
};

}

#endif
