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
#include "ffmpeg/include/ssiffmpeg.h"
#include "ioput/include/ssiioput.h"
#include "ssiml/include/ssiml.h"
#include "ssiml/include/ISTransform.h"
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
	ssi_char_t *exeDir;
	ssi_char_t *exePath;
	ssi_char_t *chainDir;
	ssi_char_t *chainPath;
	ssi_char_t *chainPathAbsolute;
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
	ssi_char_t *annoPath;
	StringList annoList;
	bool ascii;
	ssi_char_t *restClassName;
};

struct FeatureArguments
{	
	ssi_size_t n;
	params_t *params;
};

bool Parse_and_Run(int argc, char **argv);
void Run(const ssi_char_t *exepath, params_t &params);
bool Extract(params_t &params, FilePath *inPath, FilePath *outPath, FilePath *annoPath = 0);
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

#if !_DEBUG && defined _MSC_VER && _MSC_VER >= 1900	
	const ssi_char_t *default_source = "https://github.com/hcmlab/ssi/raw/master/bin/x64/vc140";
#else
	const ssi_char_t *default_source = "";
#endif

	CmdArgParser cmd;
	cmd.info(info);

	params_t params;
	params.exeDir = 0;
	params.exePath = 0;
	params.chainDir = 0;
	params.chainPath = 0;
	params.chainPathAbsolute = 0;
	params.inPath = 0;
	params.outPath = 0;
	params.logPath = 0;
	params.srcUrl = 0;
	params.step = 0;
	params.left = 0;
	params.right = 0;
	params.annoPath = 0;
	params.ascii = false;
	params.restClassName = 0;

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("chain", &params.chainPath, "path to file defining the processing chain");
	cmd.addSCmdArg("in", &params.inPath, "input path (if several separate by ;)");
	cmd.addSCmdArg("out", &params.outPath, "output path (if several separate by ;)");

	cmd.addText("\nOptions:");
	cmd.addBCmdOption("-ascii", &params.ascii, false, "store output as ASCII");
	cmd.addBCmdOption("-list", &params.list, false, "read files from list (one file per line)");
	cmd.addSCmdOption("-step", &params.step, "0", "set frame step (add ms/s for milli/seconds otherwise interpreted as number of samples)");
	cmd.addSCmdOption("-left", &params.left, "0", "set left context (see step)");
	cmd.addSCmdOption("-right", &params.right, "0", "set right context (see step)");
	cmd.addSCmdOption("-anno", &params.annoPath, "", "path to an annotation (features will be extracted over segments and stored as a sample list)");
	cmd.addSCmdOption("-rest", &params.restClassName, SSI_SAMPLE_REST_CLASS_NAME, "name of restclass (if empty not added)");
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

	delete[] params.exeDir;
	delete[] params.exePath;
	delete[] params.chainDir;
	delete[] params.chainPath;
	delete[] params.chainPathAbsolute;
	delete[] params.inPath;
	delete[] params.outPath;
	delete[] params.logPath;
	delete[] params.srcUrl;
	delete[] params.left;
	delete[] params.right;
	delete[] params.step;
	delete[] params.annoPath;
	delete[] params.restClassName;

	return true;
}

void GetDirectories(const ssi_char_t *exePath, params_t &params)
{
	// get directories
	ssi_char_t workDir[SSI_MAX_CHAR];
	FilePath exepath_fp(exePath);
	ssi_getcwd(SSI_MAX_CHAR, workDir);
	
	// set exe path	
	params.exePath = ssi_strcpy(exePath);
	if (exepath_fp.isRelative()) 
	{
#if _WIN32|_WIN64
		params.exeDir = ssi_strcat(workDir, "\\", exepath_fp.getDir());
#else
        params.exeDir = ssi_strcat(workDir, "/", exepath_fp.getDir());
#endif
	}
	else 
	{
		params.exeDir = ssi_strcpy(exepath_fp.getDir());
	}

	// download directories
	ssi_print("download source=%s\ndownload target=%s\n\n", params.srcUrl, params.exeDir);
	Factory::SetDownloadDirs(params.srcUrl, params.exeDir);

	// set chain path
	FilePath chainPath_fp(params.chainPath);
	if (chainPath_fp.isRelative()) 
	{
#if _WIN32|_WIN64
		params.chainPathAbsolute = ssi_strcat(workDir, "\\", params.chainPath);
#else
        params.chainPathAbsolute = ssi_strcat(workDir, "/", params.chainPath);
#endif
	}
	else
	{
		params.chainPathAbsolute = ssi_strcpy(params.chainPath);
	}
	params.chainDir = ssi_strcpy(FilePath(params.chainPathAbsolute).getDir());
}

