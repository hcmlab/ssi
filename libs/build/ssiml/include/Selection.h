// Selection.h
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

#ifndef SSI_MODEL_SELECTION_H
#define SSI_MODEL_SELECTION_H

#include "base/ISelection.h"
#include "ioput/file/File.h"

namespace ssi {

class Selection {

public:

	Selection ();
	Selection (Selection *pre_select);
	virtual ~Selection ();

	void set (ssi_size_t n_scores,
		const ISelection::score *scores,
		bool sort);

	ssi_size_t getSize () { return _n_selected; };
	const ssi_size_t *getSelected () { return _selected; };

	ssi_size_t selByScore (ssi_real_t score);
	ssi_size_t selNFirst (ssi_size_t n); // 0 selects all
	ssi_size_t selNBest ();

	bool load (const ssi_char_t *filename, File::TYPE type);
	bool save (const ssi_char_t *filename, File::TYPE type);

	void print (FILE *file = stdout);

protected:

	void release ();
	static int Compare (const void * a, const void * b);

	ssi_size_t _n_selected;
	ssi_size_t *_selected;

	ssi_size_t _n_scores;
	ISelection::score *_scores;

	Selection *_pre_select;

};

}

#endif
