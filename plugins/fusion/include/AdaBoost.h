// AdaBoost.h
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2011/03/03
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

#ifndef SSI_MODEL_ADABOOST_H
#define SSI_MODEL_ADABOOST_H

#include "base/IFusion.h"
#include "..\..\libs\build\ssiml\include\ModelTools.h"
#include "ioput/file/FileBinary.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class AdaBoost : public IFusion {

public:

	class Options : public OptionList {

	public:

		Options () : iter (1), size (100), error (0.5), combination_rule (0) {

			addOption ("iter", &iter, 1, SSI_SIZE, "number of iterations");
			addOption ("size", &size, 1, SSI_SIZE, "percent of samples used for one iteration");
			addOption ("error", &error, 1, SSI_REAL, "default error for model to be accepted");
			addOption ("combination rule", &combination_rule, 1, SSI_CHAR, "(0) Default Combination, (1) Product Rule, (2) Sum Rule, (3) Max Rule, (4) Mean Rule, (5) Min Rule");
		};

		ssi_size_t iter;
		ssi_size_t size;
		ssi_real_t error;
		ssi_size_t combination_rule;

	};

public:

	ssi_size_t getModelNumber(ISamples &samples){ return (_options.iter * samples.getStreamSize());};

	static const ssi_char_t *GetCreateName () { return "AdaBoost"; };
	static IObject *Create (const ssi_char_t *file) { return new AdaBoost (file); };

	void setLogLevel (ssi_size_t log_level) { ssi_log_level = log_level; };
	
	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "AdaBoost Algorithm"; };

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
	ssi_real_t coin();
	bool save (const ssi_char_t *filepath);
	bool load (const ssi_char_t *filepath);
	bool supportsMissingData() { return false; };


protected:

	int ssi_log_level;
	static ssi_char_t *ssi_log_name;

	AdaBoost (const ssi_char_t *file = 0);
	virtual ~AdaBoost ();

	AdaBoost::Options _options;

	bool select (IModel **models, ISamples &samples, ssi_size_t nstrm);
	
	ssi_char_t *_file;

	ssi_sample_t *_sample;

	ssi_size_t _n_classes;
	ssi_size_t _n_streams;
	ssi_size_t _n_models;
	ssi_size_t _n_samples;

	ssi_size_t _iterations;
	ssi_size_t _percent;
	ssi_size_t _niteration;

	ssi_real_t _error_default;
	ssi_real_t *_error_rates;

};

}

#endif
