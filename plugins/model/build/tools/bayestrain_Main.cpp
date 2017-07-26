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
#include "bayesnet/include/ssibayesnet.h"
#include "bayesnet/build/external/smilearn.h"
#include "bayesnet/build/external/smile.h"

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
	ssi_char_t *stream;
	ssi_char_t *streamOut;
	ssi_char_t *trainerTmp;
	ssi_char_t *trainer;
	ssi_char_t *netpath;
	ssi_char_t *datasetpath;
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
	bool dynamic;

};

void getSessions(StringList &list, params_t &params);

void loadDlls(params_t &params);

void trainBayesianNetwork(params_t &params);

void predictBayesianNetwork(params_t &params);
void createEventLists(params_t &params, float &sr);
void loadEventListsAndPredict(params_t &params, float sr);
bool loadEventList(const ssi_char_t *path, ssi_size_t &n_events, ssi_event_t **events);
bool saveEventList(const ssi_char_t *path, ssi_size_t n_events, ssi_event_t *events, ssi_size_t n_classes, ssi_size_t *class_ids, ssi_size_t sender_id, ssi_size_t event_id);
bool createEventList(const ssi_char_t *trainerpath, const ssi_char_t *scheme, const ssi_char_t *eventListPath, const ssi_char_t *streamPath, float &sr, bool discretize);

int main(int argc, char **argv) {
#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

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
		params.srcurl = 0;
		params.logpath = 0;
		params.filter = 0;
		params.list = 0;
		params.classname = 0;
		params.stream = 0;
		params.streamOut = 0;
		params.trainerTmp = 0;
		params.trainer = 0;
		params.finished = false;
		params.locked = false;
		params.confidence = -1.0;
		params.label_mingap = 0;
		params.label_mindur = 0;
		params.contextLeft = 0;
		params.contextRight = 0;
		params.balance = 0;
		params.netpath = 0;
		params.datasetpath = 0;
		params.dynamic = false;

	

		cmd.addMasterSwitch("--bayesnettrain");

		cmd.addText("\nArguments:");
		cmd.addSCmdArg("netpath", &params.netpath, "", "Path of bayesian network");
		cmd.addSCmdArg("datasetpath", &params.datasetpath, "", "Path of datasheet");


		cmd.addText("\nOptions:");
		cmd.addBCmdOption("-dynamic", &params.dynamic, false, "use dynamic bns");
		cmd.addSCmdOption("-log", &params.logpath, "", "output to log file");




		cmd.addMasterSwitch("--bayesnetfusion");

		cmd.addText("\nArguments:");
		cmd.addSCmdArg("root", &params.root, "path to database on disk");
		cmd.addSCmdArg("server", &params.server, "server name");
		cmd.addICmdArg("port", &params.port, "port number");
		cmd.addSCmdArg("database", &params.database, "name of database");
		cmd.addSCmdArg("role", &params.role, "name of role (if several separate by ;)");
		cmd.addSCmdArg("scheme", &params.scheme, "list of schemes (separate by ;)");
		cmd.addSCmdArg("annotator", &params.annotator, "name of annotators (separate by ;)");
		cmd.addSCmdArg("stream", &params.stream, "name of streams (separate by ;)");
		cmd.addSCmdArg("trainer", &params.trainer, "trainer paths (separate by ;)");
		cmd.addSCmdArg("output", &params.streamOut, "name of output stream");

		cmd.addText("\nOptions:");
		cmd.addSCmdOption("-filter", &params.filter, "*", "session filter (e.g. *location)");
		cmd.addSCmdOption("-list", &params.list, "", "list with sessions separated by ; (overrides filter)");
		cmd.addSCmdOption("-netpath", &params.netpath, "", "Path of bayesian network, without xdsl fileending");
		cmd.addSCmdOption("-datasetpath", &params.datasetpath, "", "Path of bayesian network, without xdsl fileending");
		cmd.addBCmdOption("-dynamic", &params.dynamic, false, "overwrite existing files");
		cmd.addBCmdOption("-force", &params.force, false, "overwrite existing files");
		cmd.addSCmdOption("-log", &params.logpath, "", "output to log file");
		cmd.addSCmdOption("-username", &params.username, "", "database username");
		cmd.addSCmdOption("-password", &params.password, "", "database password");

		if (cmd.read(argc, argv)) {
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
			
					trainBayesianNetwork(params);
				

				break;
			}


			case 2: {
				predictBayesianNetwork(params);

				break;
			}
			}

			if (params.logpath && params.logpath[0] != '\0') {
				delete ssimsg; ssimsg = 0;
			}

			Factory::Clear();
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
		delete[] params.srcurl;
		delete[] params.logpath;
		delete[] params.filter;
		delete[] params.list;
		delete[] params.stream;
		delete[] params.streamOut;
		delete[] params.classname;
		delete[] params.trainerTmp;
		delete[] params.trainer;
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



