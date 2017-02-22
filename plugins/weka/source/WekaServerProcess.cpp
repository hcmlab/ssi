// WekaServerProcess.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/03/04
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

#include "WekaServerProcess.h"

namespace ssi {

void WekaServerProcess::enter () {

	_is_running = false;
}

void WekaServerProcess::run () {

	ZeroMemory ( &_startup_info, sizeof (_startup_info));
    _startup_info.cb = sizeof (_startup_info);
    ZeroMemory (&_process_info, sizeof (_process_info));
 
	ssi_char_t cmd[SSI_MAX_CHAR];
	ssi_sprint (cmd, "java.exe -classpath \"%s\" WekaServer \"%s\" %u %u", _class_path, _model, _port, _n_buffer);
	ssi_msg (SSI_LOG_LEVEL_BASIC, "start process: %s", cmd);

    if (!CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &_startup_info, &_process_info)) {
        ssi_err ("CreateProcess() failed (%d)", GetLastError());
        return;
    } 

	ssi_msg (SSI_LOG_LEVEL_BASIC, "java process started");
	_is_running = true;

	WaitForSingleObject (_process_info.hProcess, INFINITE);

	ssi_msg (SSI_LOG_LEVEL_BASIC, "java process stopped");
}

void WekaServerProcess::flush () {

	if (_is_running) {
		CloseHandle (_process_info.hProcess );
		CloseHandle (_process_info.hThread );
	}

	ssi_msg (SSI_LOG_LEVEL_BASIC, "java process destroyed");
}

size_t WekaServerProcess::ExecuteProcess (std::wstring FullPathToExe, std::wstring Parameters, size_t SecondsToWait)
{
    size_t iMyCounter = 0, iReturnVal = 0, iPos = 0;
    DWORD dwExitCode = 0;
    std::wstring sTempStr = L"";

    /* - NOTE - You should check here to see if the exe even exists */

    /* Add a space to the beginning of the Parameters */
    if (Parameters.size() != 0)
    {
        if (Parameters[0] != L' ')
        {
            Parameters.insert(0,L" ");
        }
    }

    /* The first parameter needs to be the exe itself */
    sTempStr = FullPathToExe;
    iPos = sTempStr.find_last_of(L"\\");
    sTempStr.erase(0, iPos +1);
    Parameters = sTempStr.append(Parameters);

     /* CreateProcessW can modify Parameters thus we allocate needed memory */
    wchar_t * pwszParam = new wchar_t[Parameters.size() + 1];
    if (pwszParam == 0)
    {
        return 1;
    }
    const wchar_t* pchrTemp = Parameters.c_str();
    wcscpy_s(pwszParam, Parameters.size() + 1, pchrTemp);

    /* CreateProcess API initialization */
    STARTUPINFOW siStartupInfo;
    PROCESS_INFORMATION piProcessInfo;
    memset(&siStartupInfo, 0, sizeof(siStartupInfo));
    memset(&piProcessInfo, 0, sizeof(piProcessInfo));
    siStartupInfo.cb = sizeof(siStartupInfo);

    if (CreateProcessW(const_cast<LPCWSTR>(FullPathToExe.c_str()),
                            pwszParam, 0, 0, false,
                            CREATE_DEFAULT_ERROR_MODE, 0, 0,
                            &siStartupInfo, &piProcessInfo) != false)
    {
         /* Watch the process. */
        dwExitCode = WaitForSingleObject(piProcessInfo.hProcess, (SecondsToWait * 1000));
    }
    else
    {
        /* CreateProcess failed */
        iReturnVal = GetLastError();
    }

    /* Free memory */
    delete[]pwszParam;
    pwszParam = 0;

    /* Release handles */
    CloseHandle(piProcessInfo.hProcess);
    CloseHandle(piProcessInfo.hThread);

    return iReturnVal;
} 

}
