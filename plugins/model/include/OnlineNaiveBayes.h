// OnlineNaiveBayes.h
// author: Raphael Schwarz <raphaelschwarz@student.uni-augsbrug.de>
// created: 2017/04/12
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
//
// based on an implementation by Thurid Vogt <thurid.vogt@informatik.uni-augsburg.de>
// Copyright (C) 2003-9 University of Augsburg, Thurid Vogt

#pragma once

#ifndef SSI_MODEL_ONLINENAIVEBAYES_H
#define SSI_MODEL_ONLINENAIVEBAYES_H

#include "base/IModel.h"
#include "ioput/option/OptionList.h"

//#include "OnlineClassifier.h"

#define SSI_ONLINENAIVEBAYES_LOG(x) (((x) > (1e-20)) ? log(x) : (-46))

namespace ssi {

class OnlineNaiveBayes : public IModel {

    friend class OnlineClassifier;
    //friend class ContextDisturbanceClassifier;

public:

	class Options : public OptionList {

	public:

		Options () 
            : log (true), prior(true) {

			addOption ("log", &log, 1, SSI_BOOL, "use log normal distribution");
			addOption ("prior", &prior, 1, SSI_BOOL, "use prior probability"); 
                }

		bool log;
		bool prior;
	};

public:

        static const ssi_char_t *GetCreateName () { return "OnlineNaiveBayes"; }
        static IObject *Create (const ssi_char_t *file) { return new OnlineNaiveBayes (file); }
	virtual ~OnlineNaiveBayes ();

        Options *getOptions () { return &_options; }
        const ssi_char_t *getName () { return GetCreateName (); }
        const ssi_char_t *getInfo () { return "Online Naive Bayes classifier."; }

	bool train (ISamples &samples,
		ssi_size_t stream_index);	
        bool isTrained () { return _class_probs != 0; }
	bool forward (ssi_stream_t &stream,
		ssi_size_t n_probs,
		ssi_real_t *probs,
		ssi_real_t &confidence);
	void release ();
	bool save (const ssi_char_t *filepath);
	bool load (const ssi_char_t *filepath);

	IModel::TYPE::List getModelType() { return IModel::TYPE::CLASSIFICATION; }
    ssi_size_t getClassSize () { return _n_classes; }
    ssi_size_t getStreamDim () { return _n_features; }
    ssi_size_t getStreamByte () { return sizeof (ssi_real_t); }
    ssi_type_t getStreamType () { return SSI_REAL; }

protected:	

        OnlineNaiveBayes (const ssi_char_t *file=0);
	Options _options;
    ssi_char_t *_file;

    ssi_size_t _n_classes;
    ssi_size_t _n_features;
        ssi_size_t *_class_weight_sum;
    ssi_char_t **_class_names;
        ssi_real_t *_class_probs;
        ssi_real_t **_means;
        ssi_real_t **_variance;
        ssi_real_t **_std_dev;

        Mutex _mutex;

        void init_class_names (ISamples &samples);
	void free_class_names ();
	bool readLine (FILE *fp, ssi_size_t num, ssi_char_t *string);
		
};

}

#endif
