// Example.h
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

#ifndef SSI_IOPUT_EXAMPLE_H
#define SSI_IOPUT_EXAMPLE_H

#include "SSI_Cons.h"

namespace ssi {

class Example {

	friend class Exsemble;

public:

	typedef bool (*example_fptr_t)(void *);

public:

	Example(example_fptr_t func, void *arg, const char *name, const char *info);
	Example(const Example &example);
	~Example ();

	bool run();

protected:

	example_fptr_t _func;
	char *_name;
	char *_info;
	void *_arg;
};

}

#endif
