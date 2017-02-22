// Rank.h
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

#ifndef SSI_MODEL_RANK_H
#define SSI_MODEL_RANK_H

#include "base/ISelection.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class Rank : public ISelection {

public:

	class Options : public OptionList {

	public:

		Options () 
			: kfold (2), loo (false), louo (false) {

			addOption ("kfold", &kfold, 1, SSI_SIZE, "#folds used during evaluation");
			addOption ("loo", &loo, 1, SSI_BOOL, "use leave-one-out instead of kfold");
			addOption ("louo", &louo, 1, SSI_BOOL, "use leave-one-user-out instead of kfold");
		};

		ssi_size_t kfold;
		bool loo;
		bool louo;
	};

public:

	static const ssi_char_t *GetCreateName () { return "Rank"; };
	static IObject *Create (const ssi_char_t *file) { return new Rank (file); };
	virtual ~Rank ();
	
	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Rank feature selection."; };

	void setModel (IModel &model) { _model = &model; };
	bool train (ISamples &samples,
		ssi_size_t stream_index);
	bool isTrained () { return _model != 0; };
	bool isWrapper () { return false; };
	bool sortByScore () { return true; };
	void release ();	

	ssi_size_t getSize () { return _n_scores; };
	const score *getScores () { return _scores; };	

	virtual void print (FILE *file = stdout);

	static void SetLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	Rank (const ssi_char_t *file);
	Options _options;
	ssi_char_t *_file;

	static int ssi_log_level;
	static ssi_char_t *ssi_log_name;

	IModel *_model;
	ssi_size_t _n_scores;
	score *_scores;
};

}

#endif
