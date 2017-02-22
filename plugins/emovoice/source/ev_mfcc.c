/**
* Datei:	mfcc.c
* Autor:	Gernot A. Fink
* Datum:	5.12.1998 (ausgekoppelt aus 'fextract.c)
*
* Beschreibung:	Berechnung von MFCCs (Mel-Frequency-Cepstral-Coefficients)
*		in verschiedenen Versionen
*
*	Versionen 1.x von 'fextract.c':
*		1.1:	in Anlehnung an Erlangen/Ulmer Merkmalsberechnung
*			(T. Kuhn: Die Erkennungsphase in einem Dialogsystem,
*				Bd. 80 von Dissertationen zur kuenstlichen
*				Intelligenz, infix, 1995, S. 228ff.)
*		1.2:	zusaetzlich Kanaladaption durch Bereinigung des
*			cepstralen Mittelwerts und Minimum/Maximum-Normierung
*			des Energieverlaufs
*		1.3:	wie 1.2 aber mit uniformer Kurzzeitnormalisierung
*			des cepstralen Mittelwerts
*
*		1.4:	wie Version 1.3 aber mit Praeemphase, neuer signal-
*			basierter Energieberechnung, histogrammbasierter
*			inkrementeller Schaetzung der Energiedynamik und
*			Normierung des Energiemaximums auf das 95%-Quantil
*			der Verteilung
*		1.41:	wie 1.4 jedoch werden keine Cepstren berechnet
*		        (d.h. nur das log-mel-Spektrum) und der Kanal in
*			dieser Repraesentation kompensiert
*
*
*		1.5:	wie Version 1.3 aber mit Vorwaertsmaskierung, 
*                       neuer signalbasierter Energieberechnung, 
*                       histogrammbasierter
*			inkrementeller Schaetzung der Energiedynamik und
*			Normierung des Energiemaximums auf das 95%-Quantil
*			der Verteilung
*		1.6:	wie Version 1.4 jedoch mit hoeherer Frequenzaufloesung
*			(1024er DFT; daher auch etwas groesserer Mel-Filterbank)
*
*		spaeter auch:
*			Kompensation des Langzeitmittelwerts des
*			Leistungsspektrums (i.e. "spectral subtraction" zur
*			Geraruschunterdrueckung)
*
*		2.4:	wie Version 1.4 aber fuer Stereo-Signale mit
*			einfachem "beamforming"
*
*		2.5:	wie Version 1.5 aber fuer Stereo-Signale
*                       muss noch uebersichtlicher gestaltet werden !!!
*
**/

#include <string.h>

#include "ev_memory.h"
#include "ev_messages.h"
#include "ev_io.h"

// @begin_add_johannes
#include "ev_dsp.h"
// @end_add_johannes


#include "ev_mfcc.h"
#include "ev_fw_masking.h"

/* Konstantendefinitionen fuer ... */
/* ... Merkmalsberechnung V1.1: */
#define V1_1_SAMPLERATE		16000
#define V1_1_N_CHANNELS		1
#define V1_1_FRAME_LEN		256
#define V1_1_FRAME_SHIFT	160
#define V1_1_N_FRESOLUTION	((mx_real_t)V1_1_SAMPLERATE / V1_1_FRAME_LEN)
#define V1_1_MIN_FREQ		(3 * V1_1_N_FRESOLUTION)
#define V1_1_MAX_FREQ		(V1_1_SAMPLERATE / 2)
#define V1_1_N_FILTERS		31
#define V1_1_W_LENGTH		5
#define V1_1_N_BASEFEATURES	(1 + 12)
#define V1_1_N_FEATURES		(3 * V1_1_N_BASEFEATURES)

/* ... und zusaetzlich fuer V1.2: */
#define V1_2_I_ENERGY		0
#define V1_2_MINTIME		500
#define V1_2_MAXTIME		2000
#define V1_2_DECAY		0.0001
#define V1_2_ENERGY_PERCENT	0.5

/* ... und zusaetzlich fuer V1.3: */
#include "ev_mfcc_1_3.h"

/* ... und zusaetzlich fuer V1.4: */
#include "ev_mfcc_1_4.h"

/* ... und zusaetzlich fuer V1.5: */
#define V1_5_FFT_LEN            1024
#define V1_5_N_FILTERS         34
#define V1_5_N_FRESOLUTION	((mx_real_t)V1_1_SAMPLERATE / V1_5_FFT_LEN)

#define	V1_5_EN_HIST_RES	0.25 /*Geht besser bei Maskierung*/

/* ... Vorwaertsmaskierung ... */
#define V1_5_DB_MULTIPLIKATOR 10

/*#define V1_5_DB_SPRACHE    60*/
#define V1_5_DB_SPRACHE       ((mx_real_t) 50 - ((V1_5_EN_HIST_RES / 2.0) * V1_5_DB_MULTIPLIKATOR))   /*Wenn mit Maskierung und HIST_RES 0.25 + 1024 FFT*/ 

/* ... zusaetzlich fuer V1.6 */
#include "ev_mfcc_1_6.h"

/* ... zusaetzlich fuer V2.4 (V1.4 stereo): */
#define V2_4_N_CHANNELS		2

/* ... und zusaetzlich fuer V2.5 (V1.5 stereo): */
#define V2_5_N_CHANNELS		2
#define	V2_5_EN_HIST_LIMIT	200
#define V2_5_MIN_GAIN           0.001
#define V2_5_MAX_GAIN           0.999

	/*
	 * vordefinierte Kanalparameter fuer (Energie gemaess 1.2/1.3):
	 */
/* Gradient Desklab */
static mx_real_t chparam_desklab[1 + 2 + V1_1_N_BASEFEATURES] = {
	0,
	0, 6,
	0, 2.93657, 0.312502, 1.38028, -0.201216, 0.315178, -0.22886,
	0.76584, -0.312766, 0.491137, -0.380584, 0.513988, -0.619026
	};
static mx_real_t enparam_desklab[2] = {2.0, 6.0};	/* ... Energie fuer 1.4 */

/* DIGIAL UNIX MME */ /** HACK HACK: derzeit == desklab **/
static mx_real_t chparam_mme[1 + 2 + V1_1_N_BASEFEATURES] = {
	0,
	0, 6,
	0, 2.93657, 0.312502, 1.38028, -0.201216, 0.315178, -0.22886,
	0.76584, -0.312766, 0.491137, -0.380584, 0.513988, -0.619026
	};
static mx_real_t enparam_mme[2] = {2.0, 6.0};	/* ... Energie fuer 1.4 */

/* Linux /dev/dsp */ /** HACK HACK: derzeit == desklab **/
static mx_real_t chparam_dev_dsp[1 + 2 + V1_1_N_BASEFEATURES] = {
	0,
	0, 6,
	0, 2.93657, 0.312502, 1.38028, -0.201216, 0.315178, -0.22886,
	0.76584, -0.312766, 0.491137, -0.380584, 0.513988, -0.619026
	};
static mx_real_t enparam_dev_dsp[2] = {2.0, 6.0};	/* ... Energie fuer 1.4 */


	/*
	 * vordefinierte Kanalparameter fuer V1.5:
	 */
/* Gradient Desklab */
static mx_real_t chparam_desklab_2[1 + 2 + 1 + V1_5_N_FILTERS] = {
         0, 
	 5.0, 7.5,
	 0, 4.68116, 4.52999, 4.38691, 4.2333, 4.24544, 4.25587, 4.29393,
	 4.24197, 4.10046, 3.97659, 4.22306, 3.90903, 3.69334, 4.05346, 
	 4.04103, 4.05424, 4.26893, 4.30526, 4.25285, 3.91894, 3.68798, 
	 3.37972, 3.41668, 3.39903, 3.33286, 3.31728, 3.47433, 3.75006,
	 3.89708, 3.56624, 3.1153, 3.38988, 2.92058, 2.75598
	};

/* DIGIAL UNIX MME */ /** HACK HACK: derzeit == desklab **/
static mx_real_t chparam_mme_2[1 + 2 + 1 + V1_5_N_FILTERS] = {
         0, 
	 5.0, 7.5,
	 0, 4.68116, 4.52999, 4.38691, 4.2333, 4.24544, 4.25587, 4.29393,
	 4.24197, 4.10046, 3.97659, 4.22306, 3.90903, 3.69334, 4.05346, 
	 4.04103, 4.05424, 4.26893, 4.30526, 4.25285, 3.91894, 3.68798, 
	 3.37972, 3.41668, 3.39903, 3.33286, 3.31728, 3.47433, 3.75006,
	 3.89708, 3.56624, 3.1153, 3.38988, 2.92058, 2.75598
	};

