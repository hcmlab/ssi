// SSI_Cons.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/04/08
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

#ifndef SSI_CONS_H
#define SSI_CONS_H

#include "SSI_Define.h"

// some useful includes
#include <cstdlib>

#ifdef _MSC_VER
#include <crtdbg.h>
#include <Guiddef.h>
#else
#define _ASSERT(expr) ((void)0)
#define _ASSERTE(expr) ((void)0)
#endif
#ifdef __MINGW32__
#define _WIN32_WINNT 0x0500
#endif

#include <cstdio>
#include <cstring>
#include <ctime>
#include <cmath>
#include <cfloat>
#include <vector>
#include <algorithm>
#include <set>
#include <map>
#ifndef WIN32_LEAN_AND_MEAN // prevents windows.h to include "Winsock.h"
#define WIN32_LEAN_AND_MEAN
#endif

#if _WIN32|_WIN64
#include <windows.h>
#include <mmsystem.h>
#include <process.h>
#else
 typedef struct _GUID
 {
 #ifdef _MSC_VER
  unsigned long Data1;
 #else
  unsigned int Data1;
 #endif
  unsigned short Data2;
  unsigned short Data3;
  unsigned char Data4[ 8 ];
 } GUID;
#endif

#include <errno.h>

// leak detection
#ifdef _DEBUG
#if _MSC_VER == 1900
#else
#define USE_SSI_LEAK_DETECTOR
#endif
#endif

// data types and file endings
extern ssi_char_t *SSI_TYPE_NAMES[];
extern ssi_char_t *SSI_ESTATE_NAMES[];
extern ssi_char_t *SSI_ETYPE_NAMES[];
extern ssi_char_t *SSI_OBJECT_NAMES[];
extern ssi_char_t *SSI_SCHEME_NAMES[];
extern ssi_char_t *SSI_FILE_TYPE_STREAM;
extern ssi_char_t *SSI_FILE_TYPE_FEATURE;
extern ssi_char_t *SSI_FILE_TYPE_WAV;
extern ssi_char_t *SSI_FILE_TYPE_AVI;
extern ssi_char_t *SSI_FILE_TYPE_ANNOTATION;
extern ssi_char_t *SSI_FILE_TYPE_OPTION;
extern ssi_char_t *SSI_FILE_TYPE_TRAINDEF;
extern ssi_char_t *SSI_FILE_TYPE_TRAINING;
extern ssi_char_t *SSI_FILE_TYPE_TRAINER;
extern ssi_char_t *SSI_FILE_TYPE_MODEL;
extern ssi_char_t *SSI_FILE_TYPE_FUSION;
extern ssi_char_t *SSI_FILE_TYPE_SAMPLES;
extern ssi_char_t *SSI_FILE_TYPE_PIPELINE;
extern ssi_char_t *SSI_FILE_TYPE_PCONFIG;
extern ssi_char_t *SSI_FILE_TYPE_CHAIN;
extern ssi_char_t *SSI_FILE_TYPE_CORPUS;
extern ssi_char_t *SSI_FILE_TYPE_EVENTS;

// factory

#define SSI_FACTORY_STRINGS_MAX 1024
#define SSI_FACTORY_STRINGS_INVALID_ID -1
#define SSI_FACTORY_UNIQUE_INVALID_ID 0

// types

struct ssi_stream_t {
	ssi_size_t num; // number of used samples
	ssi_size_t num_real; // maximal number of samples im stream array
	ssi_size_t dim; // stream dimension
	ssi_size_t byte; // size in bytes of a single sample value
	ssi_size_t tot; // num * dim * byte
	ssi_size_t tot_real; // num_real * dim * byte total number of bytes in stream array
 	ssi_byte_t *ptr; // pointer to the data
	ssi_time_t sr; // sample rate in Hz
	ssi_time_t time; // time stamp in seconds
	ssi_type_t type; // data type
};

