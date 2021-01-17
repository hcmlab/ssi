/***************************************************************************
                          meta.h  -  description
                             -------------------
    begin                : Sat Apr 26 2003
    copyright            : (C) 2003 by Blake Madden
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#ifndef __COMPILE_TIME_META_H__
#define __COMPILE_TIME_META_H__

///converts unsigned char to char at compile time.
template<unsigned char N>
struct __sign_char
	{
	enum { val = (N < 128) ? N : (N - 256) };
	};
#define sign_char(x) __sign_char<x>::val

#endif //__COMPILE_TIME_META_H__
