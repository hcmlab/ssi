// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/10/11
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

#include "ssi.h"
#include "ssitorch.h"
#include "ssiml\include\ssiml.h"
using namespace ssi;

void ex_hmm_multi (); 
void ex_hmm_single (); 

ssi_size_t n_classes = 4;
ssi_size_t n_samples = 50;
ssi_size_t n_streams = 1;
ssi_size_t num_min = 25;
ssi_size_t num_max = 50;
ssi_real_t distr[][3] = { 0.3f, 0.3f, 0.8f, 0.3f, 0.6f, 0.2f, 0.8f, 0.3f, 0.8f, 0.6f, 0.6f, 0.8f };
SampleList *train_samples = 0;
SampleList *test_samples = 0;

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssitorch.dll");

	ssi_random_seed ();

	SampleList training;
	ModelTools::CreateDynamicTestSamples (training, n_classes, n_samples, n_streams, distr, num_min, num_max);			
	SampleList testing;
	ModelTools::CreateDynamicTestSamples (testing, n_classes, n_samples, n_streams, distr, num_min, num_max);	
	train_samples = &training;
	test_samples = &testing;

	Console *console = ssi_create(Console, 0, true);
	console->setPosition(ssi_rect(0, 0, 650, 800));

	ex_hmm_multi ();	
	ex_hmm_single ();	
	
	ssi_print ("\n\n\tpress a key to quit\n");
	getchar ();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

void ex_hmm_multi () {
			
	// training
	{
		TorchHMM *model = ssi_create (TorchHMM, 0, true);
		model->getOptions()->single = false;
		model->getOptions()->n_gaussians = 5;	
		model->getOptions()->connectivity = TorchHMM::ERGODIC;
		Trainer trainer (model);
		trainer.train (*train_samples);
		trainer.save ("hmm_multi");
	}

	// evaluation
	{
		Trainer trainer;
		Trainer::Load (trainer, "hmm_multi");					
		trainer.eval(*test_samples);		
	}
}

void ex_hmm_single () {
			
	// training
	{
		TorchHMM *model = ssi_create (TorchHMM, 0, true);
		model->getOptions()->single = true;		
		model->getOptions()->n_gaussians = 10;	
		Trainer trainer (model);
		trainer.train (*train_samples);
		trainer.save ("hmm_single");
	}

	// evaluation
	{
		Trainer trainer;
		Trainer::Load (trainer, "hmm_single");			
		Evaluation eval;
		trainer.eval(*test_samples);

		ssi_size_t n = 0;
		const ssi_size_t *seq = ssi_pcast (TorchHMM, trainer.getModel (0))->getStateSequence(n);
		for (ssi_size_t i = 0; i < n; i++) {
			ssi_print ("%u ", seq[i]);
		}
	}
}
