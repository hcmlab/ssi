// ssipy.h
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

#pragma once

#ifndef SSI_SSIPY_H
#define SSI_SSIPY_H

#include <Python.h>
#include "SSI_Define.h"

#define SSIPY_PRINT_NMAX 100

// type enumeration
typedef struct _ssipytype {
	PyObject_HEAD
	ssi_type_t UNDEF;
	ssi_type_t CHAR;
	ssi_type_t UCHAR;
	ssi_type_t SHORT;
	ssi_type_t USHORT;
	ssi_type_t INT;
	ssi_type_t UINT;
	ssi_type_t LONG;
	ssi_type_t ULONG;
	ssi_type_t FLOAT;
	ssi_type_t DOUBLE;
} ssipytype;
PyAPI_DATA(PyTypeObject) ssipytype_Type;

// consume & transform info
typedef struct _ssipyinfo {
	PyObject_HEAD
	ssi_time_t time;  // start time in s
	ssi_time_t dur;   // duration in s
	Py_ssize_t frame; // number of new frames
	Py_ssize_t delta; // number of overlapping frames
} ssipyinfo;
PyAPI_DATA(PyTypeObject) ssipyinfo_Type;

//array
typedef struct _ssipyarray {
	PyObject_HEAD
	Py_ssize_t len; //number of values
	ssi_real_t *ptr; // memory block
	int borrowed;    // is memory borrowed?	
} ssipyarray;
PyAPI_DATA(PyTypeObject) ssipyarray_Type;

typedef struct _ssipyarrayiter {
	PyObject_HEAD
		Py_ssize_t it_index;
	ssipyarray *it_array;
} ssipyarrayiter;
PyAPI_DATA(PyTypeObject) ssipyarrayiter_Type;

// stream
typedef struct _ssipystream {
	PyObject_HEAD
	Py_ssize_t len;  // number of values
	Py_ssize_t num;  // number of samples (=rows)
	Py_ssize_t dim;  // dimensions (=columns)	
	ssi_time_t sr;   // sample rate in hz
	ssi_time_t time; // time in seconds
	Py_ssize_t tot;  // total number of bytes
	ssi_type_t type; // type identifier
	char *type_s;	 // type string identifier
	Py_ssize_t byte; // bytes per value
	ssi_byte_t *ptr; // memory block	
	int borrowed;    // is memory borrowed?	
} ssipystream;
PyAPI_DATA(PyTypeObject) ssipystream_Type;

typedef struct _ssipystreamiter {
	PyObject_HEAD
	Py_ssize_t it_index;
	ssipystream *it_stream;
} ssipystreamiter;
PyAPI_DATA(PyTypeObject) ssipystreamiter_Type;

PyObject *ssipystream_ValueToObject(ssipystream *self, Py_ssize_t index);
void ssipystream_ValueFromObject(ssipystream *self, Py_ssize_t index, PyObject *value);
int ssipystream_ValueToString(ssipystream *self, Py_ssize_t index, char *string);

// image
typedef struct _ssipyimageparams {
	PyObject_HEAD
	Py_ssize_t width;     // width in pixels
	Py_ssize_t height;    // height in pixels
	Py_ssize_t channels;  // number of channels
	Py_ssize_t depth;     // bytes per channel	
} ssipyimageparams;
PyAPI_DATA(PyTypeObject) ssipyimageparams_Type;

typedef struct _ssipyimage {
	PyObject_HEAD
	Py_ssize_t width;		// width in pixels
	Py_ssize_t height;		// height in pixels
	Py_ssize_t channels;	// number of channels
	Py_ssize_t depth;		// bytes per channel	
	ssi_time_t sr;		    // sample rate in hz
	ssi_time_t time;        // time in seconds
	ssi_type_t stride;      // bytes per row
	Py_ssize_t tot;         // total number of bytes
	ssi_byte_t *ptr;	    // memory block	
	int borrowed;		    // is memory borrowed?	
} ssipyimage;
PyAPI_DATA(PyTypeObject) ssipyimage_Type;

// channel
typedef struct _ssipychannel {
	PyObject_HEAD		
	Py_ssize_t dim;  // dimensions (=columns)	
	Py_ssize_t byte; // bytes per value	
	ssi_type_t type; // type identifier	
	ssi_time_t sr;   // sample rate in hz				
} ssipychannel;
PyAPI_DATA(PyTypeObject) ssipychannel_Type;

// event
typedef struct _ssipyevent {
	PyObject_HEAD		
	Py_ssize_t time; // start time in ms
	Py_ssize_t dur; // duration in ms	
	char *address; // address <event@sender>
	PyObject *data; // data	
	ssi_estate_t state; // event status
	Py_ssize_t glue; // glue id	
	float prob; // confidence
} ssipyevent;
PyAPI_DATA(PyTypeObject) ssipyevent_Type;

