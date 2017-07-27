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

#if !_DEBUG && defined _MSC_VER && _MSC_VER == 1900	
	const ssi_char_t *default_source = "https://github.com/hcmlab/ssi/raw/master/bin/x64/vc140";
#else
	const ssi_char_t *default_source = "";
#endif

	//**** READ COMMAND LINE ****//

	CmdArgParser cmd;
	cmd.info (info);

	ssi_char_t *inpath = 0;
	ssi_char_t *outpath = 0;	
	ssi_char_t *tset = 0;
	ssi_char_t *dlls = 0;
	ssi_char_t *srcurl = 0;
	ssi_char_t *log = 0;
	ssi_char_t **tokens = 0;
	int eval = -1;
	int kfolds = 2;
	bool overwrite = false;

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("trainer", &inpath, "path to trainer template");
	
	cmd.addText("\nOptions:");
	cmd.addSCmdOption("-out", &outpath, "", "path to save trainer / evaluation result");
	cmd.addBCmdOption("-overwrite", &overwrite, "", "overwrite existing model");
	cmd.addSCmdOption("-dlls", &dlls, "", "list of requird dlls separated by ';' [deprecated, use register tag in trainer]");
	cmd.addSCmdOption("-url", &srcurl, default_source, "override default url for downloading missing dlls and dependencies");
	cmd.addICmdOption("-eval", &eval, -1, "set an evaluation (will not output trainer)\n\t\t\t0=KFOLD (see -kfolds)\n\t\t\t1=LOO (Leave-one-sample-out)\n\t\t\t2=LOUO (Leave-one-user-out)\n\t\t\t3=TEST (Use test set, see -tset)\n\t\t\t4=ContLOUO (continuous LOUO)");
	cmd.addICmdOption("-kfolds", &kfolds, 2, "set number of folds for KFOLD evaluation");
	cmd.addSCmdOption("-tset", &tset, "", "set sample file for TEST evaluation (if multiple files separate by ';')");
	cmd.addSCmdOption("-log", &log, "", "output to log file");	
	
	if (cmd.read (argc, argv)) 
	{		
		ssi_print("%s", info);

		// set directories
		FilePath exepath_fp(argv[0]);
		ssi_char_t workdir[SSI_MAX_CHAR];
		ssi_getcwd(SSI_MAX_CHAR, workdir);
		ssi_char_t exedir[SSI_MAX_CHAR];
		if (exepath_fp.isRelative()) {
#if _WIN32|_WIN64
			ssi_sprint(exedir, "%s\\%s", workdir, exepath_fp.getDir());
#else
			ssi_sprint(exedir, "%s/%s", workdir, exepath_fp.getDir());
#endif
		}
		else {
			strcpy(exedir, exepath_fp.getDir());
		}
		ssi_print("download source=%s\ndownload target=%s\n\n", srcurl, exedir);
		Factory::SetDownloadDirs(srcurl, exedir);

		if (log[0] != '\0') 
		{
			ssimsg = new FileMessage(log);			
		}

		// register model dll
		Factory::RegisterDLL("model", ssiout, ssimsg);

		ssi_size_t n = ssi_split_string_count(dlls, ';');
		if (n > 0) 
		{
			tokens = new ssi_char_t *[n];
			ssi_split_string(n, tokens, dlls, ';');
			for (ssi_size_t i = 0; i < n; i++) 
			{
				Factory::RegisterDLL(tokens[i]);
			}
		}

		if (eval == -1) {
			Trainer trainer;
			if (Trainer::Load(trainer, inpath)) 
			{
				if (!trainer.isTrained() || overwrite)
				{
					if (trainer.train()) 
					{
						trainer.save(outpath[0] == '\0' ? inpath : outpath);
					}
				}			
			}
		}
		else 
		{
			Trainer trainer;
			if (Trainer::Load(trainer, inpath)) 
			{	
				FILE *result = ssiout;

				if (outpath[0] != '\0')
				{
					FILE *fp = fopen(outpath, "w");
					if (fp)
					{
						result = fp;
					}
					else
					{
						ssi_wrn("could not open file '%s'", outpath);
					}
				}

				switch (eval) {
				case 0:
					trainer.evalKFold(kfolds, result);
					break;
				case 1:
					trainer.evalLOO(result);
					break;
				case 2:
					trainer.evalLOUO(result);
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
						trainer.eval(samples, result);
					} else {
						ssi_wrn("no test file provided, see -tset option");
					}
					break;
				}

				if (result != ssiout)
				{
					ssi_size_t n_content = 0;
					ssi_char_t *content = FileTools::ReadAsciiFile(outpath, n_content);
					ssi_print(content);
					fclose(result);
				}
			}
		}

		if (log[0] != '\0') 
		{
			delete ssimsg; ssimsg = 0;
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

