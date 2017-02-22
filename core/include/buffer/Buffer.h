// Buffer.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/11/23 
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

#ifndef SSI_BUFFER_BUFFER_H
#define SSI_BUFFER_BUFFER_H

#include "SSI_Cons.h"

namespace ssi {

class Buffer {

public:

	Buffer (ssi_size_t size);
	virtual ~Buffer ();
	bool get (ssi_byte_t *data, ssi_lsize_t position, ssi_size_t bytes);
	bool put (const ssi_byte_t *data, ssi_lsize_t position, ssi_size_t bytes);
	bool putZeros (ssi_lsize_t position, ssi_size_t bytes);

	const void *getMetaData (ssi_size_t &size) { size = _meta_size; return _meta; };
	void setMetaData (ssi_size_t size, const void *meta);

	// the size of the buffer as readonly
	const ssi_size_t size;

private:

	// the buffer
	ssi_byte_t *buffer;
	
	ssi_size_t _meta_size;
	ssi_byte_t *_meta;
};

}

#endif