bool IsVideoFile(const ssi_char_t *path)
{
	FilePath fp(path);
	return (ssi_strcmp(fp.getExtension(), ".mp4", false)
		|| ssi_strcmp(fp.getExtension(), ".avi", false));
}

bool IsAudioFile(const ssi_char_t *path)
{
	FilePath fp(path);
	return (ssi_strcmp(fp.getExtension(), ".wav", false)
		| ssi_strcmp(fp.getExtension(), ".aac", false)
		|| ssi_strcmp(fp.getExtension(), ".flac", false)
		|| ssi_strcmp(fp.getExtension(), ".mp3", false)
		|| ssi_strcmp(fp.getExtension(), ".ogg", false)
		|| ssi_strcmp(fp.getExtension(), ".opus", false)
		|| ssi_strcmp(fp.getExtension(), ".wma", false));
}

void ResolveDependencies(params_t &params)
{
	Factory::RegisterDLL("frame", ssiout, ssimsg);
	if (IsVideoFile(params.inList[0].str())
		|| IsAudioFile(params.inList[0].str()))
	{
		Factory::RegisterDLL("ioput", ssiout, ssimsg);
		Factory::RegisterDLL("ffmpeg", ssiout, ssimsg);

		if (params.srcUrl != 0 && params.srcUrl[0] != '\0')
		{
			ssi_char_t *depend[8] = {
				"avcodec-57.dll",
				"avdevice-57.dll",
				"avfilter-6.dll",
				"avformat-57.dll",
				"avutil-55.dll",
				"postproc-54.dll",
				"swresample-2.dll",
				"swscale-4.dll",
			};
			for (ssi_size_t i = 0; i < 8; i++)
            {
#if _WIN32|_WIN64
                ssi_char_t *dlldst = ssi_strcat(params.exeDir, "\\", depend[i]);
#else
                 ssi_char_t *dlldst = ssi_strcat(params.exeDir, "/", depend[i]);
#endif
                ssi_char_t *dllsrc = ssi_strcat(params.srcUrl, "/", depend[i]);
				if (!ssi_exists(dlldst))
				{
#if _WIN32||_WIN64
					WebTools::DownloadFile(dllsrc, dlldst);
#endif
				}
				delete[] dlldst;
				delete[] dllsrc;
			}
		}
	}
}