/* Linux /dev/dsp */ /** HACK HACK: derzeit == desklab **/
static mx_real_t chparam_dev_dsp_2[1 + 2 + 1 + V1_5_N_FILTERS] = {
         0, 
	 5.0, 7.5,
	 0, 4.68116, 4.52999, 4.38691, 4.2333, 4.24544, 4.25587, 4.29393,
	 4.24197, 4.10046, 3.97659, 4.22306, 3.90903, 3.69334, 4.05346, 
	 4.04103, 4.05424, 4.26893, 4.30526, 4.25285, 3.91894, 3.68798, 
	 3.37972, 3.41668, 3.39903, 3.33286, 3.31728, 3.47433, 3.75006,
	 3.89708, 3.56624, 3.1153, 3.38988, 2.92058, 2.75598
	};

static int _dsp_mfcc_1_1(dsp_fextract_t *fex,
		mx_real_t *features, dsp_sample_t *signal);
static int _dsp_mfcc_1_2(dsp_fextract_t *fex,
		mx_real_t *features, dsp_sample_t *signal);
static int _dsp_mfcc_1_5(dsp_fextract_t *fex,
		mx_real_t *features, dsp_sample_t *signal);

static int _dsp_mfcc_2_4(dsp_fextract_t *fex,
		mx_real_t *features, dsp_sample_t *signal);

static int _dsp_mfcc_2_5(dsp_fextract_t *fex,
		mx_real_t *features, dsp_sample_t *signal);


static int _resample(mx_real_t *dest, mx_real_t *src, int n, mx_real_t scale);

