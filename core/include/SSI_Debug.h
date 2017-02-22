// SSI_Debug.h
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

#pragma once

#ifndef SSI_DEBUG_H
#define SSI_DEBUG_H

// printf
#include "base/IMessage.h"

extern FILE *ssiout;
extern ssi::IMessage *ssimsg;

extern ssi_size_t ssi_print_offset;

#define ssi_fprint fprintf
#define ssi_sprint sprintf

#if __MINGW32__||__GNUC__
#include <ctime>
#include <stdint.h>

#define ssi_print( ...) { \
	if (ssimsg) { \
		ssimsg->print (__VA_ARGS__); \
	} \
	if (ssiout) { \
		fprintf (ssiout, __VA_ARGS__); \
	} \
}

#define ssi_print_off( ...) { \
	if (ssimsg) { \
		for (ssi_size_t ssi_print_off_i = 0; ssi_print_off_i < ssi_print_offset; ssi_print_off_i++) { \
			ssimsg->print(" "); \
		} \
		ssimsg->print (__VA_ARGS__); \
	} \
	if (ssiout) { \
		for (ssi_size_t ssi_print_off_i = 0; ssi_print_off_i < ssi_print_offset; ssi_print_off_i++) { \
			fprintf(ssiout, " "); \
		} \
		fprintf (ssiout, __VA_ARGS__); \
	} \
}

#define ssi_fprint_off(file, ...) { \
	for (ssi_size_t ssi_print_off_i = 0; ssi_print_off_i < ssi_print_offset; ssi_print_off_i++) { \
		ssi_fprint((file), " "); \
	} \
	fprintf((file), __VA_ARGS__); \
}

#else

#define ssi_print(text, ...) { \
	if (ssimsg) { \
		ssimsg->print (text, __VA_ARGS__); \
	} \
	if (ssiout) { \
		fprintf (ssiout, (text), __VA_ARGS__); \
	} \
}

#define ssi_print_off(text, ...) { \
	if (ssimsg) { \
		for (ssi_size_t ssi_print_off_i = 0; ssi_print_off_i < ssi_print_offset; ssi_print_off_i++) { \
			ssimsg->print(" "); \
		} \
		ssimsg->print (text, __VA_ARGS__); \
	} \
	if (ssiout) { \
		for (ssi_size_t ssi_print_off_i = 0; ssi_print_off_i < ssi_print_offset; ssi_print_off_i++) { \
			fprintf(ssiout, " "); \
		} \
		fprintf (ssiout, (text), __VA_ARGS__); \
	} \
}

#define ssi_fprint_off(file, text, ...) { \
	for (ssi_size_t ssi_print_off_i = 0; ssi_print_off_i < ssi_print_offset; ssi_print_off_i++) { \
		ssi_fprint((file), " "); \
	} \
	fprintf((file), (text), __VA_ARGS__); \
}
#endif // __MINGW32__

// assert
#define	SSI_ASSERT	_ASSERT

