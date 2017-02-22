// FileBinary.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/07/21
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

#ifndef SSI_IOPUT_FILEBINARY_H
#define SSI_IOPUT_FILEBINARY_H

#include "ioput/file/File.h"

namespace ssi {

class FileBinary : public File {

	friend class File;

public:

	ssi_size_t read (void *ptr, ssi_size_t size, ssi_size_t count);
	ssi_size_t write (const void *ptr, ssi_size_t size, ssi_size_t count);
	ssi_size_t readLine (ssi_size_t num, ssi_char_t *string);
	ssi_size_t writeLine (const ssi_char_t *string);
	
};

}

#endif
