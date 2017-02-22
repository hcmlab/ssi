/**
* File:		mfcc_1_4.c
* Author:	Gernot A. Fink
* Date:		16.3.2004
*
* Description:	Implementation of MFCC version 1.4
* 
* Characteristics: as version 1.3 but additionally with:
* 		- Preemphasis filter
* 		- Pure signal-based energy calculation
* 		- Histogram-based estimation of dynamic range of signal
* 		  energy and normalization to the 95% quantile of the
* 		  mid-term energy distribution
**/

#include "ev_memory.h"


#include "ev_mfcc_1_4.h"

/*
 * Local Variables
 */
#define V1_4_N_PARAM_DEFAULTS	1

static struct channel_t {
	char *name;
	mx_real_t param[V1_4_N_PARAMS];
		/*
		 * vad-thresh, e_low, e_high,
		 * E{energy}, E{C_1} ... E{C_12}
		 */
	} channel[V1_4_N_PARAM_DEFAULTS] =
{	{"desklab", {
	0,
	2.0, 6.0,
	0, 2.93657, 0.312502, 1.38028, -0.201216, 0.315178, -0.22886,
	0.76584, -0.312766, 0.491137, -0.380584, 0.513988, -0.619026
	}},
};

/*
 * _dsp_mfcc_1_4_configure(fex, params)
 */
int _dsp_mfcc_1_4_configure(dsp_fextract_t *fex, char *params)
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
		ch = channel + 0;

		for (i = 0; i < V1_4_N_PARAM_DEFAULTS; i++) {
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

		for (i = 0; i < V1_4_N_PARAMS; i++) {
			status = sscanf(cp, "%g%n", ch->param + i, &pos);

			if (status != 1) {
				if (!rs_line_is_empty(cp))
					rs_error("error in channel definition: '%s'!",
						params);
				break;
				}
			cp += pos;
			}
		for (/* i = <implicit> */; i < V1_4_N_PARAMS; i++)
			ch->param[i] = 0.0;
/********
		rs_msg("MFCC channel parameters set to '%s'!", params);
********/
		}

	cfg->channel = dsp_channel_init(V1_1_N_BASEFEATURES,
				0, V1_4_CEP_WLENGTH, V1_4_CEP_WLENGTH, 0, 0,
				ch->param);
#ifdef __USE_THE_NEW_METHODOLOGY_WHICH_WILL_GIVE_SLIGHTLY_DIFFERENT_RESULTS
	cfg->channel = dsp_channel_create(V1_4_N_CH_PARAMS);
	dsp_channel_configure(cfg->channel,
			/* {t, t_min, t_max} = */
			0, V1_4_CEP_WLENGTH, V1_4_CEP_WLENGTH,
			ch->param + V1_4_N_CFG_PARAMS);
#endif

	/* ... zusaetzlich Energiehistogramm initialisieren */
	cfg->ehist = mx_histogram_create(V1_4_EN_HIST_MIN,
				V1_4_EN_HIST_MAX, V1_4_EN_HIST_RES);
	mx_histogram_limit_set(cfg->ehist, V1_4_EN_HIST_LIMIT);
	mx_histogram_update_urange(cfg->ehist,
			ch->param[1], ch->param[2], V1_4_EN_HIST_IWEIGHT);

	/* ... und VAD-Schwellwert speichern */
	cfg->vad_threshold = ch->param[0];
	rs_msg("vad_threshold set to %g.", cfg->vad_threshold);

	return(0);
	}

/*
 * _dsp_mfcc_1_4_fprintparam(fp, fex, prefix)
 */
int _dsp_mfcc_1_4_fprintparam(FILE *fp, dsp_fextract_t *fex, char *prefix)
	{
	dsp_mfcc_t *cfg = fex->config;
	int i;

	/* aktuelle Kanaladaptions-Konfiguration ausgeben */
	fprintf(fp, "%s%g %g %g\n",
		prefix,
		cfg->vad_threshold,
		mx_histogram_invprob_le(cfg->ehist,
				V1_4_EN_HIST_PROBLOW),
		mx_histogram_invprob_le(cfg->ehist,
				V1_4_EN_HIST_PROBHIGH));
	fprintf(fp, "%s", prefix);
	for (i = 0; i < cfg->channel->nDim; i++)
		fprintf(fp, "%g ", cfg->channel->pMeans[i]);
	fprintf(fp, "\n");

	return(0);
	}

/*
 * _dsp_mfcc_1_4(features, signal)
 */
