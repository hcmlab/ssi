/**
* Datei:	dsp.h
* Autor:	Gernot A. Fink
* Datum:	ca. 10.4.1997 & 31.8.2000
*
* Beschreibung:	Definitionen fuer Verzoegerungselement
*
* letzte wichtige Aenderungen:
*
*	1.9.2000, Gernot A. Fink:
*		Moeglichkeit zur Speicherung von Markierungen der
*		im Verzoegerungselement enthaltenen Datenvektoren
**/

#ifndef __DSP_DELAY_H_INCLUDED__
#define __DSP_DELAY_H_INCLUDED__

//#include "ev_unistd.h"    /* for 'size_t' */

/* Typ eines Verzoegerungselements ... */
typedef struct {
	size_t length;		/* ... Laenge des Signalpuffers ... */
	size_t el_size;		/* ... Groesse in Bytes sowie ... */
	size_t n_elems;		/* ... Dimension der Signalvektoren, ... */
	int head;		/* ... Zeiger auf Anfang und ... */
	int tail;		/* ... Ende des internen Rinpuffers, ... */
	size_t need_elems;	/* ... # Vektoren bis zur Fuellung ... */
	void **elem;		/* ... Zeiger auf gespeicherte Daten ... */
	int *mark;		/* ... sowie optionaler Markierungen */
	} dsp_delay_t;

/*
 * Funktionsprototypen
 */
dsp_delay_t *dsp_delay_create(size_t length, size_t el_size, size_t n_elems);
void dsp_delay_destroy(dsp_delay_t *delay);
void dsp_delay_flush(dsp_delay_t *delay);
int dsp_delay_push(dsp_delay_t *delay, void *elems);
int dsp_delay_pushm(dsp_delay_t *delay, void *elems, int mark);
int dsp_delay_pop(void *elems, dsp_delay_t *delay);
int dsp_delay_popm(void *elems, int *mark, dsp_delay_t *delay);
int dsp_delay_top(void *elems, dsp_delay_t *delay);
int dsp_delay_topm(void *elems, int *mark, dsp_delay_t *delay);
int dsp_delay_access(void *elems, dsp_delay_t *delay, int offset);
int dsp_delay_accessm(void *elems, int *mark, dsp_delay_t *delay, int offset);
int dsp_delay_mark(dsp_delay_t *delay, int offset, int mark);
void *_dsp_delay_peek(dsp_delay_t *delay, int offset);

#endif /* __DSP_DELAY_H_INCLUDED__ */
