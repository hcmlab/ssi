/**
* Datei:	fextract.c
* Autor:	Gernot A. Fink
* Datum:	5.12.1998
*
* Beschreibung:	Merkmalsberechnung verschiedener Typen und Versionen
**/

#include "ev_real.h"

#include "ev_memory.h"
#include "ev_messages.h"


#include "ev_dsp.h"
#include "ev_fextract.h"
#include "ev_mfcc.h"

// @begin_add_johannes
//#include "ev_fft.c"
// @end_add_johannes

/* Definitionstexte fuer Merkmalsberechnungstypen (siehe 'dsp_fextype_t') */
char *dsp_fextype_text[] = {
	"Undefined",
	"MFCC",
	};

dsp_fextract_t *dsp_fextract_create(dsp_fextype_t type, int version,
		char *param)
	{
	dsp_fextract_t *fex;

	/* Datenstrukturen erzeugen ... */
	fex = rs_malloc(sizeof(dsp_fextract_t), "feature extraction data");

	/* ... sowie allgemeine ... */
	fex->type = type;
	fex->version = version;

	/* ... und typespezifische Daten initialisieren */
	switch (fex->type) {
		case dsp_fextype_MFCC:
			if (!dsp_mfcc_create(fex, param)) {
				rs_free(fex);
				fex = NULL;
				}
			break;
		default:
			rs_error("unknown feature extraction type %d!", type);
		}

	return(fex);
	}

void dsp_fextract_destroy(dsp_fextract_t *fex)
	{
	/* Parameter pruefen ... */
	if (!fex)
		return;

	switch (fex->type) {
		case dsp_fextype_MFCC:
			dsp_mfcc_destroy(fex);
			break;
		default:
			rs_error("illegal feature extraction data can't be destroyed!");
		}

	rs_free(fex);

	// @begin_add_johannes
	_dsp_fft_destroy ();
	// @end_add_johannes

	}

void dsp_fextract_reset(dsp_fextract_t *fex)
	{
	/* Parameter pruefen ... */
	if (!fex)
		return;

	switch (fex->type) {
		case dsp_fextype_MFCC:
			dsp_mfcc_reset(fex);
			break;
		default:
			rs_error("illegal feature extraction data can't be reset!");
		}
	}

int dsp_fextract_calc(dsp_fextract_t *fex,
		mx_real_t *features, dsp_sample_t *samples)
	{
	/* Parameter pruefen ... */
	if (!fex || !features || !samples)
		return(-1);

	switch (fex->type) {
		case dsp_fextype_MFCC:
			return(dsp_mfcc_calc(fex, features, samples));
		default:
			rs_error("illegal feature extraction data for calculation!");
		}
	}

int dsp_fextract_fprintparam(FILE *fp, dsp_fextract_t *fex, char *key)
	{
	/* Parameter pruefen ... */
	if (!fp || !fex)
		return(-1);

	switch (fex->type) {
		case dsp_fextype_MFCC:
			return(dsp_mfcc_fprintparam(fp, fex, key));
		default:
			rs_error("illegal feature extraction data for printing parameters!");
		}
	}
