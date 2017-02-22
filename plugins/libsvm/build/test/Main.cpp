// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 22/5/2015
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
#include "ssilibsvm.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

bool ex_libsvm(void *arg);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssigraphic");
	Factory::RegisterDLL ("ssilibsvm");
	
	ssi_random_seed();

	Exsemble exsemble;
	exsemble.console(0, 0, 650, 800);
	exsemble.add(&ex_libsvm, 0, "LIBSVM", "Test libsvm model.");
	exsemble.show();

	Factory::Clear();
	
	ssi_print ("\n\n\tpress a key to quit\n");
	getchar ();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool ex_libsvm(void *arg) {

	Trainer::SetLogLevel(SSI_LOG_LEVEL_DEBUG);

	ssi_size_t n_classes = 4;
	ssi_size_t n_samples = 50;
	ssi_size_t n_streams = 1;
	ssi_real_t train_distr[][3] = { 0.25f, 0.25f, 0.1f, 0.25f, 0.75f, 0.1f, 0.75f, 0.75f, 0.1f, 0.75f, 0.75f, 0.1f };
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
		LibSVM *model = ssi_create(LibSVM, 0, true);
		model->getOptions()->seed = 1234;
		model->getOptions()->silent = false;
		model->getOptions()->params.kernel_type = LibSVM::KERNEL::RADIAL;
		
		Trainer trainer(model);
		ISNorm::Params params;
		ISNorm::ZeroParams(params, ISNorm::METHOD::SCALE);
		params.limits[0] = -1.0f;
		trainer.setNormalization(&params);
		trainer.train(strain);		
		trainer.save("libsvm");
		ISNorm::ReleaseParams(params);
	}

	// eval
	{
		Trainer trainer;
		Trainer::Load(trainer, "libsvm");
		trainer.eval(sdevel);		
		trainer.cluster(stest);
		ModelTools::PlotSamples(stest, "libsvm", ssi_rect(640,0,400,400));
	}

	return true;
}