#pragma region BayesianNetworkPrediction

bool IsDigit(char c)
{
	return ('0' <= c && c <= '9');
}

void trainBayesianNetwork(params_t &params)
{

	const char* network = params.netpath;
	const char* dataset = params.datasetpath;
    bool dynamic = params.dynamic;

	DSL_dataset ds;
	ssi_print("Reading Dataset...\n");
	if (ds.ReadFile(dataset) != DSL_OKAY) {
		ssi_print("Cannot read data file... exiting.\n");
		return;
	}
	ssi_print("Success\n");

	DSL_network net;
	ssi_print("Reading Network...\n");
	if (net.ReadFile(network, DSL_XDSL_FORMAT) != DSL_OKAY) {
		ssi_print("Cannot read network file... exiting.\n");
		return;
	}
	ssi_print("Success\n");


		if (dynamic)//dynamic case A A_0 A_1 B B_0 B_1 C C_0 C_1
		{

			std::vector<DSL_datasetMatch> dsMap(ds.GetNumberOfVariables());
			int varCnt = 0;  // the number of variables occuring both in the data set and the network
			for (int i = 0; i < ds.GetNumberOfVariables(); i++) {
				std::string id = ds.GetId(i);
				const char* idc = id.c_str();

				bool done = false;
				for (int j = 0; j < (int)strlen(idc) && !done; j++) {
					if (idc[j] == '_' &&  IsDigit(idc[j+1])) {
						char* nodeId = (char*)malloc((j + 1) * sizeof(char));
						strncpy(nodeId, idc, j);
						nodeId[j] = '\0';

						int nodeHdl = net.FindNode(nodeId);
						if (nodeHdl >= 0) {
							DSL_intArray orders;
							net.GetTemporalOrders(nodeHdl, orders);

							dsMap[varCnt].node = nodeHdl;
							dsMap[varCnt].slice = atoi(idc + j + 1);
							dsMap[varCnt].column = i;
							varCnt++;

							free(nodeId);
							done = true;
						}
					}
				}
				if (!done) {
					int nodeHdl = net.FindNode(idc);
					if (nodeHdl >= 0) {
						dsMap[varCnt].node = nodeHdl;
						dsMap[varCnt].slice = 0;
						dsMap[varCnt].column = i;
						varCnt++;
					}
				}
			}
			dsMap.resize(varCnt);


			for (int i = 0; i < dsMap.size(); i++) {
				DSL_datasetMatch &m = dsMap[i];
				int nodeHdl = m.node;
				int colIdx = m.column;

				DSL_idArray* ids = net.GetNode(nodeHdl)->Definition()->GetOutcomesNames();
				const DSL_datasetVarInfo &varInfo = ds.GetVariableInfo(colIdx);
				const std::vector<std::string> &stateNames = varInfo.stateNames;
				std::vector<int> map(stateNames.size(), -1);
				for (int j = 0; j < (int)stateNames.size(); j++) {
					const char* id = stateNames[j].c_str();
					for (int k = 0; k < ids->NumItems(); k++) {
						char* tmpid = (*ids)[k];
						if (!strcmp(id, tmpid)) {
							map[j] = k;
						}
					}
				}
				for (int k = 0; k < ds.GetNumberOfRecords(); k++) {
					if (ds.GetInt(colIdx, k) >= 0) {
						ds.SetInt(colIdx, k, map[ds.GetInt(colIdx, k)]);
					}
				}
			}


			DSL_em em;
			ssi_print("Learn network using Expectation Maximization (EM)...\n");
			if (em.Learn(ds, net, dsMap) != DSL_OKAY) {
				ssi_print("Cannot learn parameters... exiting.\n");
				return;
			}
			ssi_print("Success\n");
		}

		
	else //static case A B C
	{
		std::vector<DSL_datasetMatch> matches;
		std::string err;
		ssi_print("Match network with dataset...\n");
		if (ds.MatchNetwork(net, matches, err) != DSL_OKAY) {
			ssi_print("Cannot match network... exiting.\n");
			return;
		}
		ssi_print("Success\n");
		DSL_em em;
		ssi_print("Learn network using Expectation Maximization (EM)...\n");
		if (em.Learn(ds, net, matches) != DSL_OKAY) {
			ssi_print("Cannot learn parameters... exiting.\n");
			return;
		}
		ssi_print("Success\n");

	}

	ssi_print("Learning successful.\n");
	net.WriteFile(network, DSL_XDSL_FORMAT);

}



