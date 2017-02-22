// CascadingSpecialistsMS.h
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2010/4/12
// Copyright (C) 2007-12 University of Augsburg, Florian Lingenfelser
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
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

#ifndef SSI_MODEL_CASCADINGSPECIALISTSMS_H
#define SSI_MODEL_CASCADINGSPECIALISTSMS_H

#include "base/IFusion.h"
#include "model/ModelTools.h"
#include "ioput/file/FileBinary.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class CascadingSpecialistsMS : public IFusion {

public:

	ssi_size_t getModelNumber(ISamples &samples){
		return samples.getStreamSize();
	};

	static const ssi_char_t *GetCreateName () { return "CascadingSpecialistsMS"; };
	static IObject *Create (const ssi_char_t *file) { return new CascadingSpecialistsMS (file); };
	
	IOptions *getOptions () { return 0; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Fusion: Cascading Specialist MS"; };

	void setLogLevel (ssi_size_t log_level) { ssi_log_level = log_level; };

	bool train (ssi_size_t n_models,
		IModel **models,
		ISamples &samples);
	bool isTrained () { return _n_models != 0; };
	bool forward (ssi_size_t n_models,
		IModel **models,
		ssi_size_t n_streams,
		ssi_stream_t **streams,
		ssi_size_t n_probs,
		ssi_real_t *probs);
	void release ();
	bool save (const ssi_char_t *filepath);
	bool load (const ssi_char_t *filepath);
	bool supportsMissingData() { return true; };

protected:

	int ssi_log_level;
	static ssi_char_t *ssi_log_name;

	CascadingSpecialistsMS (const ssi_char_t *file = 0);
	virtual ~CascadingSpecialistsMS ();
	
	ssi_char_t *_file;

	static void sort (ssi_size_t nSize, ssi_real_t *anArray, ssi_size_t *order);
	static void real_swap (ssi_real_t &x, ssi_real_t &y);
	static void size_swap (ssi_size_t &x, ssi_size_t &y);

	ssi_size_t _n_classes;
	ssi_size_t _n_streams;
	ssi_size_t _n_models;
	ssi_size_t _n_winners;
	ssi_real_t **_weights;
	ssi_size_t *_order;
	ssi_size_t **_winner;
	ssi_size_t *_filler;
};

}

#endif
