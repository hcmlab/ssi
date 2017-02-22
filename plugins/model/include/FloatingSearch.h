// FloatingSearch.h
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

#ifndef SSI_MODEL_FLOATINGSEARCH_H
#define SSI_MODEL_FLOATINGSEARCH_H

#include "base/ISelection.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class Trainer;
class Evaluation;

class FloatingSearch : public ISelection {

public:

	enum METHOD {
		SFS = 0,
		SBS,
		LR,
		SFFS
	};

	enum EVAL {
		CLASSWISE = 0,
		ACCURACY
	};

public:

	class Options : public OptionList {

	public:

		Options () 
			: kfold (2), loo (false), louo (false), split (0), nfirst (0), method (SFS), l (1), r (0), eval (CLASSWISE), nthread (0) {

			addOption ("kfold", &kfold, 1, SSI_SIZE, "#folds used during evaluation");
			addOption ("loo", &loo, 1, SSI_BOOL, "use leave-one-out instead of kfold");
			addOption ("louo", &louo, 1, SSI_BOOL, "use leave-one-user-out instead of kfold");
			addOption ("split", &split, 1, SSI_REAL, "use the first split% for training and the rest for testing instead of kfold (split = ]0..1[, selected if > 0)");
			addOption ("nfirst", &nfirst, 1, SSI_SIZE, "terminate after n first (0 for all)");
			addOption ("method", &method, 1, SSI_INT, "method (0=SFS, 1=SBS, 2=PLUS-L-MINUS-R, 3=SFFS)");
			addOption ("l", &l, 1, SSI_SIZE, "plus l");
			addOption ("r", &r, 1, SSI_SIZE, "minus r");
			addOption ("eval", &eval, 1, SSI_INT, "evaluation method (0=CLASSWISE, 1=ACCURACY)");
			addOption ("nthread", &nthread, 1, SSI_INT, "distribute work on n threads");
		};

		ssi_size_t kfold;
		bool loo;
		bool louo;
		ssi_size_t nfirst;
		METHOD method;
		ssi_size_t l, r;
		EVAL eval;
		ssi_size_t nthread;
		ssi_real_t split;
	};

public:

	static const ssi_char_t *GetCreateName () { return "FloatingSearch"; };
	static IObject *Create (const ssi_char_t *file) { return new FloatingSearch (file); };
	virtual ~FloatingSearch ();
	
	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Floating Feature Selection (SFS, SBS, Plus-L-Minus-R, SFFS)"; };

	void setModel (IModel &model) { _model = &model; };
	bool train (ISamples &samples,
		ssi_size_t stream_index);
	bool isTrained () { return _model != 0; };
	bool isWrapper () { return true; };
	bool sortByScore () { return false; };
	void release ();	

	ssi_size_t getSize () { return _n_scores; };
	const score *getScores () { return _scores; };	

	virtual void print (FILE *file = stdout);

	virtual void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	bool lr_search (ssi_size_t n_keep, ssi_size_t l, ssi_size_t r);
	void inclusion (ssi_size_t n_keep, ssi_size_t k, ssi_size_t l, ssi_size_t r);	
	void exclusion (ssi_size_t n_keep, ssi_size_t k, ssi_size_t l, ssi_size_t r);
	bool sffs (ssi_size_t n_keep);
	void sffs_exclusion (ssi_size_t n_keep, ssi_size_t k, ssi_real_t prob);
	void sffs_inclusion (ssi_size_t n_keep, ssi_size_t k);
	ssi_real_t eval_h (Evaluation *eval, Trainer *trainer, ISamples &samples);

	void tp_inclusion (ssi_size_t n_keep, ssi_size_t k, ssi_size_t l, ssi_size_t r);
	struct tp_eval_h_in_s {
		ssi_size_t dim;
		Options *options;
		Trainer *trainer;
		ISamples *samples;
	};
	struct tp_eval_h_out_s {
		ssi_real_t result;
	};
	static bool tp_eval_h (ssi_size_t n_in, void *in, ssi_size_t n_out, void *out);

	FloatingSearch (const ssi_char_t *file);
	Options _options;
	ssi_char_t *_file;

	int ssi_log_level;
	ssi_char_t *ssi_log_name;

	IModel *_model;
	ssi_size_t _n_scores;
	score *_scores;
	ISamples *_samples;
	ssi_size_t _stream_index;
	ssi_size_t _n_dims;

};

}

#endif
