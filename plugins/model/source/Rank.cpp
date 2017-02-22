// Rank.cpp
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

#include "Rank.h"
#include "Trainer.h"
#include "Evaluation.h"
#include "ISSelectDim.h"

namespace ssi {

int Rank::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
ssi_char_t *Rank::ssi_log_name = "rank______";

Rank::Rank (const ssi_char_t *file) :
	_model (0),
	_n_scores (0),
	_scores (0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}
	
Rank::~Rank () {

	release ();
	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

bool Rank::train (ISamples &samples,
	ssi_size_t stream_index) {

	if (!_model) {
		ssi_wrn ("a model has not been set yet");
		return false;
	}

	release ();

	_n_scores = samples.getStream (stream_index).dim;	
	_scores = new score[_n_scores];

	Evaluation eval;
	Trainer trainer (_model, stream_index);
	
	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "evaluate dimensions:");
	for (ssi_size_t ndim = 0; ndim < _n_scores; ndim++) {
		ISSelectDim samples_s (&samples);
		samples_s.setSelection (stream_index, 1, &ndim);
		if (_options.loo) {
			eval.evalLOO (&trainer, samples_s);
		} else if (_options.louo) {
			eval.evalLOUO (&trainer, samples_s);
		} else {
			eval.evalKFold (&trainer, samples_s, _options.kfold);
		}
		_scores[ndim].index = ndim;
		_scores[ndim].value = eval.get_classwise_prob ();
		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "  #%02u -> %.2f", _scores[ndim].index, _scores[ndim].value);
	} 

	trainer.release ();

	return true;
}

void Rank::print (FILE *file) {
		
	for (ssi_size_t i = 0; i < _n_scores; i++) {
		ssi_fprint (file, "%u: %.2f\n", _scores[i].index, _scores[i].value);
	}
}

void Rank::release () {

	delete[] _scores; _scores = 0;
	_n_scores = 0;
	//_model = 0;
}

}
