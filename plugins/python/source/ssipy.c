// ssipy.c
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

#include "ssipy.h"
#include <structmember.h>

//////////////////////////////////////
// Array ////////////////////////////
//////////////////////////////////////

// new
static PyObject *ssipyarray_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	ssipyarray *self = (ssipyarray *)type->tp_alloc(type, 0);
	ssipyarray_Init(self, 0);

	return (PyObject *)self;
}

// init
static int ssipyarray_init(ssipyarray *self, PyObject *args, PyObject *kwds)
{
	if (self->ptr != NULL)
	{
		free(self->ptr);
		self->ptr = 0;
	}

	Py_ssize_t len = 0;

	static char *kwlist[] = { "len", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "n", kwlist, &len))
	{
		return -1;
	}

	ssipyarray_Init(self, len);

	return 0;
}

// dealloc
static void ssipyarray_dealloc(ssipyarray* self)
{
	if (!self->borrowed)
	{
		free(self->ptr);
	}
	Py_TYPE(self)->tp_free((PyObject*)self);
}

// repr
static PyObject *ssipyarray_repr(ssipyarray * self)
{
	char s[100];
	sprintf(s, "ssiarray{len=%lld}", self->len);
	PyObject* ret = PyUnicode_FromString(s);
	return ret;
}

// str
static PyObject *ssipyarray_str(ssipyarray * self)
{
	char* s = ssipyarray_ToString(self, self->len);
	PyObject* ret = PyUnicode_FromString(s);
	free(s);
	return ret;
}

// buffer
static int ssipyarray_getbuffer(PyObject *obj, Py_buffer *view, int flags)
{
	if (view == NULL)
	{
		PyErr_SetString(PyExc_ValueError, "NULL view in getbuffer");
		return -1;
	}

	ssipyarray* self = (ssipyarray*)obj;
	view->obj = (PyObject*)self;
	view->buf = (void*)self->ptr;
	view->len = self->len * sizeof(ssi_real_t);
	view->readonly = 0;
	view->itemsize = sizeof(ssi_real_t);
	view->format = "f";
	view->ndim = 1;
	view->shape = &self->len;
	view->strides = &view->itemsize;
	view->suboffsets = NULL;
	view->internal = NULL;

	Py_INCREF(self);

	return 0;
}

static PyBufferProcs ssipyarray_as_buffer =
{
	(getbufferproc)ssipyarray_getbuffer,
	(releasebufferproc)0  // we do not require any special release function
};

// methods
static PyObject *ssipyarray_length(ssipyarray *self)
{
	return PyLong_FromSsize_t(self->len);
}

static PyObject *ssipyarray_type(ssipyarray *self)
{
	return Py_BuildValue("s", "f");
}

// mapping
static Py_ssize_t ssipyarray_len(ssipyarray *self)
{
	return self->len;
}
PyObject *ssipyarray_getItem(ssipyarray *self, PyObject *index)
{
	if (PyIndex_Check(index))
	{
		Py_ssize_t i = PyLong_AsSize_t(index);
		if (i < 0 || i >= self->len)
		{
			PyErr_SetString(PyExc_IndexError, "index out of range");
			return NULL;
		}
		return Py_BuildValue("f", *((float *)self->ptr + i));
	}
	

	PyErr_SetString(PyExc_IndexError, "not a valid index");
	return NULL;
}

int ssipyarray_setItem(ssipyarray *self, PyObject *index, PyObject *value)
{
	if (PyIndex_Check(index))
	{
		Py_ssize_t i = PyLong_AsSize_t(index);
		if (i < 0 || i >= self->len)
		{
			PyErr_SetString(PyExc_IndexError, "index out of range");
			return -1;
		}

		float v = (float)PyFloat_AsDouble(value);
		self->ptr[i] = v;
		return 0;
	}

	PyErr_SetString(PyExc_IndexError, "not a valid index");
	return -1;
}
static PyMappingMethods ssipyarray_mapping =
{
	ssipyarray_len,      // __len__
	ssipyarray_getItem,  // __getitem__
	ssipyarray_setItem   // __setitem__
};

// iterator
static PyObject *ssipyarray_iter(ssipyarray *array)
{
	ssipyarrayiter *it = PyObject_New(ssipyarrayiter, &ssipyarrayiter_Type);
	if (it == NULL)
	{
		return NULL;
	}

	it->it_index = 0;
	Py_INCREF(array);
	it->it_array = array;

	return (PyObject *)it;
}

// members
static PyMemberDef ssipyarray_members[] =
{
	{ "len", T_PYSSIZET, offsetof(ssipyarray, len), READONLY, "Number of values" },
	{ NULL }
};

// methods
static PyMethodDef ssipyarray_methods[] =
{
	{ "length", (PyCFunction)ssipyarray_length, METH_NOARGS, "Return the number of values in the array" },
	{ "type", (PyCFunction)ssipyarray_type, METH_NOARGS, "Return the type of the array (as string)" },
	{ NULL, NULL, 0, NULL }
};

// type
PyTypeObject ssipyarray_Type =
{
	PyVarObject_HEAD_INIT(NULL, 0)
	"ssipy.array",					/* tp_name */
	sizeof(ssipyarray),			/* tp_basicsize */
	0,								/* tp_itemsize */
	(destructor)ssipyarray_dealloc,/* tp_dealloc */
	0,								/* tp_print */
	0,								/* tp_getattr */
	0,								/* tp_setattr */
	0,								/* tp_reserved */
	(reprfunc)ssipyarray_repr,     /* tp_repr */
	0,								/* tp_as_number */
	0,								/* tp_as_sequence */
	&ssipyarray_mapping,		    /* tp_as_mapping */
	0,								/* tp_hash  */
	0,								/* tp_call */
	(reprfunc)ssipyarray_str,      /* tp_str */
	0,								/* tp_getattro */
	0,								/* tp_setattro */
	&ssipyarray_as_buffer,         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE,		/* tp_flags */
	"ssipyarray",			        /* tp_doc */
	0,								/* tp_traverse */
	0,								/* tp_clear */
	0,								/* tp_richcompare */
	0,								/* tp_weaklistoffset */
	ssipyarray_iter,				/* tp_iter */
	0,								/* tp_iternext */
	ssipyarray_methods,			/* tp_methods */
	ssipyarray_members,            /* tp_members */
	0,								/* tp_getset */
	0,								/* tp_base */
	0,								/* tp_dict */
	0,								/* tp_descr_get */
	0,								/* tp_descr_set */
	0,								/* tp_dictoffset */
	(initproc)ssipyarray_init,     /* tp_init */
	0,								/* tp_alloc */
	ssipyarray_new					/* tp_new */
};

//////////////////////////////////////
// array iterator ///////////////////
//////////////////////////////////////

static void ssipyarrayiter_dealloc(ssipyarrayiter *self)
{
	Py_XDECREF(self->it_array);
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *ssipyarrayiter_iter(PyObject *self)
{
	Py_INCREF(self);
	return self;
}

static PyObject* ssipyarrayiter_iternext(ssipyarrayiter *self)
{
	if (self->it_array == NULL)
	{
		return NULL;
	}

	if (self->it_index < self->it_array->len)
	{
		PyObject *tmp = Py_BuildValue("f", self->it_array->ptr[self->it_index]);
		self->it_index++;
		return tmp;
	}

	Py_DECREF(self->it_array);
	self->it_array = NULL;

	return NULL;
}

PyTypeObject ssipyarrayiter_Type = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"ssipy.arrayiter",							/* tp_name */
	sizeof(ssipyarrayiter),                    /* tp_basicsize */
	0,                                          /* tp_itemsize */
	(destructor)ssipyarrayiter_dealloc,		/* tp_dealloc */
	0,                                          /* tp_print */
	0,                                          /* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_reserved */
	0,                                          /* tp_repr */
	0,                                          /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	0,                                          /* tp_str */
	0,											/* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,							/* tp_flags */
	0,                                          /* tp_doc */
	0,											/* tp_traverse */
	0,                                          /* tp_clear */
	0,                                          /* tp_richcompare */
	0,                                          /* tp_weaklistoffset */
	ssipyarrayiter_iter,						/* tp_iter */
	(iternextfunc)ssipyarrayiter_iternext,     /* tp_iternext */
	0,											/* tp_methods */
	0,
};


//////////////////////////////////////
// stream ////////////////////////////
//////////////////////////////////////

// helper

char *stringssipystream_TypeToString(ssi_type_t type)
{
	char *string = (char*) malloc(2*sizeof(char));
	string[1] = '\0';

	switch (type)
	{
	case SSI_CHAR:
		string[0] = 'b';
		break;
	case SSI_UCHAR:
		string[0] = 'B';
		break;
	case SSI_SHORT:
		string[0] = 'h';
		break;
	case SSI_USHORT:
		string[0] = 'H';
		break;
	case SSI_INT:
		string[0] = 'i';
		break;
	case SSI_UINT:
		string[0] = 'I';
		break;
	case SSI_LONG:
		string[0] = 'k';
		break;
	case SSI_ULONG:
		string[0] = 'K';
		break;
	case SSI_FLOAT:
		string[0] = 'f';
		break;
	case SSI_DOUBLE:
		string[0] = 'd';
		break;
	default:
		string[0] = 'b';
		break;
	}

	return string;
}

