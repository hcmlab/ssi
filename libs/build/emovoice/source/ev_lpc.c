/**
* Datei:	lpc.c
* Autor:	Thomas Ploetz, Sun Nov 23 14:11:26 2003
* Time-stamp:	<04/12/23 13:59:04 tploetz>
*
* Beschreibung:	linear prediction coding
**/

#include <math.h>

#include "ev_real.h"
#include "ev_vector.h"
#include "ev_matrix.h"
#include "ev_complex.h"

#include "ev_memory.h"
#include "ev_messages.h"


#include "ev_dsp.h"
#include "ev_acf.h"

static void _dsp_lpc_init(int n);
static void _dsp_lpc_modelspectrum_init(int n);
static void _toeplitz(mx_matrix_t t, mx_vector_t r, int m);

static mx_vector_t b = NULL;
static mx_matrix_t toepl = NULL;

static mx_vector_t eat = NULL;
static mx_vector_t f_fft_p = NULL;
static mx_complex_t *f_fft = NULL;
/**
* _dsp_lpc_init(n)
*	Interne Datenstrukturen fuer eine LPC-Koeffizientenberechnung 
*	der Laenge 'n' initialisieren, naemlich:
*	  -	b[]	Ergebnisvektor für Lösung des LGS (dim: n - 1);
*
*		toepl	Toeplitzmatrix (n x n)
**/
static void _dsp_lpc_init(int n)
	{
	static int last_n = -1;	/* Laenge der letzten Initialisierung */

	int i, j;
	
	if (last_n == n)
		return;		/* Daten koennen weiterverwendet werden */

	/* alte Toeplitz-Matrix loeschen ... */
	if (toepl) 
	  mx_matrix_destroy(toepl, last_n);

	/* ... (wieder) allozieren ... */
	toepl = mx_matrix_create(n - 1, n - 1);

	/* alten Ergebnisvektor löschen */
	if (b)
	  mx_vector_destroy(b);

	b = mx_vector_create(n - 1);

	last_n = n;
	}

/**
* _dsp_lpc_modelspectrum_init(n)
*	Interne Datenstrukturen fuer eine Berechnung des LPC-basierten
*	Modellspektrums der Laenge 'n' initialisieren, naemlich:
*	  -	eat		verl. Vorhersagekoeffizientenvektor
*	  	f_fft[]		FT von eat
*		f_fft_p[]	Power-Spectrum von eat
**/
static void _dsp_lpc_modelspectrum_init(int n)
	{
	static int last_n = -1;	/* Laenge der letzten Initialisierung */

	int i, j;
	
	if (last_n == n)
		return;		/* Daten koennen weiterverwendet werden */

	if (eat) 
		mx_vector_destroy(eat);

	/* ... (wieder) allozieren ... */
	eat = mx_vector_create(n);

	if (f_fft)
		rs_free(f_fft);
	f_fft = rs_calloc(n, sizeof(mx_complex_t), 
			  "f_fft for lpc model spectr.");
	

	if (f_fft_p)
		mx_vector_destroy(f_fft_p);

	f_fft_p = mx_vector_create(n);

	last_n = n;
	}
		
/**
* dsp_lpc(t[], f[], n, m)
*	Berechnet die ersten 'm' LPC-Koeffizienten (inkl. 0 !) des reellen 
*	Signals 'f[]' der Laenge 'n' und speichert das Ergebnis in 't[]'.
**/
void dsp_lpc(mx_real_t *t, mx_real_t *f, size_t n, size_t m)
	{
	int i, j;
	mx_real_t *_t;

	_dsp_lpc_init(m);

	/* AKF Koeffizienten bestimmen */
	dsp_acf(t, f, n, m);

	/* Toepliz-Matrix aufbauen */
	_toeplitz(toepl, t, m - 1);

	/* 	fprintf(stderr, "DEBUG: Toeplitz matrix:\n"); */
	/* 	for (i = 0; i < m - 1; i++) { */
	/* 	  for (j = 0; j < m- 1; j++) */
	/* 	    fprintf(stderr, "%g\t", toepl[i][j]); */
	/* 	  fprintf(stderr, "\n"); */
	/* 	} */

	/* ACHTUNG: der 0. Koeffizient ist stets 1 */
	t[0] = 1;
	for (i = 1; i < m; i++)
	  b[i] = t[i] * -1;

	/* LGS lösen */	
	_t = &(t[1]);
	mx_matrix_linsolve(&_t, toepl, b, m - 1);

	}

/**
* dsp_lpc_modelspectrum(t[], f[], n, m)
*	Berechnet mit Hilfe des LPC das Modellspekturm der Ordnung 'm' des 
*	reellen Signals 'f[]' der Länge 'n' und speichert das Ergebnis
*	in 't[]'
**/
void dsp_lpc_modelspectrum(mx_real_t *t, mx_real_t *f, size_t n, size_t m) {
	int i = 0;

	_dsp_lpc_modelspectrum_init(n);

	/* LPC Koeffizienten bestimmen */
	dsp_lpc(t, f, n, m);

	/* erweiterten Vorhersagekoeffizientenvektor bauen */
	for (i = 0; i < m; i++)
		eat[i] = t[i];
	for (i = m; i < n; i++)
		eat[i] = 0.0;

	/* Modellspektrum bestimmen */
	for (i = 0; i < n; i++) {
		mx_re(f_fft[i]) = eat[i];
		mx_im(f_fft[i]) = 0.0;
	}
	dsp_xfft(f_fft, n, 0);

	for (i = 0; i < n; i++)
		f_fft_p[i] = (dsp_sqr(mx_re(f_fft[i])) + 
			      dsp_sqr(mx_im(f_fft[i]))) /
			dsp_sqr(n);
      
	/* get the logarithms */
	for (i = 0; i < m; i++)
		t[i] = dsp_log10(f_fft_p[i]);	
}


/* ACHTUNG: baut *symmetrische* (hermitesche) Toeplitzmatrizen! */
static void _toeplitz(mx_matrix_t t, mx_vector_t r, int m) {
	int i, j, a;
	
	for (i = 0; i < m; i++) {
		a=0;
		for (j = i; j < m; j++)
			t[i][j] = t[j][i] = r[a++];
	}
}
