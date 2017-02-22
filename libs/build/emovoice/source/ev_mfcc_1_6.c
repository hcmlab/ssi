#define DO_CHANNEL_IN_MELFB	/*** HACK HACK HACK ***/

#include <string.h>

#include "ev_memory.h"
#include "ev_messages.h"
#include "ev_io.h"


#include "ev_dct.h"

#include "ev_mfcc_1_6.h"

	/*
	 * Version 1.6:
	 *	wie Version 1.4 jedoch mit
	 *	- 1024er DFT (daher auch etwas groesserer Mel-Filterbank)
	 *	- Kompensation des Langzeitmittelwerts des Leistungsspektrums
	 *	  (i.e. "spectral subtraction" zur Geraeuschunterdrueckung)
	 */
/* Default Kanalparameter fuer bekannte Aufnamenbedingungen/-geraete */
#ifdef DO_CHANNEL_IN_CEPSTRUM
#define V1_6_N_PARAMS	(1 + 2 + V1_1_N_BASEFEATURES)
#endif /* DO_CHANNEL_IN_CEPSTRUM */

#ifdef DO_CHANNEL_IN_LOGSPEC
#define V1_6_N_PARAMS	(1 + 2 + (V1_6_FFT_LEN / 2 + 1))
#endif /* DO_CHANNEL_IN_LOGSPEC */

#ifdef DO_CHANNEL_IN_MELFB
#define V1_6_N_CFG_PARAMS	(1 + 2)
#define V1_6_N_CH_PARAMS	(V1_6_N_FILTERS + 1)
#define V1_6_N_P_HIST_PARAMS	(V1_6_FFT_LEN / 2 + 1)
#define V1_6_N_PARAMS		(V1_6_N_CFG_PARAMS + V1_6_N_CH_PARAMS + V1_6_N_P_HIST_PARAMS * 2)
#endif /* DO_CHANNEL_IN_MELFB */

#define V1_6_N_PARAM_DEFAULTS	1

static struct channel_t {
	char *name;
	mx_real_t param[V1_6_N_PARAMS];
		/*
		 * vad-thresh, e_low, e_high,
		 * E{energy}, E{C_i} ... E{n_0}, E{n_1} ...
		 */
	} channel[V1_6_N_PARAM_DEFAULTS] =
{	{"desklab",
	 { 0.0, 2, 6,
	   0, -2.95994, -0.340966, 0.00626176, -0.791733, -0.0397608, -0.17003,
	      0.349941, 0.230891, 0.188169, -0.157109, -0.272897, 0.0418244}
	 }
};

/**
* _dsp_mfcc_1_6_configure(fex, params)
**/
int _dsp_mfcc_1_6_configure(dsp_fextract_t *fex, char *params)
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

	/* ... dazu Kanaltyp bestimmen ... */
	if (!params || rs_line_is_empty(params)) {
		ch = channel + 0;

		rs_warning("no channel type for MFCCs v%d.%d given"
			" - using '%s'!",
			DSP_VERSION_MAJOR(fex->version),
			DSP_VERSION_MINOR(fex->version),
			ch->name);
		}
	else	{
		for (i = 0; i < V1_6_N_PARAM_DEFAULTS; i++) {
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

		for (i = 0; i < V1_6_N_PARAMS; i++) {
			status = sscanf(cp, "%g%n", ch->param + i, &pos);

			if (status != 1) {
				if (!rs_line_is_empty(cp))
					rs_error("error in channel definition: '%s'!",
						params);
				break;
				}
			cp += pos;
			}
		for (/* i = <implicit> */; i < V1_6_N_PARAMS; i++)
			ch->param[i] = 0.0;
/********
		rs_msg("MFCC channel parameters set to '%s'!",
			params);
********/
		}

	cfg->channel = dsp_channel_create(V1_6_N_CH_PARAMS);
	dsp_channel_configure(cfg->channel,
		/* {t, t_min, t_max} = */ 0, V1_3_MINTIME, V1_3_MAXTIME,
		ch->param + V1_6_N_CFG_PARAMS);

	/* ... zusaetzlich Energiehistogramm initialisieren */
	cfg->ehist = mx_histogram_create(V1_6_EN_HIST_MIN,
					V1_6_EN_HIST_MAX, V1_6_EN_HIST_RES);
	mx_histogram_limit_set(cfg->ehist, V1_6_EN_HIST_LIMIT);
	mx_histogram_update_urange(cfg->ehist,
		ch->param[1], ch->param[2], V1_6_EN_HIST_IWEIGHT);

	/* ... und VAD-Schwellwert speichern */
	cfg->vad_threshold = ch->param[0];
	rs_msg("vad_threshold set to %g.", cfg->vad_threshold);

	/* ... zusaetzlich Energiehistogramm fuer Mel-Spektrum */
	param = ch->param + V1_6_N_CFG_PARAMS + V1_6_N_CH_PARAMS;
	cfg->phist = rs_malloc(V1_6_N_P_HIST_PARAMS * sizeof(mx_histogram_t *),
				"mel spectrum histogram list");
	for (i = 0; i < V1_6_N_P_HIST_PARAMS; i++) {
		cfg->phist[i] = mx_histogram_create(V1_6_P_HIST_MIN,
                                        V1_6_P_HIST_MAX, V1_6_P_HIST_RES);
		mx_histogram_limit_set(cfg->phist[i], V1_6_P_HIST_LIMIT);
		mx_histogram_update_urange(cfg->phist[i],
			param[2*i], param[2*i + 1], V1_6_EN_HIST_IWEIGHT);
		}
rs_msg("power spectrum histogram time limit set to %d.", V1_6_P_HIST_LIMIT);
rs_msg("power spectrum resolution is %g.", V1_6_P_HIST_RES);
rs_msg("power spectrum floor is %g.", V1_6_P_HIST_FLOOR);

	/* ... ggf. temporaere Daten zerstoeren */
	if (!ch->name)
		rs_free(ch);

	return(0);
	}