PyObject *ssipystream_ValueToObject(ssipystream *self, Py_ssize_t index)
{
	switch (self->type)
	{
	case SSI_CHAR:
		return Py_BuildValue(self->type_s, *((char *)self->ptr + index));
	case SSI_UCHAR:
		return Py_BuildValue(self->type_s, *((unsigned char *)self->ptr + index));
	case SSI_SHORT:
		return Py_BuildValue(self->type_s, *((int16_t *)self->ptr + index));
	case SSI_USHORT:
		return Py_BuildValue(self->type_s, *((uint16_t *)self->ptr + index));
	case SSI_INT:
		return Py_BuildValue(self->type_s, *((int32_t *)self->ptr + index));
	case SSI_UINT:
		return Py_BuildValue(self->type_s, *((uint32_t *)self->ptr + index));
	case SSI_LONG:
		return Py_BuildValue(self->type_s, *((int64_t *)self->ptr + index));
	case SSI_ULONG:		
		return Py_BuildValue(self->type_s, *((uint64_t *)self->ptr + index));
	case SSI_FLOAT:
		return Py_BuildValue(self->type_s, *((float *)self->ptr + index));
	case SSI_DOUBLE:
		return Py_BuildValue(self->type_s, *((double *)self->ptr + index));
	}

	PyErr_SetString(PyExc_TypeError, "ssi type is not supported");
	return NULL;
}

void ssipystream_ValueFromObject(ssipystream *self, Py_ssize_t index, PyObject *value)
{
	switch (self->type)
	{
	case SSI_CHAR:
	{
		char v = (char)PyLong_AsLong(value);
		memcpy(self->ptr + index * self->byte, &v, self->byte);
		return;
	}
	case SSI_UCHAR:
	{
		unsigned char v = (unsigned char)PyLong_AsUnsignedLong(value);
		memcpy(self->ptr + index * self->byte, &v, self->byte);
		return;
	}
	case SSI_SHORT:
	{
		int16_t v = (int16_t)PyLong_AsLong(value);
		memcpy(self->ptr + index * self->byte, &v, self->byte);
		return;
	}
	case SSI_USHORT:
	{
		uint16_t v = (uint16_t)PyLong_AsUnsignedLong(value);
		memcpy(self->ptr + index * self->byte, &v, self->byte);
		return;
	}
	case SSI_INT:
	{
		int32_t v = (int32_t)PyLong_AsLong(value);
		memcpy(self->ptr + index * self->byte, &v, self->byte);
		return;
	}
	case SSI_UINT:
	{
		uint32_t v = (uint32_t)PyLong_AsUnsignedLong(value);
		memcpy(self->ptr + index * self->byte, &v, self->byte);
		return;
	}
	case SSI_LONG:
	{
		int64_t v = (int64_t)PyLong_AsLongLong(value);
		memcpy(self->ptr + index * self->byte, &v, self->byte);
		return;
	}
	case SSI_ULONG:
	{
		uint64_t v = (uint64_t)PyLong_AsUnsignedLongLong(value);
		memcpy(self->ptr + index * self->byte, &v, self->byte);
		return;
	}
	case SSI_FLOAT:
	{
		float v = (float)PyFloat_AsDouble(value);
		memcpy(self->ptr + index * self->byte, &v, self->byte);
		return;
	}
	case SSI_DOUBLE:
	{
		double v = PyFloat_AsDouble(value);
		memcpy(self->ptr + index * self->byte, &v, self->byte);
		return;
	}
	}

	PyErr_SetString(PyExc_TypeError, "ssi type is not supported");
}

int ssipystream_ValueToString(ssipystream *self, Py_ssize_t index, char *string)
{
	ssi_byte_t *ptr = self->ptr + index * self->byte;

	switch (self->type)
	{
	case SSI_CHAR:
	{
		ssi_char_t *val = (ssi_char_t *) ptr;
		return sprintf(string, "%5d", (int32_t) *val);
	}
	case SSI_UCHAR:
	{
		ssi_uchar_t *val = (ssi_uchar_t *)ptr;
		return sprintf(string, "%5u", (uint32_t)*val);
	}
	case SSI_SHORT:
	{
		int16_t *val = (int16_t *)ptr;
		return sprintf(string, "%10d", (int32_t)*val);
	}
	case SSI_USHORT:
	{
		uint16_t *val = (uint16_t *)ptr;
		return sprintf(string, "%10u", (uint32_t)*val);
	}
	case SSI_INT:
	{
		int32_t *val = (int32_t *)ptr;
		return sprintf(string, "%10d", *val);
	}
	case SSI_UINT:
	{
		uint32_t *val = (uint32_t *)ptr;
		return sprintf(string, "%10u", *val);
	}
	case SSI_LONG:
	{
		int64_t *val = (int64_t *)ptr;
#if __gnu_linux__
		return sprintf(string, "%10lld", *val);
#else
		return sprintf(string, "%10I64d", *val);
#endif
	}
	case SSI_ULONG:
	{
		uint64_t *val = (uint64_t *)ptr;
#if __gnu_linux__
		return sprintf(string, "%10llu", *val);
#else
		return sprintf(string, "%10I64u", *val);
#endif
	}
	case SSI_FLOAT:
	{
		float *val = (float *)ptr;
		return sprintf(string, "%10.5f", *val);
	}
	case SSI_DOUBLE:
	{
		double *val = (double *)ptr;
		return sprintf(string, "%10.5lf", *val);
	}
	}

	PyErr_SetString(PyExc_TypeError, "ssi type is not supported");
	
	return 0;
}

// new
static PyObject *ssipystream_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	ssipystream *self = (ssipystream *)type->tp_alloc(type, 0);
	ssipystream_Init(self, 0, 0, 0, SSI_UNDEF, 0, 0);

	return (PyObject *)self;
}

// init
static int ssipystream_init(ssipystream *self, PyObject *args, PyObject *kwds)
{
	if (self->ptr != NULL)
	{
		free(self->ptr);
		self->ptr = 0;
	}

	Py_ssize_t num = 0;
	Py_ssize_t dim = 0;	
	ssi_type_t type = 0;
	ssi_time_t sr = 0;
	ssi_time_t time = 0;	
	static char *kwlist[] = { "num", "dim", "type", "sr", "time", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "nni|dd", kwlist, &num, &dim, &type, &sr, &time))
	{
		return -1;
	}

	ssipystream_Init(self, num, dim, ssi_type2bytes(type), type, sr, time);

	return 0;
}

// dealloc
static void ssipystream_dealloc(ssipystream* self)
{
	if (!self->borrowed)
	{
		free(self->ptr);
	}
	free(self->type_s);
	Py_TYPE(self)->tp_free((PyObject*)self);
}

// repr
static PyObject *ssipystream_repr(ssipystream * self)
{
	char s[100];
	sprintf(s, "ssistream{shape=%lldx%lld,byte=%lld,type=%s,sr=%lf,time=%lf}", self->num, self->dim, self->byte, self->type_s, self->sr, self->time);
	PyObject* ret = PyUnicode_FromString(s);	
	return ret;
}

// str
static PyObject *ssipystream_str(ssipystream * self)
{
	char* s = ssipystream_ToString(self, self->len);
	PyObject* ret = PyUnicode_FromString(s);
	free(s);
	return ret;
}

// buffer
static int ssipystream_getbuffer(PyObject *obj, Py_buffer *view, int flags)
{
	if (view == NULL)
	{
		PyErr_SetString(PyExc_ValueError, "NULL view in getbuffer");
		return -1;
	}

	ssipystream* self = (ssipystream*)obj;
	view->obj = (PyObject*)self;
	view->buf = (void*)self->ptr;
	view->len = self->tot;
	view->readonly = 0;
	view->itemsize = self->byte;
	view->format = (char *)malloc(strlen(self->type_s)+1 * sizeof(char));
	strcpy(view->format, self->type_s);	
	view->ndim = 2;
	view->shape = (Py_ssize_t*)malloc(2 * sizeof(Py_ssize_t));
	view->shape[0] = self->num;
	view->shape[1] = self->dim;
	view->strides = (Py_ssize_t*)malloc(2 * sizeof(Py_ssize_t)); 
	view->strides[0] = self->dim * self->byte;
	view->strides[1] = self->byte;
	view->suboffsets = NULL;
	view->internal = NULL;

	Py_INCREF(self);

	return 0;
}
static PyBufferProcs ssipystream_as_buffer =
{
	(getbufferproc)ssipystream_getbuffer,
	(releasebufferproc)0  // we do not require any special release function
};

