// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2017/05/16
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

ssi_char_t _workdir[SSI_MAX_CHAR];

struct params_t {
	ssi_char_t *exepath;
	ssi_char_t *filepath;
	ssi_char_t *debug_path;
	ssi_char_t *debug_socket;
	ssi_char_t *config_str;
	ssi_char_t *config_paths;	
	ssi_char_t *srcurl;
	bool init_only;
	bool save_pipe;
	bool init;
	bool export_dlls;
	bool show_close_button;
	bool enable_quick_edit;
};

void Run(const ssi_char_t *exepath, params_t params);
bool Parse_and_Run(int argc, char **argv);
void RemoveCloseButton();
void SetQuickEditMode(bool flag);

#if _WIN32|_WIN64
BOOL WINAPI ConsoleHandler(DWORD CEvent);
#endif

void delete_args(int argc, char **argv)
{
	if (argc > 0)
	{
		for (int i = 0; i < argc; i++)
		{
			delete[] argv[i];
		}
		delete[] argv;
	}
}

char **copy_args(int argc, char **argv)
{
	char **copy = 0;
	if (argc > 0)
	{
		copy = new char *[argc];
		for (int i = 0; i < argc; i++)
		{
			copy[i] = ssi_strcpy(argv[i]);
		}
	}
	return copy;
}

void combine_last_two_args(int &argc, char **argv)
{
	if (argc > 1)
	{
		char *tmp = new char[strlen(argv[argc - 2]) + strlen(argv[argc - 1]) + 4];
		ssi_sprint(tmp, """%s %s""", argv[argc - 2], argv[argc - 1]);
		delete[] argv[argc - 1];
		argv[argc - 1] = 0;
		delete[] argv[argc - 2];
		argv[argc - 2] = tmp;
		argc--;
	}
}

int main(int argc, char **argv) {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

#if _WIN32|_WIN64
		//set up windows event listener to catch console close event
		if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE) == FALSE) {
			ssi_print("WARNING: Unable to install console handler!\n");
		}
#endif

		int n_arguments = argc;
		char **arguments = copy_args(argc, argv);

		while (!Parse_and_Run(n_arguments, arguments) && n_arguments > 2)
		{
			combine_last_two_args(n_arguments, arguments);
		}

		delete_args(n_arguments, arguments);

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
	params.filepath = 0;
	params.debug_path = 0;
	params.config_str = 0;
	params.config_paths = 0;
	params.srcurl = 0;
	params.init_only = false;
	params.export_dlls = false;
	params.show_close_button = false;
	params.save_pipe = false;
	params.enable_quick_edit = false;

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("filepath", &params.filepath, "path to pipeline");

	cmd.addText("\nOptions:");
	cmd.addSCmdOption("-confstr", &params.config_str, "", "string with key=value pairs (seperated by ;) []");
	cmd.addSCmdOption("-config", &params.config_paths, "", "paths to global config files (seperated by ;) []");	
	cmd.addBCmdOption("-save", &params.save_pipe, false, "save pipeline after applying config files [false]");
	cmd.addBCmdOption("-init", &params.init_only, false, "only initialize pipepline [false]");
	cmd.addSCmdOption("-url", &params.srcurl, default_source, "override default url for downloading missing dlls and dependencies");	
	cmd.addSCmdOption("-log", &params.debug_path, "", "debug to a file [""]");
	cmd.addSCmdOption("-debug", &params.debug_path, "", "debug to a file [""] (deprecated use 'log')");
	cmd.addSCmdOption("-log2socket", &params.debug_socket, "", "debug to an udp socket (<host>:<port>) [""]");
	cmd.addBCmdOption("-export", &params.export_dlls, false, "copy registered dlls to working directory [false]");
	cmd.addBCmdOption("-closebut", &params.show_close_button, false, "enable close button in menu of console [false]");
	cmd.addBCmdOption("-quickedit", &params.enable_quick_edit, false, "enable quick edit mode [false]");

	if (!cmd.read(argc, argv))
	{
		return false;
	}

	if (params.debug_path[0] != '\0')
	{
		ssimsg = new FileMessage(params.debug_path);
	}
	else if (params.debug_socket[0] != '\0')
	{
		ssi_size_t n = ssi_split_string_count(params.debug_path, ':');
		if (n > 1) {
			ssi_char_t **tokens = new ssi_char_t *[n];
			ssi_split_string(n, tokens, params.debug_path, ':');
			ssi_size_t port = 0;
			sscanf(tokens[1], "%u", &port);
			ssimsg = new SocketMessage(Socket::TYPE::UDP, port, tokens[0]);
			for (ssi_size_t i = 0; i < n; i++) {
				delete[] tokens[i];
			}
			delete[] tokens;
		}
	}

	ssi_print("%s", info);

	if (params.config_paths[0] != '\0')
	{
		Run(argv[0], params);
	}
	else
	{
		Run(argv[0], params);
	}

	if (params.debug_path[0] != '\0' || params.debug_socket[0] != '\0') {
		delete ssimsg;
		ssimsg = 0;
	}

	delete[] params.filepath;
	delete[] params.config_str;
	delete[] params.config_paths;
	delete[] params.debug_path;
	delete[] params.debug_socket;
	delete[] params.srcurl;

	return true;
}

