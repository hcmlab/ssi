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
#include "signal/include/ssisignal.h"
using namespace ssi;

#define N_COLORS 5
unsigned short COLORS[][3] = {
	128,0,0,
	0,128,0,
	0,0,128,
	128,0,128,
	0,128,128
};

void ex_fsel ();

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

	ex_fsel ();

	ssi_print ("\n\n\tpress enter to quit\n\n");
	getchar ();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

void ex_fsel () {

	Trainer::SetLogLevel (SSI_LOG_LEVEL_DEBUG);

	ssi_size_t n_classes = 4;
	ssi_size_t n_samples = 500;
	ssi_size_t n_streams = 1;
	ssi_real_t sgood_distr[][3] = { 0.3f, 0.3f, 0.2f, 0.3f, 0.6f, 0.2f, 0.6f, 0.3f, 0.2f, 0.6f, 0.6f, 0.2f };
	ssi_real_t sbad_distr[][3] = { 0.3f, 0.3f, 0.75f, 0.3f, 0.6f, 0.75f, 0.6f, 0.3f, 0.75f, 0.6f, 0.6f, 0.75f };
	SampleList sgood;
	SampleList sbad;
	ModelTools::CreateTestSamples (sgood, n_classes, n_samples, n_streams, sgood_distr);			
	ModelTools::CreateTestSamples (sbad, n_classes, n_samples, n_streams, sbad_distr);		
	ssi_char_t string[SSI_MAX_CHAR];	
	for (ssi_size_t n_class = 1; n_class < n_classes; n_class++) {
		ssi_sprint(string, "%c", ssi_cast(char, 'A' + n_class));
		sgood.addClassName (string);
		sbad.addClassName (string);
	}
	ISamples *list[] = { &sbad, &sgood };
	ISMergeStrms ismerge (2, list);
	ISAlignStrms isalign (&ismerge);

	ModelTools::PrintInfo (ismerge);

	// manual selection
	{
		NaiveBayes *model = ssi_create (NaiveBayes, 0, true);
		model->getOptions()->log = true;
		Trainer trainer (model);
		ssi_size_t n_sel[1] = { 2 };
		ssi_size_t *sel[1];
		sel[0] = new ssi_size_t[2];
		sel[0][0] = 1;
		sel[0][1] = 0;
		//trainer.train (isalign, 1, n_sel, sel);
		trainer.setSelection (1, n_sel, sel);

		Functionals *functs = ssi_create (Functionals, 0, true);		
		functs->getOptions()->addName("min");
		functs->getOptions()->addName("max");
		ITransformer *transf[1] = { functs };
		trainer.setTransformer (1, transf);
		
		ModelTools::PrintInfo (isalign);

		trainer.train (isalign);
		trainer.save ("bayes-fsel");
		delete[] sel[0];
	}

	// evaluation
	{
		Trainer trainer;
		Trainer::Load (trainer, "bayes-fsel");			
		Evaluation eval;
		eval.eval (&trainer, isalign);
		eval.print ();
	}

	ssi_print ("\n\n\tpress enter to continue\n\n");
	getchar ();

	// sfs selection
	{
		NaiveBayes *model = ssi_create (NaiveBayes, 0, true);
		model->getOptions()->log = true;
		Trainer trainer (model);
		FloatingSearch *fsearch = ssi_create (FloatingSearch, 0, true);
		ssi_tic ();
		trainer.setSelection (isalign, fsearch);
		ssi_print ("\nsfs with multi-threading: ");
		ssi_toc_print ();
		ssi_print ("\n\n");
		//trainer.train (isalign, fsearch);
		trainer.train (isalign);
		trainer.save ("bayes-fsel-sfs");
	}

	// evaluation
	{
		Trainer trainer;
		Trainer::Load (trainer, "bayes-fsel-sfs");			
		Evaluation eval;
		eval.eval (&trainer, isalign);
		eval.print ();
	}

	ssi_print ("\n\n\tpress enter to continue\n\n");
	getchar ();

	// multi-threaded sfs selection 
	{
		NaiveBayes *model = ssi_create (NaiveBayes, 0, true);
		model->getOptions()->log = true;
		Trainer trainer (model);
		FloatingSearch *fsearch = ssi_create (FloatingSearch, 0, true);
		fsearch->getOptions()->nthread = 5;
		ssi_tic ();
		trainer.setSelection (isalign, fsearch);
		ssi_print ("\nsfs with multi-threading: ");
		ssi_toc_print ();
		ssi_print ("\n\n");
		//trainer.train (isalign, fsearch);		
		trainer.train (isalign);		
		trainer.save ("bayes-fsel-sfs-tp");
	}

	// evaluation
	{
		Trainer trainer;
		Trainer::Load (trainer, "bayes-fsel-sfs-tp");			
		Evaluation eval;
		eval.eval (&trainer, isalign);
		eval.print ();
	}

	ssi_print ("\n\n\tpress enter to continue\n\n");
	getchar ();

	// sfs selection
	{
		NaiveBayes *model = ssi_create (NaiveBayes, 0, true);
		model->getOptions()->log = true;
		Trainer trainer (model);
		FloatingCFS *cfs = ssi_create (FloatingCFS, 0, true);
		FloatingSearch *fsearch = ssi_create (FloatingSearch, 0, true);
		//trainer.train (isalign, fsearch, cfs, 3);
		trainer.setSelection (isalign, fsearch, cfs, 3);
		trainer.train (isalign);
		trainer.save ("bayes-fsel-cfs+sfs");
	}

	// evaluation
	{
		Trainer trainer;
		Trainer::Load (trainer, "bayes-fsel-cfs+sfs");			
		Evaluation eval;
		eval.eval (&trainer, isalign);
		eval.print ();
	}

	ssi_print ("\n\n\tpress enter to continue\n\n");
	getchar ();

	// relief
	{
		Relief *relief = ssi_create (Relief, "relief", true);

		ssi_print ("relief:\n");
		relief->train(isalign, 0);
		Selection select_relief;
		select_relief.set (relief->getSize(), relief->getScores(), true);
		select_relief.print ();
		select_relief.selNFirst (10);
		select_relief.print ();
		ISSelectDim salign_relief (&isalign);		
		salign_relief.setSelection (0, select_relief.getSize (), select_relief.getSelected (), false);
		//salign_relief.print ();

		ssi_print ("relief (mem):\n");
		relief->getOptions()->mem = true;
		relief->train(isalign, 0);
		Selection select_relief_mem;
		select_relief_mem.set (relief->getSize(), relief->getScores(), true);
		select_relief_mem.print ();
		select_relief_mem.selNFirst (10);
		select_relief_mem.print ();
		ISSelectDim salign_relief_mem (&isalign);
		salign_relief_mem.setSelection (0, select_relief_mem.getSize (), select_relief_mem.getSelected (), false);
		//salign_relief_mem.print ();

		FloatingSearch *fs = ssi_create (FloatingSearch, "sfs", true);
		fs->getOptions()->nfirst = 0;
		SimpleKNN *model = ssi_create (SimpleKNN, 0, true);
		fs->setModel(*model);
		fs->train(salign_relief, 0);
		Selection select_sfs (&select_relief);
		select_sfs.set (fs->getSize(), fs->getScores(), false);
		select_sfs.print ();
		select_sfs.selNBest ();
		select_sfs.print ();
		select_sfs.save ("relief", File::ASCII);
	}

	//evaluation
	{
		Selection select;
		select.load ("relief", File::ASCII);
		SimpleKNN *model = ssi_create (SimpleKNN, 0, true);
		ISSelectDim sselect (&isalign);
		sselect.setSelection (0, select.getSize (), select.getSelected ());		
		Trainer trainer (model, 0);
		Evaluation eval;		
		eval.evalKFold (&trainer, sselect, 2);
		eval.print ();
	}

	ssi_print ("\n\n\tpress enter to continue\n\n");
	getchar ();
	
}

