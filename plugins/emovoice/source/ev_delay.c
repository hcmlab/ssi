/**
* Datei:	delay.c
* Autor:	Gernot A. Fink
* Datum:	21.4.1997
*
* Beschreibung:	Verzoegerungselement
**/

#include <string.h>

#include "ev_memory.h"


#include "ev_delay.h"

/**
* dsp_delay_create(length, el_size, n_elems)
*	Erzeugt ein Verzoegerungselement, in dem 'length' Signalvektoren
*	der Dimension 'n_elems' und Basisdatentypgroesse 'el_size'
*	gespeichert werden koennen.
*
*	Liefert einen Zeiger auf das neu erzeugte Verzoegerungselement
*	wenn erfolgreich und NULL sonst.
**/
dsp_delay_t *dsp_delay_create(size_t length, size_t el_size, size_t n_elems)
	{
	dsp_delay_t *delay;
	int i;

	/* Speicher fuer Verzoegerungselement bereitstellen ... */
	delay = rs_malloc(sizeof(dsp_delay_t), "delay element");

	/* ... und initialisieren ... */
	delay->length = length;
	delay->el_size = el_size;
	delay->n_elems = n_elems;
	delay->head = -1;
	delay->tail = 0;
	delay->need_elems = delay->length;

	/* ... sowie fuer verzoegerte Signalvektoren ... */
	delay->elem = rs_malloc(delay->length * sizeof(void *),
				"delay entry list");

	for (i = 0; i < delay->length; i++)
		delay->elem[i] = rs_malloc(delay->n_elems * el_size,
					"delay entry");

	/* ... und optionale Marken */
	delay->mark = rs_malloc(delay->length * sizeof(int),
				"delay entry mark list");

	return(delay);
	}

void dsp_delay_destroy(dsp_delay_t *delay)
	{
	int i;

	/* Parameter pruefen ... */
	if (!delay)
		return;

	/* ... Datenbereich unbrauchbar machen und freigeben */
	if (delay->elem) {
		for (i = 0; i < delay->length; i++)
			if (delay->elem[i])
				rs_free(delay->elem[i]);

		rs_free(delay->elem);
		}
	if (delay->mark)
		rs_free(delay->mark);
	memset(delay, -1, sizeof(dsp_delay_t));
        rs_free(delay);
	}
	

/**
* dsp_delay_flush(delay)
*	Leert das Verzoegerungselement 'delay', so dass bis zur vollstaendigen
*	Fuellung wieder 'delay->length' Vektoren eingetragen werden muessen.
**/
void dsp_delay_flush(dsp_delay_t *delay)
	{
	delay->head = -1;
	delay->tail = 0;
	delay->need_elems = delay->length;
	}

/**
* dsp_delay_pushm(delay, elems[], mark)
*	Traegt den Datenvektor 'elems[]' zusammen mit der Marke 'mark' ins
*	Verzoegerungselement 'delay' ein.
*
*	Liefert die Anzahl der Elemente, die bis zur vollstaendigen
*	Fuellung des Verzoegerungspuffers noch benoetigt werden, also
*	eine ganze Zahl zwischen 'delay-length - 1' beim ersten Aufruf
*	und 0, sobald einmal 'delay->length' Vektoren eingetragen wurden.
**/
int dsp_delay_pushm(dsp_delay_t *delay, void *elems, int mark)
	{
	/* Zeiger im Ringpuffer weiterschalten ... */
	delay->head = (delay->head + 1) % delay->length;
	if (delay->need_elems)
		delay->need_elems--;
	else	delay->tail = (delay->tail + 1) % delay->length;

	/* ... neuen Vektor ... */
	memcpy(delay->elem[delay->head], elems, delay->n_elems*delay->el_size);

	/* ... und zugehoerige Marke eintragen */
	delay->mark[delay->head] = mark;

	return(delay->need_elems);
	}

int dsp_delay_push(dsp_delay_t *delay, void *elems)
	{
	return(dsp_delay_pushm(delay, elems, 0));
	}

/**
* dsp_delay_popm(elems[], *mark, delay)
*	Fuellt 'elems[]' mit dem Inhalt und 'mark' mit der gespeicherten
*	Marke des aeltesten Datenvektors und entfernt diesen aus dem
*	Verzoegerungselement 'delay'.
*
*	Falls 'elems[]' oder '*mark' NULL sind unterbleibt die Eintragung
*	und der Vektor wird nur aus dem Verzoegerungselement entfernt.
*
*	Liefert die Anzahl der im Verzoegerungselement verbleibenden
*	Datevektoren und einen negativen Wert im Fehlerfalle
*	(Rueckgabewert -1 entsteht, falls 'delay' keine Vektoren enthielt!).
**/
int dsp_delay_popm(void *elems, int *mark, dsp_delay_t *delay)
	{
	/* Falls das Verzoegerungselement leer ist ... */
	if (delay->need_elems >= delay->length)
		return(-1);

	/* ... ggf. Daten aus letztem Vektor kopieren ... */
	if (elems)
		memcpy(elems, delay->elem[delay->tail],
			delay->n_elems*delay->el_size);

	/* ... ggf. Marke uebertragen ... */
	if (mark)
		*mark = delay->mark[delay->tail];

	/* ... und Zeiger im Ringpuffer weiterschalten */
	delay->tail = (delay->tail + 1) % delay->length;
	delay->need_elems++;

	return(delay->length - delay->need_elems);
	}

