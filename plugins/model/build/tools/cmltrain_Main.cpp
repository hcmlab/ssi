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

#include <vld.h>

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
	ssi_char_t *scheme;
	ssi_char_t *annotator;
	ssi_char_t *annotator_new;
	ssi_char_t *role;
	ssi_char_t *dlls;
	ssi_char_t *logpath;
	ssi_char_t *filter;
	ssi_char_t *list;
	ssi_char_t *classname;
	ssi_char_t *stream;
	//ssi_char_t *trainer;
	//ssi_time_t frame;
	bool cooperative;
	int context;
	bool finished;
	bool locked;
	double confidence;
	double label_mingap;
	double label_mindur;
	bool overwrite;
};

void getSessions(StringList &list, params_t &params);

void loadDlls(params_t &params);
bool readCredentials(params_t &params);
void uploadAnnotations(params_t &params);
void downloadAnnotations(params_t &params);
void removeAnnotations(params_t &params);
void cutStreamFromLabel(params_t &params);
void extract(params_t &params);
void train(params_t &params);
void forward(params_t &params);

int main (int argc, char **argv) {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	//--download -filter *Augsburg -log log.txt Z:\Korpora\aria-noxi 137.250.171.233 3389 aria-noxi novice wagjohan voiceactivity
	//--cut -filter *Augsburg -log log.txt Z:\Korpora\aria-noxi voiceactivity.novice.wagjohan BREATH Novice_close.wav

	//--extract -filter 066_2016-05-23_Augsburg -log log.txt D:\korpora\nova\aria-noxi expert close

	//--train -context 5 -username system -password AriaSSI -list 066_2016-05-23_Augsburg;067_2016-05-23_Augsburg -log log.txt D:\korpora\nova\aria-noxi 137.250.171.233 3389 aria-noxi novice;expert voiceactivity gold "close.mfccdd[-f 0.04 -d 0]" 
	//--train -cooperative -context 5 -username system -password AriaSSI -filter 084_2016-05-31_Augsburg -log log.txt D:\korpora\nova\aria-noxi 137.250.171.233 3389 aria-noxi expert voiceactivity gold "close.mfccdd[-f 0.04 -d 0]" 

	//--forward -context 5 -assign systemcml -username system -password AriaSSI -filter *Augsburg -log log.txt D:\korpora\nova\aria-noxi 137.250.171.233 3389 aria-noxi novice;expert voiceactivity gold "close.mfccdd[-f 0.04 -d 0]"
	//--forward -cooperative -context 5 -username system -password AriaSSI -filter 084_2016-05-31_Augsburg -log log.txt D:\korpora\nova\aria-noxi 137.250.171.233 3389 aria-noxi expert voiceactivity gold "close.mfccdd[-f 0.04 -d 0]"

	char info[1024];
	ssi_sprint (info, "\n%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL("model");
	Factory::RegisterDLL("liblinear");
	Factory::RegisterDLL("opensmile");
	Factory::RegisterDLL("signal");
	Factory::RegisterDLL("frame");
	Factory::RegisterDLL("event");

	CmdArgParser cmd;
	cmd.info (info);

	params_t params;
	params.root = 0;
	params.server = 0;
	params.username = 0;
	params.password = 0;
	params.database = 0;
	params.annotation = 0;
	params.scheme = 0;
	params.annotator = 0;
	params.annotator_new = 0;
	params.role = 0;
	params.dlls = 0;
	params.logpath = 0;
	params.filter = 0;
	params.list = 0;
	params.classname = 0;
	params.stream = 0;
//	params.trainer = 0;
	params.finished = false;
	params.locked = false;
	params.confidence = -1.0;
	params.label_mingap = 0;
	params.label_mindur = 0;
//	params.frame = 0.04;
	params.context = 0;	
	params.overwrite = false;

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
	cmd.addDCmdOption("-confidence", &params.confidence, -1.0, "force confidence value (applied if >= 0)");
	cmd.addSCmdOption("-filter", &params.filter, "*", "session filter (e.g. *location)");
	cmd.addSCmdOption("-list", &params.list, "", "list with sessions separated by ; (overrides filter)");
	cmd.addSCmdOption("-username", &params.username, "", "database username");
	cmd.addSCmdOption("-password", &params.password, "", "database password");
	cmd.addSCmdOption("-dlls", &params.dlls, "", "list of requird dlls separated by ';'");
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
	cmd.addSCmdOption("-dlls", &params.dlls, "", "list of requird dlls separated by ';'");
	cmd.addSCmdOption("-log", &params.logpath, "", "output to log file");

	cmd.addMasterSwitch("--cut");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("root", &params.root, "path to database on disk");
	cmd.addSCmdArg("annotation", &params.annotation, "name of annotation");
	cmd.addSCmdArg("classname", &params.classname, "name of class to extract");
	cmd.addSCmdArg("stream", &params.stream, "name of stream");

	cmd.addText("\nOptions:");
	cmd.addSCmdOption("-filter", &params.filter, "*", "session filter (e.g. *location)");
	cmd.addSCmdOption("-username", &params.username, "", "database username");
	cmd.addSCmdOption("-password", &params.password, "", "database password");
	cmd.addSCmdOption("-dlls", &params.dlls, "", "list of requird dlls separated by ';'");
	cmd.addSCmdOption("-log", &params.logpath, "", "output to log file");

	cmd.addMasterSwitch("--extract");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("root", &params.root, "path to database on disk");	
	cmd.addSCmdArg("role", &params.role, "name of role");
	cmd.addSCmdArg("stream", &params.stream, "name of stream");

	cmd.addText("\nOptions:");
	cmd.addSCmdOption("-filter", &params.filter, "*", "session filter (e.g. *location)");
	cmd.addSCmdOption("-list", &params.list, "", "list with sessions separated by ; (overrides filter)");
	cmd.addBCmdOption("-overwrite", &params.overwrite, false, "overwrite existing features files");
	cmd.addSCmdOption("-dlls", &params.dlls, "", "list of requird dlls separated by ';'");
	cmd.addSCmdOption("-log", &params.logpath, "", "output to log file");

	cmd.addMasterSwitch("--train");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("root", &params.root, "path to database on disk");
	cmd.addSCmdArg("server", &params.server, "server name");
	cmd.addICmdArg("port", &params.port, "port number");
	cmd.addSCmdArg("database", &params.database, "name of database");
	cmd.addSCmdArg("role", &params.role, "name of role");
	cmd.addSCmdArg("scheme", &params.scheme, "name of scheme");
	cmd.addSCmdArg("annotator", &params.annotator, "name of annotator");
	cmd.addSCmdArg("stream", &params.stream, "name of stream");	
	//cmd.addSCmdArg("trainer", &params.trainer, "name of trainer template");

	cmd.addText("\nOptions:");	
	cmd.addSCmdOption("-filter", &params.filter, "*", "session filter (e.g. *location)");
	cmd.addSCmdOption("-list", &params.list, "", "list with sessions separated by ; (overrides filter)");
	cmd.addSCmdOption("-username", &params.username, "", "database username");
	cmd.addSCmdOption("-password", &params.password, "", "database password");
	cmd.addICmdOption("-context", &params.context, 0, "context (number of frames added to the left and right of center frame)");
	cmd.addBCmdOption("-cooperative", &params.cooperative, false, "turn on cooperative learning");
	cmd.addSCmdOption("-dlls", &params.dlls, "", "list of requird dlls separated by ';'");
	cmd.addSCmdOption("-log", &params.logpath, "", "output to log file");

	cmd.addMasterSwitch("--forward");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("root", &params.root, "path to database on disk");
	cmd.addSCmdArg("server", &params.server, "server name");
	cmd.addICmdArg("port", &params.port, "port number");
	cmd.addSCmdArg("database", &params.database, "name of database");
	cmd.addSCmdArg("role", &params.role, "name of role");
	cmd.addSCmdArg("scheme", &params.scheme, "name of scheme");
	cmd.addSCmdArg("annotator", &params.annotator, "name of annotator");
	cmd.addSCmdArg("stream", &params.stream, "name of stream");
	//cmd.addSCmdArg("trainer", &params.trainer, "name of trainer template");

	cmd.addText("\nOptions:");
	cmd.addSCmdOption("-filter", &params.filter, "*", "session filter (e.g. *location)");
	cmd.addSCmdOption("-list", &params.list, "", "list with sessions separated by ; (overrides filter)");
	cmd.addSCmdOption("-username", &params.username, "", "database username");
	cmd.addSCmdOption("-password", &params.password, "", "database password");
	cmd.addBCmdOption("-finished", &params.finished, false, "annotation will be marked as finished");
	cmd.addBCmdOption("-locked", &params.locked, false, "annotation will be marked as locked");
	cmd.addICmdOption("-context", &params.context, 0, "context (number of frames added to the left and right of center frame)");
	cmd.addSCmdOption("-assign", &params.annotator_new, "", "assign a different annotator");
	cmd.addDCmdOption("-confidence", &params.confidence, -1.0, "force confidence value (applied if >= 0)");
	cmd.addDCmdOption("-mingap", &params.label_mingap, 0, "gaps between labels with same name that are smaller than this value will be closed");
	cmd.addDCmdOption("-mindur", &params.label_mindur, 0, "labels with a duration smaller or equal to this value will be removed");
	cmd.addBCmdOption("-cooperative", &params.cooperative, false, "turn on cooperative learning");
	cmd.addSCmdOption("-dlls", &params.dlls, "", "list of requird dlls separated by ';'");
	cmd.addSCmdOption("-log", &params.logpath, "", "output to log file");
	
	if (cmd.read (argc, argv)) {		

		if (params.logpath && params.logpath[0] != '\0') {
			ssimsg = new FileMessage(params.logpath);
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

			extract(params);

			break;
		}


		case 6: {

			train(params);

			break;
		}

		case 7: {

			forward(params);

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
	delete[] params.annotator;
	delete[] params.annotator_new;
	delete[] params.scheme;
	delete[] params.role;
	delete[] params.dlls;
	delete[] params.logpath;
	delete[] params.filter;
	delete[] params.list;
	delete[] params.stream;
	delete[] params.classname;
//	delete[] params.trainer;

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

	if (params.username[0] == '\0')
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

			if (!CMLAnnotation::Save(&anno, &client, session, role, anno.getScheme()->name, annotator, params.finished, params.locked))
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

void extract(params_t &params)
{
	ssi_char_t fromPath[SSI_MAX_CHAR];
	ssi_char_t toPath[SSI_MAX_CHAR];

	OSMfccChain *feat = ssi_create_id(OSMfccChain, 0, "feat");
	feat->getOptions()->deltas_enable = true;
	feat->getOSTransformFFT()->getOptions()->nfft = 2048;
	feat->getOSSpecScale()->getOptions()->firstNote = 27.05;
	feat->getOSMfcc()->getOptions()->first = 0;
	((Derivative*)feat->getDeltas())->getOptions()->set(Derivative::D0TH | Derivative::D1ST | Derivative::D2ND);

	ssi_char_t *featname = "mfccdd[-f 0.04 -d 0]";

	StringList sessions;
	getSessions(sessions, params);
	for (StringList::iterator session = sessions.begin(); session != sessions.end(); session++)
	{
		StringList roles;
		roles.parse(params.role, ';');
		
		for (StringList::iterator role = roles.begin(); role != roles.end(); role++)
		{
			if (ssi_strcmp(role->str(), "expert"))
			{
				ssi_sprint(fromPath, "%s\\Expert_%s.wav", session->str(), params.stream);
			}			
			else if (ssi_strcmp(role->str(), "novice"))
			{
				ssi_sprint(fromPath, "%s\\Novice_%s.wav", session->str(), params.stream);
			}

			ssi_sprint(toPath, "%s\\%s.%s.%s.stream", session->str(), params.stream, featname, role->str());

			ssi_print("\n-------------------------------------------\n");
			ssi_print("EXTRACT FEATURES '%s>%s'\n\n", fromPath, toPath);

			if (!params.overwrite && ssi_exists(toPath))
			{
				continue;
			}

			if (!ssi_exists(fromPath))
			{
				ssi_wrn("input file not found");
				continue;
			}

			ssi_stream_t from, to;

			WavTools::ReadWavFile(fromPath, from, true);

			ssi_size_t frame_samples = ssi_size_t(0.04 * from.sr);
			ssi_size_t delta_samples = ssi_size_t(0.0 * from.sr);
			SignalTools::Transform(from, to, *feat, frame_samples, delta_samples);

			FileTools::WriteStreamFile(File::BINARY, toPath, to);

			ssi_stream_destroy(from);
			ssi_stream_destroy(to);
			
		}
		
	}
}

void train(params_t &params)
{
	MongoURI uri(params.server, params.port, params.username, params.password);
	MongoClient client;
	if (!client.connect(uri, params.database, false, 1000))
	{
		return;
	}

	//Trainer trainer;
	//if (!Trainer::Load(trainer, params.trainer))
	//{		
	//	return;
	//}
	LibLinear *lin = ssi_create_id(LibLinear, 0, "model");
	lin->getOptions()->seed = 1234;
	lin->getOptions()->silent = true;
	lin->getOptions()->setParams("-s 0 -e 0.1 -B 0.1");
	lin->getOptions()->balance = LibLinear::BALANCE::UNDER;
	Trainer trainer(lin);

	ISNorm::Params norm;
	ISNorm::ZeroParams(norm, ISNorm::METHOD::SCALE);
	norm.limits[0] = -1.0f;
	norm.limits[1] = 1.0f;
	trainer.setNormalization(&norm);

	StringList roles;
	roles.parse(params.role, ';');

	StringList sessions;
	getSessions(sessions, params);

	if (!params.cooperative)
	{
		CMLTrainer cmltrainer;
		cmltrainer.init(&client, params.root, params.scheme, params.stream, params.context);

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

		ssi_char_t string[SSI_MAX_CHAR];
		ssi_sprint(string, "%s\\models\\%s.%s.%s", params.root, params.stream, params.scheme, params.annotator);

		ssi_print("\n-------------------------------------------\n");
		ssi_print("TRAIN '%s'\n\n", string);
		if (cmltrainer.train(&trainer))
		{
			trainer.save(string);
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
				cmltrainer.init(&client, params.root, params.scheme, params.stream, params.context);

				if (!cmltrainer.collect(session, role->str(), params.annotator, params.cooperative))
				{
					ssi_wrn("ERROR: could not load annotation from database");
					continue;
				}

				ssi_char_t string[SSI_MAX_CHAR];
				ssi_sprint(string, "%s\\%s\\%s.%s.%s", params.root, session, params.stream, params.scheme, params.annotator);

				ssi_print("\n-------------------------------------------\n");
				ssi_print("TRAIN '%s'\n\n", string);
				if (cmltrainer.train(&trainer))
				{
					trainer.save(string);
				}
			}
		}		
	}
}

void forward(params_t &params)
{
	MongoURI uri(params.server, params.port, params.username, params.password);
	MongoClient client;
	if (!client.connect(uri, params.database, false, 1000))
	{
		return;
	}

	ssi_char_t string[SSI_MAX_CHAR];

	StringList roles;
	roles.parse(params.role, ';');

	StringList sessions;
	getSessions(sessions, params);

	if (!params.cooperative)
	{
		CMLTrainer cmltrainer;
		cmltrainer.init(&client, params.root, params.scheme, params.stream, params.context);
		
		ssi_sprint(string, "%s\\models\\%s.%s.%s", params.root, params.stream, params.scheme, params.annotator);

		Trainer trainer;
		if (!Trainer::Load(trainer, string))
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
				ssi_print("\n-------------------------------------------\n");
				ssi_print("FORWARD '%s->%s.%s.%s.%s'\n\n", params.stream, session, role->str(), params.scheme, params.annotator);

				Annotation *anno = cmltrainer.forward(&trainer, session, role->str(), params.annotator, params.cooperative);
				if (!anno)
				{
					ssi_wrn("ERROR: could not create annotation");
					continue;
				}

				anno->packClass(params.label_mingap);
				if (params.label_mindur > 0)
				{
					anno->filter(params.label_mindur, Annotation::FILTER_PROPERTY::DURATION, Annotation::FILTER_OPERATOR::GREATER);
				}
				anno->packClass(params.label_mingap);

				if (params.confidence >= 0)
				{
					anno->setConfidence((ssi_real_t)params.confidence);
				}

				if (params.annotator_new[0] != '\0')
				{
					CMLAnnotation::Save(anno, &client, session, role->str(), params.scheme, params.annotator_new, params.finished, params.locked);
				}
				else
				{
					CMLAnnotation::Save(anno, &client, session, role->str(), params.scheme, params.annotator, params.finished, params.locked);
				}

				delete anno;
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
				cmltrainer.init(&client, params.root, params.scheme, params.stream, params.context);

				ssi_sprint(string, "%s\\%s\\%s.%s.%s", params.root, session, params.stream, params.scheme, params.annotator);

				Trainer trainer;
				if (!Trainer::Load(trainer, string))
				{
					ssi_wrn("ERROR: could not load trainer");
					continue;
				}

				Annotation *anno = cmltrainer.forward(&trainer, session, role->str(), params.annotator, params.cooperative);
				if (!anno)
				{
					ssi_wrn("ERROR: could not create annotation");
					continue;
				}

				anno->packClass(params.label_mingap);
				if (params.label_mindur > 0)
				{
					anno->filter(params.label_mindur, Annotation::FILTER_PROPERTY::DURATION, Annotation::FILTER_OPERATOR::GREATER);
				}
				anno->packClass(params.label_mingap);

				if (params.confidence >= 0)
				{
					anno->setConfidence((ssi_real_t)params.confidence);
				}

				if (params.annotator_new[0] != '\0')
				{
					CMLAnnotation::Save(anno, &client, session, role->str(), params.scheme, params.annotator_new, params.finished, params.locked);
				}
				else
				{
					CMLAnnotation::Save(anno, &client, session, role->str(), params.scheme, params.annotator, params.finished, params.locked);
				}

				delete anno;
			}
		}
	}
}