// Main.cpp
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 2015/06/05
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
using namespace ssi;

extern "C" {
#include "ssipy.h"
}

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

bool ex_foo(void *args);
bool ex_ssipy(void *args);
bool ex_ssipy_script(void *args);
bool ex_ssipy_script_arg(void *args);
bool ex_ssipy_script_arg_npy(void *args);


ssipyevent *convert(ssi_event_t &event)
{
	PyObject *data = NULL;

	if (event.type == SSI_ETYPE_UNDEF)
	{
		PyErr_SetString(PyExc_TypeError, "event type is undefined");
		return NULL;
	}

	switch (event.type)
	{
	case SSI_ETYPE_EMPTY:
		break;
	case SSI_ETYPE_STRING:
		data = PyUnicode_FromString((char*)event.ptr);
		break;
	case SSI_ETYPE_TUPLE:
	{
		ssi_event_tuple_t *tuple = (ssi_event_tuple_t *)event.ptr;
		Py_ssize_t n = (Py_ssize_t) (event.tot / sizeof(ssi_event_tuple_t));
		data = PyTuple_New(n);
		for (Py_ssize_t i = 0; i < n; i++)
		{
			PyTuple_SetItem(data, i, PyFloat_FromDouble(tuple[i]));
		}
		break;
	}
	case SSI_ETYPE_MAP:
	{
		ssi_event_map_t *map = (ssi_event_map_t *)event.ptr;
		Py_ssize_t n = (Py_ssize_t) (event.tot / sizeof(ssi_event_map_t));
		data = PyDict_New();
		for (Py_ssize_t i = 0; i < n; i++)
		{
			PyObject *key = PyUnicode_FromString(Factory::GetString(map[i].id));
			PyObject *value = PyFloat_FromDouble(map[i].value);
			PyDict_SetItem(data, key, value);
			Py_DECREF(key);
			Py_DECREF(value);
		}
		break;
	}
	}

	EventAddress ea;
	ea.setEvents(Factory::GetString(event.event_id));
	ea.setSender(Factory::GetString(event.sender_id));
	
	return ssipyevent_New(event.time, event.dur, ea.getAddress(), data, event.state, event.glue_id, event.prob);
}

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);
		
	Py_Initialize();
	PyInit_ssipy();

	PyObject* sysPath = PySys_GetObject((char*)"path");
	ssi_char_t *full = ssi_fullpath(".");
	PyObject *pPath = PyUnicode_FromString(full);
	PyList_Append(sysPath, pPath);
	Py_DECREF(pPath);
	delete[] full;

	ssi_type_t type = SSI_INT;

	Exsemble ex;
	ex.console(0, 0, 800, 600);		
	ex.add(ex_foo, 0, "FOO", "Calling python script from a string and from a file.");		
	ex.add(ex_ssipy, &type, "SSIPY - C", "Using ssipy module from c.");
	ex.add(ex_ssipy_script, 0, "SSIPY - SCRIPT", "Using ssipy module in a script.");
	ex.add(ex_ssipy_script_arg, &type, "SSIPY - SCRIPT + ARGUMENT", "Using a stream as an argument in a script.");
#ifndef DEBUG
	ex.add(ex_ssipy_script_arg_npy, &type, "SSIPY - SCRIPT + ARGUMENT + NUMPY", "Converting a stream to a numpy array.");
#endif
	ex.show();

	Py_Finalize();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool ex_foo(void *args)
{
	ssi_print("%s", Py_GetVersion());
	
	ssi_print("\nCALLING PYTHON FROM STRING\n\n")

	PyRun_SimpleString("from time import time,ctime\n"
		"print('Today is', ctime(time()))\n");	

	ssi_print("\nCALLING PYTHON FROM FILE\n\n")

	PyObject *pModule = PyImport_ImportModule("foo");
	if (!pModule) {
		PyErr_Print();
	}

	return true;
}

void update(void *client, ssipyevent *event)
{
	ssipyevent_Print(event);
}