dsp_fextract_t *dsp_mfcc_create(dsp_fextract_t *fex, char *param)
	{
	dsp_mfcc_t *cfg;
	int i, pos;
	char *cp;

	/* Parameter pruefen ... */
	if (!fex || fex->type != dsp_fextype_MFCC)
		return(NULL);

	/* ... speziellen Datenbereich erzeugen ... */
	cfg = rs_malloc(sizeof(dsp_mfcc_t), "MFCC configuration data");
	fex->config = cfg;

	if (fex->version == DSP_MK_VERSION(1, 3)) {
		_dsp_mfcc_1_3_configure(fex, param);
		return(fex);
		}
	if (fex->version == DSP_MK_VERSION(1, 4)) {
		_dsp_mfcc_1_4_configure(fex, param);
		return(fex);
		}
	else if (fex->version == DSP_MK_VERSION(1, 41)) {
		_dsp_mfcc_1_41_configure(fex, param);
		return(fex);
		}
	else if (fex->version == DSP_MK_VERSION(1, 6)) {
		_dsp_mfcc_1_6_configure(fex, param);
		return(fex);
		}

	/* Abtastrate etc. ... */
	switch (fex->version) {
		case DSP_MK_VERSION(1, 1):
		case DSP_MK_VERSION(1, 2):
		case DSP_MK_VERSION(1, 3):
		case DSP_MK_VERSION(1, 4):
		case DSP_MK_VERSION(1, 5):
		case DSP_MK_VERSION(1, 6):

			fex->samplerate =	V1_1_SAMPLERATE;
			fex->n_channels =	V1_1_N_CHANNELS;
			fex->frame_len =	V1_1_FRAME_LEN;
			fex->frame_shift =	V1_1_FRAME_SHIFT;
			fex->n_features =	V1_1_N_FEATURES;

			/* Verzoegerungselement fuer Ableitung erzeugen ... */
			cfg->wderiv = dsp_delay_create(V1_1_W_LENGTH,
					sizeof(mx_real_t),
					V1_1_N_BASEFEATURES);
			break;

		case DSP_MK_VERSION(2, 4):

			fex->samplerate =	V1_1_SAMPLERATE;
			fex->n_channels =	V2_4_N_CHANNELS;
			fex->frame_len =	V1_1_FRAME_LEN;
			fex->frame_shift =	V1_1_FRAME_SHIFT;
			fex->n_features =	V1_1_N_FEATURES;

			/* Verzoegerungselement fuer Ableitung erzeugen ... */
			cfg->wderiv = dsp_delay_create(V1_1_W_LENGTH,
					sizeof(mx_real_t),
					V1_1_N_BASEFEATURES);
			break;

		case DSP_MK_VERSION(2, 5):
			fex->samplerate =	V1_1_SAMPLERATE;
			fex->n_channels =	V2_5_N_CHANNELS;
			fex->frame_len =	V1_1_FRAME_LEN;
			fex->frame_shift =	V1_1_FRAME_SHIFT;
			/*fex->frame_len =	2*V1_1_FRAME_LEN;
			  fex->frame_shift =	2*V1_1_FRAME_SHIFT;*/
			fex->n_features =	V1_1_N_FEATURES;

			/* Verzoegerungselement fuer Ableitung erzeugen ... */
			cfg->wderiv = dsp_delay_create(V1_1_W_LENGTH,
					sizeof(mx_real_t),
					V1_1_N_BASEFEATURES);
			break;

		default:
			return(NULL);
		}

	/* Kanaladaption erzeugen/initialisieren */
	if (fex->version == DSP_MK_VERSION(1, 2) ||
	    fex->version == DSP_MK_VERSION(1, 3) ||
	    fex->version == DSP_MK_VERSION(1, 4) ||
	    fex->version == DSP_MK_VERSION(1, 6) ||
	    fex->version == DSP_MK_VERSION(2, 4)) {
		mx_real_t chparam_cfg[1 + 2 + V1_1_N_BASEFEATURES];
		mx_real_t *chparam;
		mx_real_t *enparam;

		/* ... dazu Kanaltyp bestimmen ... */
		if (!param || rs_line_is_empty(param)) {
			rs_warning("no channel type for MFCCs given - using 'desklab'!");
			chparam = chparam_desklab;
			enparam = enparam_desklab;
			}
		else if (strcmp(param, "desklab") == 0) {
			rs_msg("MFCCs use channel type '%s'!", param);
			chparam = chparam_desklab;
			enparam = enparam_desklab;
			}
		else if (strcmp(param, "mme") == 0) {
			rs_msg("MFCCs use channel type '%s'!", param);
			chparam = chparam_mme;
			enparam = enparam_mme;
			}
		else if (strcmp(param, "/dev/dsp") == 0) {
			rs_msg("MFCCs use channel type '%s'!", param);
			chparam = chparam_dev_dsp;
			enparam = enparam_dev_dsp;
			}
		else	{
			cp = param;
			for (i = 0; i < sizeof(chparam_cfg) /
					sizeof(mx_real_t); i++)
				chparam_cfg[i] = 0;
			for (i = 0; i < sizeof(chparam_cfg) /
					sizeof(mx_real_t); i++) {
				if (sscanf(cp, "%g%n",
				    chparam_cfg + i, &pos) != 1) {
					if (!rs_line_is_empty(cp))
						rs_error("error in channel definition: '%s'!",
							param); 
					break;
					}
				cp += pos;
				}
			chparam = chparam_cfg;
			enparam = chparam + 1;
/**
			rs_msg("MFCC channel parameters set to '%s'!",
				param);
**/
			}

		if (fex->version == DSP_MK_VERSION(1, 2))
			cfg->channel = dsp_channel_init(V1_1_N_BASEFEATURES,
				V1_2_I_ENERGY, V1_2_MINTIME, V1_2_MAXTIME,
				V1_2_DECAY, V1_2_ENERGY_PERCENT,  chparam);
		else if (fex->version == DSP_MK_VERSION(1, 3))
			cfg->channel = dsp_channel_init(V1_1_N_BASEFEATURES,
				V1_2_I_ENERGY, V1_3_MINTIME, V1_3_MAXTIME,
				V1_2_DECAY, V1_2_ENERGY_PERCENT,  chparam);
		else if (fex->version == DSP_MK_VERSION(1, 4) ||
			 fex->version == DSP_MK_VERSION(1, 6)) {
			cfg->channel = dsp_channel_init(V1_1_N_BASEFEATURES,
				V1_2_I_ENERGY, V1_3_MINTIME, V1_3_MAXTIME,
				V1_2_DECAY, V1_2_ENERGY_PERCENT,  chparam);

			/* ... zusaetzlich Energiehistogramm initialisieren */
			cfg->ehist = mx_histogram_create(V1_4_EN_HIST_MIN,
					V1_4_EN_HIST_MAX, V1_4_EN_HIST_RES);
			mx_histogram_limit_set(cfg->ehist, V1_4_EN_HIST_LIMIT);
			mx_histogram_update_urange(cfg->ehist,
				enparam[0], enparam[1], V1_4_EN_HIST_IWEIGHT);

			/* ... und VAD-Schwellwert speichern */
			cfg->vad_threshold = chparam[0];
			rs_msg("vad_threshold set to %g.", cfg->vad_threshold);
			}
		else if (fex->version == DSP_MK_VERSION(2, 4)) {
for (i = 0; i < V2_4_N_CHANNELS; i++) {
			cfg->channels[i] = dsp_channel_init(V1_1_N_BASEFEATURES,
				V1_2_I_ENERGY, V1_3_MINTIME, V1_3_MAXTIME,
				V1_2_DECAY, V1_2_ENERGY_PERCENT,  chparam);

			/* ... zusaetzlich Energiehistogramm initialisieren */
			cfg->ehists[i] = mx_histogram_create(V1_4_EN_HIST_MIN,
					V1_4_EN_HIST_MAX, V1_4_EN_HIST_RES);
			mx_histogram_limit_set(cfg->ehists[i], V1_4_EN_HIST_LIMIT);
			mx_histogram_update_urange(cfg->ehists[i],
				enparam[0], enparam[1], V1_4_EN_HIST_IWEIGHT);
	}
			}
		}

	/* Kanaladaption fuer Version 1.5 erzeugen/initialisieren */
	else if (fex->version == DSP_MK_VERSION(1, 5)) {
		mx_real_t chparam_cfg[1 + 2 + 1 + V1_5_N_FILTERS];
		mx_real_t *chparam;
		mx_real_t *enparam;

		/* ... dazu Kanaltyp bestimmen ... */
		if (!param || rs_line_is_empty(param)) {
			rs_warning("no channel type for MFCCs given - using 'desklab'!");
			chparam = chparam_desklab_2;
			enparam = enparam_desklab;
			}
		else if (strcmp(param, "desklab") == 0) {
			rs_msg("MFCCs use channel type '%s'!", param);
			chparam = chparam_desklab_2;
			enparam = enparam_desklab;
			}
		else if (strcmp(param, "mme") == 0) {
			rs_msg("MFCCs use channel type '%s'!", param);
			chparam = chparam_mme_2;
			enparam = enparam_mme;
			}
		else if (strcmp(param, "/dev/dsp") == 0) {
			rs_msg("MFCCs use channel type '%s'!", param);
			chparam = chparam_dev_dsp_2;
			enparam = enparam_dev_dsp;
			}
		else	{
			cp = param;
			for (i = 0; i < sizeof(chparam_cfg) /
					sizeof(mx_real_t); i++)
				chparam_cfg[i] = 0;
			for (i = 0; i < sizeof(chparam_cfg) /
					sizeof(mx_real_t); i++) {
				if (sscanf(cp, "%g%n",
				    chparam_cfg + i, &pos) != 1) {
					if (!rs_line_is_empty(cp))
						rs_error("error in channel definition: '%s'!",
							param); 
					break;
					}
				cp += pos;
				}
			chparam = chparam_cfg;
			enparam = chparam + 1;

/**
			rs_msg("MFCC channel parameters set to '%s'!",
				param);
**/
			}

		cfg->channel = dsp_channel_init(V1_5_N_FILTERS+1,
				V1_2_I_ENERGY, V1_3_MINTIME, V1_3_MAXTIME,
				V1_2_DECAY, V1_2_ENERGY_PERCENT,  chparam);

		/* ... zusaetzlich Energiehistogramm initialisieren */
		cfg->ehist = mx_histogram_create(V1_4_EN_HIST_MIN,
					V1_4_EN_HIST_MAX, V1_5_EN_HIST_RES);
		mx_histogram_limit_set(cfg->ehist, V1_4_EN_HIST_LIMIT);
		mx_histogram_update_urange(cfg->ehist,
				enparam[0], enparam[1], V1_4_EN_HIST_IWEIGHT);
		}
	/* Kanaladaption fuer Version 2.5 erzeugen/initialisieren */
	else if (fex->version == DSP_MK_VERSION(2, 5)) {
		mx_real_t chparam_cfg_left[1 + 2 + 1 + V1_5_N_FILTERS];
		mx_real_t chparam_cfg_right[1 + 2 + 1 + V1_5_N_FILTERS];

		mx_real_t *chparam_left;
		mx_real_t *enparam_left;

		mx_real_t *chparam_right;
		mx_real_t *enparam_right;

		/* ... dazu Kanaltyp bestimmen ... */
		if (!param || rs_line_is_empty(param)) {
			rs_warning("no channel type for MFCCs given - using 'desklab'!");
			chparam_left = chparam_desklab_2;
			enparam_left = enparam_desklab;

			chparam_right = chparam_desklab_2;
			enparam_right = enparam_desklab;
			}
		else if (strcmp(param, "desklab") == 0) {
			rs_msg("MFCCs use channel type '%s'!", param);
			chparam_left = chparam_desklab_2;
			enparam_left = enparam_desklab;

			chparam_right = chparam_desklab_2;
			enparam_right = enparam_desklab;
			}
		else if (strcmp(param, "mme") == 0) {
			rs_msg("MFCCs use channel type '%s'!", param);
			chparam_left = chparam_mme_2;
			enparam_left = enparam_mme;

			chparam_right = chparam_mme_2;
			enparam_right = enparam_mme;
			}
		else if (strcmp(param, "/dev/dsp") == 0) {
			rs_msg("MFCCs use channel type '%s'!", param);
			chparam_left = chparam_dev_dsp_2;
			enparam_left = enparam_dev_dsp;

			chparam_right = chparam_dev_dsp_2;
			enparam_right = enparam_dev_dsp;
			}
		else	{
			cp = param;
			for (i = 0; i < sizeof(chparam_cfg_left) /
					sizeof(mx_real_t); i++)
				chparam_cfg_left[i] = 0;
			for (i = 0; i < sizeof(chparam_cfg_right) /
					sizeof(mx_real_t); i++)
				chparam_cfg_right[i] = 0;

			for (i = 0; i < sizeof(chparam_cfg_left) /
					sizeof(mx_real_t); i++) {
				if (sscanf(cp, "%g%n",
				    chparam_cfg_left + i, &pos) != 1) {
					if (!rs_line_is_empty(cp))
						rs_error("error in channel definition: '%s'!",
							param); 
					break;
					}
				cp += pos;
				}

			for (i = 0; i < sizeof(chparam_cfg_right) /
					sizeof(mx_real_t); i++) {
				if (sscanf(cp, "%g%n",
				    chparam_cfg_right + i, &pos) != 1) {
					if (!rs_line_is_empty(cp))
						rs_error("error in channel definition: '%s'!",
							param); 
					break;
					}
				cp += pos;
				}
			chparam_left = chparam_cfg_left;
			enparam_left = chparam_left + 1;

			chparam_right = chparam_cfg_right;
			enparam_right = chparam_right + 1;

/**
			rs_msg("MFCC channel parameters set to '%s'!",
				param);
**/
			}

		cfg->channel_left = dsp_channel_init(V1_5_N_FILTERS+1,
				V1_2_I_ENERGY, V1_3_MINTIME, V1_3_MAXTIME,
				V1_2_DECAY, V1_2_ENERGY_PERCENT,  chparam_left);
		cfg->channel_right = dsp_channel_init(V1_5_N_FILTERS+1,
				V1_2_I_ENERGY, V1_3_MINTIME, V1_3_MAXTIME,
				V1_2_DECAY, V1_2_ENERGY_PERCENT,  chparam_right);

		/* ... zusaetzlich Energiehistogramm initialisieren */
		cfg->ehist_left = mx_histogram_create(V1_4_EN_HIST_MIN,
					V1_4_EN_HIST_MAX, V1_5_EN_HIST_RES);
		mx_histogram_limit_set(cfg->ehist_left, V1_4_EN_HIST_LIMIT);
		mx_histogram_update_urange(cfg->ehist_left,
				enparam_left[0], enparam_left[1], V1_4_EN_HIST_IWEIGHT);
		cfg->ehist_right = mx_histogram_create(V1_4_EN_HIST_MIN,
					V1_4_EN_HIST_MAX, V1_5_EN_HIST_RES);
		mx_histogram_limit_set(cfg->ehist_right, V1_4_EN_HIST_LIMIT);
		mx_histogram_update_urange(cfg->ehist_right,
				enparam_right[0], enparam_right[1], V1_4_EN_HIST_IWEIGHT);
		cfg->ehist_left_short = mx_histogram_create(V1_4_EN_HIST_MIN,
					V1_4_EN_HIST_MAX, V1_5_EN_HIST_RES);
		mx_histogram_limit_set(cfg->ehist_left_short, V2_5_EN_HIST_LIMIT);
		mx_histogram_update_urange(cfg->ehist_left_short,
				enparam_left[0], enparam_left[1], V1_4_EN_HIST_IWEIGHT);
		cfg->ehist_right_short = mx_histogram_create(V1_4_EN_HIST_MIN,
					V1_4_EN_HIST_MAX, V1_5_EN_HIST_RES);
		mx_histogram_limit_set(cfg->ehist_right_short, V2_5_EN_HIST_LIMIT);
		mx_histogram_update_urange(cfg->ehist_right_short,
				enparam_right[0], enparam_right[1], V1_4_EN_HIST_IWEIGHT);
		}

	return(fex);
	}

