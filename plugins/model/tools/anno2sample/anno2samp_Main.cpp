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
	ssi_char_t *streampath = 0;
	ssi_char_t *samplepath = 0;
	ssi_char_t *username = 0;
	ssi_char_t *log = 0;
	ssi_time_t frame = 0;
	ssi_time_t delta = 0;
    ssi_time_t percent = 0;
	ssi_char_t *label = 0;
	bool noscale = false;
	bool ascii = false;
	
	cmd.addText("\nArguments:");
	cmd.addSCmdArg("user", &username, "user name");
	cmd.addSCmdArg("annotation", &annopath, "path to anno file");
	cmd.addSCmdArg("stream", &streampath, "path to stream file(s) (if several separated by ;)");
	cmd.addSCmdArg("samples", &samplepath, "path to sample file (separated by ;)");
	
	cmd.addText("\nOptions:");	
	cmd.addSCmdOption("-log", &log, "", "output to log file");
	cmd.addBCmdOption("-ascii", &ascii, false, "output in ascii format");
	cmd.addBCmdOption("-noscale", &noscale, false, "do not convert wav files to float");
	cmd.addDCmdOption("-frame", &frame, 0, "generate samples at a continuous frame size (given in seconds), will be turned on if > 0");
	cmd.addDCmdOption("-delta", &delta, 0, "overlap with next frame in seconds (applied if frame > 0)");
	cmd.addDCmdOption("-percent", &percent, 0.5, "percentage of a frame a annotation segment has to cover (applied if frame > 0)");
	cmd.addSCmdOption("-label", &label, "", "default label if not covered by an annotation segment (applied if frame > 0)");

	if (cmd.read (argc, argv)) {		

		if (log[0] != '\0') {
			ssi_log_file_begin (log);
		}

		Annotation anno;
		ssi_print("READ ANNOTATION\t\t'%s'\n", annopath);		
		ModelTools::LoadAnnotation(anno, annopath);

		ssi_size_t n_streams = ssi_split_string_count(streampath, ';');
		ssi_char_t **tokens = new ssi_char_t *[n_streams];
		ssi_stream_t **streams = new ssi_stream_t *[n_streams];
		ssi_split_string(n_streams, tokens, streampath, ';');
		for (ssi_size_t i = 0; i < n_streams; i++) {			
			streams[i] = new ssi_stream_t;
			FilePath fp(tokens[i]);
			ssi_print("READ STREAM\t\t'%s'\n", tokens[i]);
			if (ssi_strcmp(fp.getExtension(), ".wav", false)) {				
				WavTools::ReadWavFile(tokens[i], *streams[i], !noscale);
			} else {
				FileTools::ReadStreamFile(tokens[i], *streams[i]);
			}
		}
		
		SampleList samples;
		if (frame > 0) {
			Annotation anno_c;

			if (strlen(label) == 0){				
				ssi_wrn("dropping samples with empty annotation")
			}
				
			ModelTools::ConvertToContinuousAnnotation(anno, anno_c, frame, delta, percent, strlen(label) == 0 ? 0 : label);
			ModelTools::LoadSampleList(samples, n_streams, streams, anno_c, username);
		} else {
			ModelTools::LoadSampleList(samples, n_streams, streams, anno, username);
		}

		ssi_print("SAVE SAMPLES\t\t'%s'\n", annopath);
		ModelTools::SaveSampleList(samples, samplepath, ascii ? File::ASCII : File::BINARY);

		for (ssi_size_t i = 0; i < n_streams; i++) {
			ssi_stream_destroy(*streams[i]);
			delete streams[i];
		}

		if (log[0] != '\0') {
			ssi_log_file_end();
		}

		Factory::Clear ();
	}

	delete[] username;
	delete[] annopath;
	delete[] streampath;
	delete[] samplepath;
	delete[] log;
	delete[] label;

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