int _dsp_mfcc_1_4(dsp_fextract_t *fex,
		mx_real_t *features, dsp_sample_t *signal)
	{
	static mx_real_t *mtf = NULL, *bbr = NULL;
	static dsp_filterbank_t *fb = NULL;

	static dsp_sample_t *pre;
	static mx_real_t *w;
	static mx_real_t *p;
	static mx_complex_t *z;
	static mx_real_t *e;
	static mx_real_t *C;
	static dsp_sample_t s_minus_1 = 0;

	dsp_mfcc_t *cfg = fex->config;
	int i;
	mx_real_t e_sum = 0, e_sum2 = 0, le, norm_le;
	mx_real_t speech_le;
	int update_channel = 0;
	int clipped = 0;

	/* ... Filterbank ggf. erzeugen ... */
	if (fb == NULL) {
#ifdef USE_LINEAR_POWER_SPECTRUM
rs_msg("using correct linear power spectrum!");
#endif /* USE_LINEAR_POWER_SPECTRUM */
		/* ... Mel-Skala erzeugen ... */
		mtf = rs_malloc(sizeof(mx_real_t) * V1_1_N_FILTERS, "mtf");
		bbr = rs_malloc(sizeof(mx_real_t) * V1_1_N_FILTERS, "bbr");

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

		pre = rs_malloc(sizeof(dsp_sample_t) * V1_1_FRAME_LEN, "pre");
		w = rs_malloc(sizeof(mx_real_t) * V1_1_FRAME_LEN, "w");
		p = rs_malloc(sizeof(mx_real_t) * V1_1_FRAME_LEN, "p");
		z = rs_malloc(sizeof(mx_complex_t) * V1_1_FRAME_LEN, "z");
		e = rs_malloc(sizeof(mx_real_t) * (V1_1_N_FILTERS + 1), "e");
		C = rs_malloc(sizeof(mx_real_t) * V1_1_N_FILTERS, "C");
		}

	/* Merkmalsberechnung durchfuehren, dazu ... */
	/* ... zu allererst Aussteuerung pruefen ... */
	for (i = 0; i < V1_1_FRAME_LEN; i++) {
		if (signal[i] == SHRT_MIN || signal[i] == SHRT_MAX)
			clipped++;
		}
	if (clipped)
		; //rs_warning("signal amplitued clipped!");

	/* ... zuerst Praeemphase anwenden ... */
	dsp_preemph(pre, signal, V1_1_FRAME_LEN, V1_4_PREEMPH_A, s_minus_1);
	s_minus_1 = signal[V1_1_FRAME_SHIFT - 1];

	/* ... und mittelwertbereinigte Signalenergie berechnen ... */
	for (i = 0; i < V1_1_FRAME_LEN; i++) {
		e_sum += pre[i];
		e_sum2 += dsp_sqr(pre[i]);
		}
	le = dsp_log10(e_sum2 / V1_1_FRAME_LEN -
			dsp_sqr(e_sum / V1_1_FRAME_LEN));

	/* ... Energiehistogramm aktualisieren ... */
	mx_histogram_update(cfg->ehist, le);

	/* ... und Maximum (95%-Qantil) auf 0 normieren ... */
	norm_le = le - (mx_histogram_invprob_le(cfg->ehist, V1_4_EN_HIST_PROBHIGH)
			+ cfg->ehist->resolution / 2.0);

	/* ... Hamming-Fenstern ... */
	dsp_window_hamming(w, pre, V1_1_FRAME_LEN);
	
	/* ... Leistungsdichtespektrum ... */
	for (i = 0; i < V1_1_FRAME_LEN; i++) {
		mx_re(z[i]) = w[i];
		mx_im(z[i]) = 0.0;
		}
	dsp_xfft(z, V1_1_FRAME_LEN, 0);
	/* Guenther normiert hinwaerts mit 1/n --- wir nicht */
	for (i = 0; i <= V1_1_FRAME_LEN / 2; i++)
#ifdef USE_LINEAR_POWER_SPECTRUM
		p[i] = sqrt(dsp_sqr(mx_re(z[i])) + dsp_sqr(mx_im(z[i]))) /
					V1_1_FRAME_LEN;
#else
		p[i] = (dsp_sqr(mx_re(z[i])) + dsp_sqr(mx_im(z[i]))) /
					dsp_sqr(V1_1_FRAME_LEN);
#endif /* USE_LINEAR_POWER_SPECTRUM */

	/* ... Mel-Filter ... */
	dsp_filterbank_apply(e, p, fb);

	/* ... Logarithmierung ... */
	for (i = 0; i < V1_1_N_FILTERS + 1; i++)
		e[i] = dsp_log10(e[i]);

	/* ... Cepstrum ... */
	dsp_dct(C, e + 1, V1_1_N_FILTERS, V1_1_N_BASEFEATURES);

	/* ... Gesamtmerkmalsvektor erzeugen ... */
	features[0] = norm_le;
	memcpy(features + 1, C + 1, sizeof(mx_real_t) * (V1_1_N_BASEFEATURES - 1));

	/* ... Schwellwert fuer "Sprache" bestimmen ... */
	speech_le = mx_histogram_invprob_le(cfg->ehist, cfg->vad_threshold)
			- cfg->ehist->resolution / 2.0;
	if (cfg->vad_threshold > 0.0)
		update_channel = (le >= speech_le);
	else	update_channel = 1;

	/* ... Kanaladaption OHNE Energienormierung ... */
	dsp_channel_apply(cfg->channel, features, update_channel);
/*** fprintf(stderr, "%c", (update_channel) ? 's' : '-');  ***/

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
