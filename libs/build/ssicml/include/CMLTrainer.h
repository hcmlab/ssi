// CMLTrainer.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/10/24
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

#ifndef SSI_MODEL_CMLTRAINER_H
#define SSI_MODEL_CMLTRAINER_H

#include "base/String.h"

namespace ssi 
{

class Trainer;
class SampleList;
class MongoClient;
class Annotation;

class CMLTrainer
{

public:

	CMLTrainer();
	virtual ~CMLTrainer();

	bool init(MongoClient *client,
		const ssi_char_t *rootdir,
		const ssi_char_t *scheme,
		const ssi_char_t *stream,
		ssi_size_t leftContext,
		ssi_size_t rightContext);
	bool collect(const ssi_char_t *session,
		const ssi_char_t *role,
		const ssi_char_t *annotator,
		bool cooperative, double cmlbegintime);
	bool collect_multi(const ssi_char_t *session,
		const ssi_char_t *role,
		const ssi_char_t *annotator,
		const ssi_char_t *stream, const ssi_char_t *root_dir, MongoClient *client);
	bool train(Trainer *trainer);
	bool eval(Trainer *trainer, const ssi_char_t *evalpath, bool crossval);
	Annotation *forward(Trainer *trainer,
		const ssi_char_t *session,
		const ssi_char_t *role,
		const ssi_char_t *annotator,
		bool cooperative, double cmlbegintime);

	void release();

protected:

	static ssi_char_t *ssi_log_name;

	bool _ready;
	ssi_char_t *_rootdir;
	ssi_char_t *_stream;
	ssi_char_t *_scheme;
	SampleList *_samples;
	MongoClient *_client;	
	ssi_size_t _leftContext;
	ssi_size_t _rightContext;
	ssi_int_t _rest_class_id;
};

}

#endif
