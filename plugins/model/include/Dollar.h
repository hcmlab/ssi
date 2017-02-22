// Dollar$1.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/10/28
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

// *************************************************************************************************
//
// Wobbrock, J.O., Wilson, A.D. and Li, Y. (2007) Gestures without libraries, toolkits
// or training: A $1 recognizer for user interface prototypes.
// <http://faculty.washington.edu/wobbrock/pubs/uist-07.1.pdf> Proceedings
// of the ACM Symposium on User Interface Software and Technology (UIST '07). Newport,
// Rhode Island (October 7-10, 2007). New York: ACM Press, pp. 159-168.
//
// *************************************************************************************************

#pragma once

#ifndef SSI_MODEL_DOLLAR$1_H
#define SSI_MODEL_DOLLAR$1_H

#include "base/IModel.h"
#include "model/ModelTools.h"
#include "ioput/file/FileBinary.h"
#include "ioput/option/OptionList.h"
#include "GeometricRecognizer.h"

namespace ssi {

class Dollar$1 : public IModel {

public:

	class Options : public OptionList {

	public:

		Options () 
			: indx (0), indy (1), norm (false) {

			addOption ("indx", &indx, 1, SSI_INT, "dimension of x coordinate (0)");			
			addOption ("indy", &indy, 1, SSI_INT, "dimension of y coordinate (1)");			
			addOption ("norm", &norm, 1, SSI_BOOL, "recognition probabilities to sum up to 1");		
		};

		ssi_size_t indx;
		ssi_size_t indy;
		bool norm;
	};

public:

	static const ssi_char_t *GetCreateName () { return "Dollar$1"; };
	static IObject *Create (const ssi_char_t *file) { return new Dollar$1 (file); };	

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Dollar$1 classifier."; };

	bool train (ISamples &samples,
		ssi_size_t stream_index);	
	bool isTrained () { return _dollar1.templates.size () > 0; };
	bool forward (ssi_stream_t &stream,
		ssi_size_t n_probs,
		ssi_real_t *probs);	
	void release ();
	bool save (const ssi_char_t *filepath);
	bool load (const ssi_char_t *filepath);

	ssi_size_t getClassSize () { return _n_classes; };
	ssi_size_t getStreamDim () { return _n_features; };
	ssi_size_t getStreamByte () { return sizeof (ssi_real_t); };
	ssi_type_t getStreamType () { return SSI_REAL; };


protected:

	Dollar$1 (const ssi_char_t *file = 0);
	virtual ~Dollar$1 ();
	Dollar$1::Options _options;
	ssi_char_t *_file;
	
	void addToPath (ssi_real_t *ptr, ssi_size_t num, ssi_size_t dim, ssi_size_t ind_x, ssi_size_t ind_y, Path2D &path);
	GeometricRecognizer _dollar1;

	ssi_size_t _n_classes;
	ssi_size_t _n_features;
};

}

#endif
