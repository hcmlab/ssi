/**
* Datei:	acf.c
* Autor:	Thomas Ploetz, Sun Nov 23 14:02:38 2003
* Time-stamp:	<2005-09-14 11:14:06 tploetz>
*
* Beschreibung:	Autokorrelationsfunktion
**/

#include <stdlib.h>
#include <math.h>

#include "ev_real.h"

#include "ev_messages.h"

/**
* dsp_acf(t[], f[], n, m)
*	Berechnet die Autokorrelationsfunktion des reellen Signals 'f[]' der
*	Laenge 'n' und speichert die ersten 'm' Ergebniskoeffizienten in 't[]'.
**/
void dsp_acf(mx_real_t *t, mx_real_t *f, size_t n, size_t m)
	{
	int i, j;

	if (!t)
		rs_error("Uninitialized ACF memory!");

	for (j = 0; j < m; j++) {
		t[j] = 0.0;
		for (i = 0; i < n; i++)
			if (i-j >= 0)
				t[j] += f[i] * f[i-j];
		/* 		t[j] *= -1; */
		}
	}
