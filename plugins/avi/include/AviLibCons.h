// AviLibCons.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/05/19
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

#ifndef SSI_AVI_AVILIBCONS_H
#define	SSI_AVI_AVILIBCONS_H

#include "SSI_Cons.h"
#include <sstream>
#include <vfw.h>

// link libraries
#ifdef _MSC_VER 
#pragma comment (lib, "strmiids.lib")
#pragma comment (lib, "quartz.lib")
#pragma comment (lib, "ole32.lib")
#pragma comment (lib, "comsuppw.lib")
#pragma comment (lib, "vfw32.lib")
#pragma comment(lib, "winmm.lib")
#ifdef _DEBUG
#pragma comment (lib, "msvcrtd.lib")
#else
#pragma comment (lib, "msvcrt.lib")
#endif
#endif

namespace ssi {
}

#endif

									
