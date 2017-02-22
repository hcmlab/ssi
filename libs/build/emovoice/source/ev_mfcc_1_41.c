	/*
	 * Version 1.41:
	 *	wie Version 1.4 jedoch werden keine Cepstren berechnet
	 *	(d.h. nur das log-mel-Spektrum) und der Kanal in
	 *	dieser Repraesentation kompensiert
	 */
#include "ev_mfcc_1_4.h"
#define V1_41_N_BASEFEATURES	(1+V1_1_N_FILTERS)
#define V1_41_N_FEATURES	(3*V1_41_N_BASEFEATURES)

/*
* _dsp_mfcc_1_41_configure(fex, params)
**/
int _dsp_mfcc_1_41_configure(dsp_fextract_t *fex, char *params)
	{
	dsp_mfcc_t *cfg = fex->config;
	struct channel_t *ch = NULL;
	int i, pos, status;
	char *cp;
	/** mx_real_t *param; **/

	/* Abtastrate etc. ... */
	fex->samplerate =	V1_1_SAMPLERATE;
	fex->n_channels =	V1_1_N_CHANNELS;
	fex->frame_len =	V1_1_FRAME_LEN;
	fex->frame_shift =	V1_1_FRAME_SHIFT;
	fex->n_features =	V1_41_N_FEATURES;

	/* Verzoegerungselement fuer Ableitung erzeugen ... */
	cfg->wderiv = dsp_delay_create(V1_1_W_LENGTH,
			sizeof(mx_real_t),
			V1_41_N_BASEFEATURES);

	/* Kanaladaption erzeugen/initialisieren */
	{
	mx_real_t chparam_cfg[1 + 2 + V1_41_N_BASEFEATURES];
	mx_real_t *chparam;
	mx_real_t *enparam;

	/* ... dazu Kanaltyp bestimmen ... */
	if (!params || rs_line_is_empty(params)) {
		rs_warning("no channel type for MFCCs given - using partial 'desklab'!");
		/** chparam = chparam_desklab; **/
		chparam = NULL;
		enparam = enparam_desklab;
		}
	else	{
		cp = params;
		for (i = 0; i < sizeof(chparam_cfg) /
				sizeof(mx_real_t); i++)
			chparam_cfg[i] = 0;
		for (i = 0; i < sizeof(chparam_cfg) /
				sizeof(mx_real_t); i++) {
			if (sscanf(cp, "%g%n",
			    chparam_cfg + i, &pos) != 1) {
				if (!rs_line_is_empty(cp))
					rs_error("error in channel definition: '%s'!",
						params); 
				break;
				}
			cp += pos;
			}
		chparam = chparam_cfg;
		enparam = chparam + 1;
/**
		rs_msg("MFCC channel parameters set to '%s'!", params);
**/
		}

	cfg->channel = dsp_channel_init(V1_41_N_BASEFEATURES,
				V1_2_I_ENERGY, V1_3_MINTIME, V1_3_MAXTIME,
				V1_2_DECAY, V1_2_ENERGY_PERCENT,  chparam);
	/* ... zusaetzlich Energiehistogramm initialisieren */
	cfg->ehist = mx_histogram_create(V1_4_EN_HIST_MIN,
				V1_4_EN_HIST_MAX, V1_4_EN_HIST_RES);
	mx_histogram_limit_set(cfg->ehist, V1_4_EN_HIST_LIMIT);
	mx_histogram_update_urange(cfg->ehist,
				enparam[0], enparam[1], V1_4_EN_HIST_IWEIGHT);

	/* ... und VAD-Schwellwert speichern */
	cfg->vad_threshold = (chparam) ? chparam[0] : 0;
rs_msg("vad_threshold set to %g.", cfg->vad_threshold);
	}

	return(0);
	}
/**
* _dsp_mfcc_1_41(features, signal)
**/
int _dsp_mfcc_1_41(dsp_fextract_t *fex,
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
		rs_warning("signal amplitued clipped!");

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

rs_msg("mfcc 1.41: e[1..4] = [%g, %g, %g, %g]",
		e[1], e[2], e[3], e[4]);

	/* ... Logarithmierung ... */
	for (i = 0; i < V1_1_N_FILTERS + 1; i++)
		e[i] = dsp_log10(e[i]);

#ifdef DISABLED_FROM_V1_4
	/* ... Cepstrum ... */
	dsp_dct(C, e + 1, V1_1_N_FILTERS, V1_1_N_BASEFEATURES);
#endif

	/* ... Gesamtmerkmalsvektor erzeugen ... */
	features[0] = norm_le;
	memcpy(features + 1, e + 1, sizeof(mx_real_t) * (V1_41_N_BASEFEATURES - 1));

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
	if (dsp_deriv(features + V1_41_N_BASEFEATURES, cfg->wderiv, 1, V1_1_W_LENGTH) == 0 &&
	    dsp_deriv(features + 2 * V1_41_N_BASEFEATURES, cfg->wderiv, 2, V1_1_W_LENGTH) == 0) {
		/* ... zugehoerigen mittleren Vektor getlaettet erzeugen ... */
		dsp_tirol(features, cfg->wderiv);

		/* ... und Daten als gueltig erklaeren ... */
		return(V1_41_N_FEATURES);
		}
	else	return(0);	/* ... sonst: (noch) keine Merkmale! */
	}