void dsp_mfcc_destroy(dsp_fextract_t *fex)
	{
	dsp_mfcc_t *cfg;

	/* Parameter pruefen ... */
	if (!fex || fex->type != dsp_fextype_MFCC)
		return;

	/* ... speziellen Datenbereich zugreifbar machen ... */
	cfg = fex->config;

	/* ... und ggf. existierende Eintraege loeschen */
	if (cfg->wderiv)
		// @begin_add_johannes
		dsp_delay_destroy(cfg->wderiv);
		// @end_add_johannes
	if (cfg->channel)
		// @begin_add_johannes
		//dsp_channel_destroy(cfg->channel);
		// @end_add_johannes

	// @begin_add_johannes
	rs_free (cfg);
	// @end_add_johannes
	}

void dsp_mfcc_reset(dsp_fextract_t *fex)
	{
	dsp_mfcc_t *cfg;

	/* Parameter pruefen ... */
	if (!fex || fex->type != dsp_fextype_MFCC)
		return;

	/* ... speziellen Datenbereich zugreifbar machen ... */
	cfg = fex->config;

	/* ... und ggf. existierende Datenbestaende re-initialisieren */
	if (cfg->wderiv)
		dsp_delay_flush(cfg->wderiv);
	if (cfg->channel)
		/** kein Reset fuer Kanalmodell **/ ;
	}

int dsp_mfcc_fprintparam(FILE *fp, dsp_fextract_t *fex, char *key)
	{
	dsp_mfcc_t *cfg;

	dsp_channel_t *channel;
	mx_histogram_t *ehist;

	dsp_channel_t *channel_left;
	dsp_channel_t *channel_right;
	mx_histogram_t *ehist_left;
	mx_histogram_t *ehist_right;

	int i, c;

	/* Parameter pruefen ... */
	if (!fex || fex->type != dsp_fextype_MFCC)
		return(-1);

	/* ... speziellen Datenbereich zugreifbar machen ... */
	cfg = fex->config;

	switch (fex->version) {
		case DSP_MK_VERSION(1, 1):
			break;
		case DSP_MK_VERSION(1, 2): 
		case DSP_MK_VERSION(1, 3): 
		case DSP_MK_VERSION(1, 4): 
		case DSP_MK_VERSION(1, 5): 
		case DSP_MK_VERSION(1, 6): 
		       if (!cfg->channel)
			 rs_error("no channel adaptation data present!");
		       channel = cfg->channel;
		       ehist = cfg->ehist;
		       break;
		case DSP_MK_VERSION(2, 5): 
		       if (!cfg->channel_left)
			 rs_error("no channel adaptation data present!");
		       if (!cfg->channel_right)
			 rs_error("no channel adaptation data present!");
		       channel_left = cfg->channel_left;
		       channel_right = cfg->channel_right;
		       ehist_left = cfg->ehist_left;
		       ehist_right = cfg->ehist_right;
		       break;
		}

	switch (fex->version) {
		case DSP_MK_VERSION(1, 1):
			break;
		case DSP_MK_VERSION(1, 2): 
			/* aktuelle Kanaladaptions-Konfiguration ausgeben */
			fprintf(fp, "%d %g %g ",
				channel->nTime,
				channel->fMin[0], channel->fMax[0]);
			for (i = 0; i < channel->nDim; i++)
				fprintf(fp, "%g ", channel->pMeans[i]);
			fprintf(fp, "\n");
			break;
		case DSP_MK_VERSION(1, 3): 
			_dsp_mfcc_1_3_fprintparam(fp, fex, key);
			break;
		case DSP_MK_VERSION(1, 4): 
			_dsp_mfcc_1_4_fprintparam(fp, fex, key);
			break;
		case DSP_MK_VERSION(1, 5): 
			/* aktuelle Kanaladaptions-Konfiguration ausgeben */
			fprintf(fp, "%g %g %g ",
				cfg->vad_threshold,
				mx_histogram_invprob_le(ehist,
						V1_4_EN_HIST_PROBLOW),
				mx_histogram_invprob_le(ehist,
						V1_4_EN_HIST_PROBHIGH));
			for (i = 0; i < channel->nDim; i++)
				fprintf(fp, "%g ", channel->pMeans[i]);
			fprintf(fp, "\n");
			break;
		case DSP_MK_VERSION(1, 6): 
			_dsp_mfcc_1_6_fprintparam(fp, fex, key);
			break;
		case DSP_MK_VERSION(2, 4): 
			/* aktuelle Kanaladaptions-Konfiguration ausgeben */
for (c = 0; c < V2_4_N_CHANNELS; c++) {
			fprintf(fp, "# channel %d\n", c);
			fprintf(fp, "0 %g %g ",
				mx_histogram_invprob_le(cfg->ehists[c],
						V1_4_EN_HIST_PROBLOW),
				mx_histogram_invprob_le(cfg->ehists[c],
						V1_4_EN_HIST_PROBHIGH));
			for (i = 0; i < cfg->channels[c]->nDim; i++)
				fprintf(fp, "%g ", cfg->channels[c]->pMeans[i]);
			fprintf(fp, "\n");
	}
			break;
		case DSP_MK_VERSION(2, 5): 
			/* aktuelle Kanaladaptions-Konfiguration ausgeben */
			fprintf(fp, "0 %g %g ",
				mx_histogram_invprob_le(ehist_left,
						V1_4_EN_HIST_PROBLOW),
				mx_histogram_invprob_le(ehist_left,
						V1_4_EN_HIST_PROBHIGH));
			for (i = 0; i < channel_left->nDim; i++)
				fprintf(fp, "%g ", channel_left->pMeans[i]);

			fprintf(fp, "0 %g %g ",
				mx_histogram_invprob_le(ehist_right,
						V1_4_EN_HIST_PROBLOW),
				mx_histogram_invprob_le(ehist_right,
						V1_4_EN_HIST_PROBHIGH));
			for (i = 0; i < channel_right->nDim; i++)
				fprintf(fp, "%g ", channel_right->pMeans[i]);
			fprintf(fp, "\n");

			break;

		}

	return(0);
	}

int dsp_mfcc_calc(dsp_fextract_t *fex,
		mx_real_t *features, dsp_sample_t *signal)
	{
	/* Parameter pruefen ... */
	if (!fex || fex->type != dsp_fextype_MFCC)
		return(-1);

	switch (fex->version) {
		case DSP_MK_VERSION(1, 1):
			return (_dsp_mfcc_1_1(fex, features, signal));
		case DSP_MK_VERSION(1, 2):
			return (_dsp_mfcc_1_2(fex, features, signal));
		case DSP_MK_VERSION(1, 3):
			return (_dsp_mfcc_1_3(fex, features, signal));
		case DSP_MK_VERSION(1, 41):
			return (_dsp_mfcc_1_41(fex, features, signal));
		case DSP_MK_VERSION(1, 4):
			return (_dsp_mfcc_1_4(fex, features, signal));
		case DSP_MK_VERSION(1, 5):
			return (_dsp_mfcc_1_5(fex, features, signal));
		case DSP_MK_VERSION(1, 6):
			return (_dsp_mfcc_1_6(fex, features, signal));

		case DSP_MK_VERSION(2, 4):
			return (_dsp_mfcc_2_4(fex, features, signal));

		case DSP_MK_VERSION(2, 5):
			return (_dsp_mfcc_2_5(fex, features, signal));

		default:
			return(-1);
		}
	}

	/*
	 * Version 1.1:
	 *	verbesserte/korrigierte Version der ersten
	 *	inkrementelle Merkmalsberechnung auf der Basis der
	 *	Erlangen/Ulmer V3.1N1
	 *
	 *	Frame-Laenge:	256 Samples (bei 16 kHz = 16 ms)
	 *	(Frame-Shift:	160 Samples (bei 16 kHz = 10 ms))
	 *	Mel-Filterbank (inkl. Gesamtenergie in allen Kanaelen)
	 *		NEU: wird automatisch mit 30 Koeff. erzeugt
	 *	KEINE Normierung
	 *	Logarithmus
	 *	Cepstrum (via Cosinustransformation)
	 *	1. und 2. Ableitung (Fenster: 5 Frames)
	 *		KORRIGIERT: C0 etc. ist nicht mehr im Merkmalssatz
	 *	Glaettung der Originaldaten mit "Tirolerhut"
	 */

