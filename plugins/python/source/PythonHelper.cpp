// PythonHelper.cpp
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

#include "PythonHelper.h"
#include "base/Factory.h"
#include "ioput/file/FileTools.h"
#include "event/EventAddress.h"
#include "PythonChannel.h"
#include "ssiml/include/ssiml.h"
extern "C"
{
#include "ssipy.h"
}

namespace ssi {

	ssi_char_t *PythonHelper::ssi_log_name = "pyhelper__";
	ssi_char_t *PythonHelper::FUNCTION_NAMES[] =
	{
		"getOptions",
		"getSampleNumberOut",
		"getSampleDimensionOut",
		"getSampleBytesOut",
		"getSampleTypeOut",
		"getImageFormatOut",
		"setImageFormatIn",
		"transform_enter",
		"transform",
		"transform_flush",
		"consume_enter",
		"consume",
		"consume_flush",
		"getEventAddress",
		"send_enter",
		"send_flush",
		"listen_enter",
		"update",
		"listen_flush",
		"getChannelNames",
		"initChannel",
		"connect",
		"read",
		"disconnect",
		"getModelType",
		"train",
		"forward",
		"save",
		"load",
	};

	PythonHelper::GIL::GIL()
	{
		_state = PyGILState_Ensure();
	}

	PythonHelper::GIL::~GIL()
	{
		PyGILState_Release((PyGILState_STATE)_state);
	}

	PythonHelper::PythonHelper(ssi_char_t *script_name,
		const ssi_char_t *optsfile,
		const ssi_char_t *optsstr,
		const ssi_char_t *syspath,
		const ssi_char_t *workdir)
		: _pModule(0),
		_pBoard(0),
		_pOptions(0),
		_pVariables(0),
		_listener(0),
		_address(0),
		_n_channels(0),
		_channels(0),
		_provider(0),
		_has_image_params(false)
	{
		for (int i = 0; i < FUNCTIONS::NUM; i++)
		{
			_pFunctions[i] = 0;
		}

		_script_name = ssi_strcpy(script_name);

		GIL gil;

		add_sys_path(syspath, workdir);

		ssi_msg(SSI_LOG_LEVEL_BASIC, "loading script '%s.py'", _script_name);
		_pModule = PyImport_ImportModule(_script_name);

		if (_pModule != NULL)
		{
			int n = 0;
			for (int i = 0; i < FUNCTIONS::NUM; i++)
			{
				n += register_function(FUNCTION_NAMES[i], &_pFunctions[i]);
			}
		}
		else
		{
			PyErr_Print();
			ssi_wrn("could not load script '%s.py'", script_name);
		}

		_pBoard = ssipyeventboard_New(this, PythonHelper::Update);
		_pVariables = PyDict_New();
		_pOptions = PyDict_New();
		options_get(FUNCTIONS::GET_OPTIONS, optsfile, optsstr);
	}

	PythonHelper::~PythonHelper()
	{
		GIL gil;

		for (int i = 0; i < FUNCTIONS::NUM; i++)
		{
			if (_pFunctions[i])
			{
				Py_XDECREF(_pFunctions[i]);
			}
		}
		Py_XDECREF(_pBoard); _pBoard = 0;
		Py_XDECREF(_pVariables); _pVariables = 0;
		Py_XDECREF(_pOptions); _pOptions = 0;
		Py_XDECREF(_pModule); _pModule = 0;
		delete[] _script_name; _script_name = 0;
		delete[] _address; _address = 0;

		for (ssi_size_t i = 0; i < _n_channels; i++)
		{
			delete _channels[i];
		}
		delete[] _channels; _channels = 0;
		delete[] _provider; _provider = 0;
		_n_channels = 0;
	}

	int PythonHelper::register_function(const ssi_char_t *name, PyObject **function)
	{
		if (PyObject_HasAttrString(_pModule, name))
		{
			*function = PyObject_GetAttrString(_pModule, name);
			if (*function && PyCallable_Check(*function))
			{
				ssi_msg(SSI_LOG_LEVEL_BASIC, "found function '%s'", name);
				return 1;
			}
		}
		*function = 0;

		return 0;
	}

	void PythonHelper::add_sys_path(const ssi_char_t *path, const ssi_char_t *workdir)
	{
		if (!path || path[0] == '\0')
		{
			return;
		}

		// change working directory

		ssi_char_t workdir_old[SSI_MAX_CHAR];
		if (workdir != 0)
		{		
			ssi_getcwd(SSI_MAX_CHAR, workdir_old);			
			ssi_setcwd(workdir);
		}

		// populate system path

		PyObject* sysPath = PySys_GetObject((char*)"path");
		bool changed = false;

		ssi_size_t n_tokens = ssi_split_string_count(path, ';');
		if (n_tokens > 0)
		{
			ssi_char_t **tokens = new ssi_char_t *[n_tokens];
			ssi_split_string(n_tokens, tokens, path, ';');

			for (ssi_size_t i = 0; i < n_tokens; i++)
			{
				ssi_char_t *full = ssi_fullpath(tokens[i]);
				PyObject *pPath = PyUnicode_DecodeLocale(full, NULL);

				bool found = false;
				PyObject *item;
				for (Py_ssize_t j = 0; j < PyList_Size(sysPath); j++)
				{
					item = PyList_GetItem(sysPath, j);
					if (PyUnicode_Compare(item, pPath) == 0)
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					PyList_Append(sysPath, pPath);
					changed = true;
				}

				Py_DECREF(pPath);
				delete[] tokens[i];
				delete[] full;
			}

			delete[] tokens;
		}

		if (changed)
		{
			PyObject *pSysPathStr = PyObject_Str(sysPath);
			ssi_msg(SSI_LOG_LEVEL_BASIC, "new sys path '%s'", PyUnicode_AsUTF8(pSysPathStr));
			Py_DECREF(pSysPathStr);
		}

		// reset working directory
		if (workdir != 0)
		{
			ssi_setcwd(workdir_old);
		}

	}

