// ElanDocument.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2013/04/08
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

#ifndef SSI_IOPUT_ELANDOCUMENT_H
#define SSI_IOPUT_ELANDOCUMENT_H

#include "SSI_Cons.h"
#include "base/String.h"
#include "ElanTier.h"

namespace ssi {

struct ElanMedia {
	String url;
	String type;
	String url_rel;
};

struct ElanMeta {
	String author;
	String date;
};

class ElanDocument : public std::vector<ElanTier> {                                                                

public:

	ElanDocument ();
	virtual ~ElanDocument ();

	static ElanDocument *Read (const ssi_char_t *filepath);

	ElanTier &operator[] (const ssi_char_t *name);
	std::vector<ElanMedia> &media ();
	ElanMeta &meta ();
	bool write (const ssi_char_t *filepath);

	void print (FILE *file = stdout);

protected:

	std::vector<ElanMedia> _media;
	ElanMeta _meta;

	static ssi_char_t *ssi_log_name;
	ssi_char_t *_filepath;
};

}

#endif