/**
* _dsp_mfcc_1_1(features, frames)
**/
static int _dsp_mfcc_1_1(dsp_fextract_t *fex,
		mx_real_t *features, dsp_sample_t *signal)
	{
	static mx_real_t mtf[V1_1_N_FILTERS], bbr[V1_1_N_FILTERS];
	static dsp_filterbank_t *fb = NULL;
	static mx_real_t w[V1_1_FRAME_LEN];
	static mx_real_t p[V1_1_FRAME_LEN];
	static mx_complex_t z[V1_1_FRAME_LEN];
	static mx_real_t e[V1_1_N_FILTERS + 1];
	static mx_real_t C[V1_1_N_FILTERS];

	dsp_mfcc_t *cfg = fex->config;
	int i;

	/* ... Filterbank ggf. erzeugen ... */
	if (fb == NULL) {
		/* ... Mel-Skala erzeugen ... */
		if (dsp_mel_create(mtf, bbr, V1_1_N_FRESOLUTION, 
			V1_1_MIN_FREQ, V1_1_MAX_FREQ, 1.0, V1_1_N_FILTERS)
				!= V1_1_N_FILTERS)
			rs_error("problems creating mel-scale!");

		/* ... Plateau ist 1/4 Frequenzgruppe breit ... */
		for (i = 0; i < V1_1_N_FILTERS; i++)
			bbr[i] /= 4;
		fb = dsp_filterbank_create(V1_1_N_FILTERS, mtf, bbr,
				V1_1_N_FRESOLUTION,
				V1_1_MIN_FREQ, V1_1_MAX_FREQ);
		}

	/* Merkmalsberechnung durchfuehren, dazu ... */
	/* ... Hamming-Fenstern ... */
	dsp_window_hamming(w, signal, V1_1_FRAME_LEN);
	
	/* ... Leistungsdichtespektrum ... */
	for (i = 0; i < V1_1_FRAME_LEN; i++) {
		mx_re(z[i]) = w[i];
		mx_im(z[i]) = 0.0;
		}
	dsp_xfft(z, V1_1_FRAME_LEN, 0);
	/* Guenther normiert hinwaerts mit 1/n --- wir nicht */
	for (i = 0; i < V1_1_FRAME_LEN; i++)
		p[i] = (dsp_sqr(mx_re(z[i])) + dsp_sqr(mx_im(z[i]))) /
					dsp_sqr(V1_1_FRAME_LEN);

	/* ... Mel-Filter ... */
	dsp_filterbank_apply(e, p, fb);

	/* ... Logarithmierung ... */
	for (i = 0; i < V1_1_N_FILTERS + 1; i++)
		e[i] = dsp_log10(e[i]);

	/* ... Cepstrum ... */
	dsp_dct(C, e + 1, V1_1_N_FILTERS, V1_1_N_BASEFEATURES);

	/* ... Gesamtmerkmalsvektor erzeugen ... */
	features[0] = e[0];
	memcpy(features + 1, C + 1, sizeof(mx_real_t) * (V1_1_N_BASEFEATURES - 1));

	/* ... Ableitung 1. und 2. Ordnung ... */
	dsp_delay_push(cfg->wderiv, features);

	/* ... berechnen sofern moeglich ... */
	if (dsp_deriv(features + V1_1_N_BASEFEATURES, cfg->wderiv, 1, V1_1_W_LENGTH) == 0 &&
	    dsp_deriv(features + 2 * V1_1_N_BASEFEATURES, cfg->wderiv, 2, V1_1_W_LENGTH) == 0) {
		/* ... zugehoerigen mittleren Vektor getlaettet erzeugen ... */
		dsp_tirol(features, cfg->wderiv);

		/* ... und Daten als gueltig erklaeren ... */
		return(V1_1_N_FEATURES);
		}
	else	return(0);	/* ... sonst: (noch) keine Merkmale! */
	}


	/*
	 * Version 1.2:
	 *	wie Version 1.1 jedoch mit Kanaladaption und dynamischer
	 *	Maximum- und Minimumbestimmung fuer die Signalenergie
	 */
/**
* _dsp_mfcc_1_2(features, signal)
**/
static int _dsp_mfcc_1_2(dsp_fextract_t *fex,
		mx_real_t *features, dsp_sample_t *signal)
	{
	static mx_real_t mtf[V1_1_N_FILTERS], bbr[V1_1_N_FILTERS];
	static dsp_filterbank_t *fb = NULL;
	static mx_real_t w[V1_1_FRAME_LEN];
	static mx_real_t p[V1_1_FRAME_LEN];
	static mx_complex_t z[V1_1_FRAME_LEN];
	static mx_real_t e[V1_1_N_FILTERS + 1];
	static mx_real_t C[V1_1_N_FILTERS];

	dsp_mfcc_t *cfg = fex->config;
	int i;

	/* ... Filterbank ggf. erzeugen ... */
	if (fb == NULL) {
		/* ... Mel-Skala erzeugen ... */
		if (dsp_mel_create(mtf, bbr, V1_1_N_FRESOLUTION, 
			V1_1_MIN_FREQ, V1_1_MAX_FREQ, 1.0, V1_1_N_FILTERS)
				!= V1_1_N_FILTERS)
			rs_error("problems creating mel-scale!");

		/* ... Plateau ist 1/4 Frequenzgruppe breit ... */
		for (i = 0; i < V1_1_N_FILTERS; i++)
			bbr[i] /= 4;
		fb = dsp_filterbank_create(V1_1_N_FILTERS, mtf, bbr,
				V1_1_N_FRESOLUTION,
				V1_1_MIN_FREQ, V1_1_MAX_FREQ);
		}

	/* Merkmalsberechnung durchfuehren, dazu ... */
	/* ... Hamming-Fenstern ... */
	dsp_window_hamming(w, signal, V1_1_FRAME_LEN);
	
	/* ... Leistungsdichtespektrum ... */
	for (i = 0; i < V1_1_FRAME_LEN; i++) {
		mx_re(z[i]) = w[i];
		mx_im(z[i]) = 0.0;
		}
	dsp_xfft(z, V1_1_FRAME_LEN, 0);
	/* Guenther normiert hinwaerts mit 1/n --- wir nicht */
	for (i = 0; i < V1_1_FRAME_LEN; i++)
		p[i] = (dsp_sqr(mx_re(z[i])) + dsp_sqr(mx_im(z[i]))) /
					dsp_sqr(V1_1_FRAME_LEN);

	/* ... Mel-Filter ... */
	dsp_filterbank_apply(e, p, fb);

	/* ... Logarithmierung ... */
	for (i = 0; i < V1_1_N_FILTERS + 1; i++)
		e[i] = dsp_log10(e[i]);

	/* ... Cepstrum ... */
	dsp_dct(C, e + 1, V1_1_N_FILTERS, V1_1_N_BASEFEATURES);

	/* ... Gesamtmerkmalsvektor erzeugen ... */
	features[0] = e[0];
	memcpy(features + 1, C + 1, sizeof(mx_real_t) * (V1_1_N_BASEFEATURES - 1));

	/* ... Kanaladaption durch Mittelwertsbereinigung */
	dsp_channel(cfg->channel, features);

	/* ... Ableitung 1. und 2. Ordnung ... */
	dsp_delay_push(cfg->wderiv, features);

	/* ... berechnen sofern moeglich ... */
	if (dsp_deriv(features + V1_1_N_BASEFEATURES, cfg->wderiv, 1, V1_1_W_LENGTH) == 0 &&
	    dsp_deriv(features + 2 * V1_1_N_BASEFEATURES, cfg->wderiv, 2, V1_1_W_LENGTH) == 0) {
		/* ... zugehoerigen mittleren Vektor getlaettet erzeugen ... */
		dsp_tirol(features, cfg->wderiv);

		/* ... und Daten als gueltig erklaeren ... */
		return(V1_1_N_FEATURES);
		}
	else	return(0);	/* ... sonst: (noch) keine Merkmale! */
	}

	/*
	 * Version 1.5:
	 *	wie Version 1.4 jedoch mit Vorwaertsmaskierung und ohne
	 *      Preemphase
	 */
