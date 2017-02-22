/**
* Datei:	tirol.c
* Autor:	Gernot A. Fink
* Datum:	3.6.1997
*
* Beschreibung:	Glaettung einer Vektorfolge mit dem sogenannten "Tirolerhut"
**/

#include "ev_real.h"


#include "ev_dsp.h"

/**
* dsp_tirol(tirol[], delay)
*	Es wird ein geglaettete Version des mittleren im Verzoegerungselement
*	'delay' gespeicherten Vektors x[m] aus diesem sowie seinem
*	unmittelbaren Vorgaenger x[m-1] und Nachfolger x[m+1] berechnet
*	gemaess:
*
*		tirol = 1/4 * x[m-1] + 1/2 * x[m] + 1/4 x[m+1]
*
*	Liefert im Erfolgsfalle die Daten des geglaetteten Vektors
*	in 'tirol[]' sowie Rueckgabewert 0 und einen negativen Wert sonst.
*
*	ANMERKUNG:
*		'dsp_tirol()' geht davon aus, dass in 'delay' eine UNgerade
*		Anzahl reellwertiger Vektoren gespeichert wurde!
**/
int dsp_tirol(mx_real_t *tirol, dsp_delay_t *delay)
	{
	static mx_real_t __tirol_weight[3] = {0.25, 0.5, 0.25};
	static mx_real_t *tirol_weight = __tirol_weight + 1;

	int m_minus_1, i, j, m_plus_j;
	mx_real_t *x;

	/* Falls 'delay' gefuellt ist ... */
	if (delay->need_elems > 0)
		return(-1);

	/* ... Position des Vorgaengers des mittleren Vektors bestimmen ... */
	m_minus_1 = (delay->head + delay->length / 2) % delay->length;

	/* Summe -1..1 ueber x[] * tirol_weight[] komponentenweise berechnen */
	for (i = 0; i < delay->n_elems; i++)
		tirol[i] = 0.0;
	for (j = -1, m_plus_j = m_minus_1;
	     j <= 1;
	     j++, m_plus_j = (m_plus_j + 1) % delay->length) {
		x = delay->elem[m_plus_j];

		for (i = 0; i < delay->n_elems; i++)
			tirol[i] += x[i] * tirol_weight[j];
		}

	return(0);
	}