// time functions
static const int ssi_time_size = 15;
static const int ssi_time_size_friendly = 20;
SSI_INLINE void ssi_time_format (time_t rawtime, ssi_char_t *string) {
	struct tm *timeinfo;
	timeinfo = localtime (&rawtime);
	strftime (string,ssi_time_size,"%y%m%d%H%M%S",timeinfo);
}
SSI_INLINE void ssi_time_format_friendly (time_t rawtime, ssi_char_t *string) {
	struct tm *timeinfo;
	timeinfo = localtime (&rawtime);
	strftime (string,ssi_time_size_friendly,"%Y-%m-%d_%H-%M-%S",timeinfo);
}
SSI_INLINE time_t ssi_time_parse (ssi_char_t *string) {
	tm timeinfo; char tmp[100]; int val;
	tmp[0] = string[0]; tmp[1] = string[1]; tmp[2] = '\0'; val = atoi (tmp);
	timeinfo.tm_year = 100 + val;
	tmp[0] = string[2]; tmp[1] = string[3]; tmp[2] = '\0'; val = atoi (tmp);
	timeinfo.tm_mon = val-1;
	tmp[0] = string[4]; tmp[1] = string[5]; tmp[2] = '\0'; val = atoi (tmp);
	timeinfo.tm_mday = val;
	tmp[0] = string[6]; tmp[1] = string[7]; tmp[2] = '\0'; val = atoi (tmp);
	timeinfo.tm_hour = val;
	tmp[0] = string[8]; tmp[1] = string[9]; tmp[2] = '\0'; val = atoi (tmp);
	timeinfo.tm_min = val;
	tmp[0] = string[10]; tmp[1] = string[11]; tmp[2] = '\0'; val = atoi (tmp);
	timeinfo.tm_sec = val;
	timeinfo.tm_wday = 0;
	timeinfo.tm_yday = 0;
	timeinfo.tm_isdst = 0;
	return mktime (&timeinfo);
}
SSI_INLINE static char *ssi_now () {
	time_t rawtime;
	struct tm *timeinfo;
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	return asctime (timeinfo);
}
SSI_INLINE void ssi_now (ssi_char_t *string) {
	time_t rawtime;
	time (&rawtime);
	ssi_time_format (rawtime, string);
}
SSI_INLINE void ssi_now_friendly (ssi_char_t *string) {
	time_t rawtime;
	time (&rawtime);
	ssi_time_format_friendly (rawtime, string);
}

SSI_INLINE ssi_size_t ssi_time_ms () {
	#if __gnu_linux__
	uint32_t ms=0;
	timespec ts;
	clock_gettime (CLOCK_MONOTONIC_RAW, &ts);
	ms= ts.tv_sec*1000+ (uint32_t)(ts.tv_nsec/1000000);

	return ssi_cast (ssi_size_t, ms);
	#else
	return ssi_cast (ssi_size_t, ::timeGetTime ());
	#endif
}
SSI_INLINE ssi_size_t ssi_elapsed_ms (ssi_size_t ms) {
	return ssi_time_ms () - ms;
}
SSI_INLINE void ssi_time_fprint (ssi_size_t ms, FILE *file = stdout) {
	ssi_size_t sec = ms / 1000;
	ms %= 1000;
	ssi_size_t min = sec / 60;
	sec %= 60;
	ssi_size_t hrs = min / 60;
	min %= 60;
	ssi_fprint (file, "%u:%02u:%02u:%03u", hrs, min, sec, ms);
}
SSI_INLINE void ssi_time_sprint (ssi_size_t ms, ssi_char_t *string) {
	ssi_size_t sec = ms / 1000;
	ms %= 1000;
	ssi_size_t min = sec / 60;
	sec %= 60;
	ssi_size_t hrs = min / 60;
	min %= 60;
	ssi_sprint (string, "%u:%02u:%02u:%03u", hrs, min, sec, ms);
}

extern ssi_size_t ssi_tic_start;
SSI_INLINE void ssi_tic () {
	#ifdef __gnu_linux__
	uint32_t ms=0;
	timespec ts;
	clock_gettime (CLOCK_MONOTONIC_RAW, &ts);
	ms= ts.tv_sec*1000+ (uint32_t)(ts.tv_nsec/1000000);
	ssi_tic_start = ssi_cast (ssi_size_t, ms);
	#else
	ssi_tic_start = ssi_cast (ssi_size_t, ::timeGetTime ());
	#endif
}
SSI_INLINE ssi_size_t ssi_toc () {
		#ifdef __gnu_linux__
	uint32_t ms=0;
	timespec ts;
	clock_gettime (CLOCK_MONOTONIC_RAW, &ts);
	ms= ts.tv_sec*1000+ (uint32_t)(ts.tv_nsec/1000000);
	ssi_size_t stop = ssi_cast (ssi_size_t, ms);
	#else
	ssi_size_t stop = ssi_cast (ssi_size_t, ::timeGetTime ());
	#endif
	return stop - ssi_tic_start;
}
SSI_INLINE void ssi_toc_print (FILE *file = stdout) {
	ssi_size_t ms = ssi_toc ();
	ssi_size_t sec = ms / 1000;
	ms %= 1000;
	ssi_size_t min = sec / 60;
	sec %= 60;
	ssi_size_t hrs = min / 60;
	min %= 60;
	ssi_fprint (file, "%u:%02u:%02u:%03u", hrs, min, sec, ms);
}