/**
* _dsp_mfcc_1_5(features, signal)
**/
static int _dsp_mfcc_1_5(dsp_fextract_t *fex,
		mx_real_t *features, dsp_sample_t *signal)
	{
	static mx_real_t mtf[V1_5_N_FILTERS], bbr[V1_5_N_FILTERS];
	static dsp_filterbank_t *fb = NULL;
	static mx_real_t w[V1_1_FRAME_LEN];
	static mx_real_t p[V1_5_FFT_LEN];
	static mx_complex_t z[V1_5_FFT_LEN];
	static mx_real_t e[V1_5_N_FILTERS + 1];
	static mx_real_t C[V1_5_N_FILTERS];

	/* Maskiertes Spektrum */
	static mx_real_t m[V1_5_N_FILTERS];

	/*Parameter fuer Maskierung */
	static dsp_fwm_param_t* fwm_params = NULL;

	dsp_mfcc_t *cfg = fex->config;
	mx_real_t le;
	mx_real_t m_energy, energy_diff;
	mx_real_t energy_max = 0;
	mx_real_t offset = 0;

	int i;

	/* ... Filterbank ggf. erzeugen ... */
	if (fb == NULL) {
		/* ... Mel-Skala erzeugen ... */
		if (dsp_mel_create(mtf, bbr, V1_5_N_FRESOLUTION, 
			V1_1_MIN_FREQ, V1_1_MAX_FREQ, 1.0, V1_5_N_FILTERS)
				!= V1_5_N_FILTERS)
			rs_error("problems creating mel-scale!");

		/* ... Plateau ist 1/4 Frequenzgruppe breit ... */
		for (i = 0; i < V1_5_N_FILTERS; i++)
			bbr[i] /= 4;
		fb = dsp_filterbank_create(V1_5_N_FILTERS, mtf, bbr,
				V1_5_N_FRESOLUTION,
				V1_1_MIN_FREQ, V1_1_MAX_FREQ);
		/* ... Parameter fuer Maskierung erzeugen */ 
		fwm_params=dsp_init_forward_masking(mtf,V1_5_N_FILTERS);

		}

	/* ... Hamming-Fenstern ... */
	dsp_window_hamming(w, signal, V1_1_FRAME_LEN);
	
	/* ... Leistungsdichtespektrum ... */
	for (i = 0; i < V1_1_FRAME_LEN; i++) {
		mx_re(z[i]) = w[i];
		mx_im(z[i]) = 0.0;
		}
	for (i = V1_1_FRAME_LEN; i < V1_5_FFT_LEN; i++) {
		mx_re(z[i]) = 0.0;
		mx_im(z[i]) = 0.0;
		}

	dsp_xfft(z, V1_5_FFT_LEN, 0);

	/* Guenther normiert hinwaerts mit 1/n --- wir nicht */
	for (i = 0; i < V1_5_FFT_LEN; i++)
		p[i] = (dsp_sqr(mx_re(z[i])) + dsp_sqr(mx_im(z[i]))) /
					dsp_sqr(V1_5_FFT_LEN);

	/* Merkmalsberechnung durchfuehren, dazu ... */
	/* ... Mel-Filter ... */
	dsp_filterbank_apply(e, p, fb);

	/* ... Logarithmierung ... */
	for (i = 0; i < V1_5_N_FILTERS + 1; i++)
	  e[i] = dsp_log10(e[i]);
	
	/* ... Festlegung der Minima fuer Maskierung ...*/
	dsp_set_minima(fwm_params, e + 1, V1_5_N_FILTERS);
       
	/* ... Energiehistogramm aktualisieren ... */
	mx_histogram_update(cfg->ehist, e[0]);

	/* ... und Energie-Maximum (95%-Qantil) auf 0 normieren ... */
	energy_max=(mx_histogram_invprob_le(cfg->ehist, V1_4_EN_HIST_PROBHIGH)
	  + cfg->ehist->resolution / 2.0);
	le = e[0] - energy_max;

	/* OFFSET fuer Signal-Umrechnung in dB bestimmen */
	offset=V1_5_DB_SPRACHE-(V1_5_DB_MULTIPLIKATOR*energy_max); 
	
	/* ... Kanaladaption: Mittelwertsberechnung ...*/
	dsp_channel_in(cfg->channel, e);

	/* ... Umrechnung in dB ... */
	for (i = 1; i < V1_5_N_FILTERS + 1; i++)
	  e[i] = (e[i]*V1_5_DB_MULTIPLIKATOR)+offset;
	
	/* ... Maskierung ...*/
	dsp_masking(m, e + 1, V1_5_N_FILTERS, fwm_params);
	
	/* ... Rueckrechnung der dB ...*/
	for (i = 0; i < V1_5_N_FILTERS; i++)
	  m[i] = (m[i]-offset)/V1_5_DB_MULTIPLIKATOR;
	
	/* ...  Beruecksichtigung der Minima ... */
	dsp_apply_minima(fwm_params, m, V1_5_N_FILTERS);
	
	/* ... Neuberechnung der Gesamtenergie nach Maskierung ... */
	m_energy=dsp_calc_energy(m,V1_5_N_FILTERS);
	
	/* ... Berechnung der Energiedifferenz durch Maskierung ... */ 
	energy_diff=e[0]-m_energy;
	
	/* ... Kanalenergien durch maskierte Kanalenergien ersetzten ...*/ 
	memcpy(e + 1, m, sizeof(mx_real_t) * V1_5_N_FILTERS);

	/* ... Kanaladaption: Mittelwertsbereinigung ...*/
	dsp_channel_out(cfg->channel, e);

	/* ... Cepstrum ... */
	dsp_dct(C, e + 1, V1_5_N_FILTERS, V1_1_N_BASEFEATURES);

	/* ... Gesamtmerkmalsvektor erzeugen ... */
	/* ... Gesamtenergie neu berechnen, 
	   also Subtraktion der maskierten Energie ... */
	features[0]= le-energy_diff;

	memcpy(features + 1, C + 1, sizeof(mx_real_t) * (V1_1_N_BASEFEATURES - 1));

	/* ... Ableitung 1. und 2. Ordnung ... */
	dsp_delay_push(cfg->wderiv, features);

	/* ... berechnen sofern moeglich ... */
	if (dsp_deriv(features + V1_1_N_BASEFEATURES, cfg->wderiv, 1, V1_1_W_LENGTH) == 0 &&
	    dsp_deriv(features + 2 * V1_1_N_BASEFEATURES, cfg->wderiv, 2, V1_1_W_LENGTH) == 0) {
		/* ... zugehoerigen mittleren Vektor geglaettet erzeugen ... */
		dsp_tirol(features, cfg->wderiv);

		/* ... und Daten als gueltig erklaeren ... */
		return(V1_1_N_FEATURES);
		}
	else	return(0);	/* ... sonst: (noch) keine Merkmale! */
	}


static mx_real_t _calc_gain(mx_real_t left, mx_real_t right)
{
  mx_real_t zero_test=0;
  mx_real_t gain=0.5;

  if((zero_test=left+right)!=0.0)
    gain=left/zero_test;
  else
    gain=0.5;

  if(gain>V2_5_MAX_GAIN)
    gain=V2_5_MAX_GAIN;
  else if(gain<V2_5_MIN_GAIN)
    gain=V2_5_MIN_GAIN;

  return(gain);
}

static mx_real_t _calc_SNR_gain(mx_real_t left1, mx_real_t left2, mx_real_t right1, mx_real_t right2)
{
  mx_real_t zero_test=0;
  mx_real_t gain=0.5;

  if((zero_test=(left1-left2)+(right1-right2))!=0.0)
    gain=(left1-left2)/zero_test;
  else
    gain=0.5;

  if(gain>V2_5_MAX_GAIN)
    gain=V2_5_MAX_GAIN;
  else if(gain<V2_5_MIN_GAIN)
    gain=V2_5_MIN_GAIN;

  return(gain);
}

static mx_real_t _calc_SNR_gain_pow(mx_real_t left1, mx_real_t left2, mx_real_t right1, mx_real_t right2)
{
  mx_real_t zero_test=0;
  mx_real_t gain=0.5;
  
  if((zero_test=pow(left1-left2,2)+pow(right1-right2,2))!=0.0)
    gain=pow(left1-left2,2)/zero_test;
  else
    gain=0.5;

  if(gain>V2_5_MAX_GAIN)
    gain=V2_5_MAX_GAIN;
  else if(gain<V2_5_MIN_GAIN)
    gain=V2_5_MIN_GAIN;

  return(gain);
}


	/*
	 * Version 2.5:
	 *	wie Version 1.5 jedoch fuer stereo-signale
	 */
