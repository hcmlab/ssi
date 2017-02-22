/**
* Datei:	basics.c
* Autor:	Gernot A. Fink
* Datum:	23.11.2001
*
* Beschreibung:	Definition globaler Basisdaten
*		(derzeit Versionsnummer der Bibliothek)
**/


#include "ev_basics.h"

char *dsp_version = DSP_VERSION;
char *mx_version = MX_VERSION;
char *rs_version = RS_VERSION;

// @begin_add_johannes
#define FX_KERNEL
char *fx_version = FX_VERSION;
// @end_add_johannes

