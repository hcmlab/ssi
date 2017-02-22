/**
* File:		dct.h
* Author:	Gernot A. Fink
* Date:		18.3.2004 (based on 'costrans.c', originally from 14.4.1997)
*
* Description:	sliding Discrete Fourrier Transform (SDFT)
**/

#include <math.h>

#include "ev_memory.h"


#include "ev_dsp.h"
#include "ev_sdft.h"

#ifndef M_PI
#define M_PI   3.14159265
#endif
#ifndef M_PI_2
#define M_PI_2   1.57079
#endif
#ifndef M_PI_4
#define M_PI_4   0.78539
#endif

/*
 * Local Variables
 */
static mx_complex_t *w = NULL;	/* unit root for n-point DFT */
static last_n = -1;		/* last DFT length used */

/*
 * Local Prototypes
 */
static void _dsp_sdft_init(int n);

/**
* dsp_[r]sdft(S, n, x_0, x_n, down)
*	calculates the sliding DFT for the signal {x_1, ... x_n} shifted
*	by one sample to the right/left based on the DFT 'S' of the 
*	original signal {x_0, ... x_{n-1}} or {x_2, ... x_{n+1}}.
*
*	The update equation for the frequency bin S_k is defined as
*
*		 (t-1)	t        -i2pi*k/n
*		S   =  S      * e          - x + x
*		 k	k		      n   0
*
*	for the 'downward' (left) shift ['down' == 1], i.e. when adding
*	a new sample 'x_0' at the bottom end of the current signal, and as
*
*		 t	(t-1)     i2pi*k/n		 -i2pi*k/n
*		S    = S      * e	   + (x - x ) * e
*		 k      k		       0   n
*
*	for the 'upward' (right) shift ['down' == 0], i.e. when adding
*	a new sample 'x_n' at the top end of the current signal.
*
*	As the k-th unit root w_k is the conjugate complex of the (n-k)-th
*	root w_(n-k) the 'upward' equation can be simplified as follows:
*
*		 t	  (t-1)		 *     i2pi*k/n
*		S   =  { S     + (x - x ) } * e
*		 k	  k	   n   0
*
*	If the transform is applied to real-valued data x_i the conjugate
*	complex x_i* is identical to x_i and, therefore, the update
*	of the spectrum is achieved by:
*
*		 t	  (t-1)		     i2pi*k/n
*		S   =  { S     + x - x  } * e
*		 k	  k	  n   0
*
*	cf. e.g.:
*		E. Jacobsen & R. Lyons: "The sliding DFT", 
*			IEEE Signal Processing Magazine,
*			Vol. 20, No. 2, 2003 pp 74-80.
*
*		NOTE:	In the above reference the 'downward' shift formula
*			is given with out the sign in the exponent. However,
*			the explanation seem to be describing the 'upward'
*			shifting operation =:-[
**/
int dsp_rsdft(mx_complex_t *S, size_t n, mx_real_t x_0, mx_real_t x_n, int down)
	{
	mx_real_t phi;
	int k;
	mx_complex_t S_k;

	/* first check parameters ... */
	if (!S || n <= 0)
		return(-1);

	/* ... eventually initialize calculations ... */
	_dsp_sdft_init(n);

	/* ... switch between 'upward' and 'downward' shifting ... */
	if (!down) {
		/* ... 'upward' shift, rotate by w[n-k] = w[k]* ... */
#ifdef TEST
printf("replacing x_0 = %7g with x_n = %7g\n", x_0, x_n);
#endif /* TEST */

		for (k = 0; k < n; k++) {
			mx_re(S_k) = mx_re(S[k]) - x_0 + x_n;
			mx_im(S_k) = mx_im(S[k]);

			mx_cmul(S[k], S_k, w[n - k]);
			}
		}
	else	{
		/* ... 'downward' shift, rotate by w[k] ... */
#ifdef TEST
printf("replacing x_n = %7g with x_0 = %7g\n", x_n, x_0);
#endif /* TEST */

		for (k = 0; k < n; k++) {
			mx_re(S_k) = mx_re(S[k]);
			mx_im(S_k) = mx_im(S[k]);

			mx_cmul(S[k], S_k, w[k]);

			mx_re(S[k]) += x_0 - x_n;
			}
		}

	return(0);
	}

int dsp_sdft(mx_complex_t *S, size_t n,
		mx_complex_t z_0, mx_complex_t z_n, int down)
	{
	return(0);
	}

