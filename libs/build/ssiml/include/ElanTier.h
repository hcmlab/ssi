// ElanTier.h
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

#ifndef SSI_IOPUT_ELANTIER_H
#define SSI_IOPUT_ELANTIER_H

#include "SSI_Cons.h"
#include "base/String.h"
#include "ElanAnnotation.h"

namespace ssi {

class ElanTier : public std::vector<ElanAnnotation> {                                                                

public:

	ElanTier (String name);
	virtual ~ElanTier ();

	const ssi_char_t *name () {
		return _name.str ();
	}

	ssi_size_t pack (ElanTier &tier, const char *delim = " "); // concatenates contiguous annotation items, requires sorted list; returns added items
	void unify (const char *string); // set all annotation item to string
	void sort (); // sorts annotation items by start time
	ssi_size_t invert (ElanTier &tier, String value = "", ssi_size_t tiersize = 0); // inverts annotation items; returns inverted items
	ssi_size_t split (ElanTier &tier, ssi_size_t framesize, ssi_size_t deltasize, ssi_size_t dropsize); // splits annotation items in frames of length 'framesize', drops frames <= dropsize; returns added items
	ssi_size_t split (ElanTier &tier, ElanTier &inverse, String inverseValue, ssi_size_t framesize, ssi_size_t deltasize, ssi_size_t dropsize, ssi_size_t tiersize = 0); // like split, but also returns inverted items
	 
	void print (FILE *file = stdout);

protected:

	static ssi_char_t *ssi_log_name;

	static bool SortAnnotationFunction (ElanAnnotation a, ElanAnnotation b) { 
		return (a.from == b.from ? a.to < b.to : a.from < b.from); 
	}

	String _name;	
};

}

#endif
