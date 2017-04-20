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
#include "ssiml/include/ssiml.h"
#include "ssimodel.h"
using namespace ssi;

#define N_COLORS 5
unsigned short COLORS[][3] = {
	128,0,0,
	0,128,0,
	0,0,128,
	128,0,128,
	0,128,128
};

void ex_kmeans ();

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

int main () {	

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssimodel.dll");
	Factory::RegisterDLL ("ssigraphic.dll");
	Factory::RegisterDLL ("ssisignal.dll");

#if SSI_RANDOM_LEGACY_FLAG
	ssi_random_seed();
#endif

	Console *console = ssi_create(Console, 0, true);
	console->setPosition(ssi_rect(0, 0, 650, 800));

	ex_kmeans ();

	ssi_print ("\n\n\tpress enter to quit\n\n");
	getchar ();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

void ex_kmeans () {

	SampleList samples;
	ssi_size_t n_classes = 4;
	ssi_size_t n_sampels = 200;
	ssi_size_t n_streams = 1;
	ssi_real_t distr[][3] = { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };
	ModelTools::CreateTestSamples (samples, n_classes, n_sampels, n_streams, distr);

	// training
	{
		KMeans *model = ssi_create (KMeans, "kmeans", true);
		model->getOptions()->k = n_classes;
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

	// split
	{
		KMeans *model = ssi_create (KMeans, "kmeans", true);
		model->load("kmeans.trainer.KMeans.model");

		ISSelectSample ss (&samples);
		ss.setSelection (model->getIndicesPerClusterSize(1), model->getIndicesPerCluster(1));

		ModelTools::PlotSamples (ss, "kmeans", ssi_rect(650,0,400,400));
		
	}
}
