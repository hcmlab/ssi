/**
* File:		mfcc_1_3.c
* Author:	Gernot A. Fink
* Date:		prior to 2004, separated in re-engineering
*
* Description:	Implementation of MFCC version 1.3
* 
* Characteristics: as version 1.2 but additionally with:
* 		- uniform short-time normalization of the cepstral average
**/

#include "ev_memory.h"


#include "ev_mfcc_1_3.h"

/*
 * Local Variables
 */
#define V1_3_N_PARAM_DEFAULTS	1

static struct channel_t {
	char *name;
	mx_real_t param[V1_3_N_PARAMS];
		/*
		 * vad-thresh, e_low, e_high,
		 * E{energy}, E{C_1} ... E{C_12}
		 */
	} channel[V1_3_N_PARAM_DEFAULTS] =
{	{"desklab", {
	0,
	0.0, 6.0,
	0, 2.93657, 0.312502, 1.38028, -0.201216, 0.315178, -0.22886,
	0.76584, -0.312766, 0.491137, -0.380584, 0.513988, -0.619026
	}},
};

/*
 * _dsp_mfcc_1_3_configure(fex, params)
 */
int _dsp_mfcc_1_3_configure(dsp_fextract_t *fex, char *params)
	{
	dsp_mfcc_t *cfg = fex->config;
	struct channel_t *ch = NULL;
	int i, pos, status;
	char *cp;
	mx_real_t *param;

	/* Abtastrate etc. ... */
	fex->samplerate =	V1_1_SAMPLERATE;
	fex->n_channels =	V1_1_N_CHANNELS;
	fex->frame_len =	V1_1_FRAME_LEN;
	fex->frame_shift =	V1_1_FRAME_SHIFT;
	fex->n_features =	V1_1_N_FEATURES;

	/* Verzoegerungselement fuer Ableitung erzeugen ... */
	cfg->wderiv = dsp_delay_create(V1_1_W_LENGTH,
			sizeof(mx_real_t),
			V1_1_N_BASEFEATURES);

	/* Kanaladaption erzeugen/initialisieren */
	if (!params || rs_line_is_empty(params)) {
		ch = channel + 0;

		rs_warning("no channel type for MFCCs v%d.%d given"
			" - using '%s'!",
			DSP_VERSION_MAJOR(fex->version),
			DSP_VERSION_MINOR(fex->version),
			ch->name);
		}
	else	{
		for (i = 0; i < V1_3_N_PARAM_DEFAULTS; i++) {
			if (strcmp(params, channel[i].name) == 0) {
				rs_msg("MFCCs v%d.%d use channel type '%s'!",
					DSP_VERSION_MAJOR(fex->version),
					DSP_VERSION_MINOR(fex->version),
					ch->name);
				break;
				}
			}
		
		}

	/* ... falls kein impliziter/gegebener Kanaltyp, Parameter einlesen */
	if (!ch && params) {
		/* ... create temporary channel data ... */
		ch = rs_calloc(1, sizeof(struct channel_t),
				"temporary channel data");

		/* ... scan parameters from 'params' string ... */
		cp = params;

		for (i = 0; i < V1_3_N_PARAMS; i++) {
			status = sscanf(cp, "%g%n", ch->param + i, &pos);

			if (status != 1) {
				if (!rs_line_is_empty(cp))
					rs_error("error in channel definition: '%s'!",
						params);
				break;
				}
			cp += pos;
			}
		for (/* i = <implicit> */; i < V1_3_N_PARAMS; i++)
			ch->param[i] = 0.0;
/********
		rs_msg("MFCC channel parameters set to '%s'!", params);
********/
		}

	cfg->channel = dsp_channel_init(V1_1_N_BASEFEATURES,
				V1_2_I_ENERGY, V1_3_MINTIME, V1_3_MAXTIME,
				V1_2_DECAY, V1_2_ENERGY_PERCENT,
				ch->param);

	return(0);
	}

/*
 * _dsp_mfcc_1_3_fprintparam(fp, fex, prefix)
 */
int _dsp_mfcc_1_3_fprintparam(FILE *fp, dsp_fextract_t *fex, char *prefix)
	{
	dsp_mfcc_t *cfg = fex->config;
	int i;

	/* aktuelle Kanaladaptions-Konfiguration ausgeben */
	fprintf(fp, "%s", prefix);
	fprintf(fp, "%d %g %g ",
		cfg->channel->nTime,
		cfg->channel->fMin[0], cfg->channel->fMax[0]);
	fprintf(fp, "%s", prefix);
	for (i = 0; i < cfg->channel->nDim; i++)
		fprintf(fp, "%g ", cfg->channel->pMeans[i]);
	fprintf(fp, "\n");

	return(0);
	}

	/*
	 * Version 1.3:
	 *	wie Version 1.2 jedoch mit einfacherer Kanaladaption
	 *	bei der ein konstanter Gewichtsterm fuer die Mittelwertbildung
	 *	verwendet wird
	 */
/**
* _dsp_mfcc_1_3(features, signal)
**/
int _dsp_mfcc_1_3(dsp_fextract_t *fex,
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

#ifdef DEBUG
for (i = 0; i < V1_1_FRAME_LEN; i++)
	fprintf(stderr, "%g ", p[i]);
fprintf(stderr, "\n");
#endif

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

