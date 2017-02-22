// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/03/29
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "ssi.h"
#include "ioput/api/APIGenerator.h"
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

	//**** READ COMMAND LINE ****//

	CmdArgParser cmd;
	cmd.info (info);

	ssi_char_t *dllpath = 0;	
	ssi_char_t *apipath = 0;	
	ssi_char_t *outdir = 0;
	ssi_char_t *reg = 0; 
	bool index;

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("dllpath", &dllpath, "input path to dll");	

	cmd.addText ("\nOptions:");
	cmd.addSCmdOption ("-dir", &outdir, "", "output directory []");
	cmd.addBCmdOption ("-index", &index, false, "create index [false]");
	cmd.addSCmdOption ("-reg", &reg, "", "register additional dll's (if several separate by semicolon)");
	
	if (cmd.read (argc, argv)) {		

		ssi_char_t string[SSI_MAX_CHAR];

		FilePath fp (dllpath);
		ssi_char_t *dllpath_with_ext = 0;
		if (strcmp (fp.getExtension (), ".dll") != 0) {
			dllpath_with_ext = ssi_strcat (dllpath, ".dll");
		} else {
			dllpath_with_ext = ssi_strcpy (dllpath);
		}

		if (Factory::RegisterDLL (dllpath_with_ext)) {	

			// register additional dlls
			if (reg) {
				APIGenerator::SaveCurrentComponentList ();
				ssi_size_t n_reg = ssi_split_string_count (reg, ';');
				ssi_char_t **regs = new ssi_char_t *[n_reg];
				ssi_split_string (n_reg, regs, reg, ';');
				for (ssi_size_t i = 0; i < n_reg; i++) {
					Factory::RegisterDLL (regs[i]);
					delete[] regs[i];
				}
				delete[] regs;
			}

			if (outdir[0] == '\0') {
				ssi_sprint (string, "%s", fp.getPath ());
			} else {
				ssi_sprint (string, "%s\\%s", outdir, fp.getName ());
			}

			APIGenerator::CreateAPI (string);
			APIGenerator::ResetCurrentComponentList ();
			Factory::Clear ();
		}

		if (index) {
			if (outdir[0] == '\0') {
				APIGenerator::CreateAPIIndex (fp.getDir ());
			} else {
				APIGenerator::CreateAPIIndex (outdir);
			}
		}

		delete[] dllpath_with_ext;
	}

	delete[] dllpath;	
	delete[] apipath;	
	delete[] outdir;

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}
