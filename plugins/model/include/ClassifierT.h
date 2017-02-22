// ClassifierT.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/03/04
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

#ifndef SSI_SIGNAL_CLASSIFIERT_H
#define SSI_SIGNAL_CLASSIFIERT_H

#include "base/IFeature.h"
#include "signal/SignalCons.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class Trainer;

class ClassifierT : public IFeature {

public:

	class Options : public OptionList {

	public:

		Options () {

			trainer[0] = '\0';
			
			addOption ("trainer", trainer, SSI_MAX_CHAR, SSI_CHAR, "filepath of trainer");			
		};

		void setTrainer (const ssi_char_t *filepath) {
			ssi_strcpy (trainer, filepath);
		}
		
		ssi_char_t trainer[SSI_MAX_CHAR];	
	};

public:

	static const ssi_char_t *GetCreateName () { return "ClassifierT"; };
	static IObject *Create (const ssi_char_t *file) { return new ClassifierT (file); };
	~ClassifierT ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Applies classifier to a stream and continuously outputs result to a new stream."; };

	virtual void setTrainer (Trainer *trainer);

	void transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform_flush (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
		
	void setLogLevel (int level) {
		ssi_log_level = level;
	}

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in);
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in);
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in);

protected:

	ClassifierT (const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;

	int ssi_log_level;
	static ssi_char_t ssi_log_name[];

	void loadTrainer ();

	Trainer *_trainer;
	ssi_size_t _n_classes;
	ssi_size_t _merged_sample_dimension;
};

}

#endif
