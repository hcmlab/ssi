/**
* Datei:	real.h
* Autor:	Gernot A. Fink
* Datum:	14.10.1997
*
* Beschreibung:	Definition von reellen Zahlen
**/

#ifndef __MX_REAL_H_INCLUDED__
#define __MX_REAL_H_INCLUDED__

#include <math.h>
#include <float.h>

#ifdef MX_USE_DOUBLE
#define MX_REAL_MAX            DBL_MAX
#define MX_REAL_MIN            DBL_MIN
#define MX_LOGREAL_MAX         (M_LN2 * DBL_MAX_EXP)   /* LN_MAXDOUBLE */
#define MX_LOGREAL_MIN         (M_LN2 * (DBL_MIN_EXP - 1))     /* LN_MINDOUBLE */
#else
#define MX_REAL_MAX            FLT_MAX
#define MX_REAL_MIN            FLT_MIN
#define MX_LOGREAL_MAX         (M_LN2 * FLT_MAX_EXP)   /* LN_MAXFLOAT */
#define MX_LOGREAL_MIN         (M_LN2 * (FLT_MIN_EXP - 1))     /* LN_MINFLOAT */
#endif

#define MX_REAL_LOW		(1e-20)
#define MX_LOGREAL_LOW		(-46)
#define MX_LOG10REAL_LOW	(-20)

#ifdef MX_USE_DOUBLE
typedef double mx_real_t;	/* allgemeine Gleitkommazahl */
#else
typedef float mx_real_t;	/* allgemeine Gleitkommazahl */
#endif

/**
* Makros fuer div. einfache Funktionen ...
**/
#define mx_sqr(x)	((x) * (x))
#define mx_log(x)	(((x) > MX_REAL_LOW) ? log(x) : MX_LOGREAL_LOW)
#define mx_log10(x)	(((x) > MX_REAL_LOW) ? log10(x) : MX_LOG10REAL_LOW)

#endif /* __MX_REAL_H_INCLUDED__ */
