/**
* Datei:	prob.h
* Autor:	Gernot A. Fink
* Datum:	14.10.1997
*
* Beschreibung:	Definition von (logarithmischen) Wahrscheinlichkeiten
**/

#ifndef __MX_PROB_H_INCLUDED__
#define __MX_PROB_H_INCLUDED__

#ifdef MX_KERNEL
#include "ev_real.h"
#else
#include "ev_real.h"
#endif

#define	MX_PROB_MIN		0.0
#define	MX_PROB_MAX		1.0
#define	MX_PROB_LOW		0.00001

#define MX_PROB_IGNORE		0.001

#define	MX_LOGPROB_MIN		MX_REAL_MAX
#define	MX_LOGPROB_MAX		0
#define	MX_LOGPROB_LOW		11.5	/* ~ -log(MX_PROB_LOW) */

typedef mx_real_t mx_prob_t;	/* Wahrscheinlichkeit ]0.0 ... 1.0] */
typedef mx_real_t mx_logprob_t;	/* - log(mx_prob_t) */

extern mx_prob_t mx_prob_low;
extern mx_logprob_t mx_logprob_low;

extern mx_prob_t mx_prob_ignore;

#define mx_prob2log(p)		((p < mx_prob_low) ? mx_logprob_low : -log(p))
#define mx_log2prob(l)		(exp(-(l)))

#define mx_xprob2prob(xp)	((mx_prob_t)(xp) / 10000.0)
#define mx_prob2xprob(p)	((p) * 10000.0)

/*
 * Funktionsprototypen 
 */
mx_logprob_t mx_logprob_add(mx_logprob_t p1, mx_logprob_t p2);

#endif /* __MX_PROB_H_INCLUDED__ */
