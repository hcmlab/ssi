// FloatingCFS.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/02/23
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

#ifndef SSI_MODEL_FLOATINGCFS_H
#define SSI_MODEL_FLOATINGCFS_H

#include "base/ISelection.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class FloatingCFS : public ISelection {

public:

	enum METHOD {
		SFS = 0,
		SBS,
		LR,
		SFFS
	};

public:

	class Options : public OptionList {

	public:

		Options () 
			: nfirst (0), method (SFS), l (1), r (0) {

			addOption ("nfirst", &nfirst, 1, SSI_SIZE, "terminate after n first (0 for all)");
			addOption ("method", &method, 1, SSI_INT, "method (0=SFS, 1=SBS, 2=PLUS-L-MINUS-R, 3=SFFS)");
			addOption ("l", &l, 1, SSI_SIZE, "plus l");
			addOption ("r", &r, 1, SSI_SIZE, "minus r");
		};

		ssi_size_t nfirst;
		METHOD method;
		ssi_size_t l, r;
	};

public:

	static const ssi_char_t *GetCreateName () { return "FloatingCFS"; };
	static IObject *Create (const ssi_char_t *file) { return new FloatingCFS (file); };
	virtual ~FloatingCFS ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Floating Correlation based Feature Selection (SFS, SBS, Plus-L-Minus-R, SFFS)"; };

	void setModel (IModel &model) { _model = &model; };
	bool train (ISamples &samples, ssi_size_t stream_index);
	bool isTrained () { return _model != 0; };
	bool isWrapper () { return false; };
	bool sortByScore () { return false; };
	void release ();

	ssi_size_t getSize () { return _n_scores; };
	const score *getScores () { return _scores; };	

	virtual void print (FILE *file = stdout);

	virtual void setLogLevel (int level) {
		ssi_log_level = level;
	}

	/**
	 * @brief Prints the calculated correlation matrix.
	 */
	void printCorrelationMatrix (FILE* file = stdout);

	/**
	 * @brief Prints the calculated standard deviations.
	 */
	void printStdDevs (FILE* file = stdout);

protected:

	bool lr_search (ssi_size_t n_keep, ssi_size_t l, ssi_size_t r);
	void inclusion (ssi_size_t n_keep, ssi_size_t k, ssi_size_t l, ssi_size_t r);
	void exclusion (ssi_size_t n_keep, ssi_size_t k, ssi_size_t l, ssi_size_t r);
	bool sffs (ssi_size_t n_keep);
	void sffs_exclusion (ssi_size_t n_keep, ssi_size_t k, ssi_real_t prob);
	void sffs_inclusion (ssi_size_t n_keep, ssi_size_t k);

	FloatingCFS (const ssi_char_t *file);
	Options _options;
	ssi_char_t *_file;

	int ssi_log_level;
	ssi_char_t *ssi_log_name;

	IModel *_model;
	ssi_size_t _n_scores;
	score *_scores;
	ISamples *_samples;
	ssi_size_t _stream_index;

	/**
	* @brief Evaluates a feature subset.
	*
	* @param n_dims The number of selected features, ie. the number of items in the array dims.
	* @param dims The selected feature indices.
	* @return The merit.
	*/
	ssi_real_t eval_h (ssi_size_t n_dims, const ssi_size_t* dims);

	/**
	 * @brief Initializes all values required by CFS (_corr_matrix, _std_devs, _classIndex, _numAttribs, _numInstances).
	 */
	void buildEvaluator();

	/**
	 * @brief Calculates the correlation of two features specified by att1 and att2.
	 * 
	 * The actual correlation is performed by num_num() and num_nom2().
	 * If one of the specified feature indices is the class index (att[12] == _classIndex) num_nom2() is used, otherwise num_num().
	 *
	 * @param att1 The index of the column in _train_instances for the first feature.
	 * @param att2 The index of the column in _train_instances for the second feature.
	 * @return The correlation factor of the two features.
	 */

	ssi_real_t correlate (ssi_size_t att1, ssi_size_t att2);
	/**
	 * @brief Calculates the correlation of two numerical features.
	 *
	 * @param att1 The index of the column in _train_instances for the first feature.
	 * @param att2 The index of the column in _train_instances for the second feature.
	 * @return The correlation factor of the two features.
	 */
	ssi_real_t num_num (ssi_size_t att1, ssi_size_t att2);

	/**
	 * @brief Calculates the correlation of one feature with the class.
	 *
	 * @param att1 The index of the column in _train_instances for the class (nominal feature).
	 * @param att2 The index of the column in _train_instances for the feature (numeric feature).
	 * @return The correlation factor of the feature with the class.
	 */
	ssi_real_t num_nom2 (ssi_size_t att1, ssi_size_t att2);

	/**
	 * @brief returns the mean from an array
	 * @param fray	array
	 * @param length length of array
	 */
	ssi_real_t mean_float(ssi_real_t* fray, ssi_size_t length);

	/**
	 * @brief returns the mean of the _train_instances at column att (or the mode if att=_classIndex)
	 * @param att the index of the column in _train_instances
	 */
	ssi_real_t meanOrMode(ssi_size_t att);

	/**
	 * The number of features per sample in the sample list.
	 */
	ssi_size_t _n_dims;

	/**
	 * The number of classes in the sample list.
	 */
	ssi_size_t _n_classes;

	/**
	 * Matrix of Samples/Features storing the raw feature values.
	 * First Dimension: Samples
	 * Second Dimension: Features + ClassId of the sample (index of ClassId: _classIndex)
	 */
	ssi_real_t **_train_instances;
	/**
	 * The index where the Class Id is stored in the _train_instances[x] array.
	 */
	ssi_size_t _classIndex;
	/**
	 * The number of Features per Sample plus one (the ClassIndex).
	 */
	ssi_size_t _numAttribs;
	/**
	 * The number of Samples in the sample list.
	 */
	ssi_size_t _numInstances;
	/**
	 * The correlation matrix, built like described in "thesis.dvi Table 4.2" with minor modifications.
	 *
	 *  ____f1_f2_f3
	 * |f1||1 |  |  | (Array length: 1)
	 * |f2||xx|1 |  | (Array length: 2)
	 * |f3||xx|xx|1 | (Array length: 3)
	 *
	 * One of the features is the class index. Must be handled specially. The index is stored in _classIndex.
	 * To save memory the matrix is only constructed with the lower left part.
	 * 
	 */
	ssi_real_t **_corr_matrix;

	/**
	 * The standard deviations of attributes (when using pearsons correlation).
	 */
	ssi_real_t *_std_devs;
};

}

#endif
