// IModel.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/03/01
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_IMODEL_H
#define SSI_IMODEL_H

#include "base/IObject.h"
#include "base/IOptions.h"
#include "base/ISamples.h"

namespace ssi {

class IModel : public IObject {

public:

	struct TASK
	{
		enum List
		{
			CLASSIFICATION = 0,
			REGRESSION = 1
		};
	};

public:

	virtual bool train (ISamples &samples,
		ssi_size_t stream_index) = 0;
	virtual bool isTrained () = 0;
	virtual bool forward (ssi_stream_t &stream,
		ssi_size_t n_probs,
		ssi_real_t *probs) = 0;	
	virtual void release () = 0;

	virtual bool save (const ssi_char_t *filepath) = 0;	
	virtual bool load (const ssi_char_t *filepath) = 0;	

	virtual ssi_size_t getClassSize () = 0;
	virtual ssi_size_t getStreamDim () = 0;
	virtual ssi_size_t getStreamByte () = 0;
	virtual ssi_type_t getStreamType () = 0;

	virtual ssi_size_t getMetaSize () { return 0; };
	virtual bool getMetaData (ssi_size_t n_metas,
		ssi_real_t *metas) { return false; };	

	ssi_object_t getType () { return SSI_MODEL; };

};

}

#endif
