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

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

#define N_COLORS 5
unsigned short COLORS[][3] = {
	128,0,0,
	0,128,0,
	0,0,128,
	128,0,128,
	0,128,128
};

void ex_knn (); 
void ex_gmm (); 
void ex_svm ();
void ex_kmeans ();
void ex_hmm ();

ssi_size_t n_classes = 4;
ssi_size_t n_samples = 50;
ssi_size_t n_streams = 1;
ssi_real_t distr[][3] = { 0.25f, 0.25f, 0.1f, 0.25f, 0.75f, 0.1f, 0.75f, 0.25f, 0.1f, 0.75f, 0.75f, 0.1f };
SampleList *train_samples = 0;
SampleList *test_samples = 0;
SampleList *train_dynamic_samples = 0;
SampleList *test_dynamic_samples = 0;

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssitorch.dll");
	Factory::RegisterDLL ("ssigraphic.dll");

	ssi_random_seed ();

	Console *console = ssi_create(Console, 0, true);
	console->setPosition(ssi_rect(0, 0, 650, 800));

	SampleList training;
	ModelTools::CreateTestSamples (training, n_classes, n_samples, n_streams, distr);			
	SampleList testing;
	ModelTools::CreateTestSamples (testing, n_classes, n_samples, n_streams, distr);	
	train_samples = &training;
	test_samples = &testing;

	SampleList training_dynamic;
	ModelTools::CreateDynamicTestSamples (training_dynamic, n_classes, n_samples, n_streams, distr, 5, 20, "user");
	SampleList testing_dynamic;
	ModelTools::CreateDynamicTestSamples (testing_dynamic, n_classes, n_samples, n_streams, distr, 5, 20, "user");	
	train_dynamic_samples = &training_dynamic;
	test_dynamic_samples = &testing_dynamic;

	ex_hmm ();

	ex_knn ();	
	ex_gmm ();
	ex_svm ();
	ex_kmeans ();

	ssi_print ("\n\n\tpress a key to quit\n");
	getchar ();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

void ex_knn() {
			
	// training
	{
		TorchKNN *model = ssi_create (TorchKNN, 0, true);
		model->getOptions()->k = 5;
		Trainer trainer (model);
		trainer.train (*train_samples);
		trainer.save ("knn");
	}

	// evaluation
	{
		Trainer trainer;
		Trainer::Load (trainer, "knn");			
		Evaluation eval;
		trainer.eval(*test_samples);

		ModelTools::PlotSamples (*test_samples, "knn", ssi_rect(650,0,400,400));
	}
}

void ex_gmm () {
			
	// training
	{
		TorchGMM *model = ssi_create (TorchGMM, 0, true);
		model->getOptions()->n_gaussians = 5;
		Trainer trainer (model);
		trainer.train (*train_samples);
		trainer.save ("gmm");
	}

	// evaluation
	{
		Trainer trainer;
		Trainer::Load (trainer, "gmm");			
		Evaluation eval;
		trainer.eval (*test_samples);		

		ModelTools::PlotSamples(*test_samples, "gmm", ssi_rect(650, 0, 400, 400));
	}
}

void ex_svm () {

	// training
	{
		TorchSVM *model = ssi_create (TorchSVM, 0, true);
		Trainer trainer (model);
		trainer.train (*train_samples);
		trainer.save ("svm");
	}

	// evaluation
	{
		Trainer trainer;
		Trainer::Load (trainer, "svm");			
		Evaluation eval;
		trainer.eval (*test_samples);

		ModelTools::PlotSamples(*test_samples, "svm", ssi_rect(650, 0, 400, 400));
	}
}

void ex_hmm() {
			
	// training
	{
		TorchHMM *model = ssi_create (TorchHMM, 0, true);		
		Trainer trainer (model);
		trainer.train (*train_dynamic_samples);
		trainer.save ("hmm");
	}

	// evaluation
	{
		Trainer trainer;
		Trainer::Load (trainer, "hmm");			
		Evaluation eval;
		trainer.eval (*test_dynamic_samples);		

		ModelTools::PlotSamples(*test_dynamic_samples, "hmm", ssi_rect(650, 0, 400, 400));
	}
}

void ex_kmeans () {

	SampleList samples;
	ssi_size_t n_classes = 4;
	ssi_size_t n_samples = 200;
	ssi_size_t n_streams = 1;
	ssi_real_t distr[][3] = { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };
	ModelTools::CreateTestSamples (samples, n_classes, n_samples, n_streams, distr);

	// training
	{
		TorchKMeans *model = ssi_create (TorchKMeans, 0, true);
		model->getOptions()->n_cluster = n_classes;
		Trainer trainer (model);
		trainer.train (samples);
		trainer.save ("kmeans");
	}

	// evaluation
	{
		Trainer trainer;
		Trainer::Load (trainer, "kmeans");			
		trainer.cluster (samples);

		ModelTools::PlotSamples(samples, "kmeans", ssi_rect(650, 0, 400, 400));
	} 
}

