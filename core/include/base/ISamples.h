// ISamples.h
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

#ifndef SSI_ISAMPLES_H
#define SSI_ISAMPLES_H

#include "SSI_Cons.h"

#define SSI_ISAMPLES_REST_CLASS_NAME "REST"
#define SSI_ISAMPLES_GARBAGE_CLASS_NAME "GARBAGE"
#define SSI_ISAMPLES_GARBAGE_CLASS_ID -1
#define SSI_ISAMPLES_GARBAGE_USER_NAME "NOBODY"
#define SSI_ISAMPLES_GARBAGE_USER_ID -1

namespace ssi {

class ISamples {

public:

	virtual ~ISamples () {};	

	virtual void reset () = 0;
	virtual ssi_sample_t *get (ssi_size_t index) = 0;
	virtual ssi_sample_t *next () = 0; 	

	virtual ssi_size_t getSize () = 0;
	virtual ssi_size_t getSize (ssi_size_t class_index) = 0;

	virtual ssi_size_t getClassSize () = 0;
	virtual const ssi_char_t *getClassName (ssi_size_t class_index) = 0;
	
	virtual ssi_size_t getUserSize () = 0;
	virtual const ssi_char_t *getUserName (ssi_size_t user_index) = 0;

	virtual ssi_size_t getStreamSize () = 0;
	virtual ssi_stream_t getStream (ssi_size_t stream_index) = 0;

	virtual bool hasMissingData () = 0;
	virtual bool supportsShallowCopy () = 0;

};

}

#endif