/**
* _dsp_fft_init(n)
*	Interne Datenstrukturen fuer eine FFT der Laenge 'n' initialisieren,
*	naemlich:
*	  -	'w[]'	die Liste der n-ten Einheitswurzeln exp(-i2pi/n)
**/
static void _dsp_sdft_init(int n)
	{
	int i;
	mx_real_t phi;
	
	if (last_n == n)
		return;		/* Daten koennen weiterverwendet werden */

	/* Liste der Einheitswurzeln (wieder) allozieren ... */
	w = rs_realloc(w, (n + 1) * sizeof(mx_complex_t),
			"unit roots for sliding DFT!");

	/* ... und Werte berechnen */
	mx_re(w[0]) = 1;		/* w^0 == (1,0) */
	mx_im(w[0]) = 0;
	
	phi = -2.0 * M_PI / n;
	mx_re(w[1]) = cos(phi);	/* w^1 == exp(i*2*M_PI/n) */
	mx_im(w[1]) = sin(phi);
	
	for (i = 2; i < n; i++)
		mx_cmul(w[i], w[i-1], w[1]);

	w[n] = w[0];	/* w[i] sind periodische Folge mit w[n-i] = w[i]* */

	last_n = n;

#ifdef TEST
printf("unit roots:\n");
for (i = 0; i < n; i++) {
	printf("(%7.3f+%7.3fi), ", mx_re(w[i]), mx_im(w[i]));
	}
printf("\n");
#endif /* TEST */
	}
		
#ifdef TEST
#define N		8
#define SIGNAL_LEN	16	/* 8 */

int main(int argc, char **argv)
	{
#if SIGNAL_LEN == 16
	dsp_sample_t x[SIGNAL_LEN] = {0, 1, 2, -1, -2, 2, 3, 0,
					0, 1, 2, -1, -2, 2, 3, 0};
#elif SIGNAL_LEN == 12
	dsp_sample_t x[SIGNAL_LEN] = {0, 1, 2, -1, -2, 0, 2, 1, 0, 1, 2, -1};
#elif SIGNAL_LEN == 8
	dsp_sample_t x[SIGNAL_LEN] = {0, 1, 2, -1, 0, 1, 2, -1};
#endif
	mx_complex_t S[N];
	mx_complex_t F[N];
	mx_real_t p;
	int k, n;

	for (k = 0; k < N; k++) {
		mx_re(S[k]) = x[k];
		mx_im(S[k]) = 0.0;
		}

	dsp_xfft(S, N, 0);

	/* ... upward sweep ... */
	printf("shifting the spectrum 'upward' ...\n");

	for (n = -1; n < SIGNAL_LEN - N; n++) {
		/* ... evtl. comput sliding DFT ... */
		if (n >= 0) {
			dsp_rsdft(S, N, x[n], x[n + N], 0 /* == upward */);

			/* ... compute reference spectrum */
			for (k = 0; k < N; k++) {
				mx_re(F[k]) = x[n + 1 + k];
				mx_im(F[k]) = 0.0;
				}
			dsp_xfft(F, N, 0);
			}

		/* ... print current x ... */
		printf("x[]_%d = ", n + 1);
		for (k = 0; k < N; k++)
			printf("%16d", x[n + 1 + k]);
		printf("\n");
	
		/* ... print current spectrum ... */
		printf("S[]_%d = ", n + 1);
		for (k = 0; k < N; k++) {
			printf("%7.3f+%7.3fi", mx_re(S[k]), mx_im(S[k]));
			}
if (n >= 0) {
		printf("\n");

		/* ... print FFT reference spectrum ... */
		printf("F[]_%d = ", n + 1);
		for (k = 0; k < N; k++) {
			printf("%7.3f+%7.3fi", mx_re(F[k]), mx_im(F[k]));
			}
	}
		printf("\n\n");
	
		}

	/* ... downward sweep ... */
	printf("shifting the spectrum 'downward' ...\n");

	for (n = SIGNAL_LEN - N - 1; n >= 0; n--) {
		/* ... compute sliding DFT ... */
		dsp_rsdft(S, N, x[n], x[n + N], 1 /* == downward */);

		/* ... compute reference spectrum */
		for (k = 0; k < N; k++) {
			mx_re(F[k]) = x[n + k];
			mx_im(F[k]) = 0.0;
			}
		dsp_xfft(F, N, 0);

		/* ... print current x ... */
		printf("x[]_%d = ", n);
		for (k = 0; k < N; k++)
			printf("%16d", x[n + k]);
		printf("\n");
	
		/* ... print current spectrum ... */
		printf("S[]_%d = ", n);
		for (k = 0; k < N; k++) {
			printf("%7.3f+%7.3fi", mx_re(S[k]), mx_im(S[k]));
			}
		printf("\n");

		/* ... print FFT reference spectrum ... */
		printf("F[]_%d = ", n);
		for (k = 0; k < N; k++) {
			printf("%7.3f+%7.3fi", mx_re(F[k]), mx_im(F[k]));
			}
		printf("\n\n");
		}
	}
#endif /* TEST */
