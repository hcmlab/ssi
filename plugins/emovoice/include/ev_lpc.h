/**
* Datei:	lpc.h
* Autor:	Thomas Ploetz, Wed Jul 14 12:08:44 2004
* Time-stamp:	<04/12/23 14:53:56 tploetz>
*
* Beschreibung:	Definitionen für Lineare Vorhersagekoeffizientenm
*
**/

#ifndef __DSP_LPC_H_INCLUDED__
#define __DSP_LPC_H_INCLUDED__

#include "ev_real.h"

/* 
   prototypes 
*/
void dsp_lpc(mx_real_t *t, mx_real_t *f, size_t n, size_t m);
void dsp_lpc_modelspectrum(mx_real_t *t, mx_real_t *f, size_t n, size_t m);

#endif /* __DSP_LPC_H_INCLUDED__ */