#define SSI_SAMPLE_REST_CLASS_NAME "REST"
#define SSI_SAMPLE_GARBAGE_CLASS_NAME "GARBAGE"
#define SSI_SAMPLE_GARBAGE_CLASS_ID -1
#define SSI_SAMPLE_GARBAGE_USER_NAME "NOBODY"
#define SSI_SAMPLE_GARBAGE_USER_ID -1
#define SSI_SAMPLE_INVALID_SCORE NAN

struct ssi_sample_t {
	ssi_size_t num; // number of streams
	ssi_stream_t **streams; // streams
	ssi_size_t user_id; // id of user name
	ssi_size_t class_id; // id of class (for multi-class classification)	
	ssi_real_t score; // score value (for logistic regression)
	ssi_time_t time; // time in seconds
};

struct SSI_SCHEME_TYPE
{
	enum List
	{
		DISCRETE,
		CONTINUOUS,
		FREE,
		NUM,
	};
};

struct ssi_label_t
{
	ssi_real_t confidence;
	union
	{
		struct
		{
			ssi_real_t score;
		} continuous;
		struct
		{
			ssi_time_t from;
			ssi_time_t to;
			ssi_int_t id;
		} discrete;
		struct
		{
			ssi_time_t from;
			ssi_time_t to;
			ssi_char_t *name;
		} free;
	};
};

struct ssi_scheme_t
{
	SSI_SCHEME_TYPE::List type;
	ssi_char_t *name;
	union
	{
		struct
		{
			ssi_time_t sr;
			ssi_real_t min;
			ssi_real_t max;
		} continuous;
		struct
		{
			ssi_size_t n;
			ssi_char_t **names;
			ssi_size_t *ids;
		} discrete;
		struct
		{
		} free;
	};
};

struct ssi_event_t {
	ssi_size_t glue_id; // a temporal unique id shared by sub-events belonging to the same event (set 0 if no sub-events will be generated)
	ssi_size_t sender_id; // unique sender id
	ssi_size_t event_id; // unique event id
	ssi_size_t time; // start time in ms
	ssi_size_t dur; // duration in ms
	ssi_real_t prob; // probability [0..1] to express confidence
	ssi_etype_t type; // event data type
	ssi_size_t tot; // size in bytes
	ssi_size_t tot_real; // total available size in bytes
	ssi_byte_t *ptr; // pointer to event data
	ssi_estate_t state; // events status
};

struct ssi_event_map_t {
	ssi_size_t id; // string id
	ssi_real_t value; // value
};

typedef ssi_real_t ssi_event_tuple_t;

struct ssi_matrix_t {
	ssi_size_t rows; // number of rows
	ssi_size_t cols; // number of colums
	ssi_size_t elems; // number of elements
	ssi_size_t byte; // byte per element
	ssi_size_t tot; // total number of bytes
	ssi_byte_t *ptr; // pointer to data
};

struct ssi_option_t {
	ssi_char_t *name; // unique name
	ssi_char_t *help; // information
	void *ptr; // pointer to element data
	ssi_type_t type; // element type
	ssi_size_t num; // number of elements
	bool lock; // lock at run-time
};

struct ssi_video_params_t {
	int widthInPixels;
	int heightInPixels;
	double framesPerSecond;
	int depthInBitsPerChannel;
	int numOfChannels;
	GUID outputSubTypeOfCaptureDevice;
	bool useClosestFramerateForGraph;
	bool flipImage;
	bool automaticGenerationOfGraph;
	int majorVideoType;
	GUID reserved;
};

