// ssicurl.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/01/12
// Copyright (C) 2016 University of Augsburg
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

#ifndef SSI_CURL_H
#define SSI_CURL_H

#include "curl/curl-7.21.6/include/curl/curl.h"

#ifdef _DEBUG
#	pragma comment(lib, "libcurld.lib")
#else
#	pragma comment(lib, "libcurl.lib")
#endif

#endif