// logging
enum SSI_LOG_LEVEL {
	SSI_LOG_LEVEL_ERROR = 0,	// error only			(-> ssi_err)
	SSI_LOG_LEVEL_WARNING,		// + warnings			(-> ssi_wrn)
	SSI_LOG_LEVEL_BASIC,		// + basic messages		(-> ssi_msg, SSI_DBG)
	SSI_LOG_LEVEL_DETAIL,		// + detailed messages	(-> ssi_msg, SSI_DBG)
	SSI_LOG_LEVEL_DEBUG,		// + debug information	(-> SSI_DBG)
	SSI_LOG_LEVEL_VERBOSE
};
#ifdef _DEBUG
#define SSI_LOG_LEVEL_DEFAULT SSI_LOG_LEVEL_DETAIL
#else
#define SSI_LOG_LEVEL_DEFAULT SSI_LOG_LEVEL_BASIC
#endif
static char ssi_log_name[] = "__________";
static char ssi_log_name_static[] = "_________s";
static int ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
static int ssi_log_level_static = SSI_LOG_LEVEL_DEFAULT;
#if __MINGW32__ || __GNUC__
#ifdef _DEBUG
#define ssi_err(...) { \
	if (ssimsg) { \
		ssimsg->err(ssi_log_name, __FILE__, __LINE__, __VA_ARGS__); \
	} \
	if (ssiout) { \
		fprintf (ssiout, "[%s] # !ERROR! # ", ssi_log_name); \
		fprintf (ssiout, __VA_ARGS__); \
		fprintf (ssiout, "\nlocation: %s (%d)\n", __FILE__, __LINE__); \
	} \
	if (ssi_log_file_on) { \
		ssi_log_file_end (); \
	} \
	SSI_ASSERT (false); \
}
#else
#define ssi_err( ...) { \
	if (ssimsg) { \
		ssimsg->err(ssi_log_name, __FILE__, __LINE__, __VA_ARGS__); \
	} \
	if (ssiout) { \
		fprintf (ssiout, "[%s] # !ERROR! # ", ssi_log_name); \
		fprintf (ssiout, __VA_ARGS__); \
		fprintf (ssiout, "\nlocation: %s (%d)\n", __FILE__, __LINE__); \
	} \
	FILE *errfile = fopen ("ssi_last.err", "w"); \
	ssi_fprint (errfile, "[%s] # !ERROR! # ", ssi_log_name); \
	ssi_fprint (errfile, __VA_ARGS__); \
	ssi_fprint (errfile, "\nlocation: %s (%d)\n", __FILE__, __LINE__); \
	fclose (errfile); \
	if (ssi_log_file_on) { \
		ssi_log_file_end (); \
	} \
	exit (-1); \
}
#endif

