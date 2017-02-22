/**
* Datei:	complex.h
* Autor:	Gernot A. Fink
* Datum:	7.11.1997
*
* Beschreibung:	Definition von komplexen Zahlen mit einfacher Arithmetik
**/

#ifndef __MX_COMPLEX_H_INCLUDED__
#define __MX_COMPLEX_H_INCLUDED__

#ifdef MX_KERNEL
#include "ev_real.h"
#else
#include "ev_real.h"
#endif

typedef struct {		/* komplexe Zahl mit ... */
	mx_real_t x;		/* ... Real- und ... */
	mx_real_t y;		/* ... Imaginaerteil */
	} mx_complex_t;

/**
* Makros fuer komplexe Arithmetik
**/

#define	mx_re(c)		((c).x)
#define mx_im(c)		((c).y)

#define mx_cadd(c, a, b)	\
	(mx_re(c) = mx_re(a) + mx_re(b),	\
	 mx_im(c) = mx_im(a) + mx_im(b),	\
	 (c))
#define mx_csub(c, a, b)	\
	(mx_re(c) = mx_re(a) - mx_re(b),	\
	 mx_im(c) = mx_im(a) - mx_im(b),	\
	 (c))
#define mx_cmul(c, a, b)	\
	(mx_re(c) = mx_re(a) * mx_re(b) - mx_im(a) * mx_im(b),	\
	 mx_im(c) = mx_im(a) * mx_re(b) + mx_re(a) * mx_im(b),	\
	 (c))
	/*
	 * ACHTUNG: Division nur definiert, falls |b| > 0!
	 *
	 * ACHTUNG: Dies wird derzeit nicht geprueft!
	 */
#define mx_cdiv(c, a, b)	\
	(mx_re(c) = (mx_re(a) * mx_re(b) + mx_im(a) * mx_im(b)) /	\
		(mx_re(b) * mx_re(b) + mx_im(b) * mx_im(b)),		\
	 mx_im(c) = (mx_re(b) * mx_im(a) - mx_re(a) * mx_im(b)) /	\
		(mx_re(b) * mx_re(b) + mx_im(b) * mx_im(b)),		\
	(c))

/**
* Funktionsprototypen
**/

#endif /* __MX_COMPLEX_H_INCLUDED__ */