	PyObject *PythonHelper::call_function(function_t function, PyObject *pArgs)
	{
		PyObject *pValue = PyObject_CallObject(_pFunctions[function], pArgs);

		if (pValue == NULL)
		{
			PyErr_Print();
			ssi_wrn("function call failed '%s:%s'", _script_name, FUNCTION_NAMES[function]);
		}

		return pValue;
	}

	PyObject *PythonHelper::stream_to_imageobject(ssi_stream_t *stream, ssi_video_params_t params)
	{
		ssipyimage *object = ssipyimage_From(params.widthInPixels, params.heightInPixels, params.numOfChannels, params.depthInBitsPerChannel / 8, stream->sr, stream->time, stream->ptr);

		if (object->tot != stream->byte)
		{
			ssi_wrn("could not convert stream to image, memory size differs ('%lld' != '%u')", object->tot, stream->byte);
			return 0;
		}

		return (PyObject *)object;
	}

	PyObject *PythonHelper::stream_to_object(ssi_stream_t *stream)
	{
		return (PyObject *)ssipystream_From(stream->num, stream->dim, stream->byte, stream->type, stream->sr, stream->time, stream->ptr);
	}



	PyObject *PythonHelper::samplelist_to_object(ISamples &samples, ssi_size_t stream_index)
	{

		PyObject *pList_stream = PyTuple_New(samples.getSize());

		PyObject *pObject_stream = 0;

		samples.reset();
		ssi_sample_t *sample = 0;
		int sample_counter = 0;
		while (sample = samples.next())
		{
			pObject_stream = stream_to_object(sample->streams[stream_index]);
			PyTuple_SetItem(pList_stream, sample_counter, pObject_stream);
			sample_counter++;
		}

		return pList_stream;
	}


	PyObject *PythonHelper::score_to_object(ISamples &samples, ssi_size_t stream_index)
	{

		PyObject *pList_score = PyTuple_New(samples.getSize());

		ssi_real_t score;

		samples.reset();
		ssi_sample_t *sample = 0;
		int sample_counter = 0;
		while (sample = samples.next())
		{
			score = sample->score;
			PyTuple_SetItem(pList_score, sample_counter, PyFloat_FromDouble(score));
			sample_counter++;
		}

		return pList_score;
	}


	PyObject *PythonHelper::labels_to_object(ISamples &samples, ssi_size_t stream_index)
	{

		PyObject *pList_label = PyTuple_New(samples.getSize());

		ssi_size_t label;

		samples.reset();
		ssi_sample_t *sample = 0;
		int sample_counter = 0;
		while (sample = samples.next())
		{
			label = sample->class_id;
			PyTuple_SetItem(pList_label, sample_counter, PyLong_FromLong(label));
			sample_counter++;
		}

		return pList_label;
	}

	// options

	void PythonHelper::options_parse(const ssi_char_t *string, char delim)
	{
		ssi_size_t n_split = ssi_split_string_count(string, delim);
		if (n_split > 0)
		{
			ssi_char_t **split = new ssi_char_t *[n_split];
			ssi_split_string(n_split, split, string, delim);
			for (ssi_size_t n = 0; n < n_split; n++)
			{
				ssi_char_t *key = 0;
				ssi_char_t *value = 0;
				if (ssi_parse_option(split[n], &key, &value))
				{
					if (options_apply(key, value))
					{
						if (ssi_log_level >= SSI_LOG_LEVEL_DETAIL)
						{
							ssi_print("             '%s' -> %s\n", key, value);
						}
					}
				}
				delete[] key;
				delete[] value;
				delete[] split[n];
			}
			delete[] split;
		}
	}

	void PythonHelper::options_get(function_t function, const ssi_char_t *optsfile, const ssi_char_t *optsstr)
	{
		if (_pFunctions[function])
		{
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "options from script '%s.py'", _script_name);

			PyObject *pArgs = PyTuple_New(2);
			PyTuple_SetItem(pArgs, 0, _pOptions);
			PyTuple_SetItem(pArgs, 1, _pVariables);
			Py_INCREF(_pOptions);
			Py_INCREF(_pVariables);

			call_function(FUNCTIONS::GET_OPTIONS, pArgs);
			Py_DECREF(pArgs);

			if (ssi_log_level >= SSI_LOG_LEVEL_DETAIL)
			{
				PyObject *key, *value;
				Py_ssize_t pos = 0;

				while (PyDict_Next(_pOptions, &pos, &key, &value))
				{
					PyObject *pKey = PyObject_Repr(key);
					PyObject *pValue = PyObject_Repr(value);
					ssi_print("             %s -> %s\n", PyUnicode_AsUTF8(pKey), PyUnicode_AsUTF8(pValue));
					Py_DECREF(pKey);
					Py_DECREF(pValue);
				}
			}
		}

		if (optsfile && optsfile[0] != '\0')
		{
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "options from file '%s'", optsfile);

