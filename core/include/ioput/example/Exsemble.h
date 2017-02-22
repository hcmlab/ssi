// Exsemble.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/11/13
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

#ifndef SSI_IOPUT_EXSEMBLE_H
#define SSI_IOPUT_EXSEMBLE_H

#include "ioput/example/Example.h"

namespace ssi {

class Exsemble : public std::vector<Example> {

public:

	Exsemble(ssi_size_t width = 70);
	~Exsemble ();

	void console(int top, int left, int width, int height);
	void add(Example::example_fptr_t func, void *arg, const char *name, const char *info);
	void show(FILE *fp = ssiout);

protected:

	ssi_size_t WIDTH;

	struct BORDER {
		enum List : unsigned char {
			HORIZONTAL = 205,
			VERTICAL = 186,
			TOP_LEFT = 201,
			TOP_RIGHT = 187,
			BOTTOM_LEFT = 200,
			BOTTOM_RIGHT = 188,
			INDENT_LEFT = 5,
			INDENT_RIGHT = 1,
			BORDER_WIDTH = 1
		};
	};

	void drawEmpty(FILE *fp);
	void drawExample(FILE *fp, ssi_size_t num, const ssi_char_t *name, const ssi_char_t *info);
	void drawName(FILE *fp, ssi_size_t num, const ssi_char_t *name);
	void drawInfo(FILE *fp, const ssi_char_t *info); 
	void drawResult(FILE *fp, bool result);
	void drawChars(FILE *fp, ssi_size_t n, char c);
	void drawTop(FILE *fp);
	void drawBottom(FILE *fp);
	void runExample(FILE *fp, ssi_size_t num, Example &ex);

};

}

#endif