void predictBayesianNetwork(params_t &params)
{
	float sr;
	createEventLists(params, sr);
	loadEventListsAndPredict(params, sr);
}

void createEventLists(params_t &params, float &samplerate) {

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

	StringList schemes;
	schemes.parse(params.scheme, ';');

	StringList annotators;
	annotators.parse(params.annotator, ';');

	StringList trainers;
	trainers.parse(params.trainer, ';');

	StringList streams;
	streams.parse(params.stream, ';');


	for (StringList::iterator it = sessions.begin(); it != sessions.end(); it++)
	{
		FilePath path(it->str());
		const ssi_char_t *session = path.getName();

		for (StringList::iterator role = roles.begin(); role != roles.end(); role++)
		{
			int i = 0;

			if (schemes.size() != trainers.size())
			{
				ssi_print("\n #schemes does not fit #trainers \n");
				return;
			}

			for (StringList::iterator scheme = schemes.begin(); scheme != schemes.end(); scheme++)
			{
			
				String eventListPath = String(params.root) + "\\" + session + "\\" + role->str() + "." + scheme->str() + ".eventlist";
				String streamPath = String(params.root) + "\\" + session + "\\" + role->str() + "." + streams.at(i).str();

				
				if (!createEventList(trainers.at(i).str(), scheme->str(), eventListPath.str(), streamPath.str(), samplerate, false))
				{
					ssi_wrn("ERROR: could not load annotation from database");
					continue;
				}

				i++;
			}
		}
	}
}


bool createEventList(const ssi_char_t *trainerpath, const ssi_char_t *scheme,
	const ssi_char_t *eventListPath, const ssi_char_t *streamPath, float &sr,
	bool discretize)
{

	ssi_stream_t stream;
	FileTools::ReadStreamFile(streamPath, stream);

	Trainer trainer;
	Trainer::Load(trainer, trainerpath);

	ssi_size_t n_events = stream.num;
	ssi_size_t n_classes = trainer.getClassSize();
	ssi_size_t *class_ids = new ssi_size_t[n_classes];
	for (ssi_size_t i = 0; i < n_classes; i++) {
		class_ids[i] = Factory::AddString(trainer.getClassName(i));
	}


	ssi_size_t event_id = Factory::AddString(scheme);
	ssi_size_t sender_id = Factory::AddString("cml");

 	ssi_event_t *events = new ssi_event_t[n_events];
	for (ssi_size_t i = 0; i < n_events; i++) {
		ssi_event_init(events[i], SSI_ETYPE_MAP, sender_id, event_id);
	}

	ssi_sample_t *sample = 0;
	ssi_real_t *probs = new ssi_real_t[n_classes];
	ssi_size_t count = 0;

	ssi_real_t boost = 1.0f;

	sr = stream.sr;

	while (count < stream.num) {
		events[count].time = ssi_cast(ssi_size_t, count * (1 / stream.sr) * 1000.0 + 0.5);
		events[count].dur = 0;

		ssi_stream_t *chops = new ssi_stream_t;
		ssi_stream_copy(stream, *chops,count, count+1);
		
		if (trainer.forward_probs(*chops, n_classes, probs)) {
			
			ssi_real_t minval, maxval;
			ssi_size_t minpos, maxpos;
			if (discretize) {
				ssi_minmax(n_classes, 1, probs, &minval, &minpos, &maxval, &maxpos);
			}
			ssi_event_adjust(events[count], n_classes * sizeof(ssi_event_map_t));
			ssi_event_map_t *t = ssi_pcast(ssi_event_map_t, events[count].ptr);
			for (ssi_size_t i = 0; i < n_classes; i++) {
				t[i].id = class_ids[i];
				if (discretize) {
					if (i == maxpos) {
						t[i].value = 1.0f;
					}
					else {
						t[i].value = 0.0f;
					}
				}
				else {
					t[i].value = probs[i] * boost;
				}
			}
		}
		delete chops;
		count++;
	}


	ssi_print("save to '%s' \n", eventListPath);
	if (!saveEventList(eventListPath, n_events, events, n_classes, class_ids, sender_id, event_id)) {
		return false;
	}

	for (ssi_size_t i = 0; i < n_events; i++) {
		ssi_event_destroy(events[i]);
	}
	delete[] events;

	return true;

}

