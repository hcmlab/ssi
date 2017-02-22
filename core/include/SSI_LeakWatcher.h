// SSI_LeakWatcher.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/08/30
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

// following a tutorial on The Code Project
// http://www.codeproject.com/KB/debug/consoleleak.aspx?msg=705744
// author: Chris Losinger

#pragma once

#ifndef SSI_LEAKWATCHER_H
#define SSI_LEAKWATCHER_H

#ifdef _CRTDBG_MAP_ALLOC
    #undef _CRTDBG_MAP_ALLOC
#endif
#include <crtdbg.h>
#ifdef _DEBUG
    #define THIS_FILE __FILE__
    inline void* operator new(size_t nSize, const char * lpszFileName, int nLine)
    {
        return ::operator new(nSize, _NORMAL_BLOCK, lpszFileName, nLine);
    }
    inline void __cdecl operator delete(void * _P, const char * lpszFileName, int nLine)
	{
		::operator delete(_P, _NORMAL_BLOCK, lpszFileName, nLine);
	} 
    #define DEBUG_NEW       new(THIS_FILE, __LINE__)
    #define malloc(s)       _malloc_dbg(s, _NORMAL_BLOCK, THIS_FILE, __LINE__)
    #define calloc(c, s)    _calloc_dbg(c, s, _NORMAL_BLOCK, THIS_FILE, __LINE__)
    #define realloc(p, s)   _realloc_dbg(p, s, _NORMAL_BLOCK, THIS_FILE, __LINE__)
    #define _expand(p, s)   _expand_dbg(p, s, _NORMAL_BLOCK, THIS_FILE, __LINE__)
    #define free(p)         _free_dbg(p, _NORMAL_BLOCK)
    #define _msize(p)       _msize_dbg(p, _NORMAL_BLOCK)
#endif

#endif
