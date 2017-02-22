/**
* Datei:	dft.h
* Autor:	Thomas Ploetz, Wed Aug 11 16:31:55 2004
* Time-stamp:	<04/08/11 18:05:28 tploetz>
*
* Beschreibung:	Diskrete Fourrier-Transformation
**/

#include <string.h>

#include "ev_real.h"
#include "ev_complex.h"

#include "ev_memory.h"


#include "ev_dsp.h"

#ifndef M_PI
#define M_PI   3.14159265
#endif
#ifndef M_PI_2
#define M_PI_2   1.57079
#endif
#ifndef M_PI_4
#define M_PI_4   0.78539
#endif

#define LOCAL_MAX_ELEM_SIZE	32	/* max. Elementgroesse fuer Bit-Rev.*/

static void _dsp_dft_init(int n);
		
static mx_complex_t *w = NULL;		/* Einheitswurzeln fuer FFT */
static mx_complex_t *_z = NULL;

/**
* _dsp_fft_init(n)
*	Interne Datenstrukturen fuer eine FFT der Laenge 'n' initialisieren,
*	naemlich:
*	  -	'w[]'	die Liste der n-ten Einheitswurzeln exp(-i2pi/n)
**/
static void _dsp_dft_init(int n)
	{
	static int last_n = -1;	/* Laenge der letzten Initialisierung */

	int i;
	mx_real_t phi;
	
	if (last_n == n)
		return;		/* Daten koennen weiterverwendet werden */

	/* Liste der Einheitswurzeln (wieder) allozieren ... */
	w = rs_realloc(w, (n*n) * sizeof(mx_complex_t),
			"unit roots for DFT!");
	/* Ergebnisvektor */
	_z = rs_realloc(_z, n * sizeof(mx_complex_t),
			"result vector for DFT!");

	/* ... und Werte berechnen */
	mx_re(w[0]) = 1;		/* w^0 == (1,0) */
	mx_im(w[0]) = 0;
	
	phi = -2.0 * M_PI / n;
	mx_re(w[1]) = cos(phi);	/* w^1 == exp(-i*2*M_PI/n) */
	mx_im(w[1]) = sin(phi);
	
	for (i = 2; i < n*n; i++)
		mx_cmul(w[i], w[i-1], w[1]);

	last_n = n;
	}

/**
* dsp_xdft(z, n, sign)
*	Berechnet DFT (falls 'sign == 0') sowie Inverse (sonst) der Laenge 'n'.
*
*	ANMERKUNG:
*		Der Normierungsfaktor 1/n wird derzeit NICHT beruechsichtigt.
**/
void dsp_xdft(mx_complex_t *z, size_t M, int sign) {
        int k, m;
	mx_complex_t _w,wx;
	
	/* Berechnung initialisieren ... */
	_dsp_dft_init(M);

	for (k = 0; k < M; k++) {
	  mx_re(_z[k]) = 0;
	  mx_im(_z[k]) = 0;

	  for (m = 0; m < M; m++) {
	    _w = w[m*k];
	    if (sign)
	      mx_im(_w) = -mx_im(_w);
	    mx_cmul(wx, z[m], _w);
	    mx_cadd(_z[k], _z[k], wx);
	  }
	}

	/* Ergebnis kopieren */
	for (k = 0; k < M; k++) {
	  mx_re(z[k]) = mx_re(_z[k]);
	  mx_im(z[k]) = mx_im(_z[k]);
	}
}

