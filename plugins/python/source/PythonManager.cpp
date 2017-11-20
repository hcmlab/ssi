// PythonManager.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/03/02
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

#include "PythonManager.h"

extern "C"
{
#include "ssipy.h"
#include "ssipylog.h"
}

namespace ssi {

ssi_char_t *PythonManager::ssi_log_name = "pymanager_";

PythonManager &PythonManager::Instance()
{
	static PythonManager instance;
	return instance;
}

PythonManager::PythonManager()
{
}

PythonManager::~PythonManager()
{
}

void PythonManager::log(const char *str)
{
	ssi_print("%s", str);	
}

void PythonManager::Init()
{	
	ssi_msg(SSI_LOG_LEVEL_BASIC, "init");

	//PyImport_AppendInittab("ssipylog", PyInit_ssipylog);
	Py_Initialize();
	//PyImport_ImportModule("ssipylog");

	PyEval_InitThreads();			
	
	PyInit_ssipy();
	PyInit_ssipylog();
	ssipylog_stdout_set(log);

	_state = PyEval_SaveThread();
}

void PythonManager::Quit()
{
	printf("[pymanager_] quit\n");

	PyEval_RestoreThread(_state);

	ssipylog_stdout_reset();

	Py_Finalize();
}


}