/**
* Datei:	fft.h
* Autor:	Gernot A. Fink
* Datum:	?.4.1991, aktualisiert 10.4.1997
*
* Beschreibung:	Schnelle Fourrier-Transformation (FFT)
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

static void _dsp_fft_init(int n);
static void _dsp_bitrev(void *, size_t, size_t);

static mx_complex_t *w = NULL;		/* Einheitswurzeln fuer FFT */

/**
* _dsp_fft_init(n)
*	Interne Datenstrukturen fuer eine FFT der Laenge 'n' initialisieren,
*	naemlich:
*	  -	'w[]'	die Liste der n-ten Einheitswurzeln exp(-i2pi/n)
**/
static void _dsp_fft_init(int n)
	{
	static int last_n = -1;	/* Laenge der letzten Initialisierung */

	int i;
	mx_real_t phi;
	
	if (last_n == n)
		return;		/* Daten koennen weiterverwendet werden */

	/* Liste der Einheitswurzeln (wieder) allozieren ... */
	w = rs_realloc(w, (n / 2) * sizeof(mx_complex_t),
			"unit roots for FFT!");
		/****	"can't allocate %d unit roots for FFT!\n", n / 2); ***/

	/* ... und Werte berechnen */
	mx_re(w[0]) = 1;		/* w^0 == (1,0) */
	mx_im(w[0]) = 0;
	
	phi = -2.0 * M_PI / n;
	mx_re(w[1]) = cos(phi);	/* w^1 == exp(-i*2*M_PI/n) */
	mx_im(w[1]) = sin(phi);
	
	for (i = 2; i < n / 2; i++)
		mx_cmul(w[i], w[i-1], w[1]);

	last_n = n;
	}
		
/**
* dsp_xfft(z, n, sign)
*	Berechnet FFT (falls 'sign == 0') sowie Inverse (sonst) der Laenge 'n'.
*
*	ANMERKUNG:
*		Der Normierungsfaktor 1/n wird derzeit NICHT beruechsichtigt.
**/
void dsp_xfft(mx_complex_t *z, size_t n, int sign)
	{
	int x,xs,dx,e,de;
	mx_complex_t _w,wx;
	
	/* Berechnung initialisieren ... */
	_dsp_fft_init(n);

	/* ... Bit-Reversal des Eingabevektors durchfuehren ... */
	_dsp_bitrev(z, sizeof(mx_complex_t), n);
	
	/* x-offset, exponent offset */
	for (dx = 1, de = n >> 1;  de >= 1;  dx <<= 1, de >>= 1)
		for (xs = 0; xs < n; xs += 2*dx)
			for (x = xs, e = 0; x < dx+xs; x++, e += de)
				{
				_w = w[e];
				if (sign)
					mx_im(_w) = -mx_im(_w);
				mx_cmul(wx, z[x+dx], _w);
				mx_csub(z[x+dx], z[x], wx);
				mx_cadd(z[x], z[x], wx);
				}
	}

/**
* _dsp_bitrev(_data, size, n)
*	Je 'size' Byte grosse Elemente des Vektors 'data' der Lanege 'n'
*	gemaess der Methode des Bit-Reversal umsortieren.
**/
static void _dsp_bitrev(void *_data, size_t size, size_t n)
	{
	static char buf[LOCAL_MAX_ELEM_SIZE];

	char *data = _data;
	size_t m, i, j;
		
	j = 0;
	for (i = 0; i < n; i++)
		{
		if (j > i)
			{
			memcpy(buf, data+i*size, size);
			memcpy(data+i*size, data+j*size, size);
			memcpy(data+j*size, buf, size);
			}
		m = n >> 1;
		while (m >=  1 && j >=  m)
			{
			j -= m;
			m >>= 1;
			}
		j += m;
		}
	}


// @begin_add_johannes
 void _dsp_fft_destroy () {

	if (w)
		rs_free (w);
}
// @end_add_johannes