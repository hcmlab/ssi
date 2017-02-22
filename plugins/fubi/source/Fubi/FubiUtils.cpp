// ****************************************************************************************
//
// Fubi Utils
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler 
// 
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
// 
// ****************************************************************************************
#include "FubiUtils.h"
#include "FubiConfig.h"

#include "FubiImageProcessing.h"

#include <stdarg.h>
#include <iostream>

#if defined ( WIN32 ) || defined( _WINDOWS )
#include <windows.h>
#endif

namespace Fubi
{
#define FUBI_BASEFILENAME(file) (strrchr(file, '/') ? strrchr(file, '/') + 1 : strrchr(file, '\\') ? strrchr(file, '\\') + 1 : file)

	void Logging::logDbg(const char* file, int line, const char* msg, ...)
	{
#if (FUBI_LOG_LEVEL == FUBI_LOG_VERBOSE)
		printf("%s:%d ", FUBI_BASEFILENAME(file), line);
		va_list args;
		va_start(args, msg);
		vfprintf(stderr, msg, args);
		va_end(args);
#endif
	}
	void Logging::logInfo(const char* msg, ...)
	{
#if ((FUBI_LOG_LEVEL <= FUBI_LOG_ERR_WRN_INFO)  || (FUBI_LOG_LEVEL == FUBI_LOG_ERR_INFO))
		va_list args;
		va_start(args, msg);
		vfprintf(stderr, msg, args);
		va_end(args);
#endif
	}
	void Logging::logWrn(const char* file, int line, const char* msg, ...)
	{
#if (FUBI_LOG_LEVEL <= FUBI_LOG_ERR_WRN)
		printf("Fubi Warning(%s:%d): ", FUBI_BASEFILENAME(file), line);
		va_list args;
		va_start(args, msg);
		vfprintf(stderr, msg, args);
		va_end(args);
#endif
	}
	void Logging::logErr(const char* file, int line, const char* msg, ...)
	{
#if (FUBI_LOG_LEVEL <= FUBI_LOG_ERR)
		printf("Fubi ERROR(%s:%d): ", FUBI_BASEFILENAME(file), line);
		va_list args;
		va_start(args, msg);
		vfprintf(stderr, msg, args);
		va_end(args);
#endif
	}

	namespace Timer
	{
#if defined ( WIN32 ) || defined( _WINDOWS )
		static __int64 programStart;
		static __int64 i64frequency;
		static bool usePerformanceCounter = QueryPerformanceFrequency((LARGE_INTEGER*)&i64frequency) == TRUE && QueryPerformanceCounter((LARGE_INTEGER*) &programStart) == TRUE;
		static double frequency = double(i64frequency);

		static double getTime()
		{
			if (Timer::usePerformanceCounter)
			{
				__int64 currentTime;
				QueryPerformanceCounter((LARGE_INTEGER*) &currentTime);
				return (currentTime-Timer::programStart) / Timer::frequency;
			}
			else
				return double(clock()) / CLOCKS_PER_SEC;
		}
#else
		static struct timespec programStart;
		static bool useTimeOfDay = clock_gettime(CLOCK_MONOTONIC, &programStart) == 0;
		static double start = programStart.tv_sec+.000000001*programStart.tv_nsec;
		static double getTime()
		{
			if (useTimeOfDay)
			{
				struct timespec tp;
				clock_gettime(CLOCK_MONOTONIC, &tp);
				return (tp.tv_sec+.000000001*tp.tv_nsec)-start;
			}
			else
				return double(clock()) / CLOCKS_PER_SEC;
		}
#endif
	};

	FingerCountData::~FingerCountData()
	{
		// Release any leftover data
		FubiImageProcessing::releaseImage(fingerCountImage.image);
	}

	double currentTime()
	{
		return Timer::getTime();
	}
}