void loadEventListsAndPredict(params_t &params, float samplerate)
{
	#include "bayesnet/include/ssibayesnet.h"

	Factory::RegisterDLL("frame");
	Factory::RegisterDLL("event");
	Factory::RegisterDLL("bayesnet");
	Factory::RegisterDLL("ioput");

	StringList roles;
	roles.parse(params.role, ';');

	StringList sessions;
	getSessions(sessions, params);

	StringList schemes;
	schemes.parse(params.scheme, ';');


	StringList annotators;
	annotators.parse(params.annotator, ';');

	StringList trainers;
	trainers.parse(params.trainer, ';');

	StringList streams;
	streams.parse(params.stream, ';');



	for (StringList::iterator it = sessions.begin(); it != sessions.end(); it++)
	{
		FilePath path(it->str());
		const ssi_char_t *session = path.getName();

		for (StringList::iterator role = roles.begin(); role != roles.end(); role++)
		{
			ssi_size_t n_events = INT_MAX;

			Annotation anno;
			anno.setContinuousScheme(params.streamOut, samplerate, 0, 1);

			FileAnnotationWriter *writer = ssi_create(FileAnnotationWriter, 0, true);
			writer->setAnnotation(&anno);

		    std:string networkpath = params.netpath;
			networkpath = networkpath + ".xdsl";

			BnEventConverter *converter = ssi_create(BnEventConverter, 0, true);
			BayesnetSmile *bnet = ssi_create(BayesnetSmile, 0, true);
			bnet->getOptions()->print = true;
			bnet->getOptions()->setPath(networkpath.c_str());
			bnet->getOptions()->setMonitoredNodes(params.streamOut);


			EventList list(100);
			ssi_size_t time_ms = 0;
			ssi_size_t sleep_ms = 0;
			ssi_size_t offset_ms = 0;

			converter->setEventListener(bnet);
			bnet->setEventListener(writer);

			ssi_print("simulation\n\n");
			writer->listen_enter();
			converter->listen_enter();
			bnet->listen_enter();

			vector<ssi_event_t*> event_vector = vector<ssi_event_t*>();
			int i = 0;
			for (StringList::iterator scheme = schemes.begin(); scheme != schemes.end(); scheme++)
			{
				ssi_print("\n-------------------------------------------\n");
				ssi_print("COLLECT SAMPLES '%s.%s.%s.%s->%s'\n\n", session, role->str(), scheme->str(), annotators.at(i).str(), streams.at(i).str());

				String eventListPath = String(params.root) + "\\" + session + "\\" + role->str() + "." + scheme->str() + ".eventlist";

				ssi_size_t n_newevents = 0;
				ssi_event_t *events = 0;

				if (!loadEventList(eventListPath.str(), n_newevents, &events))
				{
					ssi_print("\n error reading Eventlist \n");
				}

				event_vector.push_back(events);
				n_events = min(n_events, n_newevents);
				
				if (remove(eventListPath.str()) != 0)
				{
					ssi_print("\n error deleting Eventlist \n");
				}
				i++;
			}

			for (ssi_size_t i = 0; i < n_events; i++) {
				list.clear();

				for each (ssi_event_t *events in event_vector)
				{
					if (events[i].ptr) {
						events[i].time = time_ms;
						list.push(events[i]);
					}
				}

				int delta_ms = 1000.0 / samplerate;
				time_ms += delta_ms;
				//converter->update(list, list.getSize(), time_ms + offset_ms);
				bnet->update(list, list.getSize(), time_ms + offset_ms);
				ssi_print("\rTimeMs\t%d\tEventNo\t%d", time_ms, i);

				if (sleep_ms > 0) {
					ssi_sleep(sleep_ms);
				}
			}

			converter->listen_flush();
			bnet->listen_flush();

			String annotationpath = String(params.root) + "\\" + session + "\\" + role->str() + "." + params.streamOut;

			anno.save(annotationpath.str(), File::BINARY);
			writer->listen_flush();
		}
	}
}

