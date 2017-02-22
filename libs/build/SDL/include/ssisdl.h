// ssisdl.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/02/15
// Copyright (C) 2016 University of Augsburg, Tobias Baur
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
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

#ifndef SSI_SDL_H
#define SSI_SDL_H

#ifndef SSI_SDL_VERSION
#   define SSI_SDL_VERSION "2"
#endif
#if __gnu_linux__
#include <SDL_ttf.h>
#else

#include "SDL_ttf.h"
#ifdef _DEBUG
#	pragma comment(lib, "SDL" SSI_SDL_VERSION "d.lib")
#	pragma comment(lib, "SDL" SSI_SDL_VERSION "_ttfd.lib")
#else
#	pragma comment(lib, "SDL" SSI_SDL_VERSION ".lib")
#	pragma comment(lib, "SDL" SSI_SDL_VERSION "_ttf.lib")
#endif
#endif
#endif