// methods
static PyObject *ssipystream_length(ssipystream *self)
{	
	return PyLong_FromSsize_t(self->len);
}
static PyObject *ssipystream_shape(ssipystream *self)
{
	return Py_BuildValue("nn", self->num, self->dim);
}
static PyObject *ssipystream_type(ssipystream *self)
{
	return Py_BuildValue("s", self->type_s);
}

// mapping
static Py_ssize_t ssipystream_len(ssipystream *self)
{
	return self->len;
}
PyObject *ssipystream_getItem(ssipystream *self, PyObject *index)
{
	if (PyIndex_Check(index))
	{
		Py_ssize_t i = PyLong_AsSize_t(index);
		if (i < 0 || i >= self->len)
		{
			PyErr_SetString(PyExc_IndexError, "index out of range");
			return NULL;
		}
		return ssipystream_ValueToObject(self, i);
	}
	else if (PyTuple_Check(index))
	{
		if (PyTuple_Size(index) != 2)
		{
			PyErr_SetString(PyExc_IndexError, "not a valid index");
			return NULL;
		}

		PyObject *pRow = PyTuple_GetItem(index, 0);
		Py_ssize_t row = PyLong_AsSize_t(pRow);
		if (row < 0 || row >= self->num)
		{
			PyErr_SetString(PyExc_IndexError, "index out of range");
			return NULL;
		}

		PyObject *pColumn = PyTuple_GetItem(index, 1);
		Py_ssize_t column = PyLong_AsSize_t(pColumn);
		if (column < 0 || column >= self->dim)
		{
			PyErr_SetString(PyExc_IndexError, "index out of range");
			return NULL;
		}
		return ssipystream_ValueToObject(self, row*self->dim + column);
	}
	
	PyErr_SetString(PyExc_IndexError, "not a valid index");
	return NULL;
}
int ssipystream_setItem(ssipystream *self, PyObject *index, PyObject *value)
{
	if (PyIndex_Check(index))
	{
		Py_ssize_t i = PyLong_AsSize_t(index);
		if (i < 0 || i >= self->len)
		{
			PyErr_SetString(PyExc_IndexError, "index out of range");
			return -1;
		}
		ssipystream_ValueFromObject(self, i, value);
		return 0;
	} 
	else if (PyTuple_Check(index))
	{
		if (PyTuple_Size(index) != 2)
		{
			PyErr_SetString(PyExc_IndexError, "not a valid index");
			return -1;
		}

		PyObject *pRow = PyTuple_GetItem(index, 0); 
		Py_ssize_t row = PyLong_AsSize_t(pRow);
		if (row < 0 || row >= self->num)
		{
			PyErr_SetString(PyExc_IndexError, "index out of range");
			return -1;
		}

		PyObject *pColumn = PyTuple_GetItem(index, 1);
		Py_ssize_t column = PyLong_AsSize_t(pColumn);
		if (column < 0 || column >= self->dim)
		{
			PyErr_SetString(PyExc_IndexError, "index out of range");
			return -1;
		}

		ssipystream_ValueFromObject(self, row*self->dim+column, value);
		return 0;
	}

	PyErr_SetString(PyExc_IndexError, "not a valid index");
	return -1;
}
static PyMappingMethods ssipystream_mapping = 
{
	ssipystream_len,      // __len__
	ssipystream_getItem,  // __getitem__
	ssipystream_setItem   // __setitem__
};

// iterator
static PyObject *ssipystream_iter(ssipystream *stream)
{
	ssipystreamiter *it = PyObject_New(ssipystreamiter, &ssipystreamiter_Type);	
	if (it == NULL)
	{
		return NULL;
	}

	it->it_index = 0;
	Py_INCREF(stream);
	it->it_stream = stream;

	return (PyObject *)it;
}

// members
static PyMemberDef ssipystream_members[] = 
{
	{ "num", T_PYSSIZET, offsetof(ssipystream, num), READONLY, "Number of samples" },
	{ "dim", T_PYSSIZET, offsetof(ssipystream, dim), READONLY, "Number of dimensions" },
	{ "len", T_PYSSIZET, offsetof(ssipystream, len), READONLY, "Number of values" },
	{ "tot", T_PYSSIZET, offsetof(ssipystream, tot), READONLY, "Total number of bytes" },
	{ "byte", T_PYSSIZET, offsetof(ssipystream, byte), READONLY, "Numbe of bytes per value" },
	{ "type", T_INT, offsetof(ssipystream, type), READONLY, "Type code" },
	{ "sr", T_DOUBLE, offsetof(ssipystream, sr), READONLY, "Sample rate in hz" },
	{ "time", T_DOUBLE, offsetof(ssipystream, time), READONLY, "Time stamp in seconds" },
	{ NULL }
};

// methods
static PyMethodDef ssipystream_methods[] = 
{
	{ "length", (PyCFunction)ssipystream_length, METH_NOARGS, "Return the number of values in the stream"},
	{ "shape", (PyCFunction)ssipystream_shape, METH_NOARGS, "Return the shape of the stream (num x dim)" },
	{ "type", (PyCFunction)ssipystream_type, METH_NOARGS, "Return the type of the stream (as string)" },
	{ NULL, NULL, 0, NULL }
};

// type
PyTypeObject ssipystream_Type =
{
	PyVarObject_HEAD_INIT(NULL, 0)
	"ssipy.stream",					/* tp_name */
	sizeof(ssipystream),			/* tp_basicsize */
	0,								/* tp_itemsize */
	(destructor)ssipystream_dealloc,/* tp_dealloc */
	0,								/* tp_print */
	0,								/* tp_getattr */
	0,								/* tp_setattr */
	0,								/* tp_reserved */
	(reprfunc)ssipystream_repr,     /* tp_repr */
	0,								/* tp_as_number */
	0,								/* tp_as_sequence */
	&ssipystream_mapping,		    /* tp_as_mapping */
	0,								/* tp_hash  */
	0,								/* tp_call */
	(reprfunc)ssipystream_str,      /* tp_str */
	0,								/* tp_getattro */
	0,								/* tp_setattro */
	&ssipystream_as_buffer,         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT |
		Py_TPFLAGS_BASETYPE,		/* tp_flags */
	"ssipystream",			        /* tp_doc */
	0,								/* tp_traverse */
	0,								/* tp_clear */
	0,								/* tp_richcompare */
	0,								/* tp_weaklistoffset */
	ssipystream_iter,				/* tp_iter */
	0,								/* tp_iternext */
	ssipystream_methods,			/* tp_methods */
	ssipystream_members,            /* tp_members */
	0,								/* tp_getset */
	0,								/* tp_base */
	0,								/* tp_dict */
	0,								/* tp_descr_get */
	0,								/* tp_descr_set */
	0,								/* tp_dictoffset */
	(initproc)ssipystream_init,     /* tp_init */
	0,								/* tp_alloc */
	ssipystream_new					/* tp_new */
};

//////////////////////////////////////
// stream iterator ///////////////////
//////////////////////////////////////

static void ssipystreamiter_dealloc(ssipystreamiter *self)
{	
	Py_XDECREF(self->it_stream);
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *ssipystreamiter_iter(PyObject *self)
{
	Py_INCREF(self);
	return self;
}

static PyObject* ssipystreamiter_iternext(ssipystreamiter *self)
{	
	if (self->it_stream == NULL)
	{
		return NULL;
	}

	if (self->it_index < self->it_stream->len)
	{
		PyObject *tmp = ssipystream_ValueToObject(self->it_stream, self->it_index);			
		self->it_index++;
		return tmp;
	}
	
	Py_DECREF(self->it_stream);
	self->it_stream = NULL;

	return NULL;
}

PyTypeObject ssipystreamiter_Type = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"ssipy.streamiter",							/* tp_name */
	sizeof(ssipystreamiter),                    /* tp_basicsize */
	0,                                          /* tp_itemsize */												
	(destructor)ssipystreamiter_dealloc,		/* tp_dealloc */
	0,                                          /* tp_print */
	0,                                          /* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_reserved */
	0,                                          /* tp_repr */
	0,                                          /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	0,                                          /* tp_str */
	0,											/* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,							/* tp_flags */
	0,                                          /* tp_doc */
	0,											/* tp_traverse */
	0,                                          /* tp_clear */
	0,                                          /* tp_richcompare */
	0,                                          /* tp_weaklistoffset */
	ssipystreamiter_iter,						/* tp_iter */
	(iternextfunc)ssipystreamiter_iternext,     /* tp_iternext */
	0,											/* tp_methods */
	0,
};

//////////////////////////////////////
// type enumeration //////////////////
//////////////////////////////////////

// init
static int ssipytype_init(ssipytype *self, PyObject *args, PyObject *kwds)
{	
	ssipytype_Init(self);
	return 0;
}

static void ssipytype_dealloc(ssipytype *self)
{	
	Py_TYPE(self)->tp_free((PyObject*)self);
}

