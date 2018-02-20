// MlpXmlTrain.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/04/27
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
#ifndef SSI_MLPXMLTRAIN_H
#define SSI_MLPXMLTRAIN_H

#include "base/IObject.h"
#include "ssiml/include/Trainer.h"
#include "ioput/option/OptionList.h"
#include "MlpXmlDef.h"
#include "ssiml/include/Annotation.h"


namespace ssi {

class TiXmlElement;

class MlpXmlTrain {

public:

	enum VERSION {
		V1 = 1		// original format
	};

	MlpXmlTrain (const ssi_char_t *def_file);
	virtual ~MlpXmlTrain ();

	ssi_char_t *const*getDefNames (ssi_size_t &n_def_names);
	bool init (const ssi_char_t *defname,
		const ssi_char_t *signal_name,
		MlpXmlDef::signal_t signal_type,
		const ssi_char_t *anno_name);
	void release ();

	bool collect (const ssi_char_t *dir,
		const ssi_char_t *userfilt = "*", 
		const ssi_char_t *datefilt = "*",
		bool re_extract = false,
		int mode = -1);
	bool collect (const ssi_char_t *dir,	
		bool re_extract,
		int mode = -1);
	bool extract (const ssi_char_t *dir,
		bool re_extract,
		int mode = -1);

	bool train (const ssi_char_t *filepath);
	bool eval (FILE *file,
		int mode,
		ssi_size_t k_folds, 
		ssi_size_t reps, 
		ssi_real_t fps);
	bool eval (const ssi_char_t *filepath,
		int mode,
		ssi_size_t k_folds, 
		ssi_size_t reps, 
		ssi_real_t fps);
	bool test (FILE *file,
		ssi_stream_t &stream);	

	bool save (const ssi_char_t *filepath,
		MlpXmlTrain::VERSION version = V1);
	bool load (const ssi_char_t *filepath, bool re_extract = false, int mode = -1);

protected:

	ssi_char_t *_file;
	int ssi_log_level;
	
	static ssi_char_t *ssi_log_name;

	void loadDates (StringList &dates,
		const ssi_char_t *dir,
		const ssi_char_t *userfilt,
		const ssi_char_t *datefilt);
	void date2user (const ssi_char_t *date, 
		ssi_char_t *user);
	bool loadSignal (const ssi_char_t *dir,
		const ssi_char_t *signal,
		ssi_stream_t &stream);

	Trainer *parseTrainer (const ssi_char_t *filepath, const ssi_char_t *defname);	
	Trainer *parseTrainerItem (TiXmlElement *element);
	IObject *parseObject (TiXmlElement *element, bool auto_free);
	ssi_char_t **parseDefNames (const ssi_char_t *filepath, ssi_size_t &_n_def_names);

	ssi_char_t *_def_file;
	ssi_size_t _n_def_names;
	ssi_char_t **_def_names;

	ssi_char_t *_def_name;
	ssi_char_t *_signal_name;
	MlpXmlDef::signal_t _signal_type;
	ssi_char_t *_anno_name;

	ssi_stream_t *_stream_ref;
	Trainer *_trainer;

	SampleList _samples;
	StringList _paths;	

	ITransformer *_transf;
		
	std::vector<ssi_stream_t *> _streams;
        std::vector<old::Annotation *> _annos;
};

}

#endif
