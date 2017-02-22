/**
* Datei:	mel.c
* Autor:	Gernot A. Fink
* Datum:	13.5.1997
*
* Beschreibung:	div. Funktionen zur Erzeugung einer gehoerrichtig
*		verzerrten Frequenzskala
**/

#include "ev_real.h"


#include "ev_dsp.h"

/**
* dsp_mel_create(mel_freq[], f_width[], d_freq, f_min,f_max, scale, max_fgroups)
*	Erzeugt bei einer Frequenzaufloesung von 'd_freq' Hz einen maximal
*	'max_fgroups'-elementigen Vektor von Frequenzen 'mel_freqf[]' und
*	zugehoerigen Frequensgruppenbreiten 'f_width[]', die aequidistant auf
*	der Mel- bzw. Bark-Skala angeordnet sind, so dass der Abstand zur
*	jeweils niedrigeren Nachbarfrequenz die halbe Frequenzgruppenbreite
*	betraegt, evtl. skaliert um den Faktor 'scale'.
*	Zusaetzlich wird der "gueltige" Frequenzbereich
*	im Intervall ['f_min'...'f_max'] begrenzt.
*
*	Liefert die Anzahl der tatsaechlich erzeugten Frequenzgruppen.
*
*	Beachte:
*		Die Erzeugung des Frequenzgruppenbreitenvektors ist optional
*		und kann durch Angabe eines NULL-Pointers unterbunden werden.
**/
int dsp_mel_create(mx_real_t *mel_freq, mx_real_t *f_width,
		mx_real_t d_freq, mx_real_t f_min, mx_real_t f_max,
		mx_real_t scale, int max_fgroups)
	{
	mx_real_t f;
	mx_real_t last_mel_freq, f_width_2;
	int fgroup = 0;

	for (f = d_freq * (int)((f_min + d_freq / 2) / d_freq),
		last_mel_freq = f - d_freq,
		fgroup = 0;
	     f < f_max - d_freq;
	     f += d_freq) {
		f_width_2 = dsp_f_width(f) * scale / 2;

		if (last_mel_freq + f_width_2 > f)
			continue;

		mel_freq[fgroup] = last_mel_freq = f;
		if (f_width)
			f_width[fgroup] = f_width_2 * 2;
		fgroup++;

		if (fgroup >= max_fgroups)
			break;
		}

	return(fgroup);
	}

/**
* dsp_f_width(f)
*	Berechnet die Frequenzgruppenbreite in Hz fuer die Frequenz 'f'.
*
*	ANMERKUNG:
*		Die Frequenzgruppenbreite ergibt sich als:
*
*
*			delta f
*			       G		       f  2  0.69
*			-------- = 25 + 75 ( 1 + 1.4 (---)  )
*			   Hz			      kHz
*
*		vgl. E. Zwicker: Psychoakustik, Springer, 1982, S. 67.
**/
mx_real_t dsp_f_width(mx_real_t f)
	{
	return(25.0 + 75.0 * pow(1 + 1.4 * f*f/(1000.0*1000.0), 0.69));
	}