int _dsp_mfcc_1_6_fprintparam(FILE *fp, dsp_fextract_t *fex, char *prefix)
	{
	dsp_mfcc_t *cfg = fex->config;
	int i;

	/* aktuelle Kanaladaptions-Konfiguration ausgeben */
	fprintf(fp, "%s%g %g %g\n",
		prefix,
		cfg->vad_threshold,
		mx_histogram_invprob_le(cfg->ehist,
				V1_6_EN_HIST_PROBLOW),
		mx_histogram_invprob_le(cfg->ehist,
				V1_6_EN_HIST_PROBHIGH));
	fprintf(fp, "%s", prefix);
	for (i = 0; i < cfg->channel->nDim; i++)
		fprintf(fp, "%g ", cfg->channel->pMeans[i]);
	fprintf(fp, "\n");

	fprintf(fp, "%s", prefix);
	for (i = 0; i < V1_6_N_P_HIST_PARAMS; i++)
		fprintf(fp, "%g %g\n",
			mx_histogram_invprob_le(cfg->phist[i],
				V1_6_P_HIST_PROBLOW),
			mx_histogram_invprob_le(cfg->phist[i],
				V1_6_P_HIST_PROBHIGH));

/**********
	fprintf(fp, "%s", prefix);
	for (i = 0; i < V1_6_N_FILTERS + 1; i++)
		mx_histogram_fprint(fp, cfg->phist[i]);
	fprintf(fp, "\n");
***********/

	return(0);
	}

