// SingleFeatures.h
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2011/09/26
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

#ifndef SSI_MODEL_SINGLEFEATURES_H
#define SSI_MODEL_SINGLEFEATURES_H

#include "base/IFusion.h"
#include "..\..\libs\build\ssiml\include\ModelTools.h"
#include "ioput/file/FileBinary.h"
#include "ioput/option/OptionList.h"

using namespace std;

namespace ssi {

class SingleFeatures : public IFusion {

public:

	class Options : public OptionList {

	public:

		Options () : iter (1) {

			addOption ("iter", &iter, 1, SSI_SIZE, "number of iterations");
	
		};

		ssi_size_t iter;
		

	};

public:

	ssi_size_t getModelNumber(ISamples &samples){
		ssi_size_t nmodels = 0;
		ModelTools::PrintInfo(samples);
		samples.reset();
		for(ssi_size_t nstrm = 0; nstrm < samples.getStreamSize(); nstrm++){
			nmodels += samples.get(0)->streams[nstrm]->dim;
		}
		return nmodels;
	};

	static const ssi_char_t *GetCreateName () { return "SingleFeatures"; };
	static IObject *Create (const ssi_char_t *file) { return new SingleFeatures (file); };

	void setLogLevel (ssi_size_t log_level) { ssi_log_level = log_level; };
	
	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Fusion: SingleFeatures"; };

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
	void print_DP(ssi_real_t ** decision_profile, ssi_real_t* probs);
	bool save (const ssi_char_t *filepath);
	bool load (const ssi_char_t *filepath);
	bool supportsMissingData() { return false; };


protected:

	int ssi_log_level;
	static ssi_char_t *ssi_log_name;

	SingleFeatures (const ssi_char_t *file = 0);
	virtual ~SingleFeatures ();

	SingleFeatures::Options _options;

	ssi_char_t *_file;

	ssi_sample_t *_sample;

	ssi_size_t _n_classes;
	ssi_size_t _n_streams;
	ssi_size_t _n_models;
	ssi_size_t _n_samples;

	ssi_size_t *_n_feats;
	ssi_real_t *_error_rates;

	

};

}

#endif