// members
static PyMemberDef ssipytype_members[] =
{
	{ "UNDEF", T_INT, offsetof(ssipytype, UNDEF), READONLY, "undefined" },
	{ "CHAR", T_INT, offsetof(ssipytype, CHAR), READONLY, "1 byte integer" },
	{ "UCHAR", T_INT, offsetof(ssipytype, UCHAR), READONLY, "1 byte unsigned integer" },
	{ "SHORT", T_INT, offsetof(ssipytype, SHORT), READONLY, "2 bytes integer" },
	{ "USHORT", T_INT, offsetof(ssipytype, USHORT), READONLY, "2 bytes unsigned integer" },
	{ "INT", T_INT, offsetof(ssipytype, INT), READONLY, "4 bytes integer" },
	{ "UINT", T_INT, offsetof(ssipytype, UINT), READONLY, "4 bytes unsigned integer" },
	{ "LONG", T_INT, offsetof(ssipytype, LONG), READONLY, "8 bytes integer" },
	{ "ULONG", T_INT, offsetof(ssipytype, ULONG), READONLY, "8 bytes unsigned integer" },
	{ "FLOAT", T_INT, offsetof(ssipytype, FLOAT), READONLY, "4 bytes floating point" },
	{ "DOUBLE", T_INT, offsetof(ssipytype, DOUBLE), READONLY, "8 bytes floating point" },
	{ NULL }
};

// methods
static PyObject *ssipytype_size(PyObject *self, PyObject *pType)
{
	ssi_type_t type = (ssi_type_t) PyLong_AsLong(pType);
	long size = (long)ssi_type2bytes(type);	
	return PyLong_FromLong(size);
}

static PyMethodDef ssipytype_methods[] =
{
	{ "size", (PyCFunction)ssipytype_size, METH_O, "Return the number of bytes used by a value of that type" },
	{ NULL, NULL, 0, NULL }
};

PyTypeObject ssipytype_Type = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"ssipy.type",								/* tp_name */
	sizeof(ssipytype),		                    /* tp_basicsize */
	0,                                          /* tp_itemsize */
	(destructor)ssipytype_dealloc,				/* tp_dealloc */
	0,                                          /* tp_print */
	0,                                          /* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_reserved */
	0,                                          /* tp_repr */
	0,                                          /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	0,                                          /* tp_str */
	0,											/* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_BASETYPE							/* tp_flags */
	| Py_TPFLAGS_BASETYPE,						/* tp_flags */
	"ssipytype",								/* tp_doc */
	0,											/* tp_traverse */
	0,											/* tp_clear */
	0,											/* tp_richcompare */
	0,											/* tp_weaklistoffset */
	0,											/* tp_iter */
	0,											/* tp_iternext */
	ssipytype_methods,							/* tp_methods */
	ssipytype_members,							/* tp_members */
	0,											/* tp_getset */
	0,											/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)ssipytype_init,					/* tp_init */
	0,											/* tp_alloc */
	0											/* tp_new */
};

//////////////////////////////////////
// info structs //////////////////////
//////////////////////////////////////

// init
static int ssipyinfo_init(ssipyinfo *self, PyObject *args, PyObject *kwds)
{
	ssi_time_t time = 0;
	ssi_time_t dur = 0;
	Py_ssize_t frame = 0;
	Py_ssize_t delta = 0;
	static char *kwlist[] = { "time", "dur", "frame", "delta", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ddnn", kwlist, &time, &dur, &frame, &delta))
	{
		return -1;
	}

	ssipyinfo_Init(self, time, dur, frame, delta);

	return 0;
}

static void ssipyinfo_dealloc(ssipyinfo *self)
{
	Py_TYPE(self)->tp_free((PyObject*)self);
}

// members
static PyMemberDef ssipyinfo_members[] =
{
	{ "time", T_DOUBLE, offsetof(ssipyinfo, time), READONLY, "start time in s" },
	{ "dur", T_DOUBLE, offsetof(ssipyinfo, dur), READONLY, "duration in s" },
	{ "frame", T_PYSSIZET, offsetof(ssipyinfo, frame), READONLY, "number of new frames" },
	{ "delta", T_PYSSIZET, offsetof(ssipyinfo, delta), READONLY, "number of overlapping frames" },
	{ NULL }
};

PyTypeObject ssipyinfo_Type = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"ssipy.info",								/* tp_name */
	sizeof(ssipyinfo),		                    /* tp_basicsize */
	0,                                          /* tp_itemsize */
	(destructor)ssipyinfo_dealloc,				/* tp_dealloc */
	0,                                          /* tp_print */
	0,                                          /* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_reserved */
	0,                                          /* tp_repr */
	0,                                          /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	0,                                          /* tp_str */
	0,											/* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_BASETYPE							/* tp_flags */
	| Py_TPFLAGS_BASETYPE,						/* tp_flags */
	"ssipyinfo",								/* tp_doc */
	0,											/* tp_traverse */
	0,											/* tp_clear */
	0,											/* tp_richcompare */
	0,											/* tp_weaklistoffset */
	0,											/* tp_iter */
	0,											/* tp_iternext */
	0,											/* tp_methods */
	ssipyinfo_members,							/* tp_members */
	0,											/* tp_getset */
	0,											/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)ssipyinfo_init,					/* tp_init */
	0,											/* tp_alloc */
	0											/* tp_new */
};

//////////////////////////////////////
// image params struct ////////////////////
//////////////////////////////////////

// init
static int ssipyimageparams_init(ssipyimageparams *self, PyObject *args, PyObject *kwds)
{
	Py_ssize_t width = 0;
	Py_ssize_t height = 0;
	Py_ssize_t channels = 0;
	Py_ssize_t depth = 0;
	ssi_time_t sr = 0;
	ssi_time_t time = 0;
	static char *kwlist[] = { "width", "height", "channels", "depth", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "nnnn", kwlist, &width, &height, &channels, &depth))
	{
		return -1;
	}

	ssipyimageparams_Init(self, width, height, channels, depth);

	return 0;
}

// dealloc
static void ssipyimageparams_dealloc(ssipyimageparams *self)
{
	Py_TYPE(self)->tp_free((PyObject*)self);
}

// repr
static PyObject *ssipyimageparams_repr(ssipyimageparams *self)
{
	char s[100];
	sprintf(s, "ssiimageparams{shape=%lldx%lld,channels=%lld,depth=%lld}", self->width, self->height, self->channels, self->depth);
	PyObject* ret = PyUnicode_FromString(s);
	return ret;
}

// members
static PyMemberDef ssipyimageparams_members[] =
{
	{ "width", T_PYSSIZET, offsetof(ssipyimageparams, width), 0, "Width in pixels" },
	{ "height", T_PYSSIZET, offsetof(ssipyimageparams, height), 0, "Height in pixels" },
	{ "channels", T_PYSSIZET, offsetof(ssipyimageparams, channels), 0, "Number of channels per pixel" },
	{ "depth", T_PYSSIZET, offsetof(ssipyimageparams, depth), 0, "Bytes per channel" },
	{ NULL }
};

PyTypeObject ssipyimageparams_Type = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"ssipy.imageparams",						/* tp_name */
	sizeof(ssipyimageparams),		            /* tp_basicsize */
	0,                                          /* tp_itemsize */
	(destructor)ssipyimageparams_dealloc,		/* tp_dealloc */
	0,                                          /* tp_print */
	0,                                          /* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_reserved */
	ssipyimageparams_repr,                      /* tp_repr */
	0,                                          /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	ssipyimageparams_repr,                      /* tp_str */
	0,											/* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_BASETYPE							/* tp_flags */
	| Py_TPFLAGS_BASETYPE,						/* tp_flags */
	"ssipyimageparams",							/* tp_doc */
	0,											/* tp_traverse */
	0,											/* tp_clear */
	0,											/* tp_richcompare */
	0,											/* tp_weaklistoffset */
	0,											/* tp_iter */
	0,											/* tp_iternext */
	0,											/* tp_methods */
	ssipyimageparams_members,					/* tp_members */
	0,											/* tp_getset */
	0,											/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)ssipyimageparams_init,			/* tp_init */
	0,											/* tp_alloc */
	0											/* tp_new */
};

//////////////////////////////////////
// image struct //////////////////////
//////////////////////////////////////

// new
static PyObject *ssipyimage_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	ssipyimage *self = (ssipyimage *)type->tp_alloc(type, 0);	
	ssipyimage_Init(self, 0, 0, 0, 0, 0, 0);

	return (PyObject *)self;
}

// init
static int ssipyimage_init(ssipyimage *self, PyObject *args, PyObject *kwds)
{
	if (self->ptr != NULL)
	{
		free(self->ptr);
		self->ptr = 0;
	}

	Py_ssize_t width = 0;
	Py_ssize_t height = 0;
	Py_ssize_t channels = 0;
	Py_ssize_t depth = 0;
	ssi_time_t sr = 0;
	ssi_time_t time = 0;
	static char *kwlist[] = { "width", "height", "channels", "depth", "sr", "time", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "nnnn|dd", kwlist, &width, &height, &channels, &depth, &sr, &time))
	{
		return -1;
	}

	ssipyimage_Init(self, width, height, channels, depth, sr, time);

	return 0;
}

