// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/10/24
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
#include "ssimongo/include/ssimongo.h"
#include "ssicml/include/ssicml.h"
#include "liblinear/include/ssiliblinear.h"
#include "opensmile/include/ssiopensmile.h"
#include "signal/include/ssisignal.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

struct params_t
{
	ssi_char_t *root;
	ssi_char_t *server;
	int port;
	ssi_char_t *database;
	ssi_char_t *username;
	ssi_char_t *password;
	ssi_char_t *annotation;
	ssi_char_t *annotation_new;
	ssi_char_t *scheme;
	ssi_char_t *annotator;
	ssi_char_t *annotator_new;
	ssi_char_t *role;
	ssi_char_t *dlls;
	ssi_char_t *srcurl;
	ssi_char_t *logpath;
	ssi_char_t *filter;
	ssi_char_t *list;
	ssi_char_t *classname;
	ssi_char_t *classname_new;
	ssi_char_t *stream;
	ssi_char_t *streamOut;
	ssi_char_t *trainerTmp;
	ssi_char_t *trainer;
	ssi_char_t *evalpath;
	//ssi_time_t frame;
	bool cooperative;
	ssi_char_t *balance;
	int contextLeft;
	int contextRight;
	bool finished;
	bool locked;
	double confidence;
	double label_mingap;
	double label_mindur;
	bool force;
	bool loso;
	int scoreDim;
	int confDim;
	ssi_time_t sample_rate;
	ssi_time_t duration;
};

void getSessions(StringList &list, params_t &params);

void loadDlls(params_t &params);
bool readCredentials(params_t &params);
void uploadAnnotations(params_t &params);
void downloadAnnotations(params_t &params);
void removeAnnotations(params_t &params);
void cutStreamFromLabel(params_t &params);
void mapClassNames(params_t &params);
void convertStreamToAnnotation(params_t &params);
void convertAnnotationToStream(params_t &params);
void train(params_t &params);
void eval(params_t &params);
void forward(params_t &params);
void merge(params_t &params);

