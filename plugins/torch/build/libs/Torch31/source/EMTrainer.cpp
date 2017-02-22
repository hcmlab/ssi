// Copyright (C) 2003--2004 Samy Bengio (bengio@idiap.ch)
//                
// This file is part of Torch 3.1.
//
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "EMTrainer.h"
#include "log_add.h"

namespace Torch {

	EMTrainer::EMTrainer(Distribution *distribution_) : Trainer(distribution_)
	{
		distribution = distribution_;

		addROption("end accuracy", &end_accuracy, 0.0001,"end accuracy");
		addIOption("max iter", &max_iter, 100, "maximum number of iterations");
		addBOption("viterbi", &viterbi, false, "Viterbi training");
	}

	void EMTrainer::train(DataSet* data, MeasurerList *measurers)
	{
		int iter = 0;
		int n_train = data->n_examples;

		real prev_nll = INF;
		real nll = INF;

		DataSet **datas;
		Measurer ***mes;
		int *n_mes;
		int n_datas;

		machine->setDataSet(data);

		message("EMTrainer: training");

		if(measurers)
		{
			for(int i = 0; i < measurers->n_nodes; i++)
				measurers->nodes[i]->reset();
		}

		Allocator *allocator_ = extractMeasurers(measurers, data, &datas, &mes, &n_mes, &n_datas);

		while (1) {
			distribution->eMIterInitialize();
			nll = 0;
			int tot_n_frames = 0;
			for (int t=0;t<n_train;t++) {
				data->setExample(t);

				if (viterbi) {
					distribution->viterbiForward(data->inputs);
					nll -= distribution->log_probability;
					distribution->viterbiAccPosteriors(data->inputs,LOG_ONE);
				} else {
					distribution->eMForward(data->inputs);
					nll -= distribution->log_probability;
					distribution->eMAccPosteriors(data->inputs,LOG_ONE);
				}
				tot_n_frames += data->inputs->n_frames;

				for(int i = 0; i < n_mes[0]; i++)
					mes[0][i]->measureExample();
			}
			nll /= tot_n_frames;

			distribution->eMUpdate();

			for(int i = 0; i < n_mes[0]; i++)
				mes[0][i]->measureIteration();

			// for each supplementary dataset given, simply compute
			// test llr (not used for training)
			for(int julie = 1; julie < n_datas; julie++) {
				DataSet *dataset = datas[julie];

				for(int t=0;t<dataset->n_examples;t++) {
					dataset->setExample(t);
					if (viterbi) {
						distribution->viterbiForward(dataset->inputs);
					} else {
						distribution->eMForward(dataset->inputs);
					}

					for(int i = 0; i < n_mes[julie]; i++)
						mes[julie][i]->measureExample();
				}
				for(int i = 0; i < n_mes[julie]; i++)
					mes[julie][i]->measureIteration();
			}

			// stopping criterion
			if ((prev_nll == nll) || fabs((prev_nll - nll)/prev_nll) < end_accuracy) {
				print("\n");
				break;
			}
			prev_nll = nll;
			print(".");
			iter++;
			if ((iter >= max_iter) && (max_iter > 0)) {
				print("\n");
				warning("EMTrainer: you have reached the maximum number of iterations");
				break;
			}
		}
		for(int i=0;i<n_datas;i++) {
			for(int j=0;j<n_mes[i];j++)
				mes[i][j]->measureEnd();
		}
		delete allocator_;
	}

	void EMTrainer::test(MeasurerList *measurers)
	{
		DataSet **datas;
		Measurer ***mes;
		int *n_mes;
		int n_datas;

		print("# EMTrainer: testing [");

		//message("emtrainer: testing");

		Allocator *allocator_ = extractMeasurers(measurers, NULL, &datas, &mes, &n_mes, &n_datas);

		////
		int n_ex = 0;
		for(int andrea = 0; andrea < n_datas; andrea++)
			n_ex += datas[andrea]->n_examples;
		real n_ex_mod = ((n_ex == 0)? 0. : 10.1/((real)n_ex));
		real ex_curr = 0;
		real n_dots = 0;
		////

		for(int andrea = 0; andrea < n_datas; andrea++)
		{
			DataSet *dataset = datas[andrea];

			for(int i = 0; i < n_mes[andrea]; i++)
				mes[andrea][i]->reset();

			distribution->eMIterInitialize();
			for(int t = 0; t < dataset->n_examples; t++)
			{
				dataset->setExample(t);
				if (viterbi) {
					distribution->viterbiForward(dataset->inputs);
				} else {
					distribution->eMForward(dataset->inputs);
				}

				for(int i = 0; i < n_mes[andrea]; i++)
					mes[andrea][i]->measureExample();
				if(++ex_curr * n_ex_mod >= (n_dots+1))
				{
					if(n_ex < 10)
						print("_");
					else
						print(".");
					n_dots++;
				}
			}

			for(int i = 0; i < n_mes[andrea]; i++)
				mes[andrea][i]->measureIteration();

			for(int i = 0; i < n_mes[andrea]; i++)
				mes[andrea][i]->measureEnd();
		}

		print("]\n");
		delete allocator_;
	}

	void EMTrainer::decode(MeasurerList *measurers)
	{
		DataSet **datas;
		Measurer ***mes;
		int *n_mes;
		int n_datas;

		message("emtrainer: decoding");

		Allocator *allocator_ = extractMeasurers(measurers, NULL, &datas, &mes, &n_mes, &n_datas);

		for(int andrea = 0; andrea < n_datas; andrea++)
		{
			DataSet *dataset = datas[andrea];
			distribution->setDataSet(dataset);

			for(int i = 0; i < n_mes[andrea]; i++)
				mes[andrea][i]->reset();

			distribution->eMIterInitialize();
			for(int t = 0; t < dataset->n_examples; t++)
			{
				dataset->setExample(t);
				distribution->decode(dataset->inputs);

				for(int i = 0; i < n_mes[andrea]; i++)
					mes[andrea][i]->measureExample();
			}

			for(int i = 0; i < n_mes[andrea]; i++)
				mes[andrea][i]->measureIteration();

			for(int i = 0; i < n_mes[andrea]; i++)
				mes[andrea][i]->measureEnd();
		}

		delete allocator_;
	}

	EMTrainer::~EMTrainer()
	{
	}

}