// dealloc
static void ssipyimage_dealloc(ssipyimage* self)
{
	if (!self->borrowed)
	{
		free(self->ptr);
	}	
	Py_TYPE(self)->tp_free((PyObject*)self);
}

// repr
static PyObject *ssipyimage_repr(ssipyimage * self)
{
	char s[100];
	sprintf(s, "ssiimage{shape=%lldx%lld,channels=%lld,depth=%lld,sr=%lf,time=%lf}", self->width, self->height, self->channels, self->depth, self->sr, self->time);
	PyObject* ret = PyUnicode_FromString(s);
	return ret;
}

// buffer
static int ssipyimage_getbuffer(PyObject *obj, Py_buffer *view, int flags)
{
	if (view == NULL)
	{
		PyErr_SetString(PyExc_ValueError, "NULL view in getbuffer");
		return -1;
	}

	ssipyimage* self = (ssipyimage*)obj;
	view->obj = (PyObject*)self;
	view->buf = (void*)self->ptr;
	view->len = self->tot;
	view->readonly = 0;
	view->itemsize = self->depth;
	switch (view->itemsize)
	{
	case 1:
		view->format = "B";
		break;
	case 2:
		view->format = "H";
		break;
	case 4:
		view->format = "f";
		break;
	case 8:
		view->format = "d";
		break;
	default:
		PyErr_SetString(PyExc_ValueError, "unexpected itemsize in getbuffer");
		return -1;
	}		
	view->ndim = 3;
	view->shape = (Py_ssize_t*)malloc(3 * sizeof(Py_ssize_t));
	view->shape[0] = self->height;
	view->shape[1] = self->width;
	view->shape[2] = self->channels;
	view->strides = (Py_ssize_t*)malloc(2 * sizeof(Py_ssize_t));
	view->strides[0] = self->stride;
	view->strides[1] = self->channels * self->depth;
	view->strides[2] = self->depth;
	view->suboffsets = NULL;
	view->internal = NULL;

	Py_INCREF(self);

	return 0;
}
static PyBufferProcs ssipyimage_as_buffer =
{
	(getbufferproc)ssipyimage_getbuffer,
	(releasebufferproc)0  // we do not require any special release function
};

// methods
static PyObject *ssipyimage_pixel(ssipyimage *self)
{
	return PyLong_FromSsize_t(self->width * self->height);
}
static PyObject *ssipyimage_size(ssipyimage *self)
{
	return PyLong_FromSsize_t(self->width * self->height * self->channels);
}
static PyObject *ssipyimage_shape(ssipyimage *self)
{
	return Py_BuildValue("nnn", self->height, self->width, self->channels);
}

// members
static PyMemberDef ssipyimage_members[] =
{
	{ "width", T_PYSSIZET, offsetof(ssipyimage, width), READONLY, "Width in pixels" },
	{ "height", T_PYSSIZET, offsetof(ssipyimage, height), READONLY, "Height in pixels" },
	{ "channels", T_PYSSIZET, offsetof(ssipyimage, channels), READONLY, "Number of channels per pixel" },
	{ "depth", T_PYSSIZET, offsetof(ssipyimage, depth), READONLY, "Number of bytes per channel" },
	{ "stride", T_PYSSIZET, offsetof(ssipyimage, stride), READONLY, "Stride in bytes" },
	{ "tot", T_PYSSIZET, offsetof(ssipyimage, tot), READONLY, "Total number of bytes" },
	{ "sr", T_DOUBLE, offsetof(ssipyimage, sr), READONLY, "Sample rate in hz" },
	{ "time", T_DOUBLE, offsetof(ssipyimage, time), READONLY, "Time in seconds" },
	{ NULL }
};

// methods
static PyMethodDef ssipyimage_methods[] =
{
	{ "pixel", (PyCFunction)ssipyimage_pixel, METH_NOARGS, "Return the number of pixels" },
	{ "size", (PyCFunction)ssipyimage_size, METH_NOARGS, "Return the number of pixel values" },
	{ "shape", (PyCFunction)ssipyimage_shape, METH_NOARGS, "Return the shape of the image (height x width x channels)" },	
	{ NULL, NULL, 0, NULL }
};

// type
PyTypeObject ssipyimage_Type =
{
	PyVarObject_HEAD_INIT(NULL, 0)
	"ssipy.image",					/* tp_name */
	sizeof(ssipyimage),				/* tp_basicsize */
	0,								/* tp_itemsize */
	(destructor)ssipyimage_dealloc, /* tp_dealloc */
	0,								/* tp_print */
	0,								/* tp_getattr */
	0,								/* tp_setattr */
	0,								/* tp_reserved */
	(reprfunc)ssipyimage_repr,	    /* tp_repr */
	0,								/* tp_as_number */
	0,								/* tp_as_sequence */
	0,								/* tp_as_mapping */
	0,								/* tp_hash  */
	0,								/* tp_call */
	(reprfunc)ssipyimage_repr,      /* tp_str */
	0,								/* tp_getattro */
	0,								/* tp_setattro */
	&ssipyimage_as_buffer,          /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE,		    /* tp_flags */
	"ssipyimage",			        /* tp_doc */
	0,								/* tp_traverse */
	0,								/* tp_clear */
	0,								/* tp_richcompare */
	0,								/* tp_weaklistoffset */
	0,							    /* tp_iter */
	0,								/* tp_iternext */
	ssipyimage_methods,				/* tp_methods */
	ssipyimage_members,				/* tp_members */
	0,								/* tp_getset */
	0,								/* tp_base */
	0,								/* tp_dict */
	0,								/* tp_descr_get */
	0,								/* tp_descr_set */
	0,								/* tp_dictoffset */
	(initproc)ssipyimage_init,		/* tp_init */
	0,								/* tp_alloc */
	ssipyimage_new					/* tp_new */
};

//////////////////////////////////////
// channel struct ////////////////////
//////////////////////////////////////

// init
static int ssipychannel_init(ssipychannel *self, PyObject *args, PyObject *kwds)
{
	Py_ssize_t dim = 0;
	Py_ssize_t byte = 0;
	ssi_type_t type = SSI_UNDEF;
	ssi_time_t sr = 0.0;
	static char *kwlist[] = { "dim", "byte", "type", "sr", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "nnid", kwlist, &dim, &byte, &type, &sr))
	{
		return -1;
	}

	ssipychannel_Init(self, dim, byte, type, sr);

	return 0;
}

// dealloc
static void ssipychannel_dealloc(ssipychannel *self)
{
	Py_TYPE(self)->tp_free((PyObject*)self);
}

// repr
static PyObject *ssipychannel_repr(ssipychannel *self)
{
	char s[256];
	char *type_s = stringssipystream_TypeToString(self->type);
	sprintf(s, "ssichannel{dim=%d,byte=%d,type=%s,sr=%.2lf}", (int)self->dim, (int)self->byte, type_s, self->sr);
	free(type_s);
	PyObject* ret = PyUnicode_FromString(s);
	return ret;
}

// members
static PyMemberDef ssipychannel_members[] =
{
	{ "dim", T_PYSSIZET, offsetof(ssipychannel, dim), 0, "Number of dimensions" },
	{ "byte", T_PYSSIZET, offsetof(ssipychannel, byte), 0, "Number of bytes per value" },
	{ "type", T_INT, offsetof(ssipychannel, type), 0, "Type identifier" },
	{ "sr", T_DOUBLE, offsetof(ssipychannel, sr), 0, "Sample rate in hz" },
	{ NULL }
};

PyTypeObject ssipychannel_Type = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"ssipy.channel",							/* tp_name */
	sizeof(ssipychannel),		                /* tp_basicsize */
	0,                                          /* tp_itemsize */
	(destructor)ssipychannel_dealloc,			/* tp_dealloc */
	0,                                          /* tp_print */
	0,                                          /* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_reserved */
	ssipychannel_repr,                          /* tp_repr */
	0,                                          /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	ssipychannel_repr,                          /* tp_str */
	0,											/* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_BASETYPE							/* tp_flags */
	| Py_TPFLAGS_BASETYPE,						/* tp_flags */
	"ssipychannel",								/* tp_doc */
	0,											/* tp_traverse */
	0,											/* tp_clear */
	0,											/* tp_richcompare */
	0,											/* tp_weaklistoffset */
	0,											/* tp_iter */
	0,											/* tp_iternext */
	0,											/* tp_methods */
	ssipychannel_members,						/* tp_members */
	0,											/* tp_getset */
	0,											/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)ssipychannel_init,				/* tp_init */
	0,											/* tp_alloc */
	0											/* tp_new */
};

//////////////////////////////////////
// event ////////////////////////////
//////////////////////////////////////

