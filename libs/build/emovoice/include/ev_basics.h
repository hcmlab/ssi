/**
* Datei:	basics.h
* Autor:	Gernot A. Fink
* Datum:	11.3.2004
*
* Beschreibung:	Basis-Definitionen fuer DSP
**/

#ifndef __BASICS_H_INCLUDED__
#define __BASICS_H_INCLUDED__


// @begin_add_johannes
#include "ev_real.h"
// @end_add_johannes

#define DSP_VERSION	"1.60"
#define RS_VERSION	"1.10a"
#define MX_VERSION "1.23c"

/*
 * Konstantendefinitionen
 */
#define DSP_COMMENT_CHAR	'#'		/* Kommentarmarkierung */
#define MX_COMMENT_CHAR	'#'		/* Kommentarmarkierung */
// @begin_add_johannes
#define FX_VERSION	"1.10b"
#define FX_COMMENT_CHAR	'#'		/* Kommentarmarkierung */
// @end_add_johannes

/*
 * globale Variable
 */
extern char *dsp_version;		/* mit DSP_VERSION belegt */
extern char *mx_version;		/* mit MX_VERSION belegt */
extern char *rs_version;		/* mit RS_VERSION belegt */
// @begin_add_johannes
extern char *fx_version;		/* mit FX_VERSION belegt */
// @end_add_johannes

// @begin_add_johannes
typedef mx_real_t fx_feature_t;
// @end_add_johannes

#endif /* __BASICS_H_INCLUDED__ */