/**
* _dsp_mfcc_1_6(fex, features, signal)
**/
int _dsp_mfcc_1_6(dsp_fextract_t *fex,
		mx_real_t *features, dsp_sample_t *signal)
	{
	/** static mx_real_t mtf[V1_6_N_FILTERS], bbr[V1_6_N_FILTERS]; **/
	static mx_real_t *mtf = NULL, *bbr = NULL;
	static dsp_filterbank_t *fb = NULL;
/*****
	static dsp_sample_t pre[V1_1_FRAME_LEN];
	static mx_real_t w[V1_1_FRAME_LEN];
	static mx_real_t p[V1_6_FFT_LEN];
	static mx_complex_t z[V1_6_FFT_LEN];
	static mx_real_t e[V1_6_N_FILTERS + 1];
	static mx_real_t C[V1_6_N_FILTERS];
****/
	static dsp_sample_t *pre;
	static mx_real_t *w;
	static mx_real_t *p;
	static mx_complex_t *z;
	static mx_real_t *e;
	static mx_real_t *C;
	static dsp_sample_t s_minus_1 = 0;

	dsp_mfcc_t *cfg = fex->config;
	int i, le_idx;
	mx_real_t e_sum = 0, e_sum2 = 0, le, norm_le;
	mx_real_t speech_le;
	int update_channel = 0;
	int clipped = 0;

	/* ... Filterbank etc. ggf. erzeugen ... */
	if (fb == NULL) {
rs_msg("using correct linear power spectrum!");

		mtf = rs_malloc(sizeof(mx_real_t) * V1_6_N_FILTERS, "mtf");
		bbr = rs_malloc(sizeof(mx_real_t) * V1_6_N_FILTERS, "bbr");

		/* ... Mel-Skala erzeugen ... */
		if (dsp_mel_create(mtf, bbr, V1_6_N_FRESOLUTION, 
			V1_6_MIN_FREQ, V1_1_MAX_FREQ, 1.0, V1_6_N_FILTERS)
				!= V1_6_N_FILTERS)
			rs_error("problems creating mel-scale!");

		/* ... Plateau ist 1/4 Frequenzgruppe breit ... */
		for (i = 0; i < V1_6_N_FILTERS; i++)
			bbr[i] /= 4;
		fb = dsp_filterbank_create(V1_6_N_FILTERS, mtf, bbr,
				V1_6_N_FRESOLUTION,
				V1_6_MIN_FREQ, V1_1_MAX_FREQ);

		pre = rs_malloc(sizeof(dsp_sample_t) * V1_1_FRAME_LEN, "pre");
		w = rs_malloc(sizeof(mx_real_t) * V1_1_FRAME_LEN, "w");
		p = rs_malloc(sizeof(mx_real_t) * V1_6_FFT_LEN, "p");
		z = rs_malloc(sizeof(mx_complex_t) * V1_6_FFT_LEN, "z");
		e = rs_malloc(sizeof(mx_real_t) * (V1_6_N_FILTERS + 1), "e");
		C = rs_malloc(sizeof(mx_real_t) * V1_6_N_FILTERS, "C");
		}

	/* Merkmalsberechnung durchfuehren, dazu ... */
	/* ... zu allererst Aussteuerung pruefen ... */
	for (i = 0; i < V1_1_FRAME_LEN; i++) {
		if (signal[i] == SHRT_MIN || signal[i] == SHRT_MAX)
                        clipped++;
		else if (clipped <= 1)
			clipped = 0;
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
	norm_le = le - (mx_histogram_invprob_le(cfg->ehist, V1_6_EN_HIST_PROBHIGH)
			+ cfg->ehist->resolution / 2.0);

	/* ... Schwellwert fuer "Sprache" bestimmen ... */
	speech_le = mx_histogram_invprob_le(cfg->ehist, cfg->vad_threshold);
	if (cfg->vad_threshold > 0.0)
		update_channel = (le >= speech_le);
	else	update_channel = 1;

	/* ... Hamming-Fenstern ... */
	dsp_window_hamming(w, pre, V1_1_FRAME_LEN);
	
	/* ... Leistungsdichtespektrum ... */
	for (i = 0; i < V1_1_FRAME_LEN; i++) {
		mx_re(z[i]) = w[i];
		mx_im(z[i]) = 0.0;
		}
	for (i = V1_1_FRAME_LEN; i < V1_6_FFT_LEN; i++) {
		mx_re(z[i]) = 0.0;
		mx_im(z[i]) = 0.0;
		}
	dsp_xfft(z, V1_6_FFT_LEN, 0);
	/* Guenther normiert hinwaerts mit 1/n --- wir nicht */
	for (i = 0; i <= V1_6_FFT_LEN / 2; i++)	/* Power-Spektrum symmetrisch!*/
		p[i] = sqrt(dsp_sqr(mx_re(z[i])) + dsp_sqr(mx_im(z[i]))) /
					V1_6_FFT_LEN;

	/* ... Histogramme fuer Filterenergieen aktualisieren ... */
	for (i = 0; i < V1_6_N_P_HIST_PARAMS; i++) {
		mx_histogram_update(cfg->phist[i], p[i]);

		p[i] = p[i]	- mx_histogram_invprob_le(cfg->phist[i],
					V1_6_P_HIST_PROBLOW)
				+ V1_6_P_HIST_FLOOR;
/******
		p[i] -= (mx_histogram_invprob_le(cfg->phist[i],
				V1_6_P_HIST_PROBLOW) -
			 cfg->phist[i]->resolution);
******/
		}

#ifdef DO_CHANNEL_IN_LOGSPEC
	/* ... erste Logarithmierung ... */
	for (i = 0; i <= V1_6_FFT_LEN / 2; i++)
		p[i] = dsp_log10(p[i]);

	/* ... Kanaladaption OHNE Energienormierung ... */
	dsp_channel_apply(cfg->channel, p, update_channel);

	/* ... erste Logarithmierung rueckgaengig machen ... */
	for (i = 0; i <= V1_6_FFT_LEN / 2; i++)
		p[i] = pow(10.0, p[i]);
#endif /* DO_CHANNEL_IN_LOGSPEC */

	/* ... Mel-Filter ... */
	dsp_filterbank_apply(e, p, fb);

	/* ... Logarithmierung ... */
	for (i = 0; i < V1_6_N_FILTERS + 1; i++)
		e[i] = dsp_log10(e[i]);

#ifdef DO_CHANNEL_IN_MELFB
	/* ... Kanaladaption OHNE Energienormierung ... */
	dsp_channel_apply(cfg->channel, e, update_channel);
#endif /* DO_CHANNEL_IN_MELFB */

	/* ... Cepstrum ... */
	dsp_dct(C, e + 1, V1_6_N_FILTERS, V1_1_N_BASEFEATURES);

	/* ... Gesamtmerkmalsvektor erzeugen ... */
	features[0] = norm_le;
	memcpy(features + 1, C + 1, sizeof(mx_real_t) * (V1_1_N_BASEFEATURES - 1));

#ifdef DO_CHANNEL_IN_CEPSTRUM
	/* ... Kanaladaption OHNE Energienormierung ... */
	dsp_channel_apply(cfg->channel, features, update_channel);
#endif /* DO_CHANNEL_IN_CEPSTRUM */

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
