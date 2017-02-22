/**
* Datei:	window.c
* Autor:	Gernot A. Fink
* Datum:	10.4.1997
*
* Beschreibung:	verschiedene Fensterfunktionen fuer die Kurzzeitanalyse
**/

#include "ev_real.h"

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

static mx_real_t *w_hamming = NULL;
static mx_real_t *w_hanning = NULL;
static mx_real_t *w_gauss = NULL;

static void _dsp_window_hamming_init(int n);
static void _dsp_window_hanning_init(int n);
static void _dsp_window_gauss_init(int n);

/**
* dsp_window_hamming(w, s, n)
*	Wendet ein Hammingfenster auf das Signal 's' der Laenge 'n' an
*	und speichert das "gefensterte" Ergebnis in 'w'.
*
*	ANMERKUNG:
*		Das Hamming-Fenster ist definiert als:
*
*					      2 pi i
*			w  = 0.54 - 0.46 cos( ------ )		0 <= i < n
*			 i		        n
*
*		vgl. z.B.:
*			E. G. Schukat-Talamazzini: Automatische Spracherkennung,
*				Vieweg, 1995, S. 49.
*			(dort aber mit Normierung 1/(n-1), die "unschoenen"
*			 Frequenzgang bewirkt)
**/
void dsp_window_hamming(mx_real_t *w, dsp_sample_t *s, size_t n)
	{
	int i;

	/* Berechnung initialisieren ... */
	_dsp_window_hamming_init(n);

	/* ... und Fensterfunktion anwenden */
	for (i = 0; i < n; i++)
		w[i] = w_hamming[i] * s[i];
	}

/**
* _dsp_window_hamming_init(n)
*	Interne Datenstrukturen fuer ein Hamming-Fenster der Lanenge 'n'
*	initialisieren, naemlich:
*	  -	'w_hamming[]'	die Fensterfunktion
**/
static void _dsp_window_hamming_init(int n)
	{
	static int last_n = -1;	/* Laenge der letzten Initialisierung */

	int i;
	mx_real_t a = 2 * M_PI / (mx_real_t) n;
	
	if (last_n == n)
		return;		/* Daten koennen weiterverwendet werden */

	/* Liste der Fensterfunktionswerte (wieder) allozieren ... */
	w_hamming = rs_realloc(w_hamming, n * sizeof(mx_real_t),
				"hamming window");
		/***	"can't create hamming window of length %d!\n", n); ***/

	/* ... und Werte berechnen */
	for (i = 0; i < n; i++)
		w_hamming[i] = 0.54 - 0.46 * cos(a * i);

	last_n = n;
	}

/**
* dsp_window_hanning(w, s, n)
*	Wendet ein Hanningfenster auf das Signals 's' der Laenge 'n' an
*	und speichert das "gefensterte" Ergebnis in 'w'.
*
*	ANMERKUNG:
*		Das Hanning-Fenster ist definiert als:
*
*					      2 pi i
*			w  = 0.50 - 0.50 cos( ------ )		0 <= i < n
*			 i		        n
*
*		vgl. z.B.:
*			E. G. Schukat-Talamazzini: Automatische Spracherkennung,
*				Vieweg, 1995, S. 49.
*			(dort aber mit Normierung 1/(n-1), die "unschoenen"
*			 Frequenzgang bewirkt)
**/
void dsp_window_hanning(mx_real_t *w, dsp_sample_t *s, size_t n)
	{
	int i;

	/* Berechnung initialisieren ... */
	_dsp_window_hanning_init(n);

	/* ... und Fensterfunktion anwenden */
	for (i = 0; i < n; i++)
		w[i] = w_hanning[i] * s[i];
	}

/**
* _dsp_window_hanning_init(n)
*	Interne Datenstrukturen fuer ein Hanning-Fenster der Lanenge 'n'
*	initialisieren, naemlich:
*	  -	'w_hanning[]'	die Fensterfunktion
**/
static void _dsp_window_hanning_init(int n)
	{
	static int last_n = -1;	/* Laenge der letzten Initialisierung */

	int i;
	mx_real_t a = 2 * M_PI / (mx_real_t) n;
	
	if (last_n == n)
		return;		/* Daten koennen weiterverwendet werden */

	/* Liste der Fensterfunktionswerte (wieder) allozieren ... */
	w_hanning = rs_realloc(w_hanning, n * sizeof(mx_real_t),
				"hanning window");

	/* ... und Werte berechnen */
	for (i = 0; i < n; i++)
		w_hanning[i] = 0.50 - 0.5 * cos(a * i);

	last_n = n;
	}

/**
* dsp_window_gauss(w, s, n)
*	Wendet ein Gaussfenster auf das Signal 's' der Laenge 'n' an
*	und speichert das "gefensterte" Ergebnis in 'w'.
*
*	ANMERKUNG:
*		Das Gauss-Fenster ist definiert als:
*
*				         i - n/2  2
*			w  = exp( -0.5 (---------) )	0 <= i < n
*			 i		  s n/2
*
*		vgl. z.B.:
*			E. G. Schukat-Talamazzini: Automatische Spracherkennung,
*				Vieweg, 1995, S. 49.
**/
void dsp_window_gauss(mx_real_t *w, dsp_sample_t *s, size_t n)
	{
	int i;

	/* Berechnung initialisieren ... */
	_dsp_window_gauss_init(n);

	/* ... und Fensterfunktion anwenden */
	for (i = 0; i < n; i++)
		w[i] = w_gauss[i] * s[i];
	}

/**
* _dsp_window_gauss_init(n)
*	Interne Datenstrukturen fuer ein Gauss-Fenster der Lanenge 'n'
*	initialisieren, naemlich:
*	  -	'w_gauss[]'	die Fensterfunktion
**/
static void _dsp_window_gauss_init(int n)
	{
	static int last_n = -1;	/* Laenge der letzten Initialisierung */

	int i;
	mx_real_t a, s = 0.4; /** s = 3 ist offenbar Schmarrn :-( **/
	
	if (last_n == n)
		return;		/* Daten koennen weiterverwendet werden */

	/* Liste der Fensterfunktionswerte (wieder) allozieren ... */
	w_gauss = rs_realloc(w_gauss, n * sizeof(mx_real_t),
				"gauss window");

	/* ... und Werte berechnen */
	for (i = 0; i < n; i++) {
		a = (i - n/2) / (s * n / 2);
		w_gauss[i] = exp(-0.5 * a * a);
		}

	last_n = n;
	}

/*	dsp_window_hanning_create (n) */
/**	create a hanning window
 *
 *	Create a hanning window of length 'n'.
 *
 *	@param	n	size of hanning window.
 *
 *	@return	pointer to the hanning window.
 */
mx_real_t* dsp_window_hanning_create(size_t n)
{
	int i;
	mx_real_t* w = NULL;

	/* allocte memory for the hanning window */
	w = rs_malloc(sizeof(mx_real_t) * n, "window only");
	
	/* init window calculation ... */
	_dsp_window_hanning_init(n);

	/* ... and copy window function */
	for (i = 0; i < n; i++)
		w[i] = w_hanning[i];
	
	/* return window */
	return w;
}