// graphic types
typedef void * ssi_handle_t;
struct ssi_rectf_t {
	ssi_real_t left;
	ssi_real_t top;
	ssi_real_t width;
	ssi_real_t height;
};
SSI_INLINE static ssi_rectf_t ssi_rectf(ssi_real_t left, ssi_real_t top, ssi_real_t width, ssi_real_t height) {
	ssi_rectf_t r;
	r.left = left;
	r.top = top;
	r.width = width;
	r.height = height;
	return r;
}
struct ssi_pointf_t {
	ssi_real_t x;
	ssi_real_t y;
};
SSI_INLINE static ssi_pointf_t ssi_pointf(ssi_real_t x, ssi_real_t y) {
	ssi_pointf_t p;
	p.x = x;
	p.y = y;
	return p;
}
struct ssi_rect_t {
	ssi_int_t left;
	ssi_int_t top;
	ssi_int_t width;
	ssi_int_t height;
};
SSI_INLINE static ssi_rect_t ssi_rect(ssi_size_t left, ssi_size_t top, ssi_size_t width, ssi_size_t height) {
	ssi_rect_t r;
	r.left = left;
	r.top = top;
	r.width = width;
	r.height = height;
	return r;
}
struct ssi_point_t {
	ssi_int_t x;
	ssi_int_t y;
};
SSI_INLINE static ssi_point_t ssi_point(ssi_size_t x, ssi_size_t y) {
	ssi_point_t p;
	p.x = x;
	p.y = y;
	return p;
}
typedef uint32_t ssi_rgb_t;
#define ssi_rgb(r,g,b) ((ssi_rgb_t)(((unsigned char)(r) | ((unsigned short)((unsigned char)(g)) << 8)) | (((unsigned long)(unsigned char)(b)) << 16)))

#if _WIN32|_WIN64
extern ssi_char_t SSI_FILE_SEPARATOR;
#else
extern ssi_char_t SSI_FILE_SEPARATOR;
#endif

const GUID SSI_GUID_NULL = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };

// cast
#define ssi_pcast(type,arg) reinterpret_cast<type *> ((arg))
#define ssi_cast(type,arg) static_cast<type> ((arg))
#define ssi_ccast(type,arg) const_cast<type> ((arg))

// maps to OpenCv types as defined in cxtypes.h
#define	SSI_VIDEO_DEPTH_SIGN 0x80000000
#define	SSI_VIDEO_DEPTH_1U   1
#define	SSI_VIDEO_DEPTH_8U   8
#define	SSI_VIDEO_DEPTH_16U  16
#define	SSI_VIDEO_DEPTH_32F  32
#define	SSI_VIDEO_DEPTH_8S   (SSI_VIDEO_DEPTH_SIGN| 8)
#define	SSI_VIDEO_DEPTH_16S  (SSI_VIDEO_DEPTH_SIGN|16)
#define	SSI_VIDEO_DEPTH_32S  (SSI_VIDEO_DEPTH_SIGN|32)

#define	SSI_VIDEO_MAJOR_TYPE_VIDEO_ONLY		1
#define SSI_VIDEO_MAJOR_TYPE_INTERLEAVED	2

SSI_INLINE char *ssi_wchar2char(const wchar_t *src) {
	size_t len = wcslen(src);
	char *result = new char[len + 1];
	wcstombs(result, src, len);
	result[len] = '\0';
	return result;
}
SSI_INLINE wchar_t *ssi_char2wchar(const char *src) {
	size_t len = strlen(src) + 1;
	wchar_t *result = new wchar_t[len];
	mbstowcs(result, src, len);
	return result;
}

#ifdef SSI_USE_SDL
#if __gnu_linux__
#define RGB(R,G,B) ssi_rgb(R,G,B)
#endif

#define SSI_DEFAULT_FONT_NAME "FreeSerif.ttf"
#define SSI_DEFAULT_FONT_SIZE 12
#define SSI_DEFAULT_FONT_COLOR RGB(192, 192, 192)
#else
#define SSI_DEFAULT_FONT_NAME "Consolas"
#define SSI_DEFAULT_FONT_SIZE 16
#endif
#define SSI_DEFAULT_FONT_COLOR_FORE RGB(192, 192, 192)
#define SSI_DEFAULT_FONT_COLOR_BACK RGB(0, 0, 0)

#include "SSI_SkeletonCons.h"
#include "SSI_Debug.h"
#include "SSI_Tools.h"

#endif
