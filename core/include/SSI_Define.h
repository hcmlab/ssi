// SSI_Define.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/02/18
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

#ifndef SSI_DEFINE_H
#define SSI_DEFINE_H

#include <stdint.h>

// inline
#define SSI_INLINE inline

// version (major.minor.maintenance)
#define SSI_VERSION "v0.9.9"

// copyright
#define SSI_COPYRIGHT "Built with Social Signal Interpretation (SSI)\n\n\
(c) 2007-16 University of Augsburg, Lab for Human Centered Multimedia\n\
Johannes Wagner, Florian Lingenfelser, Ionut Damian, Tobias Baur, Andreas Seiderer, Simon Flutura, Daniel Schork, Dominik Schiller\n\n\
website: http://openssi.net\n\
contact: support@openssi.net"

#if (defined(ANDROID))
#define __gnu_linux__ 1
#endif

#if __gnu_linux__
    #ifndef HEADLESS
    #define SSI_USE_SDL
    #endif
#endif

// data types
typedef char ssi_char_t;
typedef unsigned char ssi_uchar_t;
typedef char ssi_byte_t;
typedef float ssi_real_t;
typedef uint32_t ssi_size_t;
typedef uint64_t ssi_lsize_t;
typedef int32_t ssi_int_t;
typedef double ssi_time_t;
typedef uint64_t ssi_bitmask_t;
typedef char ssi_video_t; // dummy type for video in templates
typedef enum
{
	SSI_UNDEF = 0,
	SSI_CHAR = 1,
	SSI_UCHAR = 2,
	SSI_SHORT = 3,
	SSI_USHORT = 4,
	SSI_INT = 5,
	SSI_UINT = 6,
	SSI_LONG = 7,
	SSI_ULONG = 8,
	SSI_FLOAT = 9,
	SSI_DOUBLE = 10,
	SSI_LDOUBLE = 11,
	SSI_STRUCT = 12,
	SSI_IMAGE = 13,
	SSI_BOOL = 14
} ssi_type_t;
#define SSI_REAL SSI_FLOAT
#define SSI_SIZE SSI_UINT
#define SSI_TIME SSI_DOUBLE
#define SSI_TYPE_NAME_SIZE 15

static ssi_size_t ssi_type2bytes(ssi_type_t type) {
	switch (type) {
	case SSI_CHAR:
		return sizeof(ssi_char_t);
	case SSI_UCHAR:
		return sizeof(ssi_uchar_t);
	case SSI_SHORT:
		return sizeof(int16_t);
	case SSI_USHORT:
		return sizeof(uint16_t);
	case SSI_INT:
		return sizeof(int32_t);
	case SSI_UINT:
		return sizeof(uint32_t);
	case SSI_LONG:
		return sizeof(int64_t);
	case SSI_ULONG:
		return sizeof(uint64_t);
	case SSI_FLOAT:
		return sizeof(float);
	case SSI_DOUBLE:
		return sizeof(double);
	case SSI_LDOUBLE:
		return sizeof(double);
	case SSI_BOOL:
		return sizeof(int);
	default:
		return 0;
	}
}

typedef enum 
{
	SSI_ESTATE_COMPLETED,
	SSI_ESTATE_CONTINUED
} ssi_estate_t;
#define SSI_ESTATE_NAME_SIZE 2

typedef enum
{
	SSI_ETYPE_UNDEF = 0,
	SSI_ETYPE_EMPTY = 1,
	SSI_ETYPE_STRING = 2,
	SSI_ETYPE_MAP = 3,
	SSI_ETYPE_TUPLE = 4
} ssi_etype_t;
#define SSI_ETYPE_NAME_SIZE 5

typedef enum
{
	SSI_OBJECT = 0,
	SSI_PROVIDER = 1,
	SSI_CONSUMER = 2,
	SSI_TRANSFORMER = 3,
	SSI_FEATURE = 4,
	SSI_FILTER = 5,
	SSI_TRIGGER = 6,
	SSI_MODEL = 7,
	SSI_MODEL_CONTINUOUS = 8,
	SSI_FUSION = 9,
	SSI_SENSOR = 10,
	SSI_SELECTION = 11
} ssi_object_t;
#define SSI_OBJECT_NAME_SIZE 12

// default string length
#define SSI_MAX_CHAR	1024
#define SSI_PATH_SEPERATOR '/'

#endif