#ifdef _DEBUG
#define ssi_err_static(text, ...) { \
	if (ssimsg) { \
		ssimsg->err(ssi_log_name_static, __FILE__, __LINE__, __VA_ARGS__); \
	} \
	if (ssiout) { \
		fprintf (ssiout, "[%s] # !ERROR! # ", ssi_log_name_static); \
		fprintf (ssiout,  __VA_ARGS__); \
		fprintf (ssiout, "\nlocation: %s (%d)\n", __FILE__, __LINE__); \
	} \
	if (ssi_log_file_on) { \
		ssi_log_file_end (); \
	} \
	SSI_ASSERT (false); \
}
#else
#define ssi_err_static( ...) { \
	if (ssimsg) { \
		ssimsg->err(ssi_log_name_static, __FILE__, __LINE__, __VA_ARGS__); \
	} \
	if (ssiout) { \
		fprintf (ssiout, "[%s] # !ERROR! # ", ssi_log_name_static); \
		fprintf (ssiout,  __VA_ARGS__); \
		fprintf (ssiout, "\nlocation: %s (%d)\n", __FILE__, __LINE__); \
	} \
	FILE *errfile = fopen ("ssi_last.err", "w"); \
	ssi_fprint (errfile, "[%s] # !ERROR! # ", ssi_log_name_static); \
	ssi_fprint (errfile,  __VA_ARGS__); \
	ssi_fprint (errfile, "\nlocation: %s (%d)\n", __FILE__, __LINE__); \
	fclose (errfile); \
	if (ssi_log_file_on) { \
		ssi_log_file_end (); \
	} \
	exit (-1); \
}
#endif
#define ssi_wrn( ...) { \
	if (ssi_log_level) { \
		if (ssimsg) { \
			ssimsg->wrn(ssi_log_name, __FILE__, __LINE__,  __VA_ARGS__); \
		} \
		if (ssiout) { \
			fprintf (ssiout, "[%s] # WARNING # ", ssi_log_name); \
			fprintf (ssiout,  __VA_ARGS__); \
			fprintf (ssiout, "\nlocation: %s (%d)\n", __FILE__, __LINE__); \
		} \
	} \
}
#define ssi_wrn_static( ...) { \
	if (ssi_log_level_static) { \
		if (ssimsg) { \
			ssimsg->wrn(ssi_log_name_static, __FILE__, __LINE__,  __VA_ARGS__); \
		} \
		if (ssiout) { \
			fprintf (ssiout, "[%s] # WARNING # ", ssi_log_name_static); \
			fprintf (ssiout,  __VA_ARGS__); \
			fprintf (ssiout, "\nlocation: %s (%d)\n", __FILE__, __LINE__); \
		} \
	} \
}
#define ssi_msg(level, text...) { \
	if ((level) <= ssi_log_level) { \
		if (ssimsg) { \
			ssimsg->msg(ssi_log_name, text); \
		} \
		if (ssiout) { \
			fprintf (ssiout, "[%s] ", ssi_log_name); \
			fprintf (ssiout, text); \
			fprintf (ssiout, "\n"); \
		} \
	} \
}
#define ssi_msg_static(level, text...) { \
	if ((level) <= ssi_log_level_static) { \
		if (ssimsg) { \
			ssimsg->msg(ssi_log_name_static, text); \
		} \
		if (ssiout) { \
			fprintf (ssiout, "[%s] ", ssi_log_name_static); \
			fprintf (ssiout, text); \
			fprintf (ssiout, "\n"); \
		} \
	} \
}
#ifdef _DEBUG
#define SSI_DBG( ...) ssi_msg( __VA_ARGS__);
#define SSI_DBG_STATIC( ...) ssi_msg_static( __VA_ARGS__);
#else
#define SSI_DBG( ...)
#define SSI_DBG_STATIC( ...)
#endif
#else

enum Color { DARKBLUE = 1, DARKGREEN, DARKTEAL, DARKRED, DARKPINK, DARKYELLOW, GRAY, DARKGRAY, BLUE, GREEN, TEAL, RED, PINK, YELLOW, WHITE };


