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

ssi_char_t _workdir[SSI_MAX_CHAR];

struct params_t {
	ssi_char_t *exepath;
	ssi_char_t *filepath;
	ssi_char_t *debug_path;
	ssi_char_t *config_paths;
	bool init_only;
	bool debug_to_file;
	bool save_pipe;
	bool init;
	bool export_dlls;
	bool show_close_button;
	bool enable_quick_edit;
};

void Run(const ssi_char_t *exepath, params_t params);
void RemoveCloseButton();
void SetQuickEditMode(bool flag);

#if _WIN32|_WIN64
BOOL WINAPI ConsoleHandler(DWORD CEvent);
#endif

int main (int argc, char **argv) {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	char info[1024];
	ssi_sprint (info, "\n%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

#if _WIN32|_WIN64
	//set up windows event listener to catch console close event
	if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE) == FALSE) {
		ssi_print("WARNING: Unable to install console handler!\n");
	}
#endif

	//**** READ COMMAND LINE ****//

	CmdArgParser cmd;
	cmd.info (info);

	params_t params;	
	params.filepath = 0;
	params.debug_path = 0;
	params.config_paths = 0;
	params.init_only = false;
	params.debug_to_file = false;
	params.export_dlls = false;
	params.show_close_button = false;
	params.save_pipe = false;
	params.enable_quick_edit = false;
	
	cmd.addText("\nArguments:");
	cmd.addSCmdArg("filepath", &params.filepath, "path to pipeline");

	cmd.addText ("\nOptions:");
	cmd.addSCmdOption("-config", &params.config_paths, "", "paths to global config files (seperated by ;) []");
	cmd.addBCmdOption("-save", &params.save_pipe, false, "save pipeline after applying config files [false]");
	cmd.addBCmdOption("-init", &params.init_only, false, "only initialize pipepline [false]");
	cmd.addBCmdOption("-dbg2file", &params.debug_to_file, false, "debug to file [false] (deprecated, use -debug instead)");
	cmd.addSCmdOption("-debug", &params.debug_path, "", "debug to a file or stream to an udp socket (<host>:<port>) [""]");
	cmd.addBCmdOption("-export", &params.export_dlls, false, "copy registered dlls to working directory [false]");
	cmd.addBCmdOption("-closebut", &params.show_close_button, false, "enable close button in menu of console [false]");
	cmd.addBCmdOption("-quickedit", &params.enable_quick_edit, false, "enable quick edit mode [false]");

	if (cmd.read (argc, argv)) {				

		if (params.debug_to_file) {
			ssi_log_file_begin ("ssi_dbg.txt");
		}
		if (params.debug_path[0] != '\0') {
			ssi_size_t n = ssi_split_string_count (params.debug_path, ':');
			if (n > 1) {
				ssi_char_t **tokens = new ssi_char_t *[n];
				ssi_split_string(n, tokens, params.debug_path, ':');
				ssi_size_t port = 0;
				sscanf (tokens[1], "%u", &port);
				ssimsg = new SocketMessage (Socket::UDP, port, tokens[0]);
				for (ssi_size_t i = 0; i < n; i++) {
					delete[] tokens[i];
				}
				delete[] tokens;
			} else {
				ssimsg = new FileMessage(params.debug_path);
			}
		}
		ssi_print ("%s", info);

		if (params.config_paths[0] != '\0') {
			Run(argv[0], params);
		} else {
			Run(argv[0], params);
		}

		if (params.debug_to_file){
			ssi_log_file_end ();
		}
		if (params.debug_path[0] != '\0') {
			delete ssimsg;
			ssimsg = 0;
		}

	}

	delete[] params.filepath;
	delete[] params.config_paths;
	delete[] params.debug_path;
	
#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

void Run (const ssi_char_t *exepath, params_t params) {	

	Factory::RegisterDLL ("ssiframe", ssiout, ssimsg);
	Factory::RegisterDLL ("ssievent", ssiout, ssimsg);

	// get working directory
	ssi_getcwd(SSI_MAX_CHAR, _workdir);

	// disable close button
	if (!params.show_close_button) {
		RemoveCloseButton ();
	}
	SetQuickEditMode(params.enable_quick_edit);

	// executable directory
	FilePath exepath_fp (exepath);	
	ssi_char_t exedir[SSI_MAX_CHAR];
	if (exepath_fp.isRelative ()) {
		#if _WIN32|_WIN64
		ssi_sprint (exedir, "%s\\%s", _workdir, exepath_fp.getDir ());
		#else
		ssi_sprint (exedir, "%s/%s", _workdir, exepath_fp.getDir ());
		#endif
	} else {
		strcpy (exedir, exepath_fp.getDir ());
	}

	// full pipepath
	FilePath filepath_fp(params.filepath);
	ssi_char_t pipepath[SSI_MAX_CHAR];
	if (filepath_fp.isRelative ()) {
		#if _WIN32|_WIN64
		ssi_sprint(pipepath, "%s\\%s", _workdir, params.filepath);
		#else
		ssi_sprint (pipepath, "%s/%s", _workdir, params.filepath);

		#endif
	} else {
		strcpy(pipepath, params.filepath);
	}

	// set working directory to pipeline directory		
	ssi_setcwd(filepath_fp.getDir ());

	XMLPipeline *xmlpipe = ssi_create (XMLPipeline, 0, true);

	xmlpipe->SetRegisterDllFptr (Factory::RegisterDLL);
	bool result = false;
	if (params.config_paths && params.config_paths[0] != '\0') {
		ssi_size_t n = 0;
		n = ssi_split_string_count(params.config_paths, ';');
		ssi_char_t **ns = new ssi_char_t *[n];
		ssi_split_string(n, ns, params.config_paths, ';');
		result = xmlpipe->parse(pipepath, n, ns, params.save_pipe);
	} else {
		result = xmlpipe->parse(pipepath, 0, 0, params.save_pipe);
	}

	if (!result) {
		ssi_print ("ERROR: could not parse pipeline from '%s'\n", pipepath);
		ssi_print ("\n\n\t\tpress enter to quit\n");
		getchar ();
	} else {

		if (params.export_dlls) {
			Factory::ExportDlls (exedir);
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
			} 
			while (frame->DoRestart());
			
			frame->Clear();
			if (eboard) {
				eboard->Clear();
			}

		} else 
		{
			ssi_print ("SUCCESS: initialization ok");
			ssi_print ("\n\n\t\tpress enter to quit\n");
			getchar ();
		}
	}

	Factory::Clear ();

	ssi_setcwd(filepath_fp.getDir());
}

#if _WIN32|_WIN64
BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
    switch(CEvent)
    {
    case CTRL_CLOSE_EVENT:        	
		
		ITheFramework *frame = Factory::GetFramework ();
		ITheEventBoard *eboard = Factory::GetEventBoard ();
		
		frame->Stop ();			
		if (eboard) {
			eboard->Stop ();
		}
		frame->Clear ();
		if (eboard) {
			eboard->Clear ();
		}	

		Factory::Clear ();
		ssi_setcwd (_workdir);

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
