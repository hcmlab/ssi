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
	ssi_char_t *exePath;
	ssi_char_t *chainPath;
	ssi_char_t chainPathAbsolute[SSI_MAX_CHAR];
	ssi_char_t *inPath;
	StringList inList;
	ssi_char_t *outPath;
	StringList outList;
	bool list;
	ssi_char_t *logPath;
	ssi_char_t *srcUrl;
	ssi_char_t *step;
	ssi_char_t *left;
	ssi_char_t *right;
	int nParallel;
};

struct FeatureArguments
{	
	ssi_size_t n;
	params_t *params;
};

bool Parse_and_Run(int argc, char **argv);
void Run(const ssi_char_t *exepath, params_t &params);
bool Extract(params_t &params, FilePath &inPath, FilePath &outPath);
bool ExtractJob(ssi_size_t n_in, void *in, ssi_size_t n_out, void *out);

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
	params.exePath = 0;
	params.chainPath = 0;
	params.inPath = 0;
	params.outPath = 0;
	params.logPath = 0;
	params.srcUrl = 0;
	params.step = 0;
	params.left = 0;
	params.right = 0;

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("chain", &params.chainPath, "path to file defining the processing chain");
	cmd.addSCmdArg("in", &params.inPath, "input path (if several separate by ;)");
	cmd.addSCmdArg("out", &params.outPath, "output path (if several separate by ;)");

	cmd.addText("\nOptions:");
	cmd.addBCmdOption("-list", &params.list, false, "read files from list (one file per line)");
	cmd.addSCmdOption("-step", &params.step, "0", "set frame step (add ms/s for milli/seconds otherwise interpreted as number of samples)");
	cmd.addSCmdOption("-left", &params.left, "0", "set left context (see frame)");
	cmd.addSCmdOption("-right", &params.right, "0", "set right context (see frame)");
	cmd.addICmdOption("-parallel", &params.nParallel, 1, "number of files processed in parallel (0 = all)");
	cmd.addSCmdOption("-url", &params.srcUrl, default_source, "override default url for downloading missing dlls and dependencies");
	cmd.addSCmdOption("-log", &params.logPath, "", "log to a file [""]");

	if (!cmd.read(argc, argv))
	{
		return false;
	}

	if (params.logPath[0] != '\0')
	{
		ssimsg = new FileMessage(params.logPath);
	}

	ssi_print("%s", info);

	Run(argv[0], params);

	if (params.logPath[0] != '\0') {
		delete ssimsg;
		ssimsg = 0;
	}

	delete[] params.exePath;
	delete[] params.chainPath;
	delete[] params.inPath;
	delete[] params.outPath;
	delete[] params.logPath;
	delete[] params.srcUrl;
	delete[] params.left;
	delete[] params.right;
	delete[] params.step;

	return true;
}

void splitFiles(const ssi_char_t *string, StringList &files, bool list)
{
	if (list)
	{
		FileTools::ReadFilesFromFile(files, string);
	}
	else
	{
		ssi_size_t n_tokens = ssi_split_string_count(string, ';');
		ssi_char_t **tokens = new ssi_char_t *[n_tokens];
		ssi_split_string(n_tokens, tokens, string, ';');
		for (ssi_size_t i = 0; i < n_tokens; i++)
		{
			files.add(tokens[i]);
			delete[] tokens[i];
		}
		delete[] tokens;
	}
}

