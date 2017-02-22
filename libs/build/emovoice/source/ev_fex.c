// ev_fex.c
// author: Thurid Vogt <thurid.vogt@informatik.uni-augsburg.de>
// created: 
// Copyright (C) 2003-9 University of Augsburg, Thurid Vogt
//
// This file depends in part on code of the ESMERALDA framework for speech recognition
// of Gernot Fink and Thomas Ploetz, available at http://sourceforge.net/projects/esmeralda/
// under the GNU General Public License
// *************************************************************************************************
//
// This file is part of EmoVoice/SSI developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include <stdio.h>
#include <string.h>
#if _WIN32||_WIN64
#include <winsock.h>
#endif
#include <limits.h>

#include "ev_messages.h"
#include "ev_io.h"

#include "ev_basics.h"
#include "ev_real.h"


#include "ev_basics.h"
#include "ev_dsp.h"

#define FEX_VERSION	"1.10e"
#define FEX_USAGE	"[<option> ...] <type> <version> [<src> [<dest>]]"
#define FEX_HELP	"\
    where\n\
	<type>	specifies the type of feature extraction to use.\n\
		(Available types are: 'mfcc')\n\
	<version> specifies the feature extraction version to be used.\n\
	<src>	specifies the source signal file; Standard input is used\n\
		if <src> is either omitted or set to '-'.\n\
	<dest>	specifies the destiation feature file. Standard output is\n\
		used if <dest> is either omitted or set to '-'.\n\
    valid options are\n\
	-b	process data in batch mode reading a list of file-name pairs\n\
		holding signal data and feature data, respectively, from\n\
		<src> (or standard input if <src> is omitted or set to '-')\n\
	-c	start with the first frame centered\n\
	-p <param> configure the feature extraction using parameters <param>\n\
	-P	print changed configuration parameters to standard output\n\
    general options are\n\
	-h	display usage information\n\
	-v	be more verbose (can be used multiple times)\n\
	-V	display version information\n\
"

#define FEX_STDIO_NAME	"-"

#define MAX_SAMPLES	1024
#define MAX_FEATURES	256

int fex_batch = 0;
int fex_print_param = 0;
int fex_verbose = 0;

int centered = 0;

int fextract(FILE* dest_fp, dsp_fextract_t *fex, FILE *source_fp);
int next_frame(dsp_sample_t *s, dsp_sample_t *last_s,
		int f_length, int f_shift, FILE *fp);

int fextract(FILE* dest_fp, dsp_fextract_t *fex, FILE *source_fp)
	{
	dsp_sample_t s[MAX_SAMPLES];
	dsp_sample_t last_s[MAX_SAMPLES];
	dsp_sample_t *frame, *last_frame, *__tmp;
	mx_real_t f[MAX_FEATURES];

	int frame_len, frame_shift;
	int n_features;
	int n_samples;
	int frames = 0;

	/* Parameter bestimmen ... */
	frame_len =	fex->frame_len * fex->n_channels;
	frame_shift =	fex->frame_shift * fex->n_channels;
	n_features =	fex->n_features;

	/* ersten Frame einlesen ... */
	frame = s; last_frame = last_s;
	n_samples = next_frame(frame, NULL, frame_len, frame_shift, source_fp);

	while (n_samples > 0) {
		frames++;

		n_features = dsp_fextract_calc(fex, f, frame);

		if (dest_fp && n_features > 0) {
			/* ... Daten ausgeben ... */
			fwrite (f, sizeof(mx_real_t), n_features, dest_fp);
			}

		/* ... falls aktueller Frame unvollstaendig, abbrechen! */
		if (n_samples < frame_len)
			break;

		/* ... und naechsten Frame einlesen */
		__tmp = last_frame;
		last_frame = frame;
		frame = __tmp;

		n_samples = next_frame(frame, last_frame,
				frame_len, frame_shift, source_fp);
		}

	return(frames);
	}

/**
* next_frame(s[], last_s[], f_length, f_shift, fp)
*	Liest Abtastwerte fuer den naechsten 'f_length' Samples langen Frame
*	aus der Datei 'fp' in 's[]' ein. Der sich bei einer Fortschaltrate
*	'f_shift', die kleiner als die Frame-Laenge 'f_length' ist,
*	ergebende Ueberlappungsbereich wird dabei aus dem
*	Vorgaenger-Frame 'last_s[]' uebernommen, sofern dieser angegeben ist.
*
*	Liefert die Anzahl der tatsaechlich gelesenen Abtastwerte oder Null,
*	wenn nicht ausreichend, d.h. < 'f_shift' Werte gelesen wurden.
**/
int next_frame(dsp_sample_t *s, dsp_sample_t *last_s,
		int f_length, int f_shift, FILE *fp)
	{
	int i, n_samples = 0;

	/* Falls ein Vorgaenger-Frame existiert ... */
	if (last_s && f_shift < f_length) {
		/* ... Ueberlappungsbereich kopieren ... */
		memcpy(s, last_s + f_shift,
			(f_length - f_shift) * sizeof(dsp_sample_t));
		n_samples = (f_length - f_shift);
		}
	/* ... sonst, falls Frames "zentriert" werden sollen ... */
	else if (centered) {
		/* ...  Ueberlappungsbereich loeschen ... */
		n_samples = (f_length - f_shift) / 2;
		for (i = 0; i < n_samples; i++)
			s[i] = 0;
		}

	/* ... Frame mit Abtastwerten auffuellen ... */
	n_samples += fread(s + n_samples, sizeof(dsp_sample_t),
			f_length - n_samples, fp);

	/* ... bei nicht ausreichender "Fuellung" -> Ende ... */
	if (n_samples < f_shift)
		return(0);

	/* ... sonst evtl. verbleibenden Bereich loeschen ... */
	for (i = n_samples; i < f_length; i++)
		s[i] = 0;

	/* ... und # tatsaechlich gelesener Abtastwerte zurueckliefern */
	return(n_samples);
	}