// helper
ssi_etype_t ssipyevent_ObjectToType(PyObject *object)
{
	if (!object || object == Py_None)
	{
		return SSI_ETYPE_EMPTY;
	}
	else if (PyUnicode_Check(object))
	{
		return SSI_ETYPE_STRING;
	}
	else if (PyTuple_Check(object) || PyList_Check(object) || PyFloat_Check(object))
	{
		return SSI_ETYPE_TUPLE;
	}
	else if (PyDict_Check(object))
	{
		return SSI_ETYPE_MAP;
	}
	return SSI_ETYPE_UNDEF;
}

// new
static PyObject *ssipyevent_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	ssipyevent *self = (ssipyevent *)type->tp_alloc(type, 0);
	ssipyevent_Init(self, 0, 0, 0, 0, SSI_ESTATE_COMPLETED, 0, 0);

	return (PyObject *)self;
}

// init
static int ssipyevent_init(ssipyevent *self, PyObject *args, PyObject *kwds)
{
	if (self->data != NULL)
	{
		Py_XDECREF(self->data);
		self->data = NULL;
	}	
	
	Py_ssize_t time = 0;
	Py_ssize_t dur = 0;
	char *address = 0;
	PyObject *data = Py_None;
	ssi_estate_t state = SSI_ESTATE_COMPLETED;
	Py_ssize_t glue = 0;	
	float prob = 1.0f;
	static char *kwlist[] = { "time", "dur", "address", "data", "state", "glue", "prob", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "nns|Oinf", kwlist, &time, &dur, &address, &data, &state, &glue, &prob))
	{
		return -1;
	}

	ssipyevent_Init(self, time, dur, address, data, state, glue, prob);

	return 0;
}

// dealloc
static void ssipyevent_dealloc(ssipyevent* self)
{
	Py_XDECREF(self->data);
	free(self->address);
	Py_TYPE(self)->tp_free((PyObject*)self);
}

// repr
static PyObject *ssipyevent_repr(ssipyevent * self)
{
	char s[256];
	sprintf(s, "ssievent{time=%d,dur=%d,address=%s,data=%s,state=%s,glue=%d,prob=%.2f}", (int)self->time, (int)self->dur, self->address, self->data ? self->data->ob_type->tp_name : "NULL", self->state == SSI_ESTATE_COMPLETED ? "completed" : "continued", (int)self->glue, self->prob);
	PyObject* ret = PyUnicode_FromString(s);
	return ret;
}

// str
static PyObject *ssipyevent_str(ssipyevent * self)
{
	char* s = ssipyevent_ToString(self);
	PyObject* ret = PyUnicode_FromString(s);
	free(s);
	return ret;
}

// members
static PyMemberDef ssipyevent_members[] =
{		
	{ "time", T_PYSSIZET, offsetof(ssipyevent, time), READONLY, "Time in ms" },
	{ "dur", T_PYSSIZET, offsetof(ssipyevent, dur), READONLY, "Duration in ms" },
	{ "address", T_STRING, offsetof(ssipyevent, address), READONLY, "Address <event@sender>" },
	{ "data", T_OBJECT, offsetof(ssipyevent, data), READONLY, "Data object" },	
	{ "state", T_INT, offsetof(ssipyevent, state), READONLY, "Event state" },
	{ "glue", T_PYSSIZET, offsetof(ssipyevent, glue), READONLY, "Glue id" },
	{ "prob", T_FLOAT, offsetof(ssipyevent, prob), READONLY, "Confidence" },
	{ NULL }
};

// methods
static PyMethodDef ssipyevent_methods[] =
{
	{ NULL, NULL, 0, NULL }
};

// type
PyTypeObject ssipyevent_Type =
{
	PyVarObject_HEAD_INIT(NULL, 0)
	"ssipy.event",					/* tp_name */
	sizeof(ssipyevent),			    /* tp_basicsize */
	0,								/* tp_itemsize */
	(destructor)ssipyevent_dealloc, /* tp_dealloc */
	0,								/* tp_print */
	0,								/* tp_getattr */
	0,								/* tp_setattr */
	0,								/* tp_reserved */
	(reprfunc)ssipyevent_repr,      /* tp_repr */
	0,								/* tp_as_number */
	0,								/* tp_as_sequence */
	0,								/* tp_as_mapping */
	0,								/* tp_hash  */
	0,								/* tp_call */
	(reprfunc)ssipyevent_str,       /* tp_str */
	0,								/* tp_getattro */
	0,								/* tp_setattro */
	0,								/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE,	    	/* tp_flags */
	"ssipyevent",			        /* tp_doc */
	0,								/* tp_traverse */
	0,								/* tp_clear */
	0,								/* tp_richcompare */
	0,								/* tp_weaklistoffset */
	0,								/* tp_iter */
	0,								/* tp_iternext */
	ssipyevent_methods,		    	/* tp_methods */
	ssipyevent_members,             /* tp_members */
	0,								/* tp_getset */
	0,								/* tp_base */
	0,								/* tp_dict */
	0,								/* tp_descr_get */
	0,								/* tp_descr_set */
	0,								/* tp_dictoffset */
	(initproc)ssipyevent_init,      /* tp_init */
	0,								/* tp_alloc */
	ssipyevent_new					/* tp_new */
};

//////////////////////////////////////
// event board ///////////////////////
//////////////////////////////////////

// dealloc
static void ssipyeventboard_dealloc(ssipyeventboard* self)
{	
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *ssipyeventboard_update(PyObject *self, PyObject *args, PyObject *kwds)
{
	PyObject *event = ssipyevent_new(&ssipyevent_Type, 0, 0);
	ssipyevent_init((ssipyevent*)event, args, kwds);

	if (event)
	{
		((ssipyeventboard *)self)->update(((ssipyeventboard *)self)->client, (ssipyevent *)event);
	}

	return Py_None;
}

// members
static PyMemberDef ssipyeventboard_members[] =
{	
	{ "COMPLETED", T_INT, offsetof(ssipyeventboard, COMPLETED), READONLY, "Completed event" },
	{ "CONTINUED", T_INT, offsetof(ssipyeventboard, CONTINUED), READONLY, "Continued event" },
	{ NULL }
};

// methods
static PyMethodDef ssipyeventboard_methods[] =
{
	{ "update", (PyCFunction)ssipyeventboard_update, METH_VARARGS | METH_KEYWORDS, "Sends an event to the event board" },
	{ NULL, NULL, 0, NULL }
};

// type
PyTypeObject ssipyeventboard_Type =
{
	PyVarObject_HEAD_INIT(NULL, 0)
	"ssipy.eventboard",				/* tp_name */
	sizeof(ssipyeventboard),		/* tp_basicsize */
	0,								/* tp_itemsize */
	(destructor)ssipyeventboard_dealloc, /* tp_dealloc */
	0,								/* tp_print */
	0,								/* tp_getattr */
	0,								/* tp_setattr */
	0,								/* tp_reserved */
	0,								/* tp_repr */
	0,								/* tp_as_number */
	0,								/* tp_as_sequence */
	0,								/* tp_as_mapping */
	0,								/* tp_hash  */
	0,								/* tp_call */
	0,								/* tp_str */
	0,								/* tp_getattro */
	0,								/* tp_setattro */
	0,								/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE,	    	/* tp_flags */
	"ssipyeventboard",			    /* tp_doc */
	0,								/* tp_traverse */
	0,								/* tp_clear */
	0,								/* tp_richcompare */
	0,								/* tp_weaklistoffset */
	0,								/* tp_iter */
	0,								/* tp_iternext */
	ssipyeventboard_methods,		/* tp_methods */
	ssipyeventboard_members,        /* tp_members */
	0,								/* tp_getset */
	0,								/* tp_base */
	0,								/* tp_dict */
	0,								/* tp_descr_get */
	0,								/* tp_descr_set */
	0,								/* tp_dictoffset */
	0,								/* tp_init */
	0,								/* tp_alloc */
	0								/* tp_new */
};


//////////////////////////////////////
// model type enumeration ////////////
//////////////////////////////////////

// init
static int ssipymodeltype_init(ssipymodeltype *self, PyObject *args, PyObject *kwds)
{
	ssipymodeltype_Init(self);
	return 0;
}

static void ssipymodeltype_dealloc(ssipymodeltype *self)
{
	Py_TYPE(self)->tp_free((PyObject*)self);
}

// members
static PyMemberDef ssipymodeltype_members[] =
{
	{ "CLASSIFICATION", T_INT, offsetof(ssipymodeltype, CLASSIFICATION), READONLY, "classification" },
	{ "REGRESSION", T_INT, offsetof(ssipymodeltype, REGRESSION), READONLY, "regression" },
	{ NULL }
};

PyTypeObject ssipymodeltype_Type = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"ssipy.modeltype",							/* tp_name */
	sizeof(ssipymodeltype),	                    /* tp_basicsize */
	0,                                          /* tp_itemsize */
	(destructor)ssipymodeltype_dealloc,			/* tp_dealloc */
	0,                                          /* tp_print */
	0,                                          /* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_reserved */
	0,                                          /* tp_repr */
	0,                                          /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	0,                                          /* tp_str */
	0,											/* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_BASETYPE							/* tp_flags */
	| Py_TPFLAGS_BASETYPE,						/* tp_flags */
	"ssipymodeltype",							/* tp_doc */
	0,											/* tp_traverse */
	0,											/* tp_clear */
	0,											/* tp_richcompare */
	0,											/* tp_weaklistoffset */
	0,											/* tp_iter */
	0,											/* tp_iternext */
	0,											/* tp_methods */
	ssipymodeltype_members,						/* tp_members */
	0,											/* tp_getset */
	0,											/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)ssipymodeltype_init,				/* tp_init */
	0,											/* tp_alloc */
	0											/* tp_new */
};


