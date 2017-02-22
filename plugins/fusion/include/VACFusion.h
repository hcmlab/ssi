// VACFusion.h
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2011/1/27
// Copyright (C) 2007-12 University of Augsburg, Florian Lingenfelser
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

#ifndef SSI_MODEL_VACFUSION_H
#define SSI_MODEL_VACFUSION_H

#include "base/IFusion.h"
#include "model/ModelTools.h"
#include "ioput/file/FileBinary.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class VACFusion : public IFusion {

public:

	ssi_size_t getModelNumber(ISamples &samples){ return samples.getStreamSize(); };

	static const ssi_char_t *GetCreateName () { return "VACFusion"; };
	static IObject *Create (const ssi_char_t *file) { return new VACFusion (file); };
	
	IOptions *getOptions () { return 0; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Fusion: VACFusion Rule"; };

	void setLogLevel (ssi_size_t log_level) { ssi_log_level = log_level; };
	
	bool train (ssi_size_t n_models,
		IModel **models,
		ISamples &samples);	
	bool isTrained () { return _n_models != 0; };
	bool forward (ssi_size_t n_models,
		IModel **models,
		ssi_size_t n_streams,
		ssi_stream_t *streams[],
		ssi_size_t n_probs,
		ssi_real_t *probs);
	void release ();
	bool save (const ssi_char_t *filepath);
	bool load (const ssi_char_t *filepath);
	bool supportsMissingData() { return true; };

protected:

	int ssi_log_level;
	static ssi_char_t *ssi_log_name;

	VACFusion (const ssi_char_t *file = 0);
	virtual ~VACFusion ();

	ssi_size_t productRule(ssi_real_t **probs, ssi_size_t n_probs);
	ssi_size_t sumRule(ssi_real_t **probs, ssi_size_t n_probs);
	
	ssi_char_t *_file;

	ssi_size_t _n_classes;
	ssi_size_t _n_streams;
	ssi_size_t _n_models;
	ssi_size_t _n_modalities;
};

}

#endif
