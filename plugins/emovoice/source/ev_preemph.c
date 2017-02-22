/**
* Datei:	preemph.c
* Autor:	Gernot A. Fink
* Datum:	28.8.2000
*
* Beschreibung:	Praeemphase
**/

#include "ev_real.h"

#include "ev_memory.h"


#include "ev_dsp.h"

/**
* dsp_preemph(p, s, n, a, s_minus_1)
*	Wendet einen Praeemphasefilter mit Parameter 'a' auf das Signal 's'
*	der Laenge 'n' an und speichert das Ergebnis in 'p'.
*
*	Zur Kompensation von Effekten der Fensterbildung im Signal kann
*	der dem aktuellen Frame vorangegangene Abtastwert 's_minus_1'
*	angegeben werden. Ausserdem liefert 'dsp_preemph()' den letzten
*	Wert des Frames als Ergebnis.
*
*	HINWEIS:
*		Die Signalausschnitte 'p' und 's' duerfen identisch sein!
*		Das Ergebnis wird korrekt im Ausgangsdatenbereich abgelegt.
*
*	ANMERKUNG:
*		Die Filterausgabe der Praeemphase berechnet sich gemaess:
*
*			p  = s  - a s		0 <= i < n
*			 i    i	     i-1
*
*		wobei fuer 'a' gilt:
*
*			0.9 <= a <= 1.0
*
*		vgl. z.B.:
*			L. Rabiner & B.-H. Juang: Fundamentals of Speech
*				Recognition, Prentice Hall, 1993, S. 112f.
*			E. G. Schukat-Talamazzini: Automatische Spracherkennung,
*				Vieweg, 1995, S. 64.
**/
dsp_sample_t dsp_preemph(dsp_sample_t *p, dsp_sample_t *s, size_t n,
			mx_real_t a, dsp_sample_t s_minus_1)
	{
	dsp_sample_t s_n_minus_1;
	int i;

	s_n_minus_1 = s[n - 1];

	for (i = n - 1; i > 0; i--)
		p[i] = s[i] - (dsp_sample_t)(a * (mx_real_t)s[i - 1]);

	p[0] = s[0] - (dsp_sample_t)(a * (mx_real_t)s_minus_1);

	return(s_n_minus_1);
	}