			ssi_size_t n_content = 0;
			char *content = FileTools::ReadAsciiFile(optsfile, n_content);
			if (content)
			{
				options_parse(content, '\n');
			}
			delete[] content;

		}

		if (optsstr && optsstr[0] != '\0')
		{
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "options from string '%s'", optsstr);
			options_parse(optsstr, ';');
		}

	}

	bool PythonHelper::options_apply(const ssi_char_t *key, const ssi_char_t *value)
	{
		PyObject *pKey = PyUnicode_FromString(key);
		bool result = false;

		if (pKey && PyDict_Contains(_pOptions, pKey))
		{
			PyObject *pValue = PyDict_GetItem(_pOptions, pKey);
			PyObject *pNewValue = NULL;

			if (PyBool_Check(pValue))
			{
				if (ssi_strcmp(value, "true", false))
				{
					pNewValue = Py_True;
				}
				else
				{
					pNewValue = Py_False;
				}
			}
			else if (PyLong_Check(pValue))
			{
				pNewValue = PyLong_FromString(value, 0, 0);
			}
			else if (PyFloat_Check(pValue))
			{
				PyObject *tmp = PyUnicode_FromString(value);
				pNewValue = PyFloat_FromString(tmp);
				Py_DECREF(tmp);
			}
			else if (PyUnicode_Check(pValue))
			{
				pNewValue = PyUnicode_FromString(value);
			}
			else
			{
				ssi_wrn("unknown type '%s'", pValue->ob_type->tp_name);
			}

			if (pNewValue)
			{
				if (PyDict_SetItem(_pOptions, pKey, pNewValue) != -1)
				{
					result = true;
				}
				Py_DECREF(pNewValue);
			}
		}
		else
		{
			ssi_wrn("unkown key '%s'", key);
		}

		Py_XDECREF(pKey);

		return result;
	}

	// transformer

	bool PythonHelper::transform_get_help(function_t function, ssi_size_t in, ssi_size_t &out)
	{
		out = 0;

		if (!_pFunctions[function])
		{
			out = in;
			return true;
		}

		GIL gil;

		bool result = false;

		PyObject *pArgs = PyTuple_New(3);

		PyTuple_SetItem(pArgs, 0, PyLong_FromLong(in));

		PyTuple_SetItem(pArgs, 1, _pOptions);
		Py_INCREF(_pOptions);

		PyTuple_SetItem(pArgs, 2, _pVariables);
		Py_INCREF(_pVariables);

		PyObject *pValue = call_function(function, pArgs);
		Py_DECREF(pArgs);

		if (pValue)
		{
			out = PyLong_AsUnsignedLong(pValue);
			Py_DECREF(pValue);
			result = true;
		}

		return result;
	}

	ssi_size_t PythonHelper::getSampleNumberOut(ssi_size_t sample_number_in)
	{
		ssi_size_t out = 0;
		if (transform_get_help(FUNCTIONS::GET_SAMPLE_NUMBER, sample_number_in, out))
		{
			return out;
		}

		ssi_err("could not determine sample number of output stream");

		return 0;
	}

	ssi_size_t PythonHelper::getSampleDimensionOut(ssi_size_t sample_dimension_in)
	{
		ssi_size_t out = 0;
		if (transform_get_help(FUNCTIONS::GET_SAMPLE_DIMENSION, sample_dimension_in, out))
		{
			return out;
		}

		ssi_err("could not determine sample dimension of output stream");

		return 0;
	}

	ssi_size_t PythonHelper::getSampleBytesOut(ssi_size_t sample_bytes_in)
	{
		ssi_size_t out = 0;
		if (transform_get_help(FUNCTIONS::GET_SAMPLE_BYTES, sample_bytes_in, out))
		{
			return out;
		}

		ssi_err("could not determine sample bytes of output stream");

		return 0;
	}

	ssi_type_t PythonHelper::getSampleTypeOut(ssi_type_t sample_type_in)
	{
		if (!_pFunctions[FUNCTIONS::GET_SAMPLE_TYPE])
		{
			return sample_type_in;
		}

		GIL gil;

		bool result = false;

		PyObject *pArgs = PyTuple_New(4);

		PyTuple_SetItem(pArgs, 0, PyLong_FromLong(sample_type_in));

		PyTuple_SetItem(pArgs, 1, (PyObject *)ssipytype_New());

		PyTuple_SetItem(pArgs, 2, _pOptions);
		Py_INCREF(_pOptions);

		PyTuple_SetItem(pArgs, 3, _pVariables);
		Py_INCREF(_pVariables);

		PyObject *pValue = call_function(FUNCTIONS::GET_SAMPLE_TYPE, pArgs);
		Py_DECREF(pArgs);

		ssi_type_t out = SSI_UNDEF;

		if (pValue)
		{
			out = (ssi_type_t)PyLong_AsUnsignedLong(pValue);
			Py_DECREF(pValue);
		}

		if (out == SSI_UNDEF)
		{
			ssi_err("could not determine sample type of output stream");
		}

		return out;
	}

	ssi_video_params_t PythonHelper::getImageFormatOut(ssi_video_params_t params_in)
	{
		_image_params_in = params_in;
		_image_params_out = params_in;
		_has_image_params = true;

		if (!_pFunctions[FUNCTIONS::GET_IMAGE_FORMAT])
		{
			return _image_params_out;
		}

		GIL gil;

		PyObject *pArgs = PyTuple_New(3);
		ssipyimageparams *PFormatIn = ssipyimageparams_New(params_in.widthInPixels, params_in.heightInPixels, params_in.numOfChannels, params_in.depthInBitsPerChannel / 8);
		PyTuple_SetItem(pArgs, 0, (PyObject*)PFormatIn);

		PyTuple_SetItem(pArgs, 1, _pOptions);
		Py_INCREF(_pOptions);

		PyTuple_SetItem(pArgs, 2, _pVariables);
		Py_INCREF(_pVariables);

		PyObject *pFormatOut = call_function(FUNCTIONS::GET_IMAGE_FORMAT, pArgs);
		Py_DECREF(pArgs);

		if (pFormatOut)
		{
			ssipyimageparams *p = (ssipyimageparams *)pFormatOut;

			_image_params_out.widthInPixels = (int)p->width;
			_image_params_out.heightInPixels = (int)p->height;
			_image_params_out.numOfChannels = (int)p->channels;
			_image_params_out.depthInBitsPerChannel = (int)p->depth * 8;

			Py_DECREF(pFormatOut);
		}

		return _image_params_out;
	}


	void PythonHelper::setImageFormatIn(ssi_video_params_t params_in)
	{
		_image_params_in = params_in;
		_image_params_out = params_in;
		_has_image_params = true;

		if (!_pFunctions[FUNCTIONS::SET_IMAGE_FORMAT])
		{
			return;
		}

		GIL gil;

		bool result = false;

		PyObject *pArgs = PyTuple_New(3);
		ssipyimageparams *PFormatIn = ssipyimageparams_New(params_in.widthInPixels, params_in.heightInPixels, params_in.numOfChannels, params_in.depthInBitsPerChannel / 8);

		PyTuple_SetItem(pArgs, 0, (PyObject *)PFormatIn);

		PyTuple_SetItem(pArgs, 1, _pOptions);
		Py_INCREF(_pOptions);

		PyTuple_SetItem(pArgs, 2, _pVariables);
		Py_INCREF(_pVariables);

		PyObject *pValue = call_function(FUNCTIONS::SET_IMAGE_FORMAT, pArgs);
		Py_DECREF(pArgs);

		if (pValue)
		{
			Py_DECREF(pValue);
		}
	}

	PyObject *PythonHelper::transform_args(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[],
		ITransformer::info *info)
	{
		PyObject *pArgs = PyTuple_New(6 + (info ? 1 : 0));
		int valcount = 0;

		if (info != 0)
		{
			ssipyinfo *pInfo = ssipyinfo_New(info->time, stream_in.num / stream_in.sr, info->frame_num, info->delta_num);
			PyTuple_SetItem(pArgs, valcount++, (PyObject*)pInfo);
		}

		PyObject *pObject_stream_in = 0;
		if (stream_in.type == SSI_IMAGE && _has_image_params)
		{
			pObject_stream_in = stream_to_imageobject(&stream_in, _image_params_in);
		}
		else
		{
			pObject_stream_in = stream_to_object(&stream_in);
		}

		PyTuple_SetItem(pArgs, valcount++, pObject_stream_in);

		PyObject *pObject_stream_out = 0;
		if (stream_out.type == SSI_IMAGE && _has_image_params)
		{
			pObject_stream_out = stream_to_imageobject(&stream_out, _image_params_out);
		}
		else
		{
			pObject_stream_out = stream_to_object(&stream_out);
		}

		PyTuple_SetItem(pArgs, valcount++, pObject_stream_out);

		PyObject *pList_xtra_in = PyTuple_New(xtra_stream_in_num);
		for (ssi_size_t i = 0; i < xtra_stream_in_num; i++)
		{
			PyObject *pObject_stream = stream_to_object(&xtra_stream_in[i]);
			PyTuple_SetItem(pList_xtra_in, i, pObject_stream);
		}
		PyTuple_SetItem(pArgs, valcount++, pList_xtra_in);

		PyTuple_SetItem(pArgs, valcount++, (PyObject *)_pBoard);
		Py_INCREF(_pBoard);

		PyTuple_SetItem(pArgs, valcount++, _pOptions);
		Py_INCREF(_pOptions);

		PyTuple_SetItem(pArgs, valcount++, _pVariables);
		Py_INCREF(_pVariables);

		return pArgs;
	}

	bool PythonHelper::transform_help(function_t function,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[],
		ITransformer::info *info)
	{
		if (!_pFunctions[function])
		{
			if (function == FUNCTIONS::TRANSFORM)
			{
				ssi_wrn("function is missing '%s:%s'", _script_name, FUNCTION_NAMES[function]);
			}

			return false;
		}

		GIL gil;

		bool result = false;

		PyObject *pArgs = transform_args(stream_in, stream_out, xtra_stream_in_num, xtra_stream_in, info);
		PyObject *pValue = call_function(function, pArgs);
		Py_DECREF(pArgs);

		if (pValue)
		{
			Py_DECREF(pValue);
			result = true;
		}

		return result;
	}

	bool PythonHelper::transform_enter(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[])
	{
		return transform_help(FUNCTIONS::TRANSFORM_ENTER, stream_in, stream_out, xtra_stream_in_num, xtra_stream_in);
	}

	bool PythonHelper::transform(ITransformer::info *info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[])
	{
		return transform_help(FUNCTIONS::TRANSFORM, stream_in, stream_out, xtra_stream_in_num, xtra_stream_in, info);
	}

	bool PythonHelper::transform_flush(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[])
	{
		return transform_help(FUNCTIONS::TRANSFORM_FLUSH, stream_in, stream_out, xtra_stream_in_num, xtra_stream_in);
	}

	// consumer
	PyObject *PythonHelper::consume_args(ssi_size_t stream_in_num,
		ssi_stream_t stream_in[],
		IConsumer::info *info)
	{
		PyObject *pArgs = PyTuple_New(4 + (info ? 1 : 0));
		int valcount = 0;

		if (info != 0)
		{
			ssipyinfo *pInfo = ssipyinfo_New(info->time, info->dur, 0, 0);
			PyTuple_SetItem(pArgs, valcount++, (PyObject*)pInfo);
		}

		PyObject *pList_stream = PyTuple_New(stream_in_num);
		for (ssi_size_t i = 0; i < stream_in_num; i++)
		{
			PyObject *pObject_stream = 0;
			if (stream_in[i].type == SSI_IMAGE && _has_image_params)
			{
				pObject_stream = stream_to_imageobject(&stream_in[i], _image_params_out);
			}
			else
			{
				pObject_stream = stream_to_object(&stream_in[i]);
			}
			PyTuple_SetItem(pList_stream, i, pObject_stream);
		}
		PyTuple_SetItem(pArgs, valcount++, pList_stream);

		PyTuple_SetItem(pArgs, valcount++, (PyObject *)_pBoard);
		Py_INCREF(_pBoard);

		PyTuple_SetItem(pArgs, valcount++, _pOptions);
		Py_INCREF(_pOptions);

		PyTuple_SetItem(pArgs, valcount++, _pVariables);
		Py_INCREF(_pVariables);

		return pArgs;
	}

	bool PythonHelper::consume_help(function_t function,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[],
		IConsumer::info *info)
	{
		if (!_pFunctions[function])
		{
			if (function == FUNCTIONS::CONSUME)
			{
				ssi_wrn("function is missing '%s:%s'", _script_name, FUNCTION_NAMES[function]);
			}

			return false;
		}

		GIL gil;

		bool result = false;

		PyObject *pArgs = consume_args(stream_in_num, stream_in, info);
		PyObject *pValue = call_function(function, pArgs);
		Py_DECREF(pArgs);

		if (pValue)
		{
			Py_DECREF(pValue);
			result = true;
		}

		return result;
	}

	bool PythonHelper::consume_enter(ssi_size_t stream_in_num,
		ssi_stream_t stream_in[])
	{
		return consume_help(FUNCTIONS::CONSUME_ENTER, stream_in_num, stream_in);
	}

	bool PythonHelper::consume(IConsumer::info *info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[])
	{
		return consume_help(FUNCTIONS::CONSUME, stream_in_num, stream_in, info);
	}

	bool PythonHelper::consume_flush(ssi_size_t stream_in_num,
		ssi_stream_t stream_in[])
	{
		return consume_help(FUNCTIONS::CONSUME_FLUSH, stream_in_num, stream_in);
	}

	// events
	ssi_event_t *PythonHelper::ConvertEvent(ssipyevent *event)
	{
		ssi_event_t *e = new ssi_event_t;
		e->dur = (ssi_size_t)event->dur;
		e->time = (ssi_size_t)event->time;
		e->state = event->state;
		e->prob = event->prob;
		e->glue_id = (ssi_size_t)event->glue;
		e->tot = e->tot_real = 0;
		e->ptr = 0;

		EventAddress ea;
		ea.setAddress(event->address);
		e->event_id = Factory::AddString(ea.getEvent(0));
		e->sender_id = Factory::AddString(ea.getSender(0));

		e->type = ssipyevent_ObjectToType(event->data);

		switch (e->type)
		{
		case SSI_ETYPE_EMPTY:
			break;
		case SSI_ETYPE_STRING:
		{
			char *string = PyUnicode_AsUTF8(event->data);
			e->tot = e->tot_real = ssi_strlen(string) + 1;
			e->ptr = ssi_strcpy(string);
			break;
		}
		case SSI_ETYPE_TUPLE:
		{
			if (PyFloat_Check(event->data))
			{
				e->tot = e->tot_real = sizeof(ssi_event_tuple_t);
				ssi_event_tuple_t *ptr = new ssi_event_tuple_t;
				*ptr = (ssi_event_tuple_t)PyFloat_AsDouble(event->data);
				e->ptr = ssi_pcast(ssi_byte_t, ptr);
			}
			else
			{
				Py_ssize_t n_tuple = 0;

				if (PyTuple_Check(event->data))
				{
					n_tuple = PyTuple_Size(event->data);
				}
				else if (PyList_Check(event->data))
				{
					n_tuple = PyList_Size(event->data);
				}

				if (n_tuple > 0)
				{
					e->tot = e->tot_real = (ssi_size_t)(n_tuple * sizeof(ssi_event_tuple_t));
					ssi_event_tuple_t *ptr = new ssi_event_tuple_t[n_tuple];

					PyObject *iter = PyObject_GetIter(event->data);
					PyObject *item;
					Py_ssize_t n = 0;
					while (item = PyIter_Next(iter)) {
						ptr[n++] = (ssi_event_tuple_t)PyFloat_AsDouble(item);
						Py_DECREF(item);
					}

					e->ptr = ssi_pcast(ssi_byte_t, ptr);
				}
			}

			break;
		}
		case SSI_ETYPE_MAP:
		{
			Py_ssize_t n_map = PyDict_Size(event->data);
			if (n_map > 0)
			{
				e->tot = e->tot_real = (ssi_size_t)(n_map * sizeof(ssi_event_map_t));
				ssi_event_map_t *ptr = new ssi_event_map_t[n_map];
				PyObject *key, *value;
				Py_ssize_t pos = 0;
				Py_ssize_t n = 0;
				while (PyDict_Next(event->data, &pos, &key, &value))
				{
					ptr[n].id = Factory::AddString(PyUnicode_AsUTF8(key));
					ptr[n++].value = (ssi_real_t)PyFloat_AsDouble(value);
				}
				e->ptr = ssi_pcast(ssi_byte_t, ptr);
			}
			break;
		}
		default:
			ssi_wrn("could not parse event data");
		}

		return e;
	}

	ssipyevent *PythonHelper::ConvertEvent(ssi_event_t *event)
	{
		PyObject *data = NULL;

		if (event->type == SSI_ETYPE_UNDEF)
		{
			PyErr_SetString(PyExc_TypeError, "event type is undefined");
			return NULL;
		}

		switch (event->type)
		{
		case SSI_ETYPE_EMPTY:
			break;
		case SSI_ETYPE_STRING:
			data = PyUnicode_FromString((char*)event->ptr);
			break;
		case SSI_ETYPE_TUPLE:
		{
			ssi_event_tuple_t *tuple = (ssi_event_tuple_t *)event->ptr;
			Py_ssize_t n = (Py_ssize_t)(event->tot / sizeof(ssi_event_tuple_t));
			data = PyTuple_New(n);
			for (Py_ssize_t i = 0; i < n; i++)
			{
				PyTuple_SetItem(data, i, PyFloat_FromDouble(tuple[i]));
			}
			break;
		}
		case SSI_ETYPE_MAP:
		{
			ssi_event_map_t *map = (ssi_event_map_t *)event->ptr;
			Py_ssize_t n = (Py_ssize_t)(event->tot / sizeof(ssi_event_map_t));
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
		ea.setEvents(Factory::GetString(event->event_id));
		ea.setSender(Factory::GetString(event->sender_id));

		return ssipyevent_New(event->time, event->dur, ea.getAddress(), data, event->state, event->glue_id, event->prob);
	}

	void PythonHelper::Update(void *self, ssipyevent *event)
	{
		PythonHelper *me = ssi_pcast(PythonHelper, self);
		if (me && me->_listener)
		{
			ssi_event_t *e = ConvertEvent(event);
			me->_listener->update(*e);
			ssi_event_destroy(*e);
			delete e;
		}
	}

	bool PythonHelper::setEventListener(IEventListener *listener)
	{
		if (_listener)
		{
			ssi_wrn("a listener was already set");
			return false;
		}

		_listener = listener;

		return true;
	}

	const ssi_char_t *PythonHelper::getEventAddress()
	{
		if (_address)
		{
			return _address;
		}

		if (_pFunctions[FUNCTIONS::GET_EVENT_ADDRESS])
		{
			GIL gil;

			PyObject *pArgs = listen_send_args();
			PyObject *pValue = call_function(FUNCTIONS::GET_EVENT_ADDRESS, pArgs);
			Py_DECREF(pArgs);

			if (pValue)
			{
				if (PyUnicode_Check(pValue))
				{
					char *address = PyUnicode_AsUTF8(pValue);
					if (address)
					{
						_address = ssi_strcpy(address);
						EventAddress ea;
						ea.setAddress(_address);
						for (ssi_size_t i = 0; i < ea.getEventsSize(); i++)
						{
							Factory::AddString(ea.getEvent(i));
						}
						for (ssi_size_t i = 0; i < ea.getSenderSize(); i++)
						{
							Factory::AddString(ea.getSender(i));
						}
					}
				}

				Py_DECREF(pValue);
			}
		}

		return _address;
	}

	bool PythonHelper::listen_send_help(function_t function)
	{
		if (!_pFunctions[function])
		{
			return false;
		}

		GIL gil;

		bool result = false;

		PyObject *pArgs = listen_send_args();
		PyObject *pValue = call_function(function, pArgs);
		Py_DECREF(pArgs);

		if (pValue)
		{
			Py_DECREF(pValue);
			result = true;
		}

		return result;
	}

	PyObject *PythonHelper::listen_send_args()
	{
		PyObject *pArgs = PyTuple_New(2);

		PyTuple_SetItem(pArgs, 0, _pOptions);
		Py_INCREF(_pOptions);

		PyTuple_SetItem(pArgs, 1, _pVariables);
		Py_INCREF(_pVariables);

		return pArgs;
	}

	bool PythonHelper::send_flush()
	{
		return listen_send_help(FUNCTIONS::SEND_FLUSH);
	}

	bool PythonHelper::send_enter()
	{
		return listen_send_help(FUNCTIONS::SEND_ENTER);
	}

	bool PythonHelper::listen_enter()
	{
		return listen_send_help(FUNCTIONS::LISTEN_ENTER);
	}

	bool PythonHelper::update_help(ssi_event_t &e)
	{
		if (!_pFunctions[FUNCTIONS::UPDATE])
		{
			return false;
		}

		GIL gil;

		bool result = false;

		ssipyevent *pEvent = ConvertEvent(&e);
		if (pEvent)
		{
			PyObject *pArgs = update_args(pEvent);
			PyObject *pValue = call_function(FUNCTIONS::UPDATE, pArgs);
			Py_DECREF(pArgs);

			if (pValue)
			{
				Py_DECREF(pValue);
				result = true;
			}
		}

		return result;
	}

	PyObject *PythonHelper::update_args(ssipyevent *pEvent)
	{
		PyObject *pArgs = PyTuple_New(4);

		PyTuple_SetItem(pArgs, 0, (PyObject*)pEvent);

		PyTuple_SetItem(pArgs, 1, (PyObject*)_pBoard);
		Py_INCREF(_pBoard);

		PyTuple_SetItem(pArgs, 2, _pOptions);
		Py_INCREF(_pOptions);

		PyTuple_SetItem(pArgs, 3, _pVariables);
		Py_INCREF(_pVariables);

		return pArgs;
	}

	bool PythonHelper::update(ssi_event_t &e)
	{
		return update_help(e);
	}

	bool PythonHelper::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms)
	{
		if (n_new_events == 0)
		{
			return true;
		}

		bool result = true;

		ssi_event_t **es = new ssi_event_t *[n_new_events];
		for (ssi_size_t i = 0; i < n_new_events; i++)
		{
			es[i] = events.next();
		}
		for (ssi_size_t i = n_new_events; i > 0; i--)
		{
			result = update_help(*es[i - 1]) && result;
		}
		delete[] es;

		return result;
	}

	bool PythonHelper::listen_flush()
	{
		return listen_send_help(FUNCTIONS::LISTEN_FLUSH);
	}

	bool PythonHelper::channels_get_help()
	{
		if (!_pFunctions[FUNCTIONS::GET_CHANNEL_NAMES])
		{
			ssi_wrn("function is missing '%s:%s'", _script_name, FUNCTION_NAMES[FUNCTIONS::GET_CHANNEL_NAMES]);
			return false;
		}

		GIL gil;

		PyObject *pArgs = PyTuple_New(2);

		PyTuple_SetItem(pArgs, 0, _pOptions);
		Py_INCREF(_pOptions);

		PyTuple_SetItem(pArgs, 1, _pVariables);
		Py_INCREF(_pVariables);

		PyObject *pValue = call_function(FUNCTIONS::GET_CHANNEL_NAMES, pArgs);
		Py_DECREF(pArgs);

		if (pValue)
		{
			if (PyDict_Check(pValue))
			{
				_n_channels = (ssi_size_t)PyDict_Size(pValue);
				if (_n_channels > 0)
				{
					_channels = new PythonChannel*[_n_channels];
					_provider = new IProvider*[_n_channels];
					for (ssi_size_t i = 0; i < _n_channels; i++)
					{
						_channels[i] = 0;
						_provider[i] = 0;
					}

					PyObject *key, *value;
					Py_ssize_t pos = 0;

					ssi_size_t count = 0;
					while (PyDict_Next(pValue, &pos, &key, &value))
					{
						if (PyUnicode_Check(key) && PyUnicode_Check(value))
						{
							char *name = PyUnicode_AsUTF8(key);
							char *info = PyUnicode_AsUTF8(value);
							_channels[count] = new PythonChannel(name, info);
							if (!channels_init_help(key, _channels[count]))
							{
								ssi_wrn("could not initialize channel '%s'", name);
							}
							else
							{
								ssi_msg(SSI_LOG_LEVEL_BASIC, "found channel '%s'", name);
							}
						}
						else
						{
							ssi_wrn("'%s' should return a dictory made of strings", FUNCTION_NAMES[FUNCTIONS::GET_CHANNEL_NAMES]);
						}
						count++;
					}
				}
			}
			else
			{
				ssi_wrn("'%s' should return a dictionary", FUNCTION_NAMES[FUNCTIONS::GET_CHANNEL_NAMES]);
			}
			Py_DECREF(pValue);
		}

		return _n_channels > 0;
	}

	bool PythonHelper::channels_init_help(PyObject *pName, PythonChannel *channel)
	{
		if (!_pFunctions[FUNCTIONS::INIT_CHANNEL])
		{
			ssi_wrn("function is missing '%s:%s'", _script_name, FUNCTION_NAMES[FUNCTIONS::INIT_CHANNEL]);
			return false;
		}

		bool result = false;

		PyObject *pArgs = PyTuple_New(5);

		PyTuple_SetItem(pArgs, 0, pName);
		Py_INCREF(pName);

		ssipychannel *pChannel = ssipychannel_New(0, 0, SSI_UNDEF, 0);
		PyTuple_SetItem(pArgs, 1, (PyObject*)pChannel);
		Py_INCREF(pChannel);

		PyObject *pTypes = (PyObject *)ssipytype_New();
		PyTuple_SetItem(pArgs, 2, pTypes);

		PyTuple_SetItem(pArgs, 3, _pOptions);
		Py_INCREF(_pOptions);

		PyTuple_SetItem(pArgs, 4, _pVariables);
		Py_INCREF(_pVariables);

		PyObject *pValue = call_function(FUNCTIONS::INIT_CHANNEL, pArgs);
		Py_DECREF(pArgs);

		if (pValue)
		{
			channel->getStreamPtr()->dim = (ssi_size_t)pChannel->dim;
			channel->getStreamPtr()->byte = pChannel->byte ? (ssi_size_t)pChannel->byte : ssi_type2bytes(pChannel->type);
			channel->getStreamPtr()->type = pChannel->type;
			channel->getStreamPtr()->sr = pChannel->sr;

			Py_DECREF(pChannel);
			Py_DECREF(pValue);

			result = true;
		}

		return result;
	}

	ssi_size_t PythonHelper::getChannelSize()
	{
		if (_n_channels > 0)
		{
			return _n_channels;
		}

		return channels_get_help() ? _n_channels : 0;
	}

	IChannel *PythonHelper::getChannel(ssi_size_t index)
	{
		if (index >= _n_channels)
		{
			ssi_wrn("index '%u' exceeds #channels '%u'", index, _n_channels);
			return 0;
		}

		return _channels[index];
	}

	bool PythonHelper::setProvider(const ssi_char_t *name, IProvider *provider)
	{
		if (_n_channels == 0)
		{
			channels_get_help();
		}

		for (ssi_size_t i = 0; i < _n_channels; i++)
		{
			if (_channels[i] && ssi_strcmp(_channels[i]->getName(), name))
			{
				if (!_provider[i])
				{
					_provider[i] = provider;
					_provider[i]->init(_channels[i]);
					ssi_msg(SSI_LOG_LEVEL_BASIC, "set provider for channel '%s'", name);
					return true;
				}
				else
				{
					ssi_wrn("channel '%s' already has a provider", name);
				}
			}
		}

		ssi_wrn("unkown channel '%s'", name);

		return false;
	}

	bool PythonHelper::connect()
	{
		if (!_pFunctions[FUNCTIONS::CONNECT])
		{
			return false;
		}

		for (ssi_size_t i = 0; i < _n_channels; i++)
		{
			if (_channels[i])
			{
				_channels[i]->getStreamPtr()->time = 0;
			}
		}

		GIL gil;

		bool result = false;

		PyObject *pArgs = PyTuple_New(2);

		PyTuple_SetItem(pArgs, 0, _pOptions);
		Py_INCREF(_pOptions);

		PyTuple_SetItem(pArgs, 1, _pVariables);
		Py_INCREF(_pVariables);

		PyObject *pValue = call_function(FUNCTIONS::CONNECT, pArgs);
		Py_DECREF(pArgs);

		if (pValue)
		{
			Py_DECREF(pValue);
			result = true;
		}

		return result;
	}

	bool PythonHelper::read_help(IChannel *channel, ssi_size_t n_samples, bool reset)
	{
		if (!_pFunctions[FUNCTIONS::READ])
		{
			ssi_wrn("function is missing '%s:%s'", _script_name, FUNCTION_NAMES[FUNCTIONS::READ]);
			return false;
		}

		if (n_samples == 0)
		{
			return false;
		}

		GIL gil;

		bool result = false;

		PyObject *pArgs = PyTuple_New(6);

		PyObject *pName = PyUnicode_FromString(channel->getName());
		PyTuple_SetItem(pArgs, 0, pName);

		ssi_stream_adjust(*channel->getStreamPtr(), n_samples);
		PyObject *pStream = stream_to_object(channel->getStreamPtr());
		PyTuple_SetItem(pArgs, 1, pStream);

		PyTuple_SetItem(pArgs, 2, reset ? PyLong_FromLong(1) : PyLong_FromLong(0));

		PyTuple_SetItem(pArgs, 3, (PyObject *)_pBoard);
		Py_INCREF(_pBoard);

		PyTuple_SetItem(pArgs, 4, _pOptions);
		Py_INCREF(_pOptions);

		PyTuple_SetItem(pArgs, 5, _pVariables);
		Py_INCREF(_pVariables);

		PyObject *pValue = call_function(FUNCTIONS::READ, pArgs);
		Py_DECREF(pArgs);

		if (pValue)
		{
			Py_DECREF(pValue);
			result = true;
		}

		return result;
	}

	bool PythonHelper::read(ssi_time_t seconds, bool reset)
	{
		bool result = true;

		for (ssi_size_t i = 0; i < _n_channels; i++)
		{
			if (_provider[i] && _channels[i])
			{
				ssi_size_t n_samples = (ssi_size_t)(seconds * _channels[i]->getStream().sr + 0.5);
				if (read_help(_channels[i], n_samples, reset))
				{
					bool provide = _provider[i]->provide(_channels[i]->getStream().ptr, n_samples);
					if (provide)
					{
						_channels[i]->getStreamPtr()->time += seconds;
					}
					result = result && provide;
				}
			}
		}

		return result;
	}

	bool PythonHelper::disconnect()
	{
		if (!_pFunctions[FUNCTIONS::DISCONNECT])
		{
			return false;
		}

		GIL gil;

		bool result = false;

		PyObject *pArgs = PyTuple_New(2);

		PyTuple_SetItem(pArgs, 0, _pOptions);
		Py_INCREF(_pOptions);

		PyTuple_SetItem(pArgs, 1, _pVariables);
		Py_INCREF(_pVariables);

		PyObject *pValue = call_function(FUNCTIONS::DISCONNECT, pArgs);
		Py_DECREF(pArgs);

		if (pValue)
		{
			Py_DECREF(pValue);
			result = true;
		}

		return result;
	}

	//model

	bool PythonHelper::getModelType(IModel::TYPE::List &type)
	{
		if (!_pFunctions[FUNCTIONS::GET_MODEL_TYPE])
		{
			ssi_wrn("function is missing '%s:%s'", _script_name, FUNCTION_NAMES[FUNCTIONS::TRAIN]);
			return false;
		}

		GIL gil;

		bool result = false;

		PyObject *pArgs = PyTuple_New(3);

		PyTuple_SetItem(pArgs, 0, (PyObject *)ssipymodeltype_New());		
		PyTuple_SetItem(pArgs, 1, _pOptions);
		Py_INCREF(_pOptions);
		PyTuple_SetItem(pArgs, 2, _pVariables);
		Py_INCREF(_pVariables);

		PyObject *pValue = call_function(FUNCTIONS::GET_MODEL_TYPE, pArgs);
		Py_DECREF(pArgs);

		if (pValue)
		{
			type = (IModel::TYPE::List)PyLong_AsUnsignedLong(pValue);
			Py_DECREF(pValue);

			result = true;
		}

		return result;
	}

	bool PythonHelper::train(IModel::TYPE::List type, ISamples &samples, ssi_size_t stream_index) {

		if (!_pFunctions[FUNCTIONS::TRAIN])
		{
			ssi_wrn("function is missing '%s:%s'", _script_name, FUNCTION_NAMES[FUNCTIONS::TRAIN]);
			return false;
		}

		SampleList samples_c;
		ModelTools::CopySampleList(samples, samples_c);

		GIL gil;

		bool result = false;

		PyObject *pArgs = PyTuple_New(4);
		int valcount = 0;
		
		PyTuple_SetItem(pArgs, valcount++, samplelist_to_object(samples_c, stream_index));
		
		switch (type)
		{
		case IModel::TYPE::CLASSIFICATION:
			PyTuple_SetItem(pArgs, valcount++, labels_to_object(samples_c, stream_index));
			break;
		case IModel::TYPE::REGRESSION:
			PyTuple_SetItem(pArgs, valcount++, score_to_object(samples_c, stream_index));
			break;
		}
		
		PyTuple_SetItem(pArgs, valcount++, _pOptions);
		Py_INCREF(_pOptions);
		PyTuple_SetItem(pArgs, valcount++, _pVariables);
		Py_INCREF(_pVariables);

		PyObject *pValue = call_function(FUNCTIONS::TRAIN, pArgs);
		Py_DECREF(pArgs);

		if (pValue)
		{
			Py_DECREF(pValue);
			result = true;
		}

		return result;

	}

	bool PythonHelper::forward(ssi_stream_t &stream, ssi_size_t n_probs, ssi_real_t *probs, ssi_real_t &confidence) {
		
		if (!_pFunctions[FUNCTIONS::FORWARD])
		{
			return false;
		}

		GIL gil;

		bool result = false;

		PyObject *pArgs = PyTuple_New(4);
		int valcount = 0;
		PyTuple_SetItem(pArgs, valcount++, stream_to_object(&stream));		
		PyObject *pProbs = (PyObject *)ssipyarray_From(n_probs, probs);
		PyTuple_SetItem(pArgs, valcount++, pProbs);
		PyTuple_SetItem(pArgs, valcount++, _pOptions);
		Py_INCREF(_pOptions);
		PyTuple_SetItem(pArgs, valcount++, _pVariables);
		Py_INCREF(_pVariables);

		PyObject *pValue = call_function(FUNCTIONS::FORWARD, pArgs);
		Py_DECREF(pArgs);

		if (pValue)
		{
			confidence = (ssi_real_t)PyFloat_AsDouble(pValue);		
			Py_DECREF(pValue);

			if (confidence < 0)
			{
				ssi_wrn("negative confidence '%g'", confidence);
			}
			else
			{
				result = true;
			}
		}

		return result;
	}

	bool PythonHelper::save(const ssi_char_t *filepath) {

		if (!_pFunctions[FUNCTIONS::SAVE])
		{
			return false;
		}

		GIL gil;

		bool result = false;

		PyObject *pArgs = PyTuple_New(3);
		PyObject *pFilePath = PyUnicode_FromString(filepath);

		PyTuple_SetItem(pArgs, 0, pFilePath);
		PyTuple_SetItem(pArgs, 1, _pOptions);
		Py_INCREF(_pOptions);
		PyTuple_SetItem(pArgs, 2, _pVariables);
		Py_INCREF(_pVariables);

		PyObject *pValue = call_function(FUNCTIONS::SAVE, pArgs);
		Py_DECREF(pArgs);

		if (pValue)
		{
			Py_DECREF(pValue);
			result = true;
		}


		return result;
	}

	bool PythonHelper::load(const ssi_char_t *filepath) {

		if (!_pFunctions[FUNCTIONS::LOAD])
		{
			return false;
		}

		GIL gil;

		bool result = false;

		PyObject *pArgs = PyTuple_New(3);
		PyObject *pFilePath = PyUnicode_FromString(filepath);

		PyTuple_SetItem(pArgs, 0, pFilePath);
		PyTuple_SetItem(pArgs, 1, _pOptions);
		Py_INCREF(_pOptions);
		PyTuple_SetItem(pArgs, 2, _pVariables);
		Py_INCREF(_pVariables);

		PyObject *pValue = call_function(FUNCTIONS::LOAD, pArgs);
		Py_DECREF(pArgs);

		if (pValue)
		{
			Py_DECREF(pValue);
			result = true;
		}

		return result;
	}

}