void Run(const ssi_char_t *exepath, params_t params) {

	// disable close button
	if (!params.show_close_button) {
		RemoveCloseButton();
	}
	SetQuickEditMode(params.enable_quick_edit);

	// set directories
	FilePath exepath_fp(exepath);
	ssi_getcwd(SSI_MAX_CHAR, _workdir);
	ssi_char_t exedir[SSI_MAX_CHAR];
	if (exepath_fp.isRelative()) {
#if _WIN32|_WIN64
		ssi_sprint(exedir, "%s\\%s", _workdir, exepath_fp.getDir());
#else
		ssi_sprint(exedir, "%s/%s", _workdir, exepath_fp.getDir());
#endif
	}
	else {
		strcpy(exedir, exepath_fp.getDir());
	}
	ssi_print("download source=%s\ndownload target=%s\n\n", params.srcurl, exedir);
	Factory::SetDownloadDirs(params.srcurl, exedir);

	// register frame and event dll
	Factory::RegisterDLL("frame", ssiout, ssimsg);
	Factory::RegisterDLL("event", ssiout, ssimsg);

	// full pipepath
	FilePath filepath_fp(params.filepath);
	ssi_char_t pipepath[SSI_MAX_CHAR];
	if (filepath_fp.isRelative()) {
#if _WIN32|_WIN64
		ssi_sprint(pipepath, "%s\\%s", _workdir, params.filepath);
#else
		ssi_sprint(pipepath, "%s/%s", _workdir, params.filepath);

#endif
	}
	else {
		strcpy(pipepath, params.filepath);
	}

	// set working directory to pipeline directory		
	ssi_setcwd(filepath_fp.getDir());

	XMLPipeline *xmlpipe = ssi_create_id(XMLPipeline, 0, "xmlpipe");
	xmlpipe->SetRegisterXMLFptr(Factory::RegisterXML);

	bool result = false;
	if (params.config_paths && params.config_paths[0] != '\0') {
		ssi_size_t n = 0;
		n = ssi_split_string_count(params.config_paths, ';');
		ssi_char_t **ns = new ssi_char_t *[n];
		ssi_split_string(n, ns, params.config_paths, ';');
		result = xmlpipe->parse(pipepath, params.config_str, n, ns, params.save_pipe);
	}
	else {
		result = xmlpipe->parse(pipepath, params.config_str, 0, 0, params.save_pipe);
	}

	if (!result) {
		ssi_err("could not parse pipeline from '%s'", pipepath);
		ssi_print("\n\n\n\t\tpress enter to quit\n");
		getchar();
	}
	else {

		if (params.export_dlls) {
			Factory::ExportDlls(exedir);
		}

		if (!params.init_only) {

			ITheFramework *frame = frame = Factory::GetFramework();
			ITheEventBoard *eboard = 0;
			if (xmlpipe->startEventBoard())
			{
				eboard = Factory::GetEventBoard();
			}

			do
			{
				if (xmlpipe->startEventBoard())
				{
					eboard->Start();
				}
				frame->Start();
				frame->Wait();
				frame->Stop();
				if (eboard)
				{
					eboard->Stop();
				}
			} while (frame->DoRestart());

			frame->Clear();
			if (eboard) {
				eboard->Clear();
			}

		}
		else
		{
			ssi_print("SUCCESS: initialization ok");
			ssi_print("\n\n\t\tpress enter to quit\n");
			getchar();
		}
	}

	Factory::Clear();

	ssi_setcwd(filepath_fp.getDir());
}

#if _WIN32|_WIN64
BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
	switch (CEvent)
	{
	case CTRL_SHUTDOWN_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:

		ITheFramework *frame = Factory::GetFramework();
		frame->CancelWait();

		break;
	}
	return TRUE;
}
#endif

void RemoveCloseButton() {

#if _WIN32|_WIN64

	HWND hWnd = ::GetConsoleWindow();
	// disable the [x] button.
	if (hWnd != NULL)
	{
		HMENU hMenu = ::GetSystemMenu(hWnd, 0);
		if (hMenu != NULL)
		{
			::DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
			::DrawMenuBar(hWnd);
		}
	}

#endif

}

void SetQuickEditMode(bool flag) {

#if _WIN32|_WIN64

	HANDLE hStdin = ::GetStdHandle(STD_INPUT_HANDLE);
	if (hStdin != INVALID_HANDLE_VALUE)
	{
		DWORD mode = 0;
		::GetConsoleMode(hStdin, &mode);
		if (flag) {
			::SetConsoleMode(hStdin, mode | ENABLE_QUICK_EDIT_MODE);
		}
		else {
			::SetConsoleMode(hStdin, mode & (~ENABLE_QUICK_EDIT_MODE));
		}
	}

#endif

}
