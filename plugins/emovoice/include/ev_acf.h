/**
* Datei:	acf.h
* Autor:	Thomas Ploetz, Wed Jul 14 12:08:44 2004
* Time-stamp:	<04/12/23 14:00:09 tploetz>
*
* Beschreibung:	Definitionen für Autokorrelationsfunktion
*
**/

#ifndef __DSP_ACF_H_INCLUDED__
#define __DSP_ACF_H_INCLUDED__

#include "ev_real.h"

/* 
   prototypes 
*/
void dsp_acf(mx_real_t *t, mx_real_t *f, size_t n, size_t m);

#endif /* __DSP_ACF_H_INCLUDED__ */
