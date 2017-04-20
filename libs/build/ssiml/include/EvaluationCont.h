// EvaluationCont.h
// author: Ionut Damian <damian@informatik.uni-augsburg.de>
// created: 2012/10/02
// Copyright (C) 2007-12 University of Augsburg, Ionut Damian
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

/* 
 * Lets you evaluate a continuous model by using raw data streams as input instead of presegmented sample lists
 */

#pragma once

#ifndef SSI_MODEL_EVALUATION_CONT_H
#define SSI_MODEL_EVALUATION_CONT_H

#include "Evaluation.h"
#include "Annotation.h"

namespace ssi {

class EvaluationCont : public Evaluation {

public:

	struct Counter
	{	
		Counter();

		ssi_time_t sum;
		ssi_time_t max;
		ssi_time_t min;
	};

	EvaluationCont ();
	~EvaluationCont ();

	void init_conf_mat (ISamples &samples);

	// evaluate a trainer with a whole stream frame by frame
	void eval (ssi_stream_t *stream, old::Annotation* anno, ssi_real_t fps);

	// eval using leave one user out continuously with the whole stream frame by frame (model is re-trained.. you may lose your old model!)
	void evalLOUO (Trainer &trainer, ISamples &samples, std::vector<ssi_stream_t*>* streams, std::vector<old::Annotation*>* annos, ssi_real_t fps, ssi_size_t reps);
	// eval continuously with the whole stream frame by frame (model is re-trained.. you may lose your old model!)
	void evalFull (Trainer &trainer, ISamples &samples, std::vector<ssi_stream_t*>* streams, std::vector<old::Annotation*>* annos, ssi_real_t fps, ssi_size_t reps);

	void print (FILE *file = stdout);

protected:

	ssi_size_t _n_correct; //number of correct calssifications	
	ssi_size_t *_nonevent_reco; //recognitions of unknown events, such as those which were not annotated

	Counter _eager_abs;
	Counter _eager_rel;

	std::vector<old::Annotation*>* _annos;
	ssi_size_t _repetitions;

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;

	//options
	ssi_time_t _anno_max_delay;
	ssi_size_t _chunk_size;
};

}

#endif
