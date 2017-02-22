// FScore.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/05/03
// Copyright (C) University of Augsburg
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

#ifndef SSI_MODEL_FSCORE_H
#define SSI_MODEL_FSCORE_H

#include "base/ISelection.h"
#include "ioput/option/OptionList.h"

#define SSI_FSCORE_SQR(z) ((z) * (z))

namespace ssi {

class FScore : public ISelection {

public:

	class Options : public OptionList {

	public:

		Options () 
			: norm (false), mem (false) {
			addOption ("norm", &norm, 1, SSI_BOOL, "normalize scores in interval [0..1]");
			addOption ("mem", &mem, 1, SSI_BOOL, "saves memory but less efficient");
		};

		bool norm;
		bool mem;
	};

public:

	static const ssi_char_t *GetCreateName () { return "fscore"; };
	static IObject *Create (const ssi_char_t *file) { return new FScore (file); };
	virtual ~FScore ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "FScore Feature Selection"; };

	void setModel (IModel &model) {};
	bool train (ISamples &samples,
		ssi_size_t stream_index);
	bool isTrained () { return _n_scores > 0; };
	void release ();	

	ssi_size_t getSize () { return _n_scores; }; // returns number of selected features
	const score *getScores () { return _scores; }; // returns score of selected features

	virtual void print (FILE *file = stdout);

	static void SetLogLevel (int level) {
		ssi_log_level = level;
	}
	#if __gnu_linux__
	
	bool isWrapper(){return false;}
	bool sortByScore(){return false;}
	
	#endif
	

protected:

	FScore (const ssi_char_t *file);
	Options _options;
	ssi_char_t *_file;

	static int ssi_log_level;
	static ssi_char_t *ssi_log_name;

	void matrix (ISamples &samples, ssi_size_t stream_index);
	ssi_size_t _n_samples;
	ssi_size_t _n_features;
	ssi_size_t *_classes;
	ssi_real_t **_features;

	ssi_size_t _n_scores;
	score *_scores;
};

}

#endif
