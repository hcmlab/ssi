// VectorFusionTools.h
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2012/04/25
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

#ifndef SSI_VECTORFUSIONTOOLS_H
#define SSI_VECTORFUSIONTOOLS_H

#include "base/IObject.h"
#include "base/ISamples.h"
#include "SSI_Cons.h"

namespace alglib {
	class linearmodel;
}

namespace ssi {

class VectorFusionTools : public IObject {

public:

	static const ssi_char_t *GetCreateName () { return "VectorFusionTools"; };
	static IObject *Create (const ssi_char_t *file) { return new VectorFusionTools (file); };
	~VectorFusionTools ();

	IOptions *getOptions () { return 0; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "contains help functions for vector fusion."; };

	virtual void save_lm(alglib::linearmodel *lm, const ssi_char_t *filepath);
	virtual void load_lm(alglib::linearmodel *lm, const ssi_char_t *filepath);
	virtual ssi_stream_t* anno2stream(const ssi_char_t* dir, ssi_size_t line_length, ssi_size_t timestamp_index, ssi_size_t label_index);
	virtual alglib::linearmodel* samples2regfunc(ISamples *samples, ssi_size_t data_index, ssi_size_t anno_index);

protected:

	VectorFusionTools (const ssi_char_t *file = 0);
	ssi_char_t *_file;
	static ssi_char_t *ssi_log_name;
	int ssi_log_level;
		
};

}

#endif
