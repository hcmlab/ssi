// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/12/12
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

// --run -signal cursor -anno button -user user D:\wagner\openssi\core\build\tools\xmltrain\mlp\mlp
// --train -eval -1 -kfolds 2 D:\wagner\openssi\core\build\tools\xmltrain\mlp\mlp mlp mlp mlp
// --run -trainer mlp D:\wagner\openssi\core\build\tools\xmltrain\mlp\mlp

#include "ssi.h"
#include "ssiml.h"
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

	ssi_char_t *annopath = 0;
    ssi_char_t *elanpath = 0;
	ssi_char_t *username = 0;
    ssi_char_t *mode=0;
    ssi_char_t *track0=0;
    ssi_char_t *track1=0;

	ssi_char_t *label = 0;

    ElanDocument* test0= ElanDocument::Read("in/test.eaf");

    ElanTools::Ssi2elan("in/test.anno", test0);

    ElanDocument* test= ElanDocument::Read("in/test.eaf");

    ElanTools::AddTrack_boolOp(test, "Neutral", "PositivePassive", "joined",ElanTools::OR);
    test->write("out/j.eaf");

	cmd.addText("\nArguments:");
    //cmd.addSCmdArg("user", &username, "user name");
    cmd.addSCmdArg("elan", &elanpath, "path to elan anno (eaf) file");
    cmd.addSCmdArg("annotation", &annopath, "path to ssi anno file");
    cmd.addSCmdArg("mode", &mode, "ssi2elan, elan2ssi, and, xor, or");
    cmd.addSCmdArg("track0", &track0, "track0 name");
    cmd.addSCmdArg("track1", &track1, "track1 name");
	
	cmd.addText("\nOptions:");	

	cmd.addSCmdOption("-label", &label, "", "default label if not covered by an annotation segment (applied if frame > 0)");

	if (cmd.read (argc, argv)) {		

        if(!strcmp(mode,"elan2ssi"))
             //elan_convert(argv[1], argv[2]);

        if(!strcmp(mode, "ssi2elan"))
        {
            ElanDocument elanDoc;
            ElanTools::Ssi2elan(argv[2], &elanDoc);
            elanDoc.write(argv[1]);

        }
        if(!strcmp(mode, "and"))
        {

        }

		Factory::Clear ();
	}

	delete[] username;
	delete[] annopath;

    delete[] label;

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

