/**
* Datei:	dwt.h
* Autor:	Thomas Ploetz, Wed Jul 14 12:08:44 2004
* Time-stamp:	<04/07/23 18:29:27 tploetz>
*
* Beschreibung:	Definitionen für discrete wavelet transformation
*
**/

#ifndef __DSP_DWT_H_INCLUDED__
#define __DSP_DWT_H_INCLUDED__

#include "ev_real.h"

/* 
   prototypes 
*/

int dsp_dwt(mx_real_t **u,
	    int *l_ui,
	    mx_real_t ***v,
	    int **l_v,
	    mx_real_t *signal,
	    int len_signal,
	    mx_real_t *A,
	    int len_A,
	    mx_real_t *D,
	    int len_D,
	    int level);
mx_real_t *dsp_idwt(mx_real_t *u,
		    int l_u,
		    mx_real_t **v,
		    int *l_v,
		    mx_real_t *A,
		    int len_A,
		    mx_real_t *D,
		    int len_D,
		    int level);

#endif /* __DSP_DWT_H_INCLUDED__ */
