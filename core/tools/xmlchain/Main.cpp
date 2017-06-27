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
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif
#if __gnu_linux__
#include <unistd.h>
#endif

struct params_t {
	ssi_char_t *chainPath;
	ssi_char_t *inPath;
	ssi_char_t *outPath;
	ssi_char_t *debugPath;
	ssi_char_t *srcUrl;
	ssi_char_t *step;
	ssi_char_t *left;
	ssi_char_t *right;
};

void Run(const ssi_char_t *exepath, params_t params);
bool Parse_and_Run(int argc, char **argv);

int main(int argc, char **argv) {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

		Parse_and_Run(argc, argv);

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

bool Parse_and_Run(int argc, char **argv)
{
	char info[1024];
	ssi_sprint(info, "\n%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

#if !_DEBUG && defined _MSC_VER && _MSC_VER == 1900	
	const ssi_char_t *default_source = "https://github.com/hcmlab/ssi/raw/master/bin/x64/vc140";
#else
	const ssi_char_t *default_source = "";
#endif

	CmdArgParser cmd;
	cmd.info(info);

	params_t params;
	params.chainPath = 0;
	params.inPath = 0;
	params.outPath = 0;
	params.debugPath = 0;
	params.srcUrl = 0;
	params.step = 0;
	params.left = 0;
	params.right = 0;

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("chain", &params.chainPath, "path to file defining the processing chain");
	cmd.addSCmdArg("in", &params.inPath, "input path (to which chain is applied)");
	cmd.addSCmdArg("out", &params.outPath, "output path");

	cmd.addText("\nOptions:");
	cmd.addSCmdOption("-step", &params.step, "0", "set frame step (add ms/s for milli/seconds otherwise interpreted as number of samples)");
	cmd.addSCmdOption("-left", &params.left, "0", "set left context (see frame)");
	cmd.addSCmdOption("-right", &params.right, "0", "set right context (see frame)");
	cmd.addSCmdOption("-url", &params.srcUrl, default_source, "override default url for downloading missing dlls and dependencies");
	cmd.addSCmdOption("-debug", &params.debugPath, "", "debug to a file or stream to an udp socket (<host>:<port>) [""]");

	if (!cmd.read(argc, argv))
	{
		return false;
	}

	if (params.debugPath[0] != '\0')
	{
		ssi_size_t n = ssi_split_string_count(params.debugPath, ':');
		if (n > 1) {
			ssi_char_t **tokens = new ssi_char_t *[n];
			ssi_split_string(n, tokens, params.debugPath, ':');
			ssi_size_t port = 0;
			sscanf(tokens[1], "%u", &port);
			ssimsg = new SocketMessage(Socket::UDP, port, tokens[0]);
			for (ssi_size_t i = 0; i < n; i++) {
				delete[] tokens[i];
			}
			delete[] tokens;
		}
		else {
			ssimsg = new FileMessage(params.debugPath);
		}
	}

	ssi_print("%s", info);

	Run(argv[0], params);

	if (params.debugPath[0] != '\0') {
		delete ssimsg;
		ssimsg = 0;
	}

	delete[] params.chainPath;
	delete[] params.inPath;
	delete[] params.outPath;
	delete[] params.debugPath;
	delete[] params.srcUrl;

	return true;
}

void Run(const ssi_char_t *exePath, params_t params) {

	// get directories
	ssi_char_t workDir[SSI_MAX_CHAR];
	FilePath exepath_fp(exePath);
	ssi_getcwd(SSI_MAX_CHAR, workDir);
	ssi_char_t exedir[SSI_MAX_CHAR];
	if (exepath_fp.isRelative()) {
#if _WIN32|_WIN64
		ssi_sprint(exedir, "%s\\%s", workDir, exepath_fp.getDir());
#else
		ssi_sprint(exedir, "%s/%s", _workdir, exepath_fp.getDir());
#endif
	}
	else {
		strcpy(exedir, exepath_fp.getDir());
	}
	ssi_print("download source=%s\ndownload target=%s\n\n", params.srcUrl, exedir);
	Factory::SetDownloadDirs(params.srcUrl, exedir);

	// register frame and event dll
	Factory::RegisterDLL("ssiframe", ssiout, ssimsg);

	// full chain path
	FilePath chainPath_fp(params.chainPath);
	ssi_char_t chainPath[SSI_MAX_CHAR];
	if (chainPath_fp.isRelative()) {
#if _WIN32|_WIN64
		ssi_sprint(chainPath, "%s\\%s", workDir, params.chainPath);
#else
		ssi_sprint(pipepath, "%s/%s", _workdir, params.chainPath);
#endif
	}
	else {
		strcpy(chainPath, params.chainPath);
	}

	// set working directory to pipeline directory		
	ssi_setcwd(chainPath_fp.getDir());

	// in/out path
	FilePath inPath(params.inPath);
	FilePath outPath(params.outPath);

	ssi_char_t *toPath = 0;
	if (ssi_strcmp(outPath.getExtension(), ".stream", false))
	{
		toPath = ssi_strcpy(outPath.getPath());
	}
	else
	{
		toPath = ssi_strcpy(outPath.getPathFull());
	}

	Chain *chain = ssi_create_id(Chain, 0, "chain");
	chain->getOptions()->set(chainPath);
	//chain->SetRegisterXMLFptr (Factory::RegisterXML);

	ssi_stream_t from;
	bool result = false;
	if (ssi_strcmp(inPath.getExtension(), ".wav", false))
	{
		result = WavTools::ReadWavFile(inPath.getPathFull(), from, true);
	}
	else
	{
		result = FileTools::ReadStreamFile(inPath.getPath(), from);
	}

	if (result)
	{
		ssi_stream_t to;
		if (ssi_strcmp(params.step, "0"))
		{
			SignalTools::Transform(from, to, *chain, 0u);
		}
		else
		{
			SignalTools::Transform(from, to, *chain, params.step, params.left, params.right);
		}				
		FileTools::WriteStreamFile(File::BINARY, toPath, to);
		ssi_stream_destroy(from);
		ssi_stream_destroy(to);
	}

	delete[] toPath;

	Factory::Clear();

	ssi_setcwd(chainPath_fp.getDir());
}