//////////////////////////////////////
//module function ////////////////////
//////////////////////////////////////

// init
PyMODINIT_FUNC PyInit_ssipy(void)
{
	PyObject* m;

	if (PyType_Ready(&ssipyarray_Type) < 0)
	{
		return NULL;
	}

	ssipyarrayiter_Type.tp_new = PyType_GenericNew;
	if (PyType_Ready(&ssipyarrayiter_Type) < 0)
	{
		return NULL;
	}

	if (PyType_Ready(&ssipystream_Type) < 0)
	{
		return NULL;
	}

	ssipystreamiter_Type.tp_new = PyType_GenericNew;
	if (PyType_Ready(&ssipystreamiter_Type) < 0)
	{
		return NULL;
	}

	ssipytype_Type.tp_new = PyType_GenericNew;
	if (PyType_Ready(&ssipytype_Type) < 0)
	{
		return NULL;
	}

	ssipyinfo_Type.tp_new = PyType_GenericNew;
	if (PyType_Ready(&ssipyinfo_Type) < 0)
	{
		return NULL;
	}

	ssipyimageparams_Type.tp_new = PyType_GenericNew;
	if (PyType_Ready(&ssipyimageparams_Type) < 0)
	{
		return NULL;
	}

	if (PyType_Ready(&ssipyimage_Type) < 0)
	{
		return NULL;
	}

	ssipychannel_Type.tp_new = PyType_GenericNew;
	if (PyType_Ready(&ssipychannel_Type) < 0)
	{
		return NULL;
	}

	if (PyType_Ready(&ssipyevent_Type) < 0)
	{
		return NULL;
	}

	ssipyeventboard_Type.tp_new = PyType_GenericNew;
	if (PyType_Ready(&ssipyeventboard_Type) < 0)
	{
		return NULL;
	}

	ssipymodeltype_Type.tp_new = PyType_GenericNew;
	if (PyType_Ready(&ssipymodeltype_Type) < 0)
	{
		return NULL;
	}

	m = PyModule_Create(&ssipy_module);
	if (m == NULL)
	{
		return NULL;
	}

	PyModule_AddStringConstant(m, "VERSION", SSI_VERSION);
	PyModule_AddStringConstant(m, "ABOUT", SSI_COPYRIGHT);
	PyModule_AddIntConstant(m, "UNDEF", SSI_UNDEF);
	PyModule_AddIntConstant(m, "CHAR", SSI_CHAR);
	PyModule_AddIntConstant(m, "UCHAR", SSI_UCHAR);
	PyModule_AddIntConstant(m, "SHORT", SSI_SHORT);
	PyModule_AddIntConstant(m, "USHORT", SSI_USHORT);
	PyModule_AddIntConstant(m, "INT", SSI_INT);
	PyModule_AddIntConstant(m, "UINT", SSI_UINT);
	PyModule_AddIntConstant(m, "LONG", SSI_LONG);
	PyModule_AddIntConstant(m, "ULONG", SSI_ULONG);
	PyModule_AddIntConstant(m, "FLOAT", SSI_FLOAT);
	PyModule_AddIntConstant(m, "DOUBLE", SSI_DOUBLE);
	PyModule_AddIntConstant(m, "CONTINUED", SSI_ESTATE_CONTINUED);
	PyModule_AddIntConstant(m, "COMPLETED", SSI_ESTATE_COMPLETED);
	PyModule_AddIntConstant(m, "CLASSIFICATION", 0);
	PyModule_AddIntConstant(m, "REGRESSION", 1);

	Py_INCREF(&ssipyarray_Type);
	PyModule_AddObject(m, "array", (PyObject *)&ssipyarray_Type);

	Py_INCREF(&ssipyarrayiter_Type);
	PyModule_AddObject(m, "arrayiter", (PyObject *)&ssipyarrayiter_Type);

	Py_INCREF(&ssipyarray_Type);
	PyModule_AddObject(m, "stream", (PyObject *)&ssipystream_Type);

	Py_INCREF(&ssipystreamiter_Type);
	PyModule_AddObject(m, "streamiter", (PyObject *)&ssipystreamiter_Type);

	Py_INCREF(&ssipytype_Type);
	PyModule_AddObject(m, "type", (PyObject *)&ssipytype_Type);

	Py_INCREF(&ssipyinfo_Type);
	PyModule_AddObject(m, "info", (PyObject *)&ssipyinfo_Type);

	Py_INCREF(&ssipyimageparams_Type);
	PyModule_AddObject(m, "imageparams", (PyObject *)&ssipyimageparams_Type);

	Py_INCREF(&ssipyimage_Type);
	PyModule_AddObject(m, "image", (PyObject *)&ssipyimage_Type);

	Py_INCREF(&ssipychannel_Type);
	PyModule_AddObject(m, "channel", (PyObject *)&ssipychannel_Type);

	Py_INCREF(&ssipyevent_Type);
	PyModule_AddObject(m, "event", (PyObject *)&ssipyevent_Type);

	Py_INCREF(&ssipyeventboard_Type);
	PyModule_AddObject(m, "eventboard", (PyObject *)&ssipyeventboard_Type);

	Py_INCREF(&ssipymodeltype_Type);
	PyModule_AddObject(m, "modeltype", (PyObject *)&ssipymodeltype_Type);

	return m;
}

//////////////////////////////////////
// some c functions for convenience //
//////////////////////////////////////

ssipytype *ssipytype_New()
{
	ssipytype *self = PyObject_New(ssipytype, &ssipytype_Type);	
	ssipytype_Init(self);

	return self;
}

void ssipytype_Init(ssipytype *self)
{
	if (self != NULL)
	{
		self->UNDEF = SSI_UNDEF;
		self->CHAR = SSI_CHAR;
		self->UCHAR = SSI_UCHAR;
		self->SHORT = SSI_SHORT;
		self->USHORT = SSI_USHORT;
		self->INT = SSI_INT;
		self->UINT = SSI_UINT;
		self->LONG = SSI_LONG;
		self->ULONG = SSI_ULONG;
		self->FLOAT = SSI_FLOAT;
		self->DOUBLE = SSI_DOUBLE;
	}
}

ssipyinfo *ssipyinfo_New(ssi_time_t time, ssi_time_t dur, Py_ssize_t frame, Py_ssize_t delta)
{
	ssipyinfo *self = PyObject_New(ssipyinfo, &ssipyinfo_Type);
	ssipyinfo_Init(self, time, dur, frame, delta);

	return self;
}

void ssipyinfo_Init(ssipyinfo *self, ssi_time_t time, ssi_time_t dur, Py_ssize_t frame, Py_ssize_t delta)
{
	if (self != NULL)
	{
		self->time = time;
		self->dur = dur;
		self->frame = frame;
		self->delta = delta;
	}
}

ssipychannel *ssipychannel_New(Py_ssize_t dim, Py_ssize_t byte, ssi_type_t type, ssi_time_t sr)
{
	ssipychannel *self = PyObject_New(ssipychannel, &ssipychannel_Type);
	ssipychannel_Init(self, dim, byte, type, sr);

	return self;
}

void ssipychannel_Init(ssipychannel *self, Py_ssize_t dim, Py_ssize_t byte, ssi_type_t type, ssi_time_t sr)
{
	if (self != NULL)
	{
		self->dim = dim;
		self->byte = byte;
		self->type = type;
		self->sr = sr;
	}
}

void ssipyarray_Init(ssipyarray *self, Py_ssize_t len)
{
	if (self != NULL)
	{
		self->len = len;
		self->borrowed = 0;
		if (self->len > 0)
		{
			self->ptr = (ssi_real_t*)malloc(self->len * sizeof(ssi_real_t));
			memset(self->ptr, 0, self->len * sizeof(ssi_real_t));
		}
	}
}

ssipyarray *ssipyarray_New(Py_ssize_t len)
{
	ssipyarray *self = PyObject_New(ssipyarray, &ssipyarray_Type);
	ssipyarray_Init(self, len);

	return self;
}

ssipyarray *ssipyarray_From(Py_ssize_t len, ssi_real_t *ptr)
{
	ssipyarray *self = PyObject_New(ssipyarray, &ssipyarray_Type);

	self->len = len;
	self->ptr = ptr;
	self->borrowed = 1;

	return self;
}

