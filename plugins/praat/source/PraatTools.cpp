// PraatTools.cpp
// author: Johannes Wagner
// created: 2014/06/05
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
// version 3 of the License, or any later version.
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

#include "PraatTools.h"
#include "ioput/wav/WavTools.h"
#include "ioput/file/FilePath.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {

	std::string PraatTools::RunPraat (ssi_stream_t stream, const ssi_char_t *exe, const ssi_char_t *script, const ssi_char_t *script_args, const ssi_char_t *tmpfile) {
		
		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "process audio slice with \"praat\"");

		ssi_char_t string[SSI_MAX_CHAR];
		FilePath fp (script);
		ssi_sprint (string, "%s\\%s", fp.getDir (), tmpfile);
		WavTools::WriteWavFile (string, stream);

		//run praat		
		std::string praatconarg = exe;
		praatconarg.append (" -a ");
		praatconarg.append (script);
		praatconarg.append (" ");
		if (script_args && script_args[0] != '\0') {
			praatconarg.append (script_args);
			praatconarg.append (" ");
		}
		praatconarg.append (tmpfile);
		
		return PraatTools::RunCommand (praatconarg.c_str());
	
	}


	//adapted version of http://stackoverflow.com/questions/10866944/how-can-i-read-a-child-processs-output
	std::string PraatTools::RunCommand (const ssi_char_t *command)
	{
		PROCESS_INFORMATION processInfo;
		STARTUPINFOA startupInfo; 
		SECURITY_ATTRIBUTES saAttr; 

		HANDLE stdoutReadHandle = NULL;
		HANDLE stdoutWriteHandle = NULL;

		char cmdline[256];
		std::string outbuf;
		DWORD bytes_read;
		char tBuf[257];

		DWORD exitcode;

		strcpy_s(cmdline, sizeof(cmdline), command);

		memset(&saAttr, 0, sizeof(saAttr));
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
		saAttr.bInheritHandle = TRUE; 
		saAttr.lpSecurityDescriptor = NULL; 

		// Create a pipe for the child process's STDOUT. 
		if (!CreatePipe(&stdoutReadHandle, &stdoutWriteHandle, &saAttr, 0))
		{
			ssi_msg (SSI_LOG_LEVEL_BASIC, "CreatePipe: %u\n", GetLastError());
			return NULL;
		}

		// Ensure the read handle to the pipe for STDOUT is not inherited.
		if (!SetHandleInformation(stdoutReadHandle, HANDLE_FLAG_INHERIT, 0))
		{
			ssi_msg (SSI_LOG_LEVEL_BASIC, "SetHandleInformation: %u\n", GetLastError());
			return NULL;
		}

		memset(&startupInfo, 0, sizeof(startupInfo));
		startupInfo.cb = sizeof(startupInfo);
		startupInfo.hStdError = stdoutWriteHandle;
		startupInfo.hStdOutput = stdoutWriteHandle;
		startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		startupInfo.dwFlags |= STARTF_USESTDHANDLES;

		if (!CreateProcessA(NULL, cmdline, NULL, NULL, TRUE,
			CREATE_NO_WINDOW, NULL, 0, &startupInfo, &processInfo))
		{
			ssi_msg (SSI_LOG_LEVEL_BASIC, "CreateProcessA: %u\n", GetLastError());
			return NULL;
		}

		CloseHandle(stdoutWriteHandle);

		for (;;) {
			if (!ReadFile(stdoutReadHandle, tBuf, 256, &bytes_read, NULL))
			{
				//ssi_msg (SSI_LOG_LEVEL_BASIC, "ReadFile: %u\n", GetLastError());
				break;
			}
			if (bytes_read > 0)
			{
				tBuf[bytes_read] = '\0';
				outbuf.append(tBuf);
			}
		}

		if (WaitForSingleObject(processInfo.hProcess, INFINITE) != WAIT_OBJECT_0)
		{
			ssi_msg (SSI_LOG_LEVEL_BASIC, "WaitForSingleObject: %u\n", GetLastError());
			return NULL;
		}

		if (!GetExitCodeProcess(processInfo.hProcess, &exitcode))
		{
			ssi_msg (SSI_LOG_LEVEL_BASIC, "GetExitCodeProcess: %u\n", GetLastError());
			return NULL;
		}

		CloseHandle( processInfo.hProcess );
		CloseHandle( processInfo.hThread );

		return outbuf;
	}

}
