// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 27/9/2016
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "ssi.h"
#include "ssiml.h"
#include "ssiliblinear.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

bool ex_binary(void *arg);
bool ex_multiclass(void *arg);
bool ex_regression(void *arg);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("liblinear");

#if SSI_RANDOM_LEGACY_FLAG
	ssi_random_seed();
#endif
	
	Exsemble ex;
	ex.console(0, 0, 650, 800);
	ex.add(ex_binary, 0, "BINARY", "Binary classification task");
	ex.add(ex_multiclass, 0, "MULTICLASS", "Multi-classification task");
	ex.add(ex_regression, 0, "REGRESSION", "Regression task");
	ex.show();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool ex_binary(void *arg) {

	Trainer::SetLogLevel(SSI_LOG_LEVEL_DEBUG);

	ssi_size_t n_classes = 2;
	ssi_size_t n_samples = 1000;
	ssi_size_t n_streams = 1;
	ssi_real_t train_distr[][3] = { 0.25f, 0.25f, 0.1f, 0.75f, 0.75f, 0.1f };
	ssi_real_t test_distr[][3] = { 0.5f, 0.5f, 0.5f };
	SampleList strain;
	SampleList sdevel;
	SampleList stest;
	ModelTools::CreateTestSamples(strain, n_classes, n_samples, n_streams, train_distr, "user");
	ModelTools::CreateTestSamples(sdevel, n_classes, n_samples, n_streams, train_distr, "user");
	ModelTools::CreateTestSamples(stest, 1, n_samples * n_classes, n_streams, test_distr, "user");
	ssi_char_t string[SSI_MAX_CHAR];
	for (ssi_size_t n_class = 1; n_class < n_classes; n_class++) {
		ssi_sprint(string, "CL%u", n_class);
		stest.addClassName(string);
	}

	// train
	{
		LibLinear *model = ssi_create(LibLinear, 0, true);
		model->getOptions()->seed = 1234;
		model->getOptions()->silent = false;
		model->getOptions()->setParams("-s 0 -C -e 0.1 -B 0.1");

		Trainer trainer(model);
		ISNorm::Params params;
		ISNorm::ZeroParams(params, ISNorm::METHOD::SCALE);
		params.limits[0] = -1.0f;
		params.limits[1] = 1.0f;
		trainer.setNormalization(&params);
		ModelTools::PlotSamples(strain, "TRAINING DATA", ssi_rect(640, 0, 400, 400));
		ssi_tic();
		trainer.train(strain);
		ssi_print("\nELAPSED: ")
			ssi_toc_print();
		ssi_print("\n\n")
			trainer.save("multiclass");
		ISNorm::ReleaseParams(params);
	}

	// eval
	{
		Trainer trainer;
		Trainer::Load(trainer, "multiclass");
		trainer.eval(sdevel);
		ModelTools::PlotSamples(stest, "TEST DATA", ssi_rect(640, 0, 400, 400));
		trainer.cluster(stest);
		ModelTools::PlotSamples(stest, "LABELED DATA", ssi_rect(640, 0, 400, 400));
	}

	return true;
}

bool ex_multiclass(void *arg) {

	Trainer::SetLogLevel(SSI_LOG_LEVEL_DEBUG);

	ssi_size_t n_classes = 4;
	ssi_size_t n_samples = 1000;
	ssi_size_t n_streams = 1;
	ssi_real_t train_distr[][3] = { 0.25f, 0.25f, 0.1f, 0.25f, 0.75f, 0.1f, 0.75f, 0.75f, 0.1f, 0.75f, 0.25f, 0.1f };
	ssi_real_t test_distr[][3] = { 0.5f, 0.5f, 0.5f };
	SampleList strain;
	SampleList sdevel;
	SampleList stest;
	ModelTools::CreateTestSamples(strain, n_classes, n_samples, n_streams, train_distr, "user");
	ModelTools::CreateTestSamples(sdevel, n_classes, n_samples, n_streams, train_distr, "user");
	ModelTools::CreateTestSamples(stest, 1, n_samples * n_classes, n_streams, test_distr, "user");
	ssi_char_t string[SSI_MAX_CHAR];
	for (ssi_size_t n_class = 1; n_class < n_classes; n_class++) {
		ssi_sprint(string, "CL%u", n_class);
		stest.addClassName(string);
	}

	// train
	{
		LibLinear *model = ssi_create(LibLinear, 0, true);
		model->getOptions()->seed = 1234;
		model->getOptions()->silent = false;
		model->getOptions()->setParams("-s 0 -C -e 0.1 -B 0.1");

		Trainer trainer(model);
		ISNorm::Params params;
		ISNorm::ZeroParams(params, ISNorm::METHOD::SCALE);
		params.limits[0] = -1.0f;
		params.limits[1] = 1.0f;
		trainer.setNormalization(&params);
		ModelTools::PlotSamples(strain, "TRAINING DATA", ssi_rect(640, 0, 400, 400));
		ssi_tic();
		trainer.train(strain);
		ssi_print("\nELAPSED: ")
		ssi_toc_print();
		ssi_print("\n\n")
		trainer.save("multiclass");
		ISNorm::ReleaseParams(params);
	}

	// eval
	{
		Trainer trainer;
		Trainer::Load(trainer, "multiclass");
		trainer.eval(sdevel);
		ModelTools::PlotSamples(stest, "TEST DATA", ssi_rect(640, 0, 400, 400));
		trainer.cluster(stest);
		ModelTools::PlotSamples(stest, "LABELED DATA", ssi_rect(640, 0, 400, 400));
	}

	return true;
}

bool ex_regression(void *arg) {

	Trainer::SetLogLevel(SSI_LOG_LEVEL_DEBUG);	

	ssi_size_t n_samples = 1000;

	SampleList strain;
	SampleList sdevel;
	SampleList stest;
	ModelTools::CreateTestSamplesRegression(strain, n_samples, 0.1f);
	ModelTools::CreateTestSamplesRegression(sdevel, n_samples, 0.1f);
	ModelTools::CreateTestSamplesRegression(stest, n_samples, 1.0f);

	// train
	{
		LibLinear *model = ssi_create(LibLinear, 0, true);
		model->getOptions()->seed = 1234;
		model->getOptions()->silent = false;
		model->getOptions()->setParams("-s 12 -c 0.1 -B 0.1 -e 0.1");

		Trainer trainer(model);
		ISNorm::Params params;
		ISNorm::ZeroParams(params, ISNorm::METHOD::SCALE);
		params.limits[0] = 0.0f;
		params.limits[1] = 1.0f;
		trainer.setNormalization(&params);
		ModelTools::PlotSamplesRegression(strain, "TRAINING", ssi_rect(640, 0, 400, 400));
		ssi_tic();
		trainer.train(strain);
		ssi_print("\nELAPSED: ")
		ssi_toc_print();
		ssi_print("\n\n")
		trainer.save("regression");
		ISNorm::ReleaseParams(params);
	}

	// eval
	{		
		Trainer trainer;
		Trainer::Load(trainer, "regression");
		trainer.eval(sdevel);
		trainer.cluster(stest);
		ModelTools::PlotSamplesRegression(stest, "TEST", ssi_rect(640, 0, 400, 400));
	}	

	return true;
}