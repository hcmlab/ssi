// Fisher.h
// author: Amira Elshimy
// created: 2012/03/07
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

#ifndef SSI_MODEL_FISHER_H
#define SSI_MODEL_FISHER_H

#include "base/IObject.h"
#include "base/ISamples.h"
#include "ioput/option/OptionList.h"


namespace alglib_impl {
	typedef struct ae_vector ae_vector;
	typedef struct ae_matrix ae_matrix;
}

namespace ssi {

class Fisher : public IObject {

public:

	class Options : public OptionList {

	public:

		Options ()
			: n (0) {

			addOption ("n", &n, 1, SSI_SIZE, "if > 0 keeps only the first n dimensions");			
		};

		ssi_size_t n;		
	};

public:

	static const ssi_char_t *GetCreateName () { return "Fisher"; };
	static IObject *Create (const ssi_char_t *file) { return new Fisher (file); };	
	virtual ~Fisher ();

	Fisher::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Fisher projection based on linear discriminant analysis."; };

	virtual bool build (ISamples &samples,
		ssi_size_t stream_index);	
	virtual bool isBuild () { return _is_build != 0; };
	virtual void release ();
	
	virtual bool transform (ssi_stream_t &stream, 
		ssi_stream_t &stream_t);

	virtual bool save (const ssi_char_t *filepath);
	virtual bool load (const ssi_char_t *filepath);

	virtual void print (FILE *file = stdout);


protected:

	Fisher (const ssi_char_t *file = 0);	
	Fisher::Options _options;
	ssi_char_t *_file;

	bool _is_build;
	
	alglib_impl::ae_matrix *_basis;
};

}

#endif
