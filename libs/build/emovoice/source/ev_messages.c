/**
* Datei:	messages.c
* Autor:	Gernot A. Fink
* Datum:	30.7.1997
*
* Beschreibung:	Fehler- und andere Meldungen
**/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#if _WIN32||_WIN64
#include <windows.h> 
#endif
#include "ev_messages.h"

void rs_msg(char *format, ...)
	{
#ifdef _DEBUG
	va_list ap;

	va_start(ap, format);

	rs_msgv(format, ap);

	va_end(ap);
#endif
	}

void rs_msgv(char *format, va_list ap)
	{
	fprintf(stderr, "%s: ", program);
	vfprintf(stderr, format, ap);
	fputc('\n', stderr);
	}

void rs_warning(char *format, ...)
	{
	va_list ap;

	va_start(ap, format);

	rs_warningv(format, ap);

	va_end(ap);
	}

void rs_warningv(char *format, va_list ap)
	{
	fprintf(stderr, "%s: WARNING: ", program);
	vfprintf(stderr, format, ap);
	fputc('\n', stderr);
	}

void rs_error(char *format, ...)
	{
	va_list ap;

	va_start(ap, format);

	rs_errorv(format, ap);

	va_end(ap);
	}

void rs_errorv(char *format, va_list ap)
	{
	fprintf(stderr, "%s: ", program);
	vfprintf(stderr, format, ap);
	fputc('\n', stderr);

	exit(1);
	}

void rs_perror(char *format, ...)
	{
	int _errno;
	va_list ap;

	//_errno = errno;
#if _WIN32||_WIN64
	_errno = GetLastError();
#else
    _errno=dlerror();
#endif
	va_start(ap, format);

	fprintf(stderr, "%s: ", program);
	vfprintf(stderr, format, ap);
	va_end(ap);

	fprintf(stderr, ": %s!\n", strerror(_errno));
	}
