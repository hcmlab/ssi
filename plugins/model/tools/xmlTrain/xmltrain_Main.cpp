// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/11/12
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
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

int main (int argc, char **argv) {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	char info[1024];
	ssi_sprint (info, "\n%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL("ssimodel.dll");

	//**** READ COMMAND LINE ****//

	CmdArgParser cmd;
	cmd.info (info);

	ssi_char_t *inpath = 0;
	ssi_char_t *outpath = 0;
	ssi_char_t *tset = 0;
	ssi_char_t *dlls = 0;
	ssi_char_t *log = 0;
	ssi_char_t **tokens = 0;
	int eval = -1;
	int kfolds = 2;

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("trainer", &inpath, "path to trainer template");
	
	cmd.addText("\nOptions:");
	cmd.addSCmdOption("-out", &outpath, "", "use a different path to save trainer after training");
	cmd.addSCmdOption("-dlls", &dlls, "", "list of requird dlls separated by ';'");
	cmd.addICmdOption("-eval", &eval, -1, "set an evaluation (will not output trainer)\n\t\t\t0=KFOLD (see -kfolds)\n\t\t\t1=LOO (Leave-one-sample-out)\n\t\t\t2=LOUO (Leave-one-user-out)\n\t\t\t3=TEST (Use test set, see -tset)\n\t\t\t4=ContLOUO (continuous LOUO)");
	cmd.addICmdOption("-kfolds", &kfolds, 2, "set number of folds for KFOLD evaluation");
	cmd.addSCmdOption("-tset", &tset, "", "set sample file for TEST evaluation (if multiple files separate by ';')");
	cmd.addSCmdOption("-log", &log, "", "output to log file");
	
	if (cmd.read (argc, argv)) {		

		if (log[0] != '\0') {
			ssi_log_file_begin (log);
		}

		ssi_size_t n = ssi_split_string_count(dlls, ';');
		if (n > 0) {
			tokens = new ssi_char_t *[n];
			ssi_split_string(n, tokens, dlls, ';');
			for (ssi_size_t i = 0; i < n; i++) {
				Factory::RegisterDLL(tokens[i]);
			}
		}

		if (eval == -1) {
			Trainer trainer;
			if (Trainer::Load(trainer, inpath)) {
				if (trainer.train()) {
					trainer.save(outpath[0] == '\0' ? inpath : outpath);
				}
			}
		}
		else {
			Trainer trainer;
			if (Trainer::Load(trainer, inpath)) {
				switch (eval) {
				case 0:
					trainer.evalKFold(kfolds, ssiout);
					break;
				case 1:
					trainer.evalLOO(ssiout);
					break;
				case 2:
					trainer.evalLOUO(ssiout);
					break;
				case 3:

					ssi_size_t n = ssi_split_string_count(tset, ';');
					if (n > 0) {
						ssi_char_t **files = new ssi_char_t *[n];
						ssi_split_string(n, files, tset, ';');
						SampleList samples;
						for (ssi_size_t i = 0; i < n; i++) {
							ModelTools::LoadSampleList(samples, files[i]);
							delete[] files[i];
						}
						delete[] files;
						if (!trainer.isTrained())
						{
							trainer.train();
						}
						trainer.eval(samples, ssiout);
					} else {
						ssi_wrn("no test file provided, see -tset option");
					}
					break;
				}
			}
		}

		if (log[0] != '\0') {
			ssi_log_file_end();
		}

		Factory::Clear ();
	}

	delete[] inpath;	
	delete[] outpath;
	delete[] tset;
	delete[] dlls;
	delete[] tokens;
	delete[] log;

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

