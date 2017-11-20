// ssipylog.h
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

#pragma once

#ifndef SSI_SSIPYLOG_H
#define SSI_SSIPYLOG_H

#include <Python.h>

typedef void(*ssipylog_stdout_write_type)(const char *);

typedef struct _ssipylog_stdout
{
	PyObject_HEAD
	ssipylog_stdout_write_type write;
} ssipylog_stdout;

static PyModuleDef ssipylog_module =
{
	PyModuleDef_HEAD_INIT,
	"emb", 0, -1, 0,
};

PyMODINIT_FUNC PyInit_ssipylog(void);
PyObject *ssipylog_stdout_write(PyObject *self, PyObject *args);
PyObject *ssipylog_stdout_flush(PyObject *self, PyObject *args);
void ssipylog_stdout_set(ssipylog_stdout_write_type write);
void ssipylog_stdout_reset();

#endif