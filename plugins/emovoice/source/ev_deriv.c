/**
* Datei:	deriv.c
* Autor:	Gernot A. Fink
* Datum:	21.4.1997
*
* Beschreibung:	diskrete zeitliche Ableitungen
**/

#include "ev_real.h"

#include "ev_messages.h"


#include "ev_dsp.h"

static mx_real_t p(int i, int t, int a);

/**
* dsp_deriv(d, w, r, gain)
*	Berechnet die diskrete Approximation der Ableitung der Ordnung 'r'
*	des im Verzoegerungselement 'w' gespeicherten Signalausschnitts
*	mit Hilfe von Regressionspolynomen und liefert den Ergebnisvektor in
*	'dx[]' (ggf. um den Faktor 'gain' verstaerkt) sowie den Wert 0,
*	sobald das Verzoegerungselement 'w' bis zur kompletten Laenge gefuellt
*	ist. Andernfalls bleibt 'dx[]' unveraendert und ein Wert von -1 wird
*	zurueckgeliefert.
*
*	Das Verzoegerungselement 'w' muss zuvor mit 'dsp_delay_create()'
*	und einer UNGERADEN Laenge 'length = 2 * t + 1'	erzeugt werden.
*	Die Signalvektoren muessen VOR jedem Aufruf von 'dsp_deriv()'
*	mit 'dsp_delay_push()' in das Verzoegerungselement eingetragen werden.
*
*	Es wird folgende Naeherungsformel verwendet:
*
*		     t
*		    ___
*		    \ 			 (m+j)
*		    /__	  p (j, 2t + 1) x
*			   r		 k
*	  r (m)	   j = -t
*	 d x	= ------------------------------
*	    k	     t
*		    ___
*		    \      2
*		    /__   p (j, 2t + 1)
*			   r
*		   j = -t
*
*	vgl. z.B.: 	E. G. Schukat-Talamazzini: Automatische Spracherkennung,
*				Vieweg, 1995, S. 71.
**/
int dsp_deriv(mx_real_t *dx, dsp_delay_t *w, int r, mx_real_t gain)
	{
	mx_real_t *x, S_p2;
	int i, j, m_plus_j, t;
	
	if (w->need_elems)
		return(-1);

	/* ANNAHME: w->length = 2*t + 1 */
	t = w->length / 2;

	/* Summe -t..t ueber p^2 ... */
	S_p2 = 0.0;
	for (j = -t; j <= t; j++)
		S_p2 += mx_sqr(p(r, j, 2*t + 1));
	/* ... inklusive Verstaerkungsfaktor */
	S_p2 /= gain;

	/*
	 * Ableitungsvektor loeschen (wird zur Zwischenspeicherung
	 * der Werte von Summe -t..t ueber p*x verwendet)
	 */
	for (i = 0; i < w->n_elems; i++)
		dx[i] = 0.0;

	/* Summe -t..t ueber p*x komponentenweise berechnen */
	for (j = -t, m_plus_j = w->tail;
	     j <= t;
	     j++, m_plus_j = (m_plus_j + 1) % w->length) {
		x = w->elem[m_plus_j];

		for (i = 0; i < w->n_elems; i++)
			dx[i] += p(r, j, 2*t + 1) * x[i];
		}

	/* dx[] abschliessend durch S_p2 normieren */
	for (i = 0; i < w->n_elems; i++)
		dx[i] /= S_p2;

	return(0);
	}

/**
* p(i, t, a)
*	Berechnet die Werte der Polynome des zur Ableitungsberechnung
*	verwendeten orthogonalen Polynomsystems, das wie folgt definiert ist:
*
*	  p (t, a)	= 1
*	   0
*
*	  p (t, a)	= t
*	   1
*			   2    1    2
*	  p (t, a)	= t  - --- (a  - 1)
*	   2			12
*
*			   3    1     2
*	  p (t, a)	= t  - --- (3a  - 7) t
*	   3			20
*
*	vgl. z.B.: 	E. G. Schukat-Talamazzini: Automatische Spracherkennung,
*				Vieweg, 1995, S. 71.
**/
static mx_real_t p(int i, int t, int a)
	{
	switch (i) {
		case 0:	return(1);
		case 1:	return(t);
		case 2:	return(t*t - (mx_real_t)(a*a - 1) / 12.0);
		case 3:	return(t*t*t - (mx_real_t)(3*a*a - 7) * t / 20.0);
		default:
			rs_error("derivatives of order %d not supported!", i);
		}
	}
