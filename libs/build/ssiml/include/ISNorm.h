// ISNorm.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/03/08
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

#ifndef SSI_MODEL_ISNORM_H
#define SSI_MODEL_ISNORM_H

#include "base/ISamples.h"
#include "ioput/file/File.h"

namespace ssi {

class ISNorm : public ISamples {

public:
	
	struct METHOD {
		enum List {
			NONE = 0,
			SCALE,
			ZSCORE,
			NUM
		};
	};
	static const ssi_char_t *METHOD_NAMES[];

	struct Params {
		METHOD::List method;
		bool inialized;
		ssi_size_t n_features;
		// scale
		ssi_real_t *mins;
		ssi_real_t *maxs;
		ssi_real_t limits[2];
		// zscore
		ssi_real_t *mean;
		ssi_real_t *stdv;
	};

public:

	ISNorm (ISamples *samples);
	~ISNorm ();	

	static void ZeroParams(Params &params, METHOD::List method = METHOD::NONE);
	static void InitParams(Params &params, ssi_size_t n_feature);
	static void ReleaseParams(Params &params);
	static bool SaveParams(const ssi_char_t *path, Params &params, File::TYPE type = File::BINARY);
	static bool LoadParams(const ssi_char_t *path, Params &params);
	static bool Norm(ssi_stream_t &stream, Params params);

	bool setNorm(ssi_size_t index, Params &params);

	void norm(ssi_sample_t &sample);
	void norm(ISamples &samples);
	void norm(ssi_stream_t &stream, ssi_size_t stream_index);

	void reset () { _samples.reset (); };
	ssi_sample_t *get (ssi_size_t index);
	ssi_sample_t *next (); 

	ssi_size_t getSize () { return _samples.getSize (); };
	ssi_size_t getSize (ssi_size_t class_id) { return _samples.getSize (class_id); };

	ssi_size_t getClassSize () { return _samples.getClassSize (); };
	const ssi_char_t *getClassName (ssi_size_t class_id) { return _samples.getClassName (class_id); };
	
	ssi_size_t getUserSize () { return _samples.getUserSize (); };
	const ssi_char_t *getUserName (ssi_size_t user_id) { return _samples.getUserName (user_id); };

	ssi_size_t getStreamSize () { return _samples.getStreamSize (); };
	ssi_stream_t getStream (ssi_size_t index) { return _samples.getStream (index); };

	bool hasMissingData () { return _samples.hasMissingData (); };
	bool supportsShallowCopy () { return false; };

protected:

	ISamples &_samples;
	ssi_sample_t _sample;
	
	ssi_size_t _n_samples;
	ssi_size_t _n_streams;
	ssi_size_t *_n_features;
	Params **_params;
};

}

#endif