#ifdef _DEBUG
#define ssi_err(text, ...) { \
	if (ssimsg) { \
		ssimsg->err(ssi_log_name, __FILE__, __LINE__, (text), __VA_ARGS__); \
		} \
	if (ssiout) { \
		/*set console color to red */ \
		HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);	\
		SetConsoleTextAttribute(hCon, RED); \
		\
		fprintf (ssiout, "[%s] # !ERROR! # ", ssi_log_name); \
		fprintf (ssiout, (text), __VA_ARGS__); \
		fprintf (ssiout, "\nlocation: %s (%d)\n", __FILE__, __LINE__); \
		\
		/*set console color to gray (default) */ \
		SetConsoleTextAttribute(hCon, GRAY); \
		} \
	if (ssi_log_file_on) { \
		ssi_log_file_end (); \
		} \
	SSI_ASSERT (false); \
}
#else
#define ssi_err(text, ...) { \
	if (ssimsg) { \
		ssimsg->err(ssi_log_name, __FILE__, __LINE__, (text), __VA_ARGS__); \
		} \
	if (ssiout) { \
		/*set console color to red */ \
		HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);	\
		SetConsoleTextAttribute(hCon, RED); \
		\
		fprintf (ssiout, "[%s] # !ERROR! # ", ssi_log_name); \
		fprintf (ssiout, (text), __VA_ARGS__); \
		fprintf (ssiout, "\nlocation: %s (%d)\n", __FILE__, __LINE__); \
		\
		/*set console color to gray (default) */ \
		SetConsoleTextAttribute(hCon, GRAY); \
		} \
	FILE *errfile = fopen ("ssi_last.err", "w"); \
	ssi_fprint (errfile, "[%s] # !ERROR! # ", ssi_log_name); \
	ssi_fprint (errfile, (text), __VA_ARGS__); \
	ssi_fprint (errfile, "\nlocation: %s (%d)\n", __FILE__, __LINE__); \
	fclose (errfile); \
	if (ssi_log_file_on) { \
		ssi_log_file_end (); \
		} \
	exit (-1); \
}
#endif

