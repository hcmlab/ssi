// TestMain.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2010/12/07
// Copyright (C) 2007-12 University of Augsburg, Florian Lingenfelser
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
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "ssi.h"
#include "ssifusion.h"
#include "ssiml/include/ssiml.h"
#include "model/include/ssimodel.h"
using namespace ssi;

//#include <vld.h>

#define LOG_LEVEL 3

void ex_NoFuse();

void ex_Fusion(const ssi_char_t *fname);

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

#define BOOST_COMBINATION_SIZE 6
#define FNAMES_SIZE 23
const ssi_char_t *fnames[] = {

	ssi::AdaBoost::GetCreateName(),
	ssi::BKS::GetCreateName(),
	ssi::BordaCount::GetCreateName(),
	ssi::CascadingSpecialists::GetCreateName(),
	ssi::CascadingSpecialistsMS::GetCreateName(),
	ssi::DecisionTemplate::GetCreateName(),
	ssi::DempsterShafer::GetCreateName(),
	ssi::FeatureFusion::GetCreateName(),
	ssi::Grading::GetCreateName(),
	ssi::MajorityVoting::GetCreateName(),
	ssi::MaxRule::GetCreateName(),
	ssi::MeanRule::GetCreateName(),
	ssi::MedianRule::GetCreateName(),
	ssi::MinRule::GetCreateName(),
	ssi::OvR::GetCreateName(),
	ssi::OvRSpecialist::GetCreateName(),
	ssi::ProductRule::GetCreateName(),
	ssi::SingleFeatures::GetCreateName(),
	ssi::StackedGeneralization::GetCreateName(),
	ssi::SumRule::GetCreateName(),
	ssi::VACFusion::GetCreateName(),
	ssi::WeightedAverage::GetCreateName(),
	ssi::WeightedMajorityVoting::GetCreateName()

};

	ssi_size_t n_classes = 4;
	ssi_size_t n_sampels = 50;
	ssi_size_t n_streams = 2;
	ssi_real_t distr_train[][3] = { 0.3f, 0.3f, 0.2f, 0.3f, 0.6f, 0.2f, 0.6f, 0.3f, 0.2f, 0.6f, 0.6f, 0.2f };
	ssi_real_t distr_test[][3]  = { 0.3f, 0.3f, 0.2f, 0.3f, 0.6f, 0.2f, 0.6f, 0.3f, 0.2f, 0.6f, 0.6f, 0.2f };
	SampleList *train_samples = 0;
	SampleList *test_samples = 0;


int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssimodel.dll");
	Factory::RegisterDLL ("ssifusion.dll");

	ssi_random_seed();
	SampleList training;
	ModelTools::CreateTestSamples (training, n_classes, n_sampels, n_streams, distr_train);			
	SampleList testing;
	ModelTools::CreateTestSamples (testing, n_classes, n_sampels, n_streams, distr_test);	
	train_samples = &training;
	test_samples = &testing;

	ex_NoFuse();

	for (ssi_size_t i = 0; i < FNAMES_SIZE; i++) {
		ex_Fusion(fnames[i]);
	}
	
	ssi_print ("\n\n\tpress a key to quit\n");
	getchar ();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

void ex_NoFuse(){

	ssi_char_t *name = "Save\\NoFusion";
	{	

		{
			ssi_size_t n_models = train_samples->getStreamSize();

			IModel **models = new IModel *[n_models];
			ssi_char_t string[SSI_MAX_CHAR];

			for (ssi_size_t nmodel = 0; nmodel < n_models; nmodel++) {
				ssi_sprint (string, "%s.%02d", name, nmodel);
				models[nmodel] = ssi_pcast (IModel, Factory::Create (ssi::NaiveBayes::GetCreateName (), string));
				
				Trainer trainer(models[nmodel], nmodel);
				trainer.train (*train_samples);
				trainer.eval (*test_samples);
				ssi_print("\nModel: %i \n", nmodel);				
			}

			delete[] models;
		}
	}

	ssi_print ("\n\n\tpress enter to continue\n");
	getchar ();
}

void ex_Fusion(const ssi_char_t *fname){

	ssi_char_t name[SSI_MAX_CHAR];
		
	//TRAINING
	{
		
		if(fname == ssi::AdaBoost::GetCreateName()){

			for(ssi_size_t combo = 0; combo < BOOST_COMBINATION_SIZE; combo++){

				ssi_sprint(name, "Save\\%s.%d", fname, combo);
		
				AdaBoost *fusion = ssi_pcast (AdaBoost, Factory::Create (AdaBoost::GetCreateName (), name));
				fusion->getOptions()->iter = 2;
				fusion->getOptions()->size = 60;
				fusion->getOptions()->error = 0.8f;
				fusion->getOptions()->combination_rule = combo;

				fusion->setLogLevel(LOG_LEVEL);
			
				ssi_size_t n_models = fusion->getModelNumber(*train_samples);
				ssi_size_t n_meta_models = fusion->getMetaModelNumber(*train_samples);

				IModel **models = new IModel *[(n_models + n_meta_models)];
				
				for (ssi_size_t i = 0; i < n_models; i++) {
					models[i] = ssi_pcast (IModel, Factory::Create (ssi::NaiveBayes::GetCreateName(), 0, true));
				}
				for (ssi_size_t i = n_models; i < (n_models + n_meta_models); i++) {
					models[i] = ssi_pcast (IModel, Factory::Create (ssi::NaiveBayes::GetCreateName(), 0, true));
				}

				Trainer trainer ((n_models + n_meta_models), models, fusion);
				trainer.train (*train_samples);
				trainer.save (name);

				delete[] models;
				models = 0;

			}
		
		}
		
		else{

			ssi_sprint(name, "Save\\%s", fname);

			IFusion *fusion = ssi_pcast (IFusion, Factory::Create (fname, 0, true));
			
			fusion->setLogLevel(LOG_LEVEL);
		
			ssi_size_t n_models = fusion->getModelNumber(*train_samples);
			ssi_size_t n_meta_models = fusion->getMetaModelNumber(*train_samples);

			IModel **models = new IModel *[(n_models + n_meta_models)];
			
			for (ssi_size_t i = 0; i < n_models; i++) {
				models[i] = ssi_pcast (IModel, Factory::Create (ssi::NaiveBayes::GetCreateName(), 0, true));
			}
			for (ssi_size_t i = n_models; i < (n_models + n_meta_models); i++) {
				models[i] = ssi_pcast (IModel, Factory::Create (ssi::NaiveBayes::GetCreateName(), 0, true));
			}

			Trainer trainer ((n_models + n_meta_models), models, fusion);
			trainer.train (*train_samples);
			trainer.save (name);

			delete[] models;
			models = 0;

		}
		
	}

	//EVALUATION
	{
		if(fname == ssi::AdaBoost::GetCreateName()){

			for(ssi_size_t combo = 0; combo < BOOST_COMBINATION_SIZE; combo++){

				ssi_sprint(name, "Save\\%s.%d", fname, combo);

				Trainer trainer;
				Trainer::Load (trainer, name);					
				trainer.eval (*test_samples);
				ssi_fprint(ssiout, "\nFusion '%s.%d'\n\n", fname, combo);				

				ssi_print ("\n\n\tpress enter to continue\n");
				getchar();

			}

		}

		else{

			Trainer trainer;
			Trainer::Load (trainer, name);					
			trainer.eval (*test_samples);
			ssi_fprint(ssiout, "\nFusion '%s'\n\n", fname);
			
		}
	}

}