ssi_etype_t ssipyevent_ObjectToType(PyObject *object);

typedef void(*ssipyupdatefunc_t)(void *client, ssipyevent *event);
typedef struct _ssipyeventboard {
	PyObject_HEAD
	void *client;
	ssi_etype_t COMPLETED;
	ssi_etype_t CONTINUED;
	ssipyupdatefunc_t update;
} ssipyeventboard;
PyAPI_DATA(PyTypeObject) ssipyeventboard_Type;

// model type
typedef struct _ssipymodeltype {
	PyObject_HEAD
	ssi_type_t CLASSIFICATION;
	ssi_type_t REGRESSION;	
} ssipymodeltype;
PyAPI_DATA(PyTypeObject) ssipymodeltype_Type;

// module
static PyModuleDef ssipy_module = {
	PyModuleDef_HEAD_INIT,
	"ssipy",
	"A python wrapper for ssi.",
	-1,
	NULL, NULL, NULL, NULL, NULL
};

// init
PyMODINIT_FUNC PyInit_ssipy(void);

// c functions for convenience
ssipytype *ssipytype_New();
void ssipytype_Init(ssipytype *self);

ssipyinfo *ssipyinfo_New(ssi_time_t time_s, ssi_time_t dur_s, Py_ssize_t frame, Py_ssize_t delta);
void ssipyinfo_Init(ssipyinfo *self, ssi_time_t time_s, ssi_time_t dur_s, Py_ssize_t frame, Py_ssize_t delta);

ssipychannel *ssipychannel_New(Py_ssize_t dim, Py_ssize_t byte, ssi_type_t type, ssi_time_t sr);
void ssipychannel_Init(ssipychannel *self, Py_ssize_t dim, Py_ssize_t byte, ssi_type_t type, ssi_time_t sr);

void ssipystream_Init(ssipystream *self, Py_ssize_t num, Py_ssize_t dim, Py_ssize_t byte, ssi_type_t type, ssi_time_t sr, ssi_time_t time);
ssipystream *ssipystream_New(Py_ssize_t num, Py_ssize_t dim, ssi_type_t type, ssi_time_t sr, ssi_time_t time);
ssipystream *ssipystream_From(Py_ssize_t num, Py_ssize_t dim, Py_ssize_t byte, ssi_type_t type, ssi_time_t sr, ssi_time_t time, ssi_byte_t *ptr);
void ssipystream_Print(ssipystream *self);
char *ssipystream_ToString(ssipystream *self, Py_ssize_t nmax);

void ssipyarray_Init(ssipyarray *self, Py_ssize_t len);
ssipyarray * ssipyarray_New(Py_ssize_t len);
ssipyarray * ssipyarray_From(Py_ssize_t len, ssi_real_t *ptr);
void  ssipyarray_Print(ssipyarray *self);
char *ssipyarray_ToString(ssipyarray *self, Py_ssize_t nmax);

ssipyimageparams *ssipyimageparams_New(Py_ssize_t width, Py_ssize_t height, Py_ssize_t channels, Py_ssize_t depth);
void ssipyimageparams_Init(ssipyimageparams *self, Py_ssize_t width, Py_ssize_t height, Py_ssize_t channels, Py_ssize_t depth);

Py_ssize_t ssipyimage_Stride(Py_ssize_t width, Py_ssize_t channels, Py_ssize_t depth);
void ssipyimage_Init(ssipyimage *self, Py_ssize_t width, Py_ssize_t height, Py_ssize_t channels, Py_ssize_t depth, ssi_time_t sr, ssi_time_t time);
ssipyimage *ssipyimage_New(Py_ssize_t width, Py_ssize_t height, Py_ssize_t channels, Py_ssize_t depth, ssi_time_t sr, ssi_time_t time);
ssipyimage *ssipyimage_From(Py_ssize_t width, Py_ssize_t height, Py_ssize_t channels, Py_ssize_t depth, ssi_time_t sr, ssi_time_t time, ssi_byte_t *ptr);

void ssipyevent_Init(ssipyevent *self, Py_ssize_t time_ms, Py_ssize_t dur_ms, const char *address, PyObject *data, ssi_estate_t state, Py_ssize_t glue, float prob);
ssipyevent *ssipyevent_New(Py_ssize_t time_ms, Py_ssize_t dur_ms, const char *address, PyObject *data, ssi_estate_t state, Py_ssize_t glue, float prob);
void ssipyevent_Print(ssipyevent *self);
char *ssipyevent_ToString(ssipyevent *self);

void ssipyeventboard_Init(ssipyeventboard *self, void *client, ssipyupdatefunc_t update);
ssipyeventboard *ssipyeventboard_New(void *client, ssipyupdatefunc_t update);

ssipymodeltype *ssipymodeltype_New();
void ssipymodeltype_Init(ssipymodeltype *self);

#endif