bool ex_ssipy(void *args)
{	
	ssipytype *type = ssipytype_New();
	Py_DECREF(type);

	ssipyinfo *info = ssipyinfo_New(1.0, 2.0, 30, 40);	
	Py_DECREF(info);

	ssipychannel *channel = ssipychannel_New(2, sizeof(ssi_real_t), SSI_FLOAT, 100.0);
	Py_DECREF(info);

	ssipyimageparams *imageparams = ssipyimageparams_New(320, 160, 3, 1);
	Py_DECREF(imageparams);

	ssipyimage *image = ssipyimage_New(320, 160, 3, 1, 0, 0);

	PyObject* pStr = PyObject_Str((PyObject *)image);
	char *str = PyUnicode_AsUTF8(pStr);
	printf("%s\n", str);
	Py_DECREF(pStr);

	Py_DECREF(image);

	if (PyErr_Occurred())
	{
		PyErr_Print();
	}

	ssipyarray *array = ssipyarray_New(5);

	ssipyarray_Print(array);

	for (Py_ssize_t i = 0; i < array->len; i++)
	{
		PyObject *value = PyFloat_FromDouble((double)i);
		PyObject *key = PyLong_FromSsize_t(i);

		PyObject_SetItem((PyObject *)array, key, value);

		if (PyErr_Occurred())
		{
			PyErr_Print();
		}

		Py_DECREF(value);
		Py_DECREF(key);
	}

	PyObject *iter = PyObject_GetIter((PyObject *)array);

	if (PyErr_Occurred())
	{
		PyErr_Print();
	}

	PyObject *item;
	while (item = PyIter_Next(iter)) {		
		PyObject* pStr = PyObject_Str(item);
		char *str = PyUnicode_AsUTF8(pStr);
		printf("%s\n", str);
		Py_DECREF(pStr);
		Py_DECREF(item);
	}
	Py_DECREF(iter);

	if (PyErr_Occurred())
	{
		PyErr_Print();
	}

	ssipyarray_Print(array);

	Py_DECREF(array);


	ssipystream *stream = ssipystream_New(5, 3, *((ssi_type_t *)args), 0, 0);

	ssipystream_Print(stream);

	for (Py_ssize_t i = 0; i < stream->len; i++)
	{
		PyObject *value = PyLong_FromSsize_t(i);
		PyObject *key = PyLong_FromSsize_t(i);

		PyObject_SetItem((PyObject *)stream, key, value);

		if (PyErr_Occurred())
		{
			PyErr_Print();
		}

		Py_DECREF(value);
		Py_DECREF(key);
	}

	iter = PyObject_GetIter((PyObject *)stream);

	if (PyErr_Occurred())
	{
		PyErr_Print();
	}

	item;
	while (item = PyIter_Next(iter)) {		
		PyObject* pStr = PyObject_Str(item);
		char *str = PyUnicode_AsUTF8(pStr);
		printf("%s\n", str);
		Py_DECREF(pStr);
		Py_DECREF(item);
	}
	Py_DECREF(iter);

	if (PyErr_Occurred())
	{
		PyErr_Print();
	}

	ssipystream_Print(stream);

	Py_DECREF(stream);

	ssi_size_t event_id = Factory::AddString("event");
	ssi_size_t sender_id = Factory::AddString("sender");

	ssipyeventboard *board = ssipyeventboard_New(0, update);

	{
		ssi_event_t event;
		ssi_event_init(event, SSI_ETYPE_EMPTY, event_id, sender_id);
		ssipyevent *pEvent = convert(event);
		board->update(0, pEvent);		
		Py_DECREF(pEvent);
	}

	{
		ssi_event_t event;
		ssi_char_t *string = "hello world";
		ssi_event_init(event, SSI_ETYPE_STRING, event_id, sender_id);
		event.tot_real = event.tot = ssi_strlen(string) + 1;
		event.ptr = (ssi_byte_t *)string;
		ssipyevent *pEvent = convert(event);		
		board->update(0, pEvent);
		Py_DECREF(pEvent);		
	}

	{
		ssi_event_t event;
		ssi_size_t n_tuple = 4;
		ssi_event_tuple_t tuple[] = { 0, 1, 2, 3 };
		ssi_event_init(event, SSI_ETYPE_TUPLE, event_id, sender_id);
		event.tot_real = event.tot = n_tuple * sizeof(ssi_event_tuple_t);
		event.ptr = (ssi_byte_t *)tuple;
		ssipyevent *pEvent = convert(event);
		board->update(0, pEvent);
		Py_DECREF(pEvent);		
	}

	{
		ssi_event_t event;
		ssi_size_t n_map = 4;
		ssi_event_map_t map[4];
		for (ssi_size_t n = 0; n < n_map; n++)
		{
			char string[10];
			ssi_sprint(string, "id%u", n);
			map[n].id = Factory::AddString(string);
			map[n].value = ssi_real_t (n);
		}
		ssi_event_init(event, SSI_ETYPE_MAP, event_id, sender_id);
		event.tot_real = event.tot = n_map * sizeof(ssi_event_map_t);
		event.ptr = (ssi_byte_t *)map;
		ssipyevent *pEvent = convert(event);
		board->update(0, pEvent);
		Py_DECREF(pEvent);
	}

	Py_DECREF(board);

	Factory::Clear();

	return true;
}

bool ex_ssipy_script(void *args)
{
	PyObject *pModule = PyImport_ImportModule("ssipy_test");
	if (pModule)
	{
		PyObject *pFunc = PyObject_GetAttrString(pModule, "call");
		if (pFunc)
		{
			PyObject *pArgs = PyTuple_New(0);
			PyObject_Call(pFunc, pArgs, NULL);
			Py_DECREF(pArgs);
			Py_DECREF(pFunc);
		}
		else
		{
			PyErr_Print();
		}
		Py_DECREF(pModule);
	}
	else
	{
		PyErr_Print();
	}

	if (PyErr_Occurred())
	{
		PyErr_Print();
	}

	return true;
}