/**
* _dsp_mfcc_2_5(features, signal)
**/
static int _dsp_mfcc_2_5(dsp_fextract_t *fex,
		mx_real_t *features, dsp_sample_t *signal)
	{
	static mx_real_t mtf[V1_5_N_FILTERS], bbr[V1_5_N_FILTERS];
	static dsp_filterbank_t *fb = NULL;

	static mx_real_t w_left[V1_1_FRAME_LEN];
	static mx_real_t p_left[V1_5_FFT_LEN];
	static mx_complex_t z_left[V1_5_FFT_LEN];
	static mx_real_t e_left[V1_5_N_FILTERS + 1];
	/*static mx_real_t C_left[V1_5_N_FILTERS];*/

	static mx_real_t w_right[V1_1_FRAME_LEN];
	static mx_real_t p_right[V1_5_FFT_LEN];
	static mx_complex_t z_right[V1_5_FFT_LEN];
	static mx_real_t e_right[V1_5_N_FILTERS + 1];
	/*static mx_real_t C_right[V1_5_N_FILTERS];*/

	static mx_real_t e_total[V1_5_N_FILTERS + 1];
	static mx_real_t C_total[V1_5_N_FILTERS];

	/* Maskiertes Spektrum */
	static mx_real_t m_left[V1_5_N_FILTERS];
	static mx_real_t m_right[V1_5_N_FILTERS];

	/*Parameter fuer Maskierung */
	static dsp_fwm_param_t* fwm_params_left = NULL;
	static dsp_fwm_param_t* fwm_params_right = NULL;

	dsp_mfcc_t *cfg = fex->config;

	mx_real_t le_left;
	mx_real_t m_energy_left, energy_diff_left;
	mx_real_t energy_max_left = 0;
	mx_real_t offset_left = 0;

	mx_real_t le_right;
	mx_real_t m_energy_right, energy_diff_right;
	mx_real_t energy_max_right = 0;
	mx_real_t offset_right = 0;

	static dsp_sample_t signal_left[V1_1_FRAME_LEN];
	static dsp_sample_t signal_right[V1_1_FRAME_LEN];
	
	mx_real_t energy_max_left_short = 0;
	mx_real_t energy_max_right_short = 0;
	mx_real_t energy_min_left = 0;
	mx_real_t energy_min_right = 0;
	mx_real_t energy_min_left_short = 0;
	mx_real_t energy_min_right_short = 0;
	mx_real_t energy_min2_left = 0;
	mx_real_t energy_min2_right = 0;
	mx_real_t energy_min2_left_short = 0;
	mx_real_t energy_min2_right_short = 0;
	mx_real_t energy_max2_left = 0;
	mx_real_t energy_max2_right = 0;

	mx_real_t energy_mittel_left = 0;
	mx_real_t energy_mittel_right = 0;


	mx_real_t gain=0.5;
	mx_real_t gain_2=0;
	mx_real_t gain_3=0;
	mx_real_t gain_4=0;
	mx_real_t gain_5=0;

	int i;

	/* ... Filterbank ggf. erzeugen ... */
	if (fb == NULL) {
		/* ... Mel-Skala erzeugen ... */
		if (dsp_mel_create(mtf, bbr, V1_5_N_FRESOLUTION, 
			V1_1_MIN_FREQ, V1_1_MAX_FREQ, 1.0, V1_5_N_FILTERS)
				!= V1_5_N_FILTERS)
			rs_error("problems creating mel-scale!");

		/* ... Plateau ist 1/4 Frequenzgruppe breit ... */
		for (i = 0; i < V1_5_N_FILTERS; i++)
			bbr[i] /= 4;
		fb = dsp_filterbank_create(V1_5_N_FILTERS, mtf, bbr,
				V1_5_N_FRESOLUTION,
				V1_1_MIN_FREQ, V1_1_MAX_FREQ);
		/* ... Parameter fuer Maskierung erzeugen */ 
		fwm_params_left=dsp_init_forward_masking(mtf,V1_5_N_FILTERS);
		fwm_params_right=dsp_init_forward_masking(mtf,V1_5_N_FILTERS);

		}

	/* ... Extraktion des Linken und Rechten Kanals aus signal ... */
	for(i=0;i<V1_1_FRAME_LEN;i++)
	  {
	    signal_left[i]=signal[i*2];
	    signal_right[i]=signal[(i*2)+1];
	  }

	/* ... Hamming-Fenstern ... */
	dsp_window_hamming(w_left, signal_left, V1_1_FRAME_LEN);
	dsp_window_hamming(w_right, signal_right, V1_1_FRAME_LEN);

	/* ... Leistungsdichtespektrum ... */
	for (i = 0; i < V1_1_FRAME_LEN; i++) {
		mx_re(z_left[i]) = w_left[i];
		mx_im(z_left[i]) = 0.0;
		}
	for (i = V1_1_FRAME_LEN; i < V1_5_FFT_LEN; i++) {
		mx_re(z_left[i]) = 0.0;
		mx_im(z_left[i]) = 0.0;
		}
	for (i = 0; i < V1_1_FRAME_LEN; i++) {
		mx_re(z_right[i]) = w_right[i];
		mx_im(z_right[i]) = 0.0;
		}
	for (i = V1_1_FRAME_LEN; i < V1_5_FFT_LEN; i++) {
		mx_re(z_right[i]) = 0.0;
		mx_im(z_right[i]) = 0.0;
		}

	dsp_xfft(z_left, V1_5_FFT_LEN, 0);
	dsp_xfft(z_right, V1_5_FFT_LEN, 0);

	/* Guenther normiert hinwaerts mit 1/n --- wir nicht */
	for (i = 0; i < V1_5_FFT_LEN; i++)
		p_left[i] = (dsp_sqr(mx_re(z_left[i])) + dsp_sqr(mx_im(z_left[i]))) /
					dsp_sqr(V1_5_FFT_LEN);
	for (i = 0; i < V1_5_FFT_LEN; i++)
		p_right[i] = (dsp_sqr(mx_re(z_right[i])) + dsp_sqr(mx_im(z_right[i]))) /
					dsp_sqr(V1_5_FFT_LEN);


	/* Merkmalsberechnung durchfuehren, dazu ... */
	/* ... Mel-Filter ... */
	dsp_filterbank_apply(e_left, p_left, fb);
	dsp_filterbank_apply(e_right, p_right, fb);

	/* ... Logarithmierung ... */
	for (i = 0; i < V1_5_N_FILTERS + 1; i++)
	  {
	    e_left[i] = dsp_log10(e_left[i]);
	    e_right[i] = dsp_log10(e_right[i]);
	  }

	/* ... Festlegung der Minima fuer Maskierung ...*/
	dsp_set_minima(fwm_params_left, e_left + 1, V1_5_N_FILTERS);
	dsp_set_minima(fwm_params_right, e_right + 1, V1_5_N_FILTERS);
       
	/* ... Energiehistogramm aktualisieren ... */
	mx_histogram_update(cfg->ehist_left, e_left[0]);
	mx_histogram_update(cfg->ehist_right, e_right[0]);

	mx_histogram_update(cfg->ehist_left_short, e_left[0]);
	mx_histogram_update(cfg->ehist_right_short, e_right[0]);

	/* ... und Energie-Maximum (95%-Qantil) auf 0 normieren ... */
	energy_max_left=(mx_histogram_invprob_le(cfg->ehist_left, V1_4_EN_HIST_PROBHIGH)
	  + cfg->ehist_left->resolution / 2.0);
	le_left = e_left[0] - energy_max_left;

	energy_max_right=(mx_histogram_invprob_le(cfg->ehist_right, V1_4_EN_HIST_PROBHIGH)
	  + cfg->ehist_right->resolution / 2.0);
	le_right = e_right[0] - energy_max_right;

	energy_max_left_short=(mx_histogram_invprob_le(cfg->ehist_left_short, V1_4_EN_HIST_PROBHIGH)
	  + cfg->ehist_left_short->resolution / 2.0);

	energy_max_right_short=(mx_histogram_invprob_le(cfg->ehist_right_short, V1_4_EN_HIST_PROBHIGH)
	  + cfg->ehist_right_short->resolution / 2.0);

	/* ... und Energie-Mittelwert (50%-Qantil) berechnen ... */
	energy_min_left=(mx_histogram_invprob_le(cfg->ehist_left, 0.05)
	  + cfg->ehist_left->resolution / 2.0);

	energy_min_right=(mx_histogram_invprob_le(cfg->ehist_right, 0.05)
	  + cfg->ehist_right->resolution / 2.0);

	energy_min_left_short=(mx_histogram_invprob_le(cfg->ehist_left_short, 0.05)
	  + cfg->ehist_left_short->resolution / 2.0);

	energy_min_right_short=(mx_histogram_invprob_le(cfg->ehist_right_short, 0.05)
	  + cfg->ehist_right_short->resolution / 2.0);

	energy_min2_left=(mx_histogram_invprob_le(cfg->ehist_left, 0.1)
	  + cfg->ehist_left->resolution / 2.0);

	energy_min2_right=(mx_histogram_invprob_le(cfg->ehist_right, 0.1)
	  + cfg->ehist_right->resolution / 2.0);

	energy_min2_left_short=(mx_histogram_invprob_le(cfg->ehist_left_short, 0.1)
	  + cfg->ehist_left_short->resolution / 2.0);

	energy_min2_right_short=(mx_histogram_invprob_le(cfg->ehist_right_short, 0.1)
	  + cfg->ehist_right_short->resolution / 2.0);

	energy_max2_left=(mx_histogram_invprob_le(cfg->ehist_left, 0.9)
	  + cfg->ehist_left->resolution / 2.0);

	energy_max2_right=(mx_histogram_invprob_le(cfg->ehist_right, 0.9)
	  + cfg->ehist_right->resolution / 2.0);

	energy_mittel_left=(mx_histogram_invprob_le(cfg->ehist_left, 0.5)
	  + cfg->ehist_left->resolution / 2.0);

	energy_mittel_right=(mx_histogram_invprob_le(cfg->ehist_right, 0.5)
	  + cfg->ehist_right->resolution / 2.0);


	/* OFFSET fuer Signal-Umrechnung in dB bestimmen */
	offset_left=V1_5_DB_SPRACHE-(V1_5_DB_MULTIPLIKATOR*energy_max_left); 
	offset_right=V1_5_DB_SPRACHE-(V1_5_DB_MULTIPLIKATOR*energy_max_right); 
	
	/* ... Kanaladaption: Mittelwertsberechnung ...*/
	dsp_channel_in(cfg->channel_left, e_left);
	dsp_channel_in(cfg->channel_right, e_right);

	/* ... Umrechnung in dB ... */
	for (i = 1; i < V1_5_N_FILTERS + 1; i++)
	  {
	    e_left[i] = (e_left[i]*V1_5_DB_MULTIPLIKATOR)+offset_left;
	    e_right[i] = (e_right[i]*V1_5_DB_MULTIPLIKATOR)+offset_right;
	  }
	
	/* ... Maskierung ...*/
	dsp_masking(m_left, e_left + 1, V1_5_N_FILTERS, fwm_params_left);
	dsp_masking(m_right, e_right + 1, V1_5_N_FILTERS, fwm_params_right);
	
	/* ... Rueckrechnung der dB ...*/
	for (i = 0; i < V1_5_N_FILTERS; i++)
	  {
	    m_left[i] = (m_left[i]-offset_left)/V1_5_DB_MULTIPLIKATOR;
	    m_right[i] = (m_right[i]-offset_right)/V1_5_DB_MULTIPLIKATOR;
	    }
	
	/* ...  Beruecksichtigung der Minima ... */
	dsp_apply_minima(fwm_params_left, m_left, V1_5_N_FILTERS);
	dsp_apply_minima(fwm_params_right, m_right, V1_5_N_FILTERS);
	
	/* ... Neuberechnung der Gesamtenergie nach Maskierung ... */
	m_energy_left=dsp_calc_energy(m_left,V1_5_N_FILTERS);
	m_energy_right=dsp_calc_energy(m_right,V1_5_N_FILTERS);
	
	/* ... Berechnung der Energiedifferenz durch Maskierung ... */ 
	energy_diff_left=e_left[0]-m_energy_left;
	energy_diff_right=e_right[0]-m_energy_right;
	
	/* ... Kanalenergien durch maskierte Kanalenergien ersetzten ...*/ 
	memcpy(e_left + 1, m_left, sizeof(mx_real_t) * V1_5_N_FILTERS);
	memcpy(e_right + 1, m_right, sizeof(mx_real_t) * V1_5_N_FILTERS);

	/* ... Kanaladaption: Mittelwertsbereinigung ...*/
	dsp_channel_out(cfg->channel_left, e_left);
	dsp_channel_out(cfg->channel_right, e_right);


	/* ... Berechnung der verschiedenen Gains fuer Beam-Forming ... */


	/* ... Zusammenfassung der log-Power-Spektren beider Kanaele zu einem 
	   log-Power-Spektrum ... */

	gain_4=_calc_gain(energy_mittel_left,energy_mittel_right);

	/** rs_msg("Gain_4 mittel: %f",gain_4); **/

	gain=_calc_SNR_gain(energy_max_left,energy_min_left,energy_max_right,energy_min_right);
	gain+=_calc_SNR_gain(energy_max2_left,energy_min2_left,energy_max2_right,energy_min2_right);

	gain/=2;

	gain_3=_calc_SNR_gain_pow(energy_max_left_short,energy_min_left_short,energy_max_right_short,energy_min_right_short);
	gain_3+=_calc_SNR_gain_pow(energy_max_left_short,energy_min2_left_short,energy_max_right_short,energy_min2_right_short);

	gain_3/=2;

	if(gain>0.5)
	  gain_2=1/(1-gain);
	else
	  gain_2=1/gain;

	if(gain_4>0.5)
	  gain_5=1/(1-gain_4);
	else
	  gain_5=1/gain_4;


	/* ... Zusammenfassung der log-Power-Spektren beider Kanaele zu einem 
	   log-Power-Spektrum ... */

	for(i = 1; i < V1_5_N_FILTERS + 1; i++)
	  {
	    e_total[i]=((e_left[i]*(1-gain)*gain_2*gain_3)+(e_right[i]*gain*gain_2*(1-gain_3)));
	  }


	/* ... Cepstrum ... */

	dsp_dct(C_total, e_total + 1, V1_5_N_FILTERS, V1_1_N_BASEFEATURES);

	/* ... Gesamtmerkmalsvektor erzeugen ... */

	/* ... Gesamtenergie neu berechnen, 
	   also Subtraktion der maskierten Energie ... */

	gain_4=_calc_gain(energy_min2_left,energy_min2_right);
	gain_4+=_calc_gain(energy_min_left,energy_min_right);
	gain_4+=_calc_gain(energy_min2_left_short,energy_min2_right_short);
	gain_4+=_calc_gain(energy_min_left_short,energy_min_right_short);

	gain_4/=4;

	gain=_calc_SNR_gain(energy_max_left,energy_min_left,energy_max_right,energy_min_right);
	gain+=_calc_SNR_gain(energy_max2_left,energy_min2_left,energy_max2_right,energy_min2_right);

	gain/=2;

	gain_3=_calc_SNR_gain(energy_max_left_short,energy_min_left_short,energy_max_right_short,energy_min_right_short);
	gain_3+=_calc_SNR_gain(energy_max_left_short,energy_min2_left_short,energy_max_right_short,energy_min2_right_short);

	gain_3/=2;

	if(gain>0.5)
	  gain_2=1/(1-gain);
	else
	  gain_2=1/gain;

	if(gain_4>0.5)
	  gain_5=1/(1-gain_4);
	else
	  gain_5=1/gain_4;

	features[0]=(((le_left-energy_diff_left)*(1-gain)*gain_2*(1-gain_4)*gain_5*gain_3)+((le_right-energy_diff_right)*gain*gain_2*gain_4*gain_5*(1-gain_3)));


	memcpy(features + 1, C_total + 1, sizeof(mx_real_t) * (V1_1_N_BASEFEATURES - 1));

	/* ... Ableitung 1. und 2. Ordnung ... */
	dsp_delay_push(cfg->wderiv, features);

	/* ... berechnen sofern moeglich ... */
	if (dsp_deriv(features + V1_1_N_BASEFEATURES, cfg->wderiv, 1, V1_1_W_LENGTH) == 0 &&
	    dsp_deriv(features + 2 * V1_1_N_BASEFEATURES, cfg->wderiv, 2, V1_1_W_LENGTH) == 0) {
		/* ... zugehoerigen mittleren Vektor geglaettet erzeugen ... */
		dsp_tirol(features, cfg->wderiv);

		/* ... und Daten als gueltig erklaeren ... */
		return(V1_1_N_FEATURES);
		}
	else	return(0);	/* ... sonst: (noch) keine Merkmale! */
	}

#ifdef USE_THIS_GARBAGE
static int _resample(mx_real_t *dest, mx_real_t *src, int n, mx_real_t scale)
	{
	int i, low, idx;
	mx_real_t x;

	for (i = 0; i < n; i++) {
		if (scale == 1.0)
			idx = i;
		else	{
			x = (mx_real_t)i / scale;
			low = x;

			idx = ((x - low) <= 0.5) ? low : low + 1;
			}

		if (idx >= n)
			dest[i] = 0.0;
		else	dest[i] = src[idx];
		}
	}
#endif /* USE_THIS_GARBAGE */

#include "ev_mfcc_1_41.c"

#include "ev_mfcc_2_4.c"
