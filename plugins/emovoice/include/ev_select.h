/**
* Datei:	select.h
* Autor:	Gernot A. Fink
* Datum:	14.2.2005
*
* Beschreibung:	Basis-Definitionen fuer Merkmalsselektion
**/

#ifndef __FX_SELECT_H_INCLUDED__
#define __FX_SELECT_H_INCLUDED__

#ifdef FX_KERNEL
#include "ev_basics.h"
#else
#include "ev_basics.h"
#endif

/*
 * Konstantendefinitionen
 */

/*
 * globale Variable
 */

/*
 * Datentypen
 */
/* Typ einer Merkmalsselektionsvorschrift */
typedef struct {
	int n_features;		/* Gesamtanzahl der Vektorkomponenten .. */
	int n_selected;		/* ... sowie der ausgewaehlten Eintraege */
	int *selected;		/* characteristische Funktion der Selektion */
	} fx_select_t;

/*
 * Prototypen
 */
fx_select_t *fx_select_create(int n_features);
void fx_select_destroy(fx_select_t *select);
int fx_select_fprint(FILE *fp, fx_select_t *select);
int fx_select_sscan(fx_select_t *select, char *s);
int fx_select_apply(fx_feature_t **dest, fx_select_t *select, fx_feature_t *src);

#endif /* __FX_SELECT_H_INCLUDED__ */