bool ex_ssipy_script_arg(void *args)
{
	ssipystream *pStream = ssipystream_New(5, 3, *((ssi_type_t *)args), 0, 0);
	ssipyarray *pArray = ssipyarray_New(5);
	ssipyarray_Print(pArray);
	ssipystream_Print(pStream);

	ssi_event_t event;
	ssi_char_t *string = "hello python";
	ssi_event_init(event, SSI_ETYPE_STRING, Factory::AddString("event"), Factory::AddString("sender"));
	event.tot_real = event.tot = ssi_strlen(string) + 1;
	event.ptr = (ssi_byte_t *)string;
	ssipyevent *pEvent = convert(event);	

	ssipyeventboard *pBoard = ssipyeventboard_New(0, update);

	PyObject *pModule = PyImport_ImportModule("ssipy_test_arg");
	if (pModule)
	{
		PyObject *pFunc = 0;

		// send event

		pFunc = PyObject_GetAttrString(pModule, "update");
		if (pFunc)
		{
			PyObject *pArgs = PyTuple_New(1);
			PyTuple_SetItem(pArgs, 0, (PyObject*)pEvent);
			Py_INCREF(pEvent);

			PyObject_Call(pFunc, pArgs, NULL);

			Py_DECREF(pArgs);
			Py_DECREF(pFunc);
		}
		else
		{
			PyErr_Print();
		}

		// receive event

		pFunc = PyObject_GetAttrString(pModule, "eboard");
		if (pFunc)
		{
			PyObject *pArgs = PyTuple_New(1);
			PyTuple_SetItem(pArgs, 0, (PyObject*)pBoard);
			Py_INCREF(pBoard);

			PyObject_Call(pFunc, pArgs, NULL);

			Py_DECREF(pArgs);
			Py_DECREF(pFunc);
		}
		else
		{
			PyErr_Print();
		}
		
		// array
		pFunc = PyObject_GetAttrString(pModule, "callArray");
		if (pFunc)
		{
			PyObject *pArgs = PyTuple_New(1);
			PyTuple_SetItem(pArgs, 0, (PyObject*)pArray);
			Py_INCREF(pArray);

			printf("\nBEFORE SCRIPT()\n\n");
			PyObject_Call(pFunc, pArgs, NULL);
			printf("\nAFTER SCRIPT\n");

			Py_DECREF(pArgs);
			Py_DECREF(pFunc);
		}
		else
		{
			PyErr_Print();
		}


		// stream

		pFunc = PyObject_GetAttrString(pModule, "call");
		if (pFunc)
		{
			PyObject *pArgs = PyTuple_New(1);
			PyTuple_SetItem(pArgs, 0, (PyObject*)pStream);
			Py_INCREF(pStream);

			printf("\nBEFORE SCRIPT()\n\n");
			PyObject_Call(pFunc, pArgs, NULL);
			printf("\nAFTER SCRIPT\n");
			
			Py_DECREF(pArgs);
			Py_DECREF(pFunc);
		}
		else
		{
			PyErr_Print();
		}		

		Py_DECREF(pModule);
	}
	else
	{
		PyErr_Print();
	}

	printf("\n");
	ssipystream_Print(pStream);
	printf("\n");

	Py_DECREF(pStream);
	Py_DECREF(pEvent);
	Py_DECREF(pBoard);

	if (PyErr_Occurred())
	{
		PyErr_Print();
	}

	return true;
}


bool ex_ssipy_script_arg_npy(void *args)
{
	ssipyarray *array = ssipyarray_New(5);
	ssipyarray_Print(array);

	ssipystream *stream = ssipystream_New(5, 3, *((ssi_type_t *)args), 0, 0);
	ssipystream_Print(stream);

	PyObject *pModule = PyImport_ImportModule("ssipy_test_arg_npy");
	if (pModule)
	{
		PyObject *pFunc = PyObject_GetAttrString(pModule, "callArray");
		if (pFunc)
		{
			PyObject *pArgs = PyTuple_New(1);
			PyTuple_SetItem(pArgs, 0, (PyObject*)array);
			Py_INCREF(array);

			printf("\nBEFORE SCRIPT()\n\n");
			PyObject_Call(pFunc, pArgs, NULL);
			printf("\nAFTER SCRIPT\n");

			Py_DECREF(pArgs);
			Py_DECREF(pFunc);
		}
		else
		{
			PyErr_Print();
		}

		pFunc = PyObject_GetAttrString(pModule, "call");
		if (pFunc)
		{
			PyObject *pArgs = PyTuple_New(1);
			PyTuple_SetItem(pArgs, 0, (PyObject*)stream);
			Py_INCREF(stream);

			printf("\nBEFORE SCRIPT()\n\n");
			PyObject_Call(pFunc, pArgs, NULL);
			printf("\nAFTER SCRIPT\n");

			Py_DECREF(pArgs);
			Py_DECREF(pFunc);
		}
		else
		{
			PyErr_Print();
		}
		Py_DECREF(pModule);
	}
	else
	{
		PyErr_Print();
	}

	printf("\n");
	ssipystream_Print(stream);
	printf("\n");

	Py_DECREF(stream);

	if (PyErr_Occurred())
	{
		PyErr_Print();
	}

	return true;
}
