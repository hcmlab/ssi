// ssipylog.c
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2017/11/16
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

// Copyright (C) 2011 Mateusz Loskot <mateusz@loskot.net>
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Blog article: http://mateusz.loskot.net/post/2011/12/01/python-sys-stdout-redirection-in-cpp/
// Github: https://github.com/mloskot/workshop/tree/master/python

#include "ssipylog.h"

// Internal state
PyObject* ssipylog_g_stdout;
PyObject* ssipylog_g_stdout_saved;

PyObject *ssipylog_stdout_write(PyObject *self, PyObject *args)
{
    size_t written = 0;
	ssipylog_stdout *selfimpl = (ssipylog_stdout *)self;
    if (selfimpl->write)
    {
        char* data;
        if (!PyArg_ParseTuple(args, "s", &data))
            return 0;

        selfimpl->write(data);
        written = strlen(data);
    }
    return PyLong_FromSize_t(written);
}

PyObject *ssipylog_stdout_flush(PyObject *self, PyObject *args)
{
    // no-op
    return Py_BuildValue("");
}

PyMethodDef ssipylog_stdout_methods[] =
{
    {"write", ssipylog_stdout_write, METH_VARARGS, "sys.stdout.write"},
    {"flush", ssipylog_stdout_flush, METH_VARARGS, "sys.stdout.write"},
    {0, 0, 0, 0} // sentinel
};

PyTypeObject ssipylog_stdout_Type =
{
    PyVarObject_HEAD_INIT(0, 0)
    "ssipylog.stdout",     /* tp_name */
    sizeof(ssipylog_stdout),       /* tp_basicsize */
    0,                    /* tp_itemsize */
    0,                    /* tp_dealloc */
    0,                    /* tp_print */
    0,                    /* tp_getattr */
    0,                    /* tp_setattr */
    0,                    /* tp_reserved */
    0,                    /* tp_repr */
    0,                    /* tp_as_number */
    0,                    /* tp_as_sequence */
    0,                    /* tp_as_mapping */
    0,                    /* tp_hash  */
    0,                    /* tp_call */
    0,                    /* tp_str */
    0,                    /* tp_getattro */
    0,                    /* tp_setattro */
    0,                    /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,   /* tp_flags */
    "ssipylog.stdout objects", /* tp_doc */
    0,                    /* tp_traverse */
    0,                    /* tp_clear */
    0,                    /* tp_richcompare */
    0,                    /* tp_weaklistoffset */
    0,                    /* tp_iter */
    0,                    /* tp_iternext */
	ssipylog_stdout_methods,       /* tp_methods */
    0,                    /* tp_members */
    0,                    /* tp_getset */
    0,                    /* tp_base */
    0,                    /* tp_dict */
    0,                    /* tp_descr_get */
    0,                    /* tp_descr_set */
    0,                    /* tp_dictoffset */
    0,                    /* tp_init */
    0,                    /* tp_alloc */
    0,                    /* tp_new */
};

PyMODINIT_FUNC PyInit_ssipylog(void)
{
	ssipylog_g_stdout = 0;
	ssipylog_g_stdout_saved = 0;

	ssipylog_stdout_Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&ssipylog_stdout_Type) < 0)
        return 0;

    PyObject* m = PyModule_Create(&ssipylog_module);
    if (m)
    {
        Py_INCREF(&ssipylog_stdout_Type);
        PyModule_AddObject(m, "stdout", (PyObject *)&ssipylog_stdout_Type);
    }
    return m;
}

void ssipylog_stdout_set(ssipylog_stdout_write_type write)
{
    if (!ssipylog_g_stdout)
    {
		ssipylog_g_stdout_saved = PySys_GetObject("stdout"); // borrowed
		ssipylog_g_stdout = ssipylog_stdout_Type.tp_new(&ssipylog_stdout_Type, 0, 0);
    }

	ssipylog_stdout *impl = (ssipylog_stdout *)ssipylog_g_stdout;
    impl->write = write;
    PySys_SetObject("stdout", ssipylog_g_stdout);
}

void ssipylog_stdout_reset()
{
    if (ssipylog_g_stdout_saved)
        PySys_SetObject("stdout", ssipylog_g_stdout_saved);

    Py_XDECREF(ssipylog_g_stdout);
	ssipylog_g_stdout = 0;
}