bool saveEventList(const ssi_char_t *path, ssi_size_t n_events, ssi_event_t *events, ssi_size_t n_classes, ssi_size_t *class_ids, ssi_size_t sender_id, ssi_size_t event_id) {

	FILE *fp = ssi_fopen(path, "wb");

	if (!fp) {
		ssi_wrn("cannot create file '%s'", path);
		return false;
	}

	const ssi_char_t *name = 0;
	ssi_size_t len = 0;

	fwrite(&n_events, sizeof(n_events), 1, fp);
	fwrite(&n_classes, sizeof(n_events), 1, fp);

	for (ssi_size_t i = 0; i < n_classes; i++) {
		name = Factory::GetString(class_ids[i]);
		len = ssi_cast(ssi_size_t, strlen(name));
		fwrite(&len, sizeof(len), 1, fp);
		fwrite(name, sizeof(ssi_char_t), len, fp);
	}

	name = Factory::GetString(sender_id);
	len = ssi_cast(ssi_size_t, strlen(name));
	fwrite(&len, sizeof(len), 1, fp);
	fwrite(name, sizeof(ssi_char_t), len, fp);

	name = Factory::GetString(event_id);
	len = ssi_cast(ssi_size_t, strlen(name));
	fwrite(&len, sizeof(len), 1, fp);
	fwrite(name, sizeof(ssi_char_t), len, fp);

	for (ssi_size_t i = 0; i < n_events; i++) {
		fwrite(&events[i], sizeof(ssi_event_t), 1, fp);
		if (events[i].tot_real > 0) {
			fwrite(events[i].ptr, sizeof(ssi_byte_t), events[i].tot, fp);
		}
	}

	fclose(fp);

	return true;
}

bool loadEventList(const ssi_char_t *path, ssi_size_t &n_events, ssi_event_t **events) {
	FILE *fp = ssi_fopen(path, "rb");

	if (!fp) {
		ssi_wrn("cannot open file '%s'", path);
		return false;
	}

	ssi_size_t len = 0;
	ssi_char_t *name = 0;

	fread(&n_events, sizeof(n_events), 1, fp);
	*events = new ssi_event_t[n_events];

	ssi_size_t n_classes;
	fread(&n_classes, sizeof(n_events), 1, fp);
	ssi_size_t *class_ids = new ssi_size_t[n_classes];
	for (ssi_size_t i = 0; i < n_classes; i++) {
		fread(&len, sizeof(len), 1, fp);
		name = new ssi_char_t[len + 1];
		fread(name, sizeof(ssi_char_t), len, fp);
		name[len] = '\0';
		class_ids[i] = Factory::AddString(name);
		delete[] name;
	}

	ssi_size_t sender_id;
	fread(&len, sizeof(len), 1, fp);
	name = new ssi_char_t[len + 1];
	fread(name, sizeof(ssi_char_t), len, fp);
	name[len] = '\0';
	sender_id = Factory::AddString(name);
	delete[] name;

	ssi_size_t event_id;
	fread(&len, sizeof(len), 1, fp);
	name = new ssi_char_t[len + 1];
	fread(name, sizeof(ssi_char_t), len, fp);
	name[len] = '\0';
	event_id = Factory::AddString(name);
	delete[] name;

	for (ssi_size_t i = 0; i < n_events; i++) {
		fread(&(*events)[i], sizeof(ssi_event_t), 1, fp);
		(*events)[i].sender_id = sender_id;
		(*events)[i].event_id = event_id;
		if ((*events)[i].tot_real > 0) {
			(*events)[i].ptr = new ssi_byte_t[(*events)[i].tot_real];
			fread((*events)[i].ptr, sizeof(ssi_byte_t), (*events)[i].tot_real, fp);
			ssi_event_map_t *t = ssi_pcast(ssi_event_map_t, (*events)[i].ptr);
			for (ssi_size_t i = 0; i < n_classes; i++) {
				t[i].id = class_ids[i];
			}
		}
	}

	fclose(fp);

	return true;
}

#pragma endregion