void SplitFiles(const ssi_char_t *string, StringList &files, bool list)
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
	GetDirectories(exePath, params);

	// set working directory to chain directory		
	ssi_setcwd(params.chainDir);

	// fill lists
	SplitFiles(params.inPath, params.inList, params.list);
	SplitFiles(params.outPath, params.outList, params.list);
	SplitFiles(params.annoPath, params.annoList, params.list);

	if (params.inList.size() != params.outList.size())
	{
		ssi_wrn("number of input files differs from number of output files");
		return;
	}

	if (params.annoList.size() > 0 && params.annoList.size() != params.inList.size())
	{
		ssi_wrn("number of annotation files differs from number of input/output files");
		return;
	}

	// start processing

	ssi_size_t nFiles = (ssi_size_t)params.inList.size();
	ssi_size_t nThreads = params.nParallel <= 0 ? nFiles : params.nParallel;

	if (nFiles != 0)
	{		
		// register dependencies
		ResolveDependencies(params);		

		if (nFiles == 1 || nThreads == 1)
		{
			for (ssi_size_t n = 0; n < nFiles; n++)
			{
				FilePath inPath(params.inList[n].str());
				FilePath outPath(params.outList[n].str());
				if (params.annoList.size() > 0)
				{
					FilePath annoPath(params.annoList[n].str());
					Extract(params, &inPath, &outPath, &annoPath);
				}
				else
				{
					Extract(params, &inPath, &outPath);
				}
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

	}
	
	Factory::Clear();
}

String paramsToArgs(params_t *params, ssi_size_t n)
{
	ssi_char_t *inPath = params->inList[n].str();
	ssi_char_t *outPath = params->outList[n].str();
	ssi_char_t *annoPath = 0;
	if (params->annoList.size() > 0)
	{
		annoPath = params->annoList[n].str();
	}

	ssi_char_t logPath[SSI_MAX_CHAR];
	ssi_sprint(logPath, "%s.%04u", params->logPath, n);

	String string = String("") +
		"-step " + params->step +
		" -left " + params->left +
		" -right " + params->right +
		" -anno \"" + (annoPath ? annoPath : "") + "\"" +
		" -url \"\"" +
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
}

bool Extract(params_t &params, FilePath *inPath, FilePath *outPath, FilePath *annoPath)
{	
	ssi_char_t *toPath = 0;
	if (ssi_strcmp(outPath->getExtension(), annoPath ? SSI_FILE_TYPE_SAMPLES : SSI_FILE_TYPE_STREAM, false))
	{
		toPath = ssi_strcpy(outPath->getPath());
	}
	else
	{
		toPath = ssi_strcpy(outPath->getPathFull());
	}

	Chain *chain = ssi_create_id(Chain, 0, "chain");
	chain->getOptions()->set(params.chainPathAbsolute);	

	ssi_stream_t from;
	bool result = false;

	bool isVideoFile = IsVideoFile(inPath->getNameFull());
	bool isAudioFile = IsAudioFile(inPath->getNameFull());

	if (isVideoFile)
	{				
		if (annoPath)
		{
			ssi_wrn("cannot extract video features for an annotation");
			return false;
		}

		FFMPEGReader *reader = ssi_create(FFMPEGReader, 0, false);
		reader->getOptions()->setUrl(inPath->getPathFull());
		reader->getOptions()->bestEffort = true;

		FileWriter *writer = ssi_create(FileWriter, 0, false);
		writer->getOptions()->overwrite = true;
		writer->getOptions()->setPath(toPath);
		writer->getOptions()->type = File::BINARY;
		
		FileProvider *provider = new FileProvider(writer, chain);
		reader->setProvider(SSI_FFMPEGREADER_VIDEO_PROVIDER_NAME, provider);

		reader->connect();
		reader->start();
		reader->wait();		
		reader->stop();
		reader->disconnect();
		
		delete provider;
		delete reader;
		delete writer;
	}
	else if (isAudioFile)
	{
		FFMPEGReader *reader = ssi_create(FFMPEGReader, 0, false);
		reader->getOptions()->setUrl(inPath->getPathFull());
		reader->getOptions()->ablock = 0.05;
		reader->getOptions()->bestEffort = true;

		if (!reader->initAudioStream(inPath->getPathFull(), from)
			|| from.num == 0)
		{			
			return false;
		}

		MemoryWriter *writer = ssi_create(MemoryWriter, 0, false);
		writer->setStream(from);		

		FileProvider *provider = new FileProvider(writer);
		reader->setProvider(SSI_FFMPEGREADER_AUDIO_PROVIDER_NAME, provider);

		reader->connect();
		reader->start();
		reader->wait();
		reader->stop();
		reader->disconnect();

		delete provider;
		delete reader;
		delete writer;

 		result = true;
	}
	else
	{
		result = FileTools::ReadStreamFile(inPath->getPathFull(), from);
	}

	if (result)
	{
		ssi_stream_t to;

		if (annoPath)
		{			
			Annotation annotation;
			if (result &= annotation.load(annoPath->getPathFull()))
			{
				if (annotation.getScheme()->type != SSI_SCHEME_TYPE::DISCRETE)
				{
					ssi_wrn("cannot extract features from a continuous annotation");
					return false;
				}

				if (!ssi_strcmp(params.step, "0")) {
					ssi_size_t step = 0, left = 0, right = 0;

					if (!ssi_parse_samples(params.step, step, from.sr)) {
						ssi_wrn("could not parse step size '%s'", params.step);
						return false;
					}

					if (!ssi_parse_samples(params.left, left, from.sr)) {
						ssi_wrn("could not parse left size '%s'", params.left);
						return false;
					}

					if (!ssi_parse_samples(params.right, right, from.sr)) {
						ssi_wrn("could not parse right size '%s'", params.right);
						return false;
					}

					ssi_time_t step_t = step / from.sr;
					ssi_time_t left_t = left / from.sr;
					ssi_time_t right_t = right / from.sr;
					
					annotation.convertToFrames(step_t, params.restClassName[0] == '\0' ? 0 : params.restClassName);

					if (left_t > 0.0 || right_t > 0.0)
					{
						annotation.addOffset(left_t, right_t);
					}
				}

				SampleList samples;

				if (result &= annotation.extractSamples(from, &samples))
				{
					ISTransform samples_t(&samples);
					samples_t.setTransformer(0, *chain); 
					samples_t.callEnter();
					result &= ModelTools::SaveSampleList(samples_t, toPath, params.ascii ? File::ASCII : File::BINARY);
					samples_t.callFlush();
				}
			}			
		}
		else
		{
			if (ssi_strcmp(params.step, "0"))
			{
				SignalTools::Transform(from, to, *chain, 0u);
			}
			else
			{
				SignalTools::Transform(from, to, *chain, params.step, params.left, params.right);
			}

			result &= FileTools::WriteStreamFile(params.ascii ? File::ASCII : File::BINARY, toPath, to);
			
			ssi_stream_destroy(to);
		}

		ssi_stream_destroy(from);
	}

	delete[] toPath;

	return result;
}