void Run(const ssi_char_t *exePath, params_t &params) {

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

	// set exe path
	params.exePath = ssi_strcpy(exePath);

	// register frame dll
	Factory::RegisterDLL("ssiframe", ssiout, ssimsg);

	// full chain path
	FilePath chainPath_fp(params.chainPath);	
	if (chainPath_fp.isRelative()) {
#if _WIN32|_WIN64
		ssi_sprint(params.chainPathAbsolute, "%s\\%s", workDir, params.chainPath);
#else
		ssi_sprint(pipepath, "%s/%s", _workdir, params.chainPath);
#endif
	}
	else {
		strcpy(params.chainPathAbsolute, params.chainPath);
	}

	// set working directory to pipeline directory		
	ssi_setcwd(chainPath_fp.getDir());

	// fill lists
	splitFiles(params.inPath, params.inList, params.list);
	splitFiles(params.outPath, params.outList, params.list);

	if (params.inList.size() != params.outList.size())
	{
		ssi_wrn("number of input files differs from number of output files");
		return;
	}

	// start processing

	ssi_size_t nFiles = (ssi_size_t)params.inList.size();
	ssi_size_t nThreads = params.nParallel <= 0 ? nFiles : params.nParallel;
	
	if (nFiles == 1 || nThreads == 1)
	{
		for (ssi_size_t n = 0; n < nFiles; n++)
		{
			FilePath inPath(params.inList[n].str());
			FilePath outPath(params.outList[n].str());
			Extract(params, inPath, outPath);
		}
	}
	else
	{
		// make sure dependecies are resolved
		Chain *chain = ssi_create(Chain, 0, false);
		chain->getOptions()->set(params.chainPathAbsolute);
		chain->parse();
		chain->release();
		delete chain;

		ThreadPool pool("extract", nThreads);

		for (ssi_size_t n = 0; n < nFiles; n++)
		{
			FilePath inPath(params.inList[n].str());
			FilePath outPath(params.outList[n].str());

			FeatureArguments *args = new FeatureArguments;
			args->params = &params;
			args->n = n;

			ThreadPool::job_s job;
			job.n_in = 1;
			job.in = args;
			job.n_out = 0;
			job.out = 0;
			job.job = ExtractJob;

			pool.add(job);
		}

		if (!pool.work())
		{
			ssi_wrn("one or more jobs failed");
		}

		ssi_char_t logPath[SSI_MAX_CHAR];
		for (ssi_size_t m = 0; m < pool.size(); m++)
		{
			ThreadPool::job_s job = pool.get(m);
			FeatureArguments *args = (FeatureArguments *)job.in;
			
			ssi_sprint(logPath, "%s.%04u", params.logPath, args->n);
			ssi_size_t len;
			ssi_char_t *str;
			if (str = FileTools::ReadAsciiFile(logPath, len))
			{
				ssi_print(str);
			}
			ssi_remove(logPath);

			delete args;
		}
	}

	Factory::Clear();

	ssi_setcwd(chainPath_fp.getDir());
}

String paramsToArgs(params_t *params, ssi_size_t n)
{
	ssi_char_t *inPath = params->inList[n].str();
	ssi_char_t *outPath = params->outList[n].str();

	ssi_char_t logPath[SSI_MAX_CHAR];
	ssi_sprint(logPath, "%s.%04u", params->logPath, n);

	String string = String("") +
		"-step " + params->step +
		" -left " + params->left +
		" -right " + params->right +
		" -url \"" + params->srcUrl + "\"" +
		" -log \"" + logPath + "\" " +
		"\"" + params->chainPath + "\" "
		"\"" + inPath + "\" "
		"\"" + outPath + "\"";

	return string;
}

bool ExtractJob(ssi_size_t n_in, void *in, ssi_size_t n_out, void *out)
{
	FeatureArguments *featureArguments = ssi_pcast(FeatureArguments, in);
	
	params_t *params = featureArguments->params;
	ssi_size_t n = featureArguments->n;

	String processArguments = paramsToArgs(params, n);

	ssi_print_off("%s %s\n", params->exePath, processArguments.str());
	
	return ssi_execute(params->exePath, processArguments.str(), -1, false);
	//return Extract(*params, inPath, outPath);	
}

bool Extract(params_t &params, FilePath &inPath, FilePath &outPath)
{	
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
	chain->getOptions()->set(params.chainPathAbsolute);	

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

		result &= FileTools::WriteStreamFile(File::BINARY, toPath, to);

		ssi_stream_destroy(from);
		ssi_stream_destroy(to);
	}

	delete[] toPath;

	return result;
}