void ssipyarray_Print(ssipyarray *self)
{
	char *tmp = ssipyarray_ToString(self, self->len);
	printf("%s\n", tmp);
	free(tmp);
}

char *ssipyarray_ToString(ssipyarray *self, Py_ssize_t nmax)
{
	char* output = (char*)malloc(nmax * self->len * 20);

	int pos = sprintf(output, "[ ");
	for (Py_ssize_t k = 0; k < self->len && k < nmax; k++)
	{
		pos += sprintf(output + pos, "%f ", self->ptr[k]);
	}
	if (self->len > nmax)
	{
		pos += sprintf(output + pos, "...");
	}
	sprintf(output + pos, " ]");

	return output;
}

void ssipystream_Init(ssipystream *self, Py_ssize_t num, Py_ssize_t dim, Py_ssize_t byte, ssi_type_t type, ssi_time_t sr, ssi_time_t time)
{	
	if (self != NULL)
	{
		self->num = max(0, num);
		self->dim = max(0, dim);
		self->len = self->num * self->dim;
		self->sr = sr;
		self->time = time;
		self->type = type;
		self->type_s = stringssipystream_TypeToString(type);
		self->byte = byte;
		self->tot = self->len * self->byte;
		self->ptr = 0;
		self->borrowed = 0;
		if (self->len > 0)
		{
			self->ptr = (ssi_byte_t*)malloc(self->tot);
			memset(self->ptr, 0, self->tot);
		}
	}
}

ssipystream *ssipystream_New(Py_ssize_t num, Py_ssize_t dim, ssi_type_t type, ssi_time_t sr, ssi_time_t time)
{
	ssipystream *self = PyObject_New(ssipystream, &ssipystream_Type);
	Py_ssize_t byte = ssi_type2bytes(type);
	if (byte == 0)
	{
		PyErr_SetString(PyExc_TypeError, "ssi type is not supported");		
		return NULL;
	}

	ssipystream_Init(self, num, dim, byte, type, sr, time);

	return self;
}

ssipystream *ssipystream_From(Py_ssize_t num, Py_ssize_t dim, Py_ssize_t byte, ssi_type_t type, ssi_time_t sr, ssi_time_t time, ssi_byte_t *ptr)
{
	ssipystream *self = PyObject_New(ssipystream, &ssipystream_Type);

	self->num = num;
	self->dim = dim;
	self->len = num * dim;
	self->byte = byte;
	self->sr = sr;
	self->time = time;
	self->type = type;
	self->type_s = stringssipystream_TypeToString(type);
	self->tot = self->len * self->byte;
	self->ptr = ptr;
	self->borrowed = 1;

	return self;
}

void ssipystream_Print(ssipystream *self)
{
	char *tmp = ssipystream_ToString(self, self->len);
	printf("%s\n", tmp);
	free(tmp);
}

char *ssipystream_ToString(ssipystream *self, Py_ssize_t nmax)
{
	char* output = (char*)malloc(nmax * self->dim * 20);
	
	int pos = sprintf(output, "[ ");
	for (Py_ssize_t k = 0; k < self->len && k < nmax; k++) 
	{
		pos += ssipystream_ValueToString(self, k, output + pos);
		if (k + 1 < self->len && k + 1 < nmax)
		{
			if (((k + 1) % self->dim) == 0)
			{
				pos += sprintf(output + pos, "\n  ");
			}
			else
			{
				pos += sprintf(output + pos, " ");
			}
		}
	}
	if (self->len > nmax)
	{
		pos += sprintf(output + pos, "...");
	}
	sprintf(output+pos, " ]");

	return output;
}

ssipyimageparams *ssipyimageparams_New(Py_ssize_t width, Py_ssize_t height, Py_ssize_t channels, Py_ssize_t depth)
{
	ssipyimageparams *self = PyObject_New(ssipyimageparams, &ssipyimageparams_Type);
	ssipyimageparams_Init(self, width, height, channels, depth);

	return self;
}

void ssipyimageparams_Init(ssipyimageparams *self, Py_ssize_t width, Py_ssize_t height, Py_ssize_t channels, Py_ssize_t depth)
{
	if (self != NULL)
	{
		self->width = width;
		self->height = height;
		self->channels = channels;
		self->depth = depth;
	}
}

Py_ssize_t ssipyimage_Stride(Py_ssize_t width, Py_ssize_t channels, Py_ssize_t depth)
{
	return ((((width * channels * ((depth * 8) & ~0x80000000) + 7) >> 3) + 3) & (~3));
}

void ssipyimage_Init(ssipyimage *self, Py_ssize_t width, Py_ssize_t height, Py_ssize_t channels, Py_ssize_t depth, ssi_time_t sr, ssi_time_t time)
{
	if (self != NULL)
	{
		self->width = width;
		self->height = height;
		self->channels = channels;
		self->depth = depth;
		self->sr = sr;
		self->time = time;
		self->stride = ssipyimage_Stride(width, channels, depth);
		self->tot = self->stride * height;		
		self->ptr = 0;
		self->borrowed = 0;
		if (self->tot > 0)
		{
			self->ptr = (ssi_byte_t*)malloc(self->tot);
			memset(self->ptr, 0, self->tot);
		}
	}
}

ssipyimage *ssipyimage_New(Py_ssize_t width, Py_ssize_t height, Py_ssize_t channels, Py_ssize_t depth, ssi_time_t sr, ssi_time_t time)
{
	ssipyimage *self = PyObject_New(ssipyimage, &ssipyimage_Type);	
	ssipyimage_Init(self, width, height, channels, depth, sr, time);

	return self;
}

ssipyimage *ssipyimage_From(Py_ssize_t width, Py_ssize_t height, Py_ssize_t channels, Py_ssize_t depth, ssi_time_t sr, ssi_time_t time, ssi_byte_t *ptr)
{
	ssipyimage *self = PyObject_New(ssipyimage, &ssipyimage_Type);

	self->width = width;
	self->height = height;
	self->channels = channels;
	self->depth = depth;
	self->sr = sr;
	self->time = time;
	self->stride = ssipyimage_Stride(width, channels, depth);
	self->tot = self->stride * height;
	self->ptr = ptr;
	self->borrowed = 1;

	return self;
}

void ssipyevent_Init(ssipyevent *self, Py_ssize_t time_ms, Py_ssize_t dur_ms, const char *address, PyObject *data, ssi_estate_t state, Py_ssize_t glue, float prob)
{
	if (data)		
	{
		if (ssipyevent_ObjectToType(data) == SSI_ETYPE_UNDEF)
		{
			PyErr_SetString(PyExc_TypeError, "event type is undefined");
		}
		Py_INCREF(data);
	}	


	self->time = time_ms;
	self->dur = dur_ms;
	if (address)
	{
		self->address = (char *)malloc((strlen(address) + 1) * sizeof(char));
		strcpy(self->address, address);
	}
	else
	{
		self->address = 0;
	}
	self->data = data;
	self->state = state;
	self->glue = glue;
	self->prob = prob;
}

ssipyevent *ssipyevent_New(Py_ssize_t time_ms, Py_ssize_t dur_ms, const char *address, PyObject *data, ssi_estate_t state, Py_ssize_t glue, float prob)
{
	ssipyevent *self = PyObject_New(ssipyevent, &ssipyevent_Type);
	
	ssipyevent_Init(self, time_ms, dur_ms, address, data, glue, state, prob);

	return self;
}

void ssipyevent_Print(ssipyevent *self)
{
	char *str = ssipyevent_ToString(self);
	printf("%s\n", str);
	free(str);
}

char *ssipyevent_ToString(ssipyevent *self)
{
	char *result;

	if (self->data)
	{
		PyObject *pStr = PyObject_Repr(self->data);
		char *str = PyUnicode_AsUTF8(pStr);
		result = (char *)malloc(sizeof(char) * (strlen(str) + 1));
		strcpy(result, str);
		Py_DECREF(pStr);
	}
	else
	{
		result = (char *)malloc(sizeof(char));
		result[0] = '\0';
	}	

	return result;
}

void ssipyeventboard_Init(ssipyeventboard *self, void *client, ssipyupdatefunc_t update)
{
	if (self != NULL)
	{
		self->client = client;
		self->update = update;
		self->COMPLETED = SSI_ESTATE_COMPLETED;
		self->CONTINUED = SSI_ESTATE_CONTINUED;
	}
}

ssipyeventboard *ssipyeventboard_New(void *client, ssipyupdatefunc_t update)
{
	ssipyeventboard *self = PyObject_New(ssipyeventboard, &ssipyeventboard_Type);

	ssipyeventboard_Init(self, client, update);

	return self;
}

ssipymodeltype *ssipymodeltype_New()
{
	ssipymodeltype *self = PyObject_New(ssipymodeltype, &ssipymodeltype_Type);
	ssipymodeltype_Init(self);

	return self;
}

void ssipymodeltype_Init(ssipymodeltype *self)
{
	if (self != NULL)
	{
		self->CLASSIFICATION = 0;
		self->REGRESSION = 1;
	}
}