int dsp_delay_pop(void *elems, dsp_delay_t *delay)
	{
	return(dsp_delay_popm(elems, NULL, delay));
	}

/**
* dsp_delay_topm(elems[], *mark, delay)
*	Fuellt 'elems[]' mit dem Inhalt und 'mark' mit der gespeicherten
*	Marke des aeltesten Datenvektors aus dem Verzoegerungselement 'delay'.
*
*	Falls 'elems[]' oder '*mark' NULL sind unterbleibt die Eintragung.
*
*	Liefert die Anzahl der im Verzoegerungselement verbleibenden
*	Datenvektoren und einen negativen Wert im Fehlerfalle
*	(Rueckgabewert -1 entsteht, falls 'delay' keine Vektoren enthaelt!).
**/
int dsp_delay_topm(void *elems, int *mark, dsp_delay_t *delay)
	{
	/* Falls das Verzoegerungselement leer ist ... */
	if (delay->need_elems >= delay->length)
		return(-1);

	/* ... ggf. Daten aus letztem Vektor kopieren ... */
	if (elems)
		memcpy(elems, delay->elem[delay->tail],
			delay->n_elems*delay->el_size);

	/* ... ggf. Marke uebertragen ... */
	if (mark)
		*mark = delay->mark[delay->tail];

	return(delay->length - delay->need_elems);
	}

int dsp_delay_top(void *elems, dsp_delay_t *delay)
	{
	return(dsp_delay_topm(elems, NULL, delay));
	}

/**
* dsp_delay_accessm(elems[], *mark, delay, offset)
*	Ermoeglicht wahlfreien Zugriff auf einen 'offset' Frames in
*	der "Vergangenheit" liegenden Eintrag im Verzoegerungselement 'delay'.
*
*	Liefert im Erfolgsfalle die Daten des betreffenden Signalvektors
*	in 'elems[]', die zugehoerige Marke in '*mark' sowie den
*	Rueckgabewert 0 und einen negativen Wert im Fehlerfalle.
**/
int dsp_delay_accessm(void *elems, int *mark, dsp_delay_t *delay, int offset)
	{
	int pos;

	/* Falls 'offset' im adressierbaren Bereich liegt ... */
	if (delay->length - delay->need_elems <= offset)
		return(-1);

	/* ... die Position des Signalvektors bestimmen ... */
	pos = (delay->head - offset + delay->length) % delay->length;

	/* ... ggf. Daten ... */	
	if (elems)
		memcpy(elems, delay->elem[pos], delay->n_elems*delay->el_size);

	/* ... und ggf. Marke kopieren */
	if (mark)
		*mark = delay->mark[pos];

	return(0);
	}

int dsp_delay_access(void *elems, dsp_delay_t *delay, int offset)
	{
	return(dsp_delay_accessm(elems, NULL, delay, offset));
	}

/**
* dsp_delay_mark(delay, offset, mark)
*	Ersetzt die Marke eines 'offset' Frames in der "Vergangenheit"
*	liegenden Eintrags im Verzoegerungselement 'delay' durch 'mark'.
*
*	Liefert im Erfolgsfalle den Rueckgabewert 0 und einen negativen
*	Wert sonst.
**/
int dsp_delay_mark(dsp_delay_t *delay, int offset, int mark)
	{
	int pos;

	/* Falls 'offset' im adressierbaren Bereich liegt ... */
	if (delay->length - delay->need_elems <= offset)
		return(-1);

	/* ... die Position des Eintrags bestimmen ... */
	pos = (delay->head - offset + delay->length) % delay->length;

	/* ... und Marke ersetzen */
	delay->mark[pos] = mark;

	return(0);
	}

/**
* void *_dsp_delay_peek(delay, offset)
*	Ermoeglicht wahlfreien Zugriff auf einen 'offset' Frames in
*	der "Vergangenheit" liegenden Eintrag im Verzoegerungselement 'delay',
*	kopiert diesen aber nicht, sondern liefert nur einen - voruebergehend
*	gueltigen - Zeiger darauf.
**/
void *_dsp_delay_peek(dsp_delay_t *delay, int offset)
	{
	int pos;

	/* Falls 'offset' im adressierbaren Bereich liegt ... */
	if (delay->length - delay->need_elems <= offset)
		return(NULL);

	/* ... die Position des Signalvektors bestimmen ... */
	pos = (delay->head - offset + delay->length) % delay->length;

	/* ... und Zeiger auf betreffende Daten als Ergebnis liefern */	
	return(delay->elem[pos]);
	}

