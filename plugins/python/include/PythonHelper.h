// PythonHelper.h
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

#pragma once

#ifndef SSI_PYTHON_HELPER_H
#define SSI_PYTHON_HELPER_H

#include "base/ISensor.h"
#include "base/IConsumer.h"
#include "base/ITransformer.h"
#include "base/IEvents.h"
#include "base/IOptions.h"
#include "base/ISamples.h"
#include "base/IModel.h"

typedef struct _object PyObject;
typedef struct _ts PyThreadState;
typedef struct _ssipyinfo ssipyinfo;
typedef struct _ssipyevent ssipyevent;
typedef struct _ssipyeventboard ssipyeventboard;

namespace ssi {

class PythonChannel;

class PythonHelper
{

public:

struct FUNCTIONS
{
	enum List
	{
		// transformer functions
		GET_OPTIONS = 0,
		GET_SAMPLE_NUMBER,
		GET_SAMPLE_DIMENSION,
		GET_SAMPLE_BYTES,
		GET_SAMPLE_TYPE,
		GET_IMAGE_FORMAT,
		SET_IMAGE_FORMAT,
		TRANSFORM_ENTER,
		TRANSFORM,
		TRANSFORM_FLUSH,
		CONSUME_ENTER,
		CONSUME,
		CONSUME_FLUSH,
		GET_EVENT_ADDRESS,
		SEND_ENTER,		
		SEND_FLUSH,
		LISTEN_ENTER,
		UPDATE,
		LISTEN_FLUSH,
		GET_CHANNEL_NAMES,		
		INIT_CHANNEL,
		CONNECT,
		READ,
		DISCONNECT,
		GET_MODEL_TYPE,
		TRAIN,
		FORWARD,
		SAVE,
		LOAD,
		NUM
	};
};
typedef FUNCTIONS::List function_t;
static ssi_char_t *FUNCTION_NAMES[FUNCTIONS::NUM];

public:

	PythonHelper(ssi_char_t *script_name, 
		const ssi_char_t *optsfile, 
		const ssi_char_t *optsstr, 
		const ssi_char_t *syspath,
		const ssi_char_t *workdir = 0);
	~PythonHelper();

	ssi_size_t getSampleNumberOut(ssi_size_t sample_number_in);
	ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in);
	ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in);
	ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in);
	ssi_video_params_t getImageFormatOut(ssi_video_params_t params_in);
	void setImageFormatIn(ssi_video_params_t params_in);
	bool transform_enter(ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num, ssi_stream_t xtra_stream_in[]);
	bool transform(ITransformer::info *info, ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num, ssi_stream_t xtra_stream_in[]);	
	bool transform_flush(ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num, ssi_stream_t xtra_stream_in[]);

	bool consume_enter(ssi_size_t stream_in_num, ssi_stream_t stream_in[]);
	bool consume(IConsumer::info *info, ssi_size_t stream_in_num, ssi_stream_t stream_in[]);
	bool consume_flush(ssi_size_t stream_in_num, ssi_stream_t stream_in[]);

	bool setEventListener(IEventListener *listener);
	const ssi_char_t *getEventAddress();
	bool send_enter();	
	bool send_flush();

	bool listen_enter();
	bool update(ssi_event_t &e);
	bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
	bool listen_flush();

	ssi_size_t getChannelSize();
	IChannel *getChannel(ssi_size_t index);
	bool setProvider(const ssi_char_t *name, IProvider *provider);
	bool connect();
	bool disconnect();
	bool read(ssi_time_t seconds, bool reset);

	bool getModelType(IModel::TYPE::List &type);
	bool train(IModel::TYPE::List type, ISamples &samples, ssi_size_t stream_index);
	bool forward(ssi_stream_t &stream, ssi_size_t n_probs, ssi_real_t *probs, ssi_real_t &confidence);
	bool save(const ssi_char_t *filepath);
	bool load(const ssi_char_t *filepath);

protected:
	
	struct GIL
	{
		GIL();
		~GIL();
		int _state;
	};

	static ssi_char_t *ssi_log_name;

	int register_function(const ssi_char_t *name, PyObject **function);
	PyObject *call_function(function_t function, PyObject *args);

	void add_sys_path(const ssi_char_t *path, const ssi_char_t *workdir);

	void options_get(function_t function, const ssi_char_t *optsfile, const ssi_char_t *optsstr);
	void options_parse(const ssi_char_t *string, char delim);
	bool options_apply(const ssi_char_t *key, const ssi_char_t *value);
	PyObject *stream_to_object(ssi_stream_t *stream);
	PyObject *stream_to_imageobject(ssi_stream_t *stream, ssi_video_params_t params);
	PyObject *samplelist_to_object(ISamples &samples, ssi_size_t stream_index);
	PyObject *labels_to_object(ISamples &samples, ssi_size_t stream_index);
	PyObject *score_to_object(ISamples &samples, ssi_size_t stream_index);


	bool transform_get_help(function_t function, ssi_size_t in, ssi_size_t &out);
	bool transform_help(function_t function, ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num, ssi_stream_t xtra_stream_in[], ITransformer::info *info = 0);
	PyObject *transform_args(ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num, ssi_stream_t xtra_stream_in[], ITransformer::info *info = 0);

	bool consume_help(function_t function, ssi_size_t stream_in_num, ssi_stream_t stream_in[], IConsumer::info *info = 0);
	PyObject *consume_args(ssi_size_t stream_in_num, ssi_stream_t stream_in[], IConsumer::info *info = 0);

	bool listen_send_help(function_t function);
	PyObject *listen_send_args();

	bool update_help(ssi_event_t &e);
	PyObject *update_args(ssipyevent *event);

	bool channels_get_help();
	bool channels_init_help(PyObject *pName, PythonChannel *channel);
	bool read_help(IChannel *channel, ssi_size_t n_samples, bool reset);

	ssi_char_t *_script_name;
	PyObject *_pFunctions[FUNCTIONS::NUM];
	PyObject *_pModule;
	PyObject *_pOptions;
	PyObject *_pVariables;

	ssipyeventboard *_pBoard;
	IEventListener *_listener;
	ssi_char_t *_address;
	static ssi_event_t *ConvertEvent(ssipyevent *event);
	static ssipyevent *ConvertEvent(ssi_event_t *event);
	static void Update(void *self, ssipyevent *event);

	ssi_size_t _n_channels;	
	PythonChannel **_channels;
	IProvider **_provider;

	bool _has_image_params;
	ssi_video_params_t _image_params_in;
	ssi_video_params_t _image_params_out;
};

}

#endif