#ifdef _DEBUG
#define ssi_err_static(text, ...) { \
	if (ssimsg) { \
		ssimsg->err(ssi_log_name_static, __FILE__, __LINE__, (text), __VA_ARGS__); \
		} \
	if (ssiout) { \
		 \
		/*set console color to red */ \
		HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);	\
		SetConsoleTextAttribute(hCon, RED); \
		\
		fprintf (ssiout, "[%s] # !ERROR! # ", ssi_log_name_static); \
		fprintf (ssiout, (text), __VA_ARGS__); \
		fprintf (ssiout, "\nlocation: %s (%d)\n", __FILE__, __LINE__); \
		\
		/*set console color to gray (default) */ \
		SetConsoleTextAttribute(hCon, GRAY); \
		} \
	if (ssi_log_file_on) { \
		ssi_log_file_end (); \
		} \
	SSI_ASSERT (false); \
}
#else
#define ssi_err_static(text, ...) { \
	if (ssimsg) { \
		ssimsg->err(ssi_log_name_static, __FILE__, __LINE__, (text), __VA_ARGS__); \
		} \
	if (ssiout) { \
		 \
		/*set console color to red */ \
		HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);	\
		SetConsoleTextAttribute(hCon, RED); \
		\
		fprintf (ssiout, "[%s] # !ERROR! # ", ssi_log_name_static); \
		fprintf (ssiout, (text), __VA_ARGS__); \
		fprintf (ssiout, "\nlocation: %s (%d)\n", __FILE__, __LINE__); \
		\
		/*set console color to gray (default) */ \
		SetConsoleTextAttribute(hCon, GRAY); \
		} \
	FILE *errfile = fopen ("ssi_last.err", "w"); \
	ssi_fprint (errfile, "[%s] # !ERROR! # ", ssi_log_name_static); \
	ssi_fprint (errfile, (text), __VA_ARGS__); \
	ssi_fprint (errfile, "\nlocation: %s (%d)\n", __FILE__, __LINE__); \
	fclose (errfile); \
	if (ssi_log_file_on) { \
		ssi_log_file_end (); \
		} \
	exit (-1); \
}
#endif
#define ssi_wrn(text, ...) { \
	if (ssi_log_level) { \
		if (ssimsg) { \
			ssimsg->wrn(ssi_log_name, __FILE__, __LINE__, (text), __VA_ARGS__); \
		} \
		if (ssiout) { \
			/*set console color to yellow */ \
			HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);	\
			SetConsoleTextAttribute(hCon, YELLOW); \
			\
			fprintf (ssiout, "[%s] # WARNING # ", ssi_log_name); \
			fprintf (ssiout, (text), __VA_ARGS__); \
			fprintf (ssiout, "\nlocation: %s (%d)\n", __FILE__, __LINE__); \
			\
			/*set console color to gray (default) */ \
			SetConsoleTextAttribute(hCon, GRAY); \
		} \
	} \
}
#define ssi_wrn_static(text, ...) { \
	if (ssi_log_level_static) { \
		if (ssimsg) { \
			ssimsg->wrn(ssi_log_name_static, __FILE__, __LINE__, (text), __VA_ARGS__); \
		} \
		if (ssiout) { \
			/*set console color to yellow */ \
			HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);	\
			SetConsoleTextAttribute(hCon, YELLOW); \
			\
			fprintf (ssiout, "[%s] # WARNING # ", ssi_log_name_static); \
			fprintf (ssiout, (text), __VA_ARGS__); \
			fprintf (ssiout, "\nlocation: %s (%d)\n", __FILE__, __LINE__); \
			\
			/*set console color to gray (default) */ \
			SetConsoleTextAttribute(hCon, GRAY); \
		} \
	} \
}
#define ssi_msg(level, text, ...) { \
	if ((level) <= ssi_log_level) { \
		if (ssimsg) { \
			ssimsg->msg(ssi_log_name, (text), __VA_ARGS__); \
		} \
		if (ssiout) { \
			fprintf (ssiout, "[%s] ", ssi_log_name); \
			fprintf (ssiout, (text), __VA_ARGS__); \
			fprintf (ssiout, "\n"); \
		} \
	} \
}
#define ssi_msg_static(level, text, ...) { \
	if ((level) <= ssi_log_level_static) { \
		if (ssimsg) { \
			ssimsg->msg(ssi_log_name_static, (text), __VA_ARGS__); \
		} \
		if (ssiout) { \
			fprintf (ssiout, "[%s] ", ssi_log_name_static); \
			fprintf (ssiout, (text), __VA_ARGS__); \
			fprintf (ssiout, "\n"); \
		} \
	} \
}
#ifdef _DEBUG
#define SSI_DBG(level, text, ...) ssi_msg(level, text, __VA_ARGS__);
#define SSI_DBG_STATIC(level, text, ...) ssi_msg_static(level, text, __VA_ARGS__);
#else
#define SSI_DBG(level, text, ...)
#define SSI_DBG_STATIC(level, text, ...)
#endif

#endif // __MINGW32__
extern bool ssi_log_file_on;
SSI_INLINE void ssi_log_file_end () {
	fprintf (ssiout, "-------------------------------------------\n\t%s-------------------------------------------\n", ssi_now ());
	//freopen ("CON", "w", stdout);
	fclose (ssiout);
	ssiout = stdout;
	ssi_log_file_on = false;
}

SSI_INLINE void ssi_log_file_begin (const char *file_name) {
	//freopen ((file_name), "a", stdout);
	FILE *file = fopen (file_name, "a");
	if (!file) {
		ssi_wrn ("cannot open '%s'", file_name);
		return;
	}
	ssiout = file;
	fprintf (ssiout, "-------------------------------------------\n\t%s-------------------------------------------\n", ssi_now ());
	ssi_log_file_on = true;
}

SSI_INLINE void ssi_PrintLastError() {

#if _WIN32|_WIN64

	LPTSTR lpMsgBuf;
	DWORD dw = ::GetLastError();

	::FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);
	
#ifdef UNICODE
	char *msgBuf = ssi_wchar2char(0);
	ssi_wrn(msgBuf);
	delete[] msgBuf;
#else
	ssi_wrn(lpMsgBuf);
#endif // !UNICODE

#endif

}

#endif