int main (int argc, char **argv) {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	char info[1024];
	ssi_sprint (info, "\n%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

#if !_DEBUG && defined _MSC_VER && _MSC_VER >= 1900	
	const ssi_char_t *default_source = "https://github.com/hcmlab/ssi/raw/master/bin/x64/vc140";
#else
	const ssi_char_t *default_source = "";
#endif

	CmdArgParser cmd;
	cmd.info (info);

	params_t params;
	params.root = 0;
	params.server = 0;
	params.username = 0;
	params.password = 0;
	params.database = 0;
	params.annotation = 0;
	params.annotation_new = 0;
	params.scheme = 0;
	params.annotator = 0;
	params.annotator_new = 0;
	params.role = 0;
	params.dlls = 0;
	params.srcurl = 0;
	params.logpath = 0;
	params.filter = 0;
	params.list = 0;
	params.classname = 0;
	params.classname_new = 0;
	params.stream = 0;
	params.streamOut = 0;
	params.trainerTmp = 0;
	params.trainer = 0;
	params.evalpath = 0;
	params.finished = false;
	params.locked = false;
	params.loso = false;
	params.confidence = -1.0;
	params.label_mingap = 0;
	params.label_mindur = 0;
	params.contextLeft = 0;	
	params.contextRight = 0;
	params.balance = 0;
	params.scoreDim = 0;
	params.confDim = -1;

	cmd.addMasterSwitch("--remove");

	cmd.addText("\nArguments:");	
	cmd.addSCmdArg("root", &params.root, "path to database on disk");
	cmd.addSCmdArg("server", &params.server, "server name");
	cmd.addICmdArg("port", &params.port, "port number");
	cmd.addSCmdArg("database", &params.database, "name of database");
	cmd.addSCmdArg("role", &params.role, "name of role");	
	cmd.addSCmdArg("scheme", &params.scheme, "name of scheme");
	cmd.addSCmdArg("annotator", &params.annotator, "name of annotator");
	
	cmd.addText("\nOptions:");
	cmd.addSCmdOption("-filter", &params.filter, "*", "session filter (e.g. *location)");
	cmd.addSCmdOption("-list", &params.list, "", "list with sessions separated by ; (overrides filter)");
	cmd.addSCmdOption("-username", &params.username, "", "database username");
	cmd.addSCmdOption("-password", &params.password, "", "database password");	
	cmd.addSCmdOption("-url", &params.srcurl, default_source, "override default url for downloading missing dlls and dependencies");
	cmd.addSCmdOption("-log", &params.logpath, "", "output to log file");

	cmd.addMasterSwitch("--upload");

	cmd.addText("\nArguments:");	
	cmd.addSCmdArg("root", &params.root, "path to database on disk");
	cmd.addSCmdArg("server", &params.server, "server name");
	cmd.addICmdArg("port", &params.port, "port number");	
	cmd.addSCmdArg("database", &params.database, "name of database");
	cmd.addSCmdArg("annotation", &params.annotation, "name of annotation");
	
	cmd.addText("\nOptions:");
	cmd.addSCmdOption("-annotator", &params.annotator, "", "set a fixed annotator (otherwise read from meta data)");
	cmd.addSCmdOption("-role", &params.role, "", "set a fixed role (otherwise read from meta data)");
	cmd.addBCmdOption("-finished", &params.finished, false, "annotation will be marked as finished");
	cmd.addBCmdOption("-locked", &params.locked, false, "annotation will be marked as locked");
	cmd.addBCmdOption("-force", &params.force, false, "overwrite locked annotations");
	cmd.addDCmdOption("-confidence", &params.confidence, -1.0, "force confidence value (applied if >= 0)");
	cmd.addSCmdOption("-filter", &params.filter, "*", "session filter (e.g. *location)");
	cmd.addSCmdOption("-list", &params.list, "", "list with sessions separated by ; (overrides filter)");
	cmd.addSCmdOption("-username", &params.username, "", "database username");
	cmd.addSCmdOption("-password", &params.password, "", "database password");
	cmd.addSCmdOption("-url", &params.srcurl, default_source, "override default url for downloading missing dlls and dependencies");
	cmd.addSCmdOption("-log", &params.logpath, "", "output to log file");

	cmd.addMasterSwitch("--download");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("root", &params.root, "path to database on disk");
	cmd.addSCmdArg("server", &params.server, "server name");
	cmd.addICmdArg("port", &params.port, "port number");
	cmd.addSCmdArg("database", &params.database, "name of database");	
	cmd.addSCmdArg("role", &params.role, "name of role");
	cmd.addSCmdArg("scheme", &params.scheme, "name of scheme");	
	cmd.addSCmdArg("annotator", &params.annotator, "name of annotator");

	cmd.addText("\nOptions:");	
	cmd.addSCmdOption("-filter", &params.filter, "*", "session filter (e.g. *location)");
	cmd.addSCmdOption("-username", &params.username, "", "database username");
	cmd.addSCmdOption("-password", &params.password, "", "database password");
	cmd.addSCmdOption("-url", &params.srcurl, default_source, "override default url for downloading missing dlls and dependencies");
	cmd.addSCmdOption("-log", &params.logpath, "", "output to log file");

	cmd.addMasterSwitch("--cut");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("root", &params.root, "path to database on disk");
	cmd.addSCmdArg("annotation", &params.annotation, "name of annotation");
	cmd.addSCmdArg("classname", &params.classname, "name of class to extract");
	cmd.addSCmdArg("stream", &params.stream, "name of stream");

	cmd.addText("\nOptions:");
	cmd.addSCmdOption("-filter", &params.filter, "*", "session filter (e.g. *location)");
	cmd.addSCmdOption("-url", &params.srcurl, default_source, "override default url for downloading missing dlls and dependencies");
	cmd.addSCmdOption("-log", &params.logpath, "", "output to log file");

	cmd.addMasterSwitch("--convert-stream-to-anno");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("root", &params.root, "path to database on disk");
	cmd.addSCmdArg("stream", &params.stream, "name of stream");
	cmd.addSCmdArg("scheme", &params.scheme, "path to scheme file");
	cmd.addSCmdArg("annotation", &params.annotation, "name of annotation (will be generated)");

	cmd.addText("\nOptions:");
	cmd.addICmdOption("-scoreDim", &params.scoreDim, 0, "dimension of score values");
	cmd.addICmdOption("-confDim", &params.confDim, -1, "dimension of confidence values (if -1 set to 1.0)");
	cmd.addSCmdOption("-filter", &params.filter, "*", "session filter (e.g. *location)");
	cmd.addSCmdOption("-url", &params.srcurl, default_source, "override default url for downloading missing dlls and dependencies");
	cmd.addSCmdOption("-log", &params.logpath, "", "output to log file");

	cmd.addMasterSwitch("--convert-anno-to-stream");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("root", &params.root, "path to database on disk");	
	cmd.addSCmdArg("annotation", &params.annotation, "name of annotation");
	cmd.addSCmdArg("stream", &params.stream, "name of stream (will be generated)");

	cmd.addText("\nOptions:");	
	cmd.addDCmdOption("-sr", &params.sample_rate, 1.0, "sample rate in Hz (discrete only)");
	cmd.addDCmdOption("-duration", &params.duration, 0.0, "duration in seconds (discrete only)");
	cmd.addSCmdOption("-filter", &params.filter, "*", "session filter (e.g. *location)");
	cmd.addSCmdOption("-url", &params.srcurl, default_source, "override default url for downloading missing dlls and dependencies");
	cmd.addSCmdOption("-log", &params.logpath, "", "output to log file");

	cmd.addMasterSwitch("--map");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("root", &params.root, "path to database on disk");
	cmd.addSCmdArg("annotation", &params.annotation, "name of annotation");
	cmd.addSCmdArg("newAnnotation", &params.annotation_new, "name of new annotation");
	cmd.addSCmdArg("from", &params.classname, "original class names (discrete) , or upper thresholds (continuous) (separated by ;)");
	cmd.addSCmdArg("to", &params.classname_new, "new class names (separated by ;)");

	cmd.addText("\nOptions:");
	cmd.addSCmdOption("-scheme", &params.scheme, "", "scheme name");
	cmd.addSCmdOption("-filter", &params.filter, "*", "session filter (e.g. *location)");
	cmd.addSCmdOption("-url", &params.srcurl, default_source, "override default url for downloading missing dlls and dependencies");
	cmd.addSCmdOption("-log", &params.logpath, "", "output to log file");


	cmd.addMasterSwitch("--train");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("root", &params.root, "path to database on disk");
	cmd.addSCmdArg("server", &params.server, "server name");
	cmd.addICmdArg("port", &params.port, "port number");
	cmd.addSCmdArg("database", &params.database, "name of database");
	cmd.addSCmdArg("role", &params.role, "name of role (if several separate by ;)");
	cmd.addSCmdArg("scheme", &params.scheme, "name of scheme");
	cmd.addSCmdArg("annotator", &params.annotator, "name of annotator");
	cmd.addSCmdArg("stream", &params.stream, "name of stream");	
	cmd.addSCmdArg("template", &params.trainerTmp, "trainer template path");
	cmd.addSCmdArg("trainer", &params.trainer, "trainer path");

	cmd.addText("\nOptions:");	
	cmd.addSCmdOption("-filter", &params.filter, "*", "session filter (e.g. *location)");
	cmd.addSCmdOption("-list", &params.list, "", "list with sessions separated by ; (overrides filter)");
	cmd.addSCmdOption("-username", &params.username, "", "database username");
	cmd.addSCmdOption("-password", &params.password, "", "database password");
	cmd.addICmdOption("-left", &params.contextLeft, 0, "left context (number of frames added to the left of center frame)");
	cmd.addICmdOption("-right", &params.contextRight, 0, "right context (number of frames added to the right of center frame)");
	cmd.addSCmdOption("-balance", &params.balance, "none", "set sample balancing strategy (none,under,over)");
	cmd.addBCmdOption("-cooperative", &params.cooperative, false, "turn on cooperative learning");	
	cmd.addSCmdOption("-dlls", &params.dlls, "", "list of requird dlls separated by ';' [deprecated, use register tag in trainer]");
	cmd.addSCmdOption("-url", &params.srcurl, default_source, "override default url for downloading missing dlls and dependencies");
	cmd.addSCmdOption("-log", &params.logpath, "", "output to log file");

	cmd.addMasterSwitch("--eval");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("root", &params.root, "path to database on disk");
	cmd.addSCmdArg("server", &params.server, "server name");
	cmd.addICmdArg("port", &params.port, "port number");
	cmd.addSCmdArg("database", &params.database, "name of database");
	cmd.addSCmdArg("role", &params.role, "name of role (if several separate by ;)");
	cmd.addSCmdArg("scheme", &params.scheme, "name of scheme");
	cmd.addSCmdArg("annotator", &params.annotator, "name of annotator");
	cmd.addSCmdArg("stream", &params.stream, "name of stream");	
	cmd.addSCmdArg("trainer", &params.trainer, "trainer path");
	cmd.addSCmdArg("result", &params.evalpath, "result path");

	cmd.addText("\nOptions:");
	cmd.addSCmdOption("-filter", &params.filter, "*", "session filter (e.g. *location)");
	cmd.addSCmdOption("-list", &params.list, "", "list with sessions separated by ; (overrides filter)");
	cmd.addSCmdOption("-username", &params.username, "", "database username");
	cmd.addSCmdOption("-password", &params.password, "", "database password");;
	cmd.addSCmdOption("-balance", &params.balance, "none", "set sample balancing strategy (none,under,over)");
	cmd.addBCmdOption("-cooperative", &params.cooperative, false, "turn on cooperative learning");
	cmd.addSCmdOption("-dlls", &params.dlls, "", "list of requird dlls separated by ';' [deprecated, use register tag in trainer]");
	cmd.addSCmdOption("-url", &params.srcurl, default_source, "override default url for downloading missing dlls and dependencies");
	cmd.addSCmdOption("-log", &params.logpath, "", "output to log file");
	cmd.addBCmdOption("-loso", &params.loso, false, "apply leave-one-session-out cross-validation");

	cmd.addMasterSwitch("--forward");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("root", &params.root, "path to database on disk");
	cmd.addSCmdArg("server", &params.server, "server name");
	cmd.addICmdArg("port", &params.port, "port number");
	cmd.addSCmdArg("database", &params.database, "name of database");
	cmd.addSCmdArg("role", &params.role, "name of role (if several separate by ;)");
	cmd.addSCmdArg("scheme", &params.scheme, "name of scheme");
	cmd.addSCmdArg("annotator", &params.annotator, "name of annotator");
	cmd.addSCmdArg("stream", &params.stream, "name of stream");
	cmd.addSCmdArg("trainer", &params.trainer, "trainer path");

	cmd.addText("\nOptions:");
	cmd.addSCmdOption("-filter", &params.filter, "*", "session filter (e.g. *location)");
	cmd.addSCmdOption("-list", &params.list, "", "list with sessions separated by ; (overrides filter)");
	cmd.addSCmdOption("-username", &params.username, "", "database username");
	cmd.addSCmdOption("-password", &params.password, "", "database password");
	cmd.addBCmdOption("-finished", &params.finished, false, "annotation will be marked as finished");
	cmd.addBCmdOption("-locked", &params.locked, false, "annotation will be marked as locked");
	cmd.addICmdOption("-left", &params.contextLeft, 0, "left context (number of frames added to the left of center frame)");
	cmd.addICmdOption("-right", &params.contextRight, 0, "right context (number of frames added to the right of center frame)");
	cmd.addSCmdOption("-assign", &params.annotator_new, "", "assign a different annotator");
	cmd.addDCmdOption("-confidence", &params.confidence, -1.0, "force confidence value (applied if >= 0)");
	cmd.addDCmdOption("-mingap", &params.label_mingap, 0, "gaps between labels with same name that are smaller than this value will be closed");
	cmd.addDCmdOption("-mindur", &params.label_mindur, 0, "labels with a duration smaller or equal to this value will be removed");
	cmd.addBCmdOption("-cooperative", &params.cooperative, false, "turn on cooperative learning");
	cmd.addSCmdOption("-dlls", &params.dlls, "", "list of requird dlls separated by ';' [deprecated, use register tag in trainer]");
	cmd.addSCmdOption("-url", &params.srcurl, default_source, "override default url for downloading missing dlls and dependencies");
	cmd.addSCmdOption("-log", &params.logpath, "", "output to log file");

	cmd.addMasterSwitch("--merge");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("root", &params.root, "path to database on disk");
	cmd.addSCmdArg("role", &params.role, "name of role (if several separate by ;)");
	cmd.addSCmdArg("input", &params.stream, "name of streams separated by ;");
	cmd.addSCmdArg("output", &params.streamOut, "name of output stream");

	cmd.addText("\nOptions:");
	cmd.addSCmdOption("-filter", &params.filter, "*", "session filter (e.g. *location)");
	cmd.addSCmdOption("-list", &params.list, "", "list with sessions separated by ; (overrides filter)");
	cmd.addBCmdOption("-force", &params.force, false, "overwrite existing files");
	cmd.addSCmdOption("-url", &params.srcurl, default_source, "override default url for downloading missing dlls and dependencies");
	cmd.addSCmdOption("-log", &params.logpath, "", "output to log file");
	
	if (cmd.read (argc, argv)) {		

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
		ssi_print("download source=%s\ndownload target=%s\n\n", params.srcurl, exedir);
		Factory::SetDownloadDirs(params.srcurl, exedir);
		
		if (params.srcurl != 0 && params.srcurl[0] != '\0')
		{
			ssi_char_t *depend[2] = { "libbson-1.0.dll", "libmongoc-1.0.dll" };
			for (ssi_size_t i = 0; i < 2; i++)
			{
				ssi_char_t *dlldst = ssi_strcat(exedir, "\\", depend[i]);
				ssi_char_t *dllsrc = ssi_strcat(params.srcurl, "/", depend[i]);
				if (!ssi_exists(dlldst))
				{
					WebTools::DownloadFile(dllsrc, dlldst);
				}
				delete[] dlldst;
				delete[] dllsrc;
			}
		}

		if (params.logpath && params.logpath[0] != '\0') {
			ssimsg = new FileMessage(params.logpath);
		}

		if (params.stream != 0)
		{
			FilePath stream_fp(params.stream);
			if (ssi_strcmp(stream_fp.getExtension(), SSI_FILE_TYPE_STREAM, false))
			{
				delete[] params.stream;
				params.stream = ssi_strcpy(stream_fp.getPath());
			}			
		}

		loadDlls(params);		

		switch (cmd.master_switch) {

		case 1: {

			if (readCredentials(params))
			{
				removeAnnotations(params);
			}

			break;
		}

		case 2: {

			if (readCredentials(params))
			{
				uploadAnnotations(params);
			}

			break;
		}

		case 3: {

			if (readCredentials(params))
			{
				downloadAnnotations(params);
			}

			break;
		}

		case 4: {

			cutStreamFromLabel(params);

			break;
		}

		case 5: {

			convertStreamToAnnotation(params);

			break;
		}

		case 6: {

			convertAnnotationToStream(params);

			break;
		}

		case 7: {

			mapClassNames(params);
			
			break;
		}

		case 8: {

			train(params);

			break;
		}

		case 9: {

			eval(params);

			break;
		}

		case 10: {

			forward(params);

			break;
		}

		case 11: {

			merge(params);

			break;
		}

		}

		if (params.logpath && params.logpath[0] != '\0') {
			delete ssimsg; ssimsg = 0;
		}

		Factory::Clear ();
	}

	delete[] params.root;	
	delete[] params.server;
	delete[] params.database;
	delete[] params.username;
	delete[] params.password;	
	delete[] params.annotation;
	delete[] params.annotation_new;
	delete[] params.annotator;
	delete[] params.annotator_new;
	delete[] params.scheme;
	delete[] params.role;
	delete[] params.dlls;
	delete[] params.srcurl;
	delete[] params.logpath;
	delete[] params.filter;
	delete[] params.list;
	delete[] params.stream;
	delete[] params.streamOut;
	delete[] params.classname; 
	delete[] params.classname_new;
	delete[] params.trainerTmp;
	delete[] params.trainer;
	delete[] params.evalpath;
	delete[] params.balance;

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

void getSessions(StringList &list, params_t &params)
{
	ssi_char_t string[SSI_MAX_CHAR];

	if (params.list != 0 && params.list[0] != '\0')
	{
		StringList sessions;
		sessions.parse(params.list, ';');
		for (StringList::iterator it = sessions.begin(); it != sessions.end(); it++)
		{
			ssi_sprint(string, "%s\\%s", params.root, it->str());
			list.add(string);
		}
	}		
	else
	{
		FileTools::ReadDirsFromDir(list, params.root, params.filter);
	}

	list.remove("models");
}

void loadDlls(params_t &params)
{
	if (!params.dlls || params.dlls[0] == '\0')
	{
		return;
	}

	ssi_size_t n = ssi_split_string_count(params.dlls, ';');
	if (n > 0) {
		ssi_char_t **tokens = new ssi_char_t *[n];
		ssi_split_string(n, tokens, params.dlls, ';');
		for (ssi_size_t i = 0; i < n; i++) {
			Factory::RegisterDLL(tokens[i]);
			delete[] tokens[i];
		}
		delete[] tokens;
	}
}

bool readCredentials(params_t &params)
{
	char c, password[SSI_MAX_CHAR], username[SSI_MAX_CHAR];
	int i = 0;

	if (params.username[0] == '\0')
	{
		printf("username: ");
		while ((c = getch()) != '\r') {
			username[i++] = c;
			printf("%c", c);
		}
		username[i] = '\0';
		params.username = ssi_strcpy(username);
	}

	if (params.password[0] == '\0')
	{
		printf("\npassword: ");
		i = 0;
		while ((c = getch()) != '\r') {
			password[i++] = c;
			printf("*");
		}
		password[i] = '\0';
		printf("\n");
		params.password = ssi_strcpy(password);
	}

	return true;
}

void uploadAnnotations(params_t &params)
{
	MongoURI uri(params.server, params.port, params.username, params.password);
	MongoClient client;
	if (!client.connect(uri, params.database, false, 1000))
	{
		return;
	}

	ssi_char_t string[SSI_MAX_CHAR];
	StringList sessions;
	getSessions(sessions, params);
	for (StringList::iterator it = sessions.begin(); it != sessions.end(); it++)
	{
		ssi_sprint(string, "%s\\%s.annotation", it->str(), params.annotation);

		ssi_print("\n-------------------------------------------\n");
		ssi_print("UPLOAD ANNOTATION TO DATABASE '%s'\n\n", string);

		if (ssi_exists(string))
		{
			Annotation anno;
			if (!anno.load(string))
			{
				ssi_wrn("ERROR: could not load annotation");				
				continue;
			}

			FilePath path(it->str());
			const ssi_char_t *session = path.getName();
			const ssi_char_t *annotator = params.annotator[0] == '\0' ? anno.getMeta("annotator") : params.annotator;
			const ssi_char_t *role = params.role[0] == '\0' ? anno.getMeta("role") : params.role;

			if (!session || !annotator || !role)
			{
				ssi_wrn("ERROR: invalid meta information (session=%s,annotator=%s,role=%s)", session, annotator, role);
				continue;
			}


			if (params.confidence >= 0)
			{
				anno.setConfidence((ssi_real_t)params.confidence);
			}

			if (!CMLAnnotation::Save(&anno, &client, session, role, anno.getScheme()->name, annotator, params.finished, params.locked, params.force))
			{		
				ssi_wrn("could not upload annotation to database");
				continue;
			}
		}
		else
		{
			ssi_wrn("annotation file not found '%s'", string);
		}
	}
}

void removeAnnotations(params_t &params)
{
	MongoURI uri(params.server, params.port, params.username, params.password);
	MongoClient client;
	if (!client.connect(uri, params.database, false, 1000))
	{
		return;
	}

	StringList sessions;
	getSessions(sessions, params);
	for (StringList::iterator it = sessions.begin(); it != sessions.end(); it++)
	{
		ssi_print("\n-------------------------------------------\n");
		ssi_print("REMOVE ANNOTATION FROM DATABASE '%s'\n\n", it->str());

		FilePath path(it->str());
		const ssi_char_t *session = path.getName();
		if (!CMLAnnotation::Remove(&client, session, params.role, params.scheme, params.annotator))
		{
			ssi_wrn("ERROR: failed to remove annotation from database");
			continue;
		}
	}
}

void downloadAnnotations(params_t &params)
{
	MongoURI uri(params.server, params.port, params.username, params.password);
	MongoClient client;
	if (!client.connect(uri, params.database, false, 1000))
	{
		return;
	}

	ssi_char_t string[SSI_MAX_CHAR];
	StringList sessions;
	getSessions(sessions, params);
	for (StringList::iterator it = sessions.begin(); it != sessions.end(); it++)
	{		
		FilePath path(it->str());
		const ssi_char_t *session = path.getName();

		ssi_print("\n-------------------------------------------\n");
		ssi_print("DOWNLOAD ANNOTATION FROM DATABASE '%s.%s.%s.%s'\n\n", session, params.role, params.scheme, params.annotator);

		Annotation anno;
			
		if (!CMLAnnotation::Load(&anno, &client, session, params.role, params.scheme, params.annotator))
		{
			ssi_wrn("ERROR: could not load annotation from database");
			continue;
		}			

		ssi_sprint(string, "%s\\%s.%s.%s", it->str(), params.role, params.scheme, params.annotator);
		if (!anno.save(string, File::ASCII))
		{
			ssi_wrn("ERROR: could not save annotation to file");
			continue;
		}
	}
}

void cutStreamFromLabel(params_t &params)
{
	ssi_char_t string[SSI_MAX_CHAR];
	StringList sessions;
	getSessions(sessions, params);
	for (StringList::iterator it = sessions.begin(); it != sessions.end(); it++)
	{
		ssi_sprint(string, "%s\\%s.annotation", it->str(), params.annotation);

		ssi_print("\n-------------------------------------------\n");
		ssi_print("EXTRACT LABEL FROM STREAM '%s:%s>%s'\n\n", string, params.classname, params.stream);

		if (ssi_exists(string))
		{
			Annotation anno;
			if (!anno.load(string))
			{
				ssi_wrn("ERROR: could not load annotation from file");
				continue;
			}

			if (anno.getScheme()->type != SSI_SCHEME_TYPE::DISCRETE)
			{
				ssi_wrn("ERROR: only discrete schemes are supported");
				continue;
			}

			ssi_sprint(string, "%s\\%s", it->str(), params.stream);
			if (ssi_exists(string))
			{
				FilePath path(string);
				bool is_wav = false;
				ssi_stream_t from;
				if (ssi_strcmp(path.getExtension(), ".wav", false))
				{
					WavTools::ReadWavFile(string, from, false);
					is_wav = true;
				}
				else if (ssi_strcmp(path.getExtension(), ".stream", false))
				{
					FileTools::ReadStreamFile(string, from);
				}
				else
				{
					ssi_wrn("ERROR: stream type not supported");
					continue;
				}

				if (!anno.keepClass(params.classname))
				{
					ssi_wrn("ERROR: could not remove labels");
					continue;
				}

				if (anno.size() == 0)
				{
					ssi_wrn("ERROR: no label with that class name");
					continue;
				}

				ssi_stream_t to;
				if (!anno.extractStream(from, to))
				{
					ssi_wrn("ERROR: could not extract stream");
					continue;
				}

				if (is_wav)
				{
					ssi_sprint(string, "%s.%s.%s.wav", path.getPath(), params.annotation, params.classname);
					if (!WavTools::WriteWavFile(string, to))
					{
						ssi_wrn("ERROR: could not extract stream");
						continue;
					}
				}
				else
				{
					ssi_sprint(string, "%s.%s.%s", path.getPath(), params.annotation, params.classname);
					if (!FileTools::WriteStreamFile(File::BINARY, string, to))
					{
						ssi_wrn("ERROR: could not extract stream");
						continue;
					}
				}

				ssi_stream_destroy(from);
				ssi_stream_destroy(to);
			}
			else
			{
				ssi_wrn("ERROR: stream file not found");
				continue;
			}
		}
		else
		{
			ssi_wrn("ERROR: annotation file not found");
			continue;
		}
	}
}

void convertStreamToAnnotation(params_t &params)
{
	ssi_char_t string[SSI_MAX_CHAR];
	
	StringList sessions;
	getSessions(sessions, params);
	for (StringList::iterator it = sessions.begin(); it != sessions.end(); it++)
	{
		ssi_sprint(string, "%s\\%s.stream", it->str(), params.stream);

		ssi_print("\n-------------------------------------------\n");
		ssi_print("CONVERT STREAM TO ANNOTATION '%s:%s>%s'\n\n", string, params.scheme, params.annotation);

		if (ssi_exists(string))
		{
			ssi_stream_t stream;
			if (!FileTools::ReadStreamFile(string, stream))
			{
				ssi_wrn("ERROR: could not load stream from file");
				continue;
			}

			Annotation anno;			
			if (!anno.loadScheme(params.scheme))
			{
				ssi_wrn("ERROR: could not load scheme from file");
				continue;
			}

			if (anno.getScheme()->type != SSI_SCHEME_TYPE::CONTINUOUS)
			{
				ssi_wrn("ERROR: only continuous schemes are supported");
				continue;
			}

			if (int(stream.dim) <= params.scoreDim || int(stream.dim) <= params.confDim)
			{
				ssi_wrn("ERROR: invalid dimension");
				continue;
			}

			anno.addStream(stream, params.scoreDim, params.confDim >= 0 ? params.confDim : 1.0f);

			ssi_sprint(string, "%s\\%s.annotation", it->str(), params.annotation);			
			anno.save(string, File::ASCII);
		}
		else
		{
			ssi_wrn("ERROR: stream file not found");
			continue;
		}
	}
}

void convertAnnotationToStream(params_t &params)
{
	ssi_char_t string[SSI_MAX_CHAR];

	StringList sessions;
	getSessions(sessions, params);
	for (StringList::iterator it = sessions.begin(); it != sessions.end(); it++)
	{
		ssi_sprint(string, "%s\\%s.annotation", it->str(), params.annotation);
		

		ssi_print("\n-------------------------------------------\n");
		ssi_print("CONVERT ANNOTATION TO STREAM '%s>%s'\n\n", string, params.stream);

		if (ssi_exists(string))
		{
			Annotation anno;
			if (!anno.load(string))
			{
				ssi_wrn("ERROR: could not load annotation from file");
				continue;
			}

			ssi_stream_t stream;
			
			if (!anno.convertToStream(stream, params.sample_rate, params.duration))
			{
				continue;
			}			

			ssi_sprint(string, "%s\\%s.stream", it->str(), params.stream);
			FileTools::WriteStreamFile(File::BINARY, string, stream);

			ssi_stream_destroy(stream);
		}
		else
		{
			ssi_wrn("ERROR: annotation file not found");
			continue;
		}
	}
}

void mapClassNames(params_t &params)
{
	ssi_char_t string[SSI_MAX_CHAR];

	StringList from;
	from.parse(params.classname, ';');
	StringList to;
	to.parse(params.classname_new, ';');
	StringList to_unique;
	for (StringList::iterator it = to.begin(); it != to.end(); it++)
	{
		if (std::find(to_unique.begin(), to_unique.end(), *it) == to_unique.end())
		{
			to_unique.push_back(*it);
		}
	}

	if (from.size() != to.size())
	{
		ssi_wrn("ERROR: number of classes differ");
		return;
	}

	std::map<String, String> mapping;
	for (ssi_size_t i = 0; i < from.size(); i++)
	{
		mapping[from[i]] = to[i];
	}

	StringList sessions;
	getSessions(sessions, params);
	for (StringList::iterator it = sessions.begin(); it != sessions.end(); it++)
	{
		ssi_sprint(string, "%s\\%s.annotation", it->str(), params.annotation);

		ssi_print("\n-------------------------------------------\n");
		ssi_print("MAP CLASS NAMES '%s:%s>%s:%s'\n\n", string, params.classname, params.annotation_new, params.classname_new);

		if (ssi_exists(string))
		{
			Annotation anno;
			if (!anno.load(string))
			{
				ssi_wrn("ERROR: could not load annotation from file");
				continue;
			}


			if (!(anno.getScheme()->type == SSI_SCHEME_TYPE::DISCRETE || anno.getScheme()->type == SSI_SCHEME_TYPE::CONTINUOUS))
			{
				ssi_wrn("ERROR: annotation type not supported");
				continue;
			}

			Annotation anno_new;
			anno_new.setDiscreteScheme(params.scheme[0] == '\0' ? params.annotation_new : params.scheme, to_unique);


			if (anno.getScheme()->type == SSI_SCHEME_TYPE::DISCRETE)
			{

				for (Annotation::iterator it = anno.begin(); it != anno.end(); it++)
				{
					map<String, String>::iterator pos = mapping.find(String(anno.getClassName(it->discrete.id)));
					if (pos != mapping.end())
					{
						anno_new.add(it->discrete.from, it->discrete.to, pos->second.str(), it->confidence);
					}
				}

				anno_new.packClass();
			}

			else if (anno.getScheme()->type == SSI_SCHEME_TYPE::CONTINUOUS)
			{

				anno_new.addClass("NaN");
				
				int count = 0;
				while (count < anno.size()) {
					float start = ssi_cast(ssi_real_t, count * (1.0f / anno.getScheme()->continuous.sr));
					float dur = ssi_cast(ssi_real_t, (1.0f / anno.getScheme()->continuous.sr));

					if (isnan(anno.at(count).continuous.score))
					{
						anno_new.add(start, start + dur, "NaN", 1.0);
					}

					else
					{
						for (int i = 0; i < from.size(); i++)
						{
							float highthres = strtof(from[i].str(), 0);
							if (anno.at(count).continuous.score <= highthres)
							{
								anno_new.add(start, start + dur, to[i].str(), 1.0);
								break;
							}
						}
					}
					count++;
				}

				anno_new.packClass(0.1);
				anno_new.removeClass("NaN");
				
			}

			ssi_sprint(string, "%s\\%s.annotation", it->str(), params.annotation_new);
			anno_new.save(string, File::ASCII);	
		}
		else
		{
			ssi_wrn("ERROR: annotation file not found");
			continue;
		}
	}
}

bool train_h(params_t &params, Trainer &trainer, CMLTrainer &cmltrainer)
{
	ssi_print("\n-------------------------------------------\n");
	ssi_print("TRAIN '%s'\n\n", params.trainer);

	if (ssi_strcmp(params.balance, "under", false))
	{
		trainer.setBalance(Trainer::BALANCE::UNDER);
	}
	else if (ssi_strcmp(params.balance, "over", false))
	{
		trainer.setBalance(Trainer::BALANCE::OVER);
	}

	if (!cmltrainer.train(&trainer))
	{
		return false;
	}
	
	trainer.Meta["leftContext"] = params.contextLeft;
	trainer.Meta["rightContext"] = params.contextRight;
	trainer.Meta["balance"] = params.balance;
	trainer.save(params.trainer);
	
	return true;
}

void train(params_t &params)
{
	MongoURI uri(params.server, params.port, params.username, params.password);
	MongoClient client;
	if (!client.connect(uri, params.database, false, 1000))
	{
		return;
	}

	Trainer trainer;
	if (!Trainer::Load(trainer, params.trainerTmp))
	{
		return;
	}

	StringList roles;
	roles.parse(params.role, ';');

	StringList sessions;
	getSessions(sessions, params);

	if (!params.cooperative)
	{
		CMLTrainer cmltrainer;
		cmltrainer.init(&client, params.root, params.scheme, params.stream, params.contextLeft, params.contextRight);

		for (StringList::iterator it = sessions.begin(); it != sessions.end(); it++)
		{
			FilePath path(it->str());
			const ssi_char_t *session = path.getName();

			for (StringList::iterator role = roles.begin(); role != roles.end(); role++)
			{
				ssi_print("\n-------------------------------------------\n");
				ssi_print("COLLECT SAMPLES '%s.%s.%s.%s->%s'\n\n", session, role->str(), params.scheme, params.annotator, params.stream);

				if (!cmltrainer.collect(session, role->str(), params.annotator, params.cooperative))
				{
					ssi_wrn("ERROR: could not load annotation from database");
					continue;
				}
			}
		}

		if (!train_h(params, trainer, cmltrainer))
		{
			ssi_wrn("ERROR: training failed");
		}
	}
	else
	{
		for (StringList::iterator it = sessions.begin(); it != sessions.end(); it++)
		{
			FilePath path(it->str());
			const ssi_char_t *session = path.getName();

			for (StringList::iterator role = roles.begin(); role != roles.end(); role++)
			{
				ssi_print("\n-------------------------------------------\n");
				ssi_print("COLLECT SAMPLES '%s.%s.%s.%s->%s'\n\n", session, role->str(), params.scheme, params.annotator, params.stream);

				CMLTrainer cmltrainer;
				cmltrainer.init(&client, params.root, params.scheme, params.stream, params.contextLeft, params.contextRight);

				if (!cmltrainer.collect(session, role->str(), params.annotator, params.cooperative))
				{
					ssi_wrn("ERROR: could not load annotation from database");
					continue;
				}

				if (!train_h(params, trainer, cmltrainer))
				{
					ssi_wrn("ERROR: training failed");
				}
			}
		}		
	}
}

bool eval_h(params_t &params, Trainer &trainer, CMLTrainer &cmltrainer)
{
	ssi_print("\n-------------------------------------------\n");
	ssi_print("EVAL '%s'\n\n", params.trainer);

		if (!cmltrainer.eval(&trainer, params.evalpath, params.loso))
		{
			return false;
		}


	return true;
}

void eval(params_t &params)
{
	MongoURI uri(params.server, params.port, params.username, params.password);
	MongoClient client;
	if (!client.connect(uri, params.database, false, 1000))
	{
		return;
	}

	Trainer trainer;
	if (!Trainer::Load(trainer, params.trainer))
	{
		return;
	}

	StringList roles;
	roles.parse(params.role, ';');

	StringList sessions;
	getSessions(sessions, params);

	if (trainer.Meta.count("leftContext") > 0)
	{
		params.contextLeft = atoi(trainer.Meta["leftContext"].str());
	}
	if (trainer.Meta.count("rightContext") > 0)
	{
		params.contextRight = atoi(trainer.Meta["rightContext"].str());
	}

	CMLTrainer cmltrainer;
	cmltrainer.init(&client, params.root, params.scheme, params.stream, params.contextLeft, params.contextRight);

	for (StringList::iterator it = sessions.begin(); it != sessions.end(); it++)
	{
		FilePath path(it->str());
		const ssi_char_t *session = path.getName();

		for (StringList::iterator role = roles.begin(); role != roles.end(); role++)
		{
			ssi_print("\n-------------------------------------------\n");
			ssi_print("COLLECT SAMPLES '%s.%s.%s.%s->%s'\n\n", session, role->str(), params.scheme, params.annotator, params.stream);

			if (!cmltrainer.collect(session, role->str(), params.annotator, params.cooperative))
			{
				ssi_wrn("ERROR: could not load annotation from database");
				continue;
			}
		}
	}

	if (!eval_h(params, trainer, cmltrainer))
	{
		ssi_wrn("ERROR: evaluation failed");
	}

}


bool forward_h(params_t &params, MongoClient &client, Trainer &trainer, CMLTrainer &cmltrainer, const ssi_char_t *session, const ssi_char_t *role)
{
	ssi_print("\n-------------------------------------------\n");
	ssi_print("FORWARD '%s->%s.%s.%s'\n\n", params.stream, session, params.scheme, params.annotator);

	Annotation *anno = cmltrainer.forward(&trainer, session, role, params.annotator, params.cooperative);
	if (!anno)
	{
		return false;
	}

	if (anno->getScheme()->type == SSI_SCHEME_TYPE::DISCRETE)
	{
		anno->packClass(params.label_mingap);
		if (params.label_mindur > 0)
		{
			anno->filter(params.label_mindur, Annotation::FILTER_PROPERTY::DURATION, Annotation::FILTER_OPERATOR::GREATER);
		}
		anno->packClass(params.label_mingap);
	}

	if (params.confidence >= 0)
	{
		anno->setConfidence((ssi_real_t)params.confidence);
	}

	if (params.annotator_new[0] != '\0')
	{
		if (!CMLAnnotation::Save(anno, &client, session, role, params.scheme, params.annotator_new, params.finished, params.locked))
		{
			return false;
		}
	}
	else
	{
		if (!CMLAnnotation::Save(anno, &client, session, role, params.scheme, params.annotator, params.finished, params.locked))
		{
			return false;
		}
	}

	delete anno;

	return true;
}

void forward(params_t &params)
{
	MongoURI uri(params.server, params.port, params.username, params.password);
	MongoClient client;
	if (!client.connect(uri, params.database, false, 1000))
	{
		return;
	}

	StringList roles;
	roles.parse(params.role, ';');

	StringList sessions;
	getSessions(sessions, params);

	if (!params.cooperative)
	{
		CMLTrainer cmltrainer;
		cmltrainer.init(&client, params.root, params.scheme, params.stream, params.contextLeft, params.contextRight);

		Trainer trainer;
		if (!Trainer::Load(trainer, params.trainer))
		{
			ssi_wrn("ERROR: could not load trainer");
			return;
		}

		for (StringList::iterator it = sessions.begin(); it != sessions.end(); it++)
		{
			FilePath path(it->str());
			const ssi_char_t *session = path.getName();

			for (StringList::iterator role = roles.begin(); role != roles.end(); role++)
			{
				if (!forward_h(params, client, trainer, cmltrainer, session, role->str()))
				{
					ssi_wrn("ERROR: could not create annotation");
				}
			}
		}
	}
	else
	{	
		for (StringList::iterator it = sessions.begin(); it != sessions.end(); it++)
		{
			FilePath path(it->str());
			const ssi_char_t *session = path.getName();

			for (StringList::iterator role = roles.begin(); role != roles.end(); role++)
			{
				ssi_print("\n-------------------------------------------\n");
				ssi_print("FORWARD '%s->%s.%s.%s.%s'\n\n", params.stream, session, role->str(), params.scheme, params.annotator);

				CMLTrainer cmltrainer;
				cmltrainer.init(&client, params.root, params.scheme, params.stream, params.contextLeft, params.contextRight);				

				Trainer trainer;
				if (!Trainer::Load(trainer, params.trainer))
				{
					ssi_wrn("ERROR: could not load trainer");
					continue;
				}

				if (!forward_h(params, client, trainer, cmltrainer, session, role->str()))
				{
					ssi_wrn("ERROR: could not create annotation");
				}
			}
		}	
	}	
}

void merge(params_t &params)
{
	StringList roles;
	roles.parse(params.role, ';');

	StringList sessions;
	getSessions(sessions, params);

	StringList streams;
	streams.parse(params.stream, ';');

	ssi_size_t n_streams = streams.size();
	if (n_streams <= 1)
	{
		return;
	}

	ssi_stream_t *inputStreams = new ssi_stream_t[n_streams];	
	ssi_byte_t **inputPtrs = new ssi_byte_t *[n_streams];
	ssi_stream_t outputStream;
	ssi_byte_t *outputPtr = 0;
	ssi_stream_init(outputStream, 0, 0, 0, SSI_UNDEF, 0, 0);

	for (StringList::iterator it = sessions.begin(); it != sessions.end(); it++)
	{
		FilePath path(it->str());
		const ssi_char_t *session = path.getName();

		for (StringList::iterator role = roles.begin(); role != roles.end(); role++)
		{
			ssi_print("\n-------------------------------------------\n");
			ssi_print("MERGE STREAMS '%s.%s.{%s}->%s'\n\n", session, role->str(), params.stream, params.streamOut);

			String toFile = String(params.root) + "\\" + session + "\\" + role->str() + "." + params.streamOut + ".stream";

			if (!params.force && ssi_exists(toFile.str()))
			{
				ssi_print("skip\n");
				continue;
			}

			// read input streams			

			bool foundFiles = true;
			for (ssi_size_t i = 0; i < n_streams; i++)
			{
				String fromFile = String(params.root) + "\\" + session + "\\" + role->str() + "." + streams[i];
				if (ssi_exists(fromFile.str()))
				{
					foundFiles = false;
					continue;
				}

				if (!FileTools::ReadStreamFile(fromFile.str(), inputStreams[i]))
				{
					foundFiles = false;
					continue;
				}

				inputPtrs[i] = inputStreams[i].ptr;
 			}

			if (foundFiles)
			{
				// calculate ouput properties

				ssi_type_t type = inputStreams[0].type;
				ssi_size_t byte = inputStreams[0].byte;
				ssi_size_t num = inputStreams[0].num;
				ssi_size_t dim = inputStreams[0].dim;
				ssi_time_t sr = inputStreams[0].sr;

				bool compatible = true;
				for (ssi_size_t i = 1; i < n_streams; i++)
				{
					num = min(num, inputStreams[i].num);
					dim += inputStreams[i].dim;
					if (type != inputStreams[i].type || byte != inputStreams[i].byte || sr != inputStreams[i].sr)
					{
						compatible = false;
						break;
					}
				}

				if (compatible)
				{
					ssi_stream_init(outputStream, num, dim, byte, type, sr, 0);
					outputPtr = outputStream.ptr;

					// merge streams

					ssi_size_t bytes_to_copy;

					for (ssi_size_t n = 0; n < num; n++)
					{
						for (ssi_size_t i = 0; i < n_streams; i++)
						{
							bytes_to_copy = inputStreams[i].dim * byte;
							memcpy(outputPtr, inputPtrs[i], bytes_to_copy);
							outputPtr += bytes_to_copy;
							inputPtrs[i] += bytes_to_copy;
						}

					}

					// save stream

					FileTools::WriteStreamFile(File::BINARY, toFile.str(), outputStream);
				}
				else
				{
					ssi_wrn("ERROR: files not compatible");
				}
			}
			else
			{
				ssi_wrn("ERROR: missing file(s)");
			}

			// clean up

			ssi_stream_destroy(outputStream);
			for (ssi_size_t i = 0; i < n_streams; i++)
			{
				ssi_stream_destroy(inputStreams[i]);
			}
		}

	}

	delete[] inputStreams;
	delete[] inputPtrs;
}