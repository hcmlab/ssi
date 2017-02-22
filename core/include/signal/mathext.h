// mathext.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/01/18
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

#ifndef SSI_SIGNAL_MATHEXT_H
#define SSI_SIGNAL_MATHEXT_H

#include <cmath>

SSI_INLINE char sin (char x) {
	return static_cast<char> (sin (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned char sin (unsigned char x) {
	return static_cast<unsigned char> (sin (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE short sin (short x) {
	return static_cast<short> (sin (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned short sin (unsigned short x) {
	return static_cast<unsigned short> (sin (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE int sin (int x) {
	return static_cast<int> (sin (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned int sin (unsigned int x) {
	return static_cast<unsigned int> (sin (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE long sin (long x) {
	return static_cast<long> (sin (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned long sin (unsigned long x) {
	return static_cast<unsigned long> (sin (static_cast<float> (x)) + 0.5f);
}

SSI_INLINE char cos (char x) {
	return static_cast<char> (cos (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned char cos (unsigned char x) {
	return static_cast<unsigned char> (cos (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE short cos (unsigned short x) {
	return static_cast<unsigned short> (cos (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned short cos (short x) {
	return static_cast<short> (cos (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE int cos (unsigned int x) {
	return static_cast<unsigned int> (cos (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned int cos (int x) {
	return static_cast<int> (cos (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE long cos (unsigned long x) {
	return static_cast<unsigned long> (cos (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned long cos (long x) {
	return static_cast<long> (cos (static_cast<float> (x)) + 0.5f);
}

SSI_INLINE char sqrt (char x) {
	return static_cast<char> (sqrt (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned char sqrt (unsigned char x) {
	return static_cast<unsigned char> (sqrt (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE short sqrt (short x) {
	return static_cast<short> (sqrt (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned short sqrt (unsigned short x) {
	return static_cast<short> (sqrt (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE int sqrt (int x) {
	return static_cast<int> (sqrt (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned int sqrt (unsigned int x) {
	return static_cast<unsigned int> (sqrt (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE long sqrt (long x) {
	return static_cast<long> (sqrt (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned long sqrt (unsigned long x) {
	return static_cast<unsigned long> (sqrt (static_cast<float> (x)) + 0.5f);
}

SSI_INLINE char exp (char x) {
	return static_cast<char> (exp (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned char exp (unsigned char x) {
	return static_cast<unsigned char> (exp (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE short exp (short x) {
	return static_cast<short> (exp (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned short exp (unsigned short x) {
	return static_cast<unsigned short> (exp (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE int exp (int x) {
	return static_cast<int> (exp (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned int exp (unsigned int x) {
	return static_cast<unsigned int> (exp (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE long exp (long x) {
	return static_cast<long> (exp (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned long exp (unsigned long x) {
	return static_cast<unsigned long> (exp (static_cast<float> (x)) + 0.5f);
}

SSI_INLINE char pow (char x, char n) {
	return static_cast<char> (pow (static_cast<float> (x), static_cast<float> (n)) + 0.5f);
}
SSI_INLINE unsigned char pow (unsigned char x, unsigned char n) {
	return static_cast<unsigned char> (pow (static_cast<float> (x), static_cast<float> (n)) + 0.5f);
}
SSI_INLINE short pow (short x, short n) {
	return static_cast<short> (pow (static_cast<float> (x), static_cast<float> (n)) + 0.5f);
}
SSI_INLINE unsigned short pow (unsigned short x, unsigned short n) {
	return static_cast<unsigned short> (pow (static_cast<float> (x), static_cast<float> (n)) + 0.5f);
}
SSI_INLINE int pow (int x, int n) {
	return static_cast<int> (pow (static_cast<float> (x), static_cast<float> (n)) + 0.5f);
}
SSI_INLINE unsigned int pow (unsigned int x, unsigned int n) {
	return static_cast<unsigned int> (pow (static_cast<float> (x), static_cast<float> (n)) + 0.5f);
}
SSI_INLINE long pow (long x, long n) {
	return static_cast<long> (pow (static_cast<float> (x), static_cast<float> (n)) + 0.5f);
}
SSI_INLINE unsigned long pow (unsigned long x, unsigned long n) {
	return static_cast<unsigned long> (pow (static_cast<float> (x), static_cast<float> (n)) + 0.5f);
}

SSI_INLINE char log (char x) {
	return static_cast<char> (log (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned char log (unsigned char x) {
	return static_cast<unsigned char> (log (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE short log (short x) {
	return static_cast<short> (log (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned short log (unsigned short x) {
	return static_cast<unsigned short> (log (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE int log (int x) {
	return static_cast<int> (log (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned int log (unsigned int x) {
	return static_cast<unsigned int> (log (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE long log (long x) {
	return static_cast<long> (log (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned long log (unsigned long x) {
	return static_cast<unsigned long> (log (static_cast<float> (x)) + 0.5f);
}

SSI_INLINE char log10 (char x) {
	return static_cast<char> (log10 (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned char log10 (unsigned char x) {
	return static_cast<unsigned char> (log10 (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE short log10 (short x) {
	return static_cast<short> (log10 (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned short log10 (unsigned short x) {
	return static_cast<unsigned short> (log10 (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE int log10 (int x) {
	return static_cast<int> (log10 (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned int log10 (unsigned int x) {
	return static_cast<unsigned int> (log10 (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE long log10 (long x) {
	return static_cast<long> (log10 (static_cast<float> (x)) + 0.5f);
}
SSI_INLINE unsigned long log10 (unsigned long x) {
	return static_cast<unsigned long> (log10 (static_cast<float> (x)) + 0.5f);
}

SSI_INLINE char log2 (char x) {
	static const float xxx = 1.0f/log(2.0f);
	return static_cast<char> (log (static_cast<float> (x)) * xxx + 0.5f);
}
SSI_INLINE unsigned char log2 (unsigned char x) {
	static const float xxx = 1.0f/log(2.0f);
	return static_cast<unsigned char> (log (static_cast<float> (x)) * xxx + 0.5f);
}
SSI_INLINE short log2 (short x) {
	static const float xxx = 1.0f/log(2.0f);
	return static_cast<short> (log (static_cast<float> (x)) * xxx + 0.5f);
}
SSI_INLINE unsigned short log2 (unsigned short x) {
	static const float xxx = 1.0f/log(2.0f);
	return static_cast<unsigned short> (log (static_cast<float> (x)) * xxx + 0.5f);
}
SSI_INLINE int log2 (int x) {
	static const float xxx = 1.0f/log(2.0f);
	return static_cast<int> (log (static_cast<float> (x)) * xxx + 0.5f);
}
SSI_INLINE unsigned int log2 (unsigned int x) {
	static const float xxx = 1.0f/log(2.0f);
	return static_cast<unsigned int> (log (static_cast<float> (x)) * xxx + 0.5f);
}
SSI_INLINE long log2 (long x) {
	static const float xxx = 1.0f/log(2.0f);
	return static_cast<long> (log (static_cast<float> (x)) * xxx + 0.5f);
}
SSI_INLINE unsigned long log2 (unsigned long x) {
	static const float xxx = 1.0f/log(2.0f);
	return static_cast<unsigned long> (log (static_cast<float> (x)) * xxx + 0.5f);
}
/*SSI_INLINE float log2 (float x) { //this needs to be commented out, or else vs2013 wont compile the core
	static const float xxx = 1.0f/log(2.0f);
	return log (x)*xxx;
}*/
SSI_INLINE double log2 (double x) {
	static const double xxx = 1.0/log(2.0);
	return log (x)*xxx;
}

SSI_INLINE char fabs (char x) {
	return x < 0 ? -1 * x : x;
}
SSI_INLINE unsigned char fabs (unsigned char x) {
	return x < 0 ? -1 * x : x;
}
SSI_INLINE short fabs (short x) {
	return x < 0 ? -1 * x : x;
}
SSI_INLINE unsigned short fabs (unsigned short x) {
	return x < 0 ? -1 * x : x;
}
SSI_INLINE int fabs (int x) {
	return x < 0 ? -1 * x : x;
}
SSI_INLINE unsigned int fabs (unsigned int x) {
	return x < 0 ? -1 * x : x;
}
SSI_INLINE long fabs (long x) {
	return x < 0 ? -1 * x : x;
}
SSI_INLINE unsigned long fabs (unsigned long x) {
	return x < 0 ? -1 * x : x;
}

#endif
