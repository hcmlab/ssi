	/*
	 * Version 2.4:
	 *	wie Version 1.4 jedoch stereo mit beamforming
	 */
/**
* _dsp_mfcc_2_4(features, signal)
**/
static int _dsp_mfcc_2_4(dsp_fextract_t *fex,
		mx_real_t *features, dsp_sample_t *signal)
	{
	static dsp_sample_t mono[V1_1_FRAME_LEN];
	static mx_real_t mtf[V1_1_N_FILTERS], bbr[V1_1_N_FILTERS];
	static dsp_filterbank_t *fb = NULL;
	static dsp_sample_t pre[V1_1_FRAME_LEN];
	static mx_real_t w[V1_1_FRAME_LEN];
	static mx_real_t p[V1_1_FRAME_LEN];
	static mx_complex_t z[V1_1_FRAME_LEN];
	static mx_real_t e[V1_1_N_FILTERS + 1];
	static mx_real_t C[V2_4_N_CHANNELS][V1_1_N_FILTERS];
	static dsp_sample_t s_minus_1[V2_4_N_CHANNELS] = {0, 0};

	dsp_mfcc_t *cfg = fex->config;
	int c, i, le_idx;
	mx_real_t e_sum, e_sum2, le;

	/* ... Filterbank ggf. erzeugen ... */
	if (fb == NULL) {
/*
 * NOTE: initialisation message!
 */
/*******
rs_msg("MFCC-type feature extraction v2.4: beamforming = avg. cepstrum");
*******/
rs_msg("MFCC-type feature extraction v2.4: beamforming = log avg. exp cepstrum");

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

#ifdef USETHIS
/* parameter checken */
dsp_mfcc_fprintparam(stderr, fex, "");
#endif
		}

	/* Merkmalsberechnung durchfuehren, dazu ... */

/* ... fuer alle (2, stereo) Kanaele ... */
for (c = 0; c < V2_4_N_CHANNELS; c++) {

	/* ... aktuellen Kanal extrahieren ... */
	for (i = 0; i < V1_1_FRAME_LEN; i++)
		mono[i] = signal[i * V2_4_N_CHANNELS + c];

	/* ... zuerst Praeemphase anwenden ... */
	dsp_preemph(pre, mono, V1_1_FRAME_LEN, V1_4_PREEMPH_A, s_minus_1[c]);
	s_minus_1[c] = mono[V1_1_FRAME_SHIFT - 1];

	/* ... und mittelwertbereinigte Signalenergie berechnen ... */
	e_sum = e_sum2 = 0;	/* Accumulatorwerte INITIALISIEREN! */
	for (i = 0; i < V1_1_FRAME_LEN; i++) {
		e_sum += pre[i];
		e_sum2 += dsp_sqr(pre[i]);
		}
	le = dsp_log10(e_sum2 / V1_1_FRAME_LEN -
			dsp_sqr(e_sum / V1_1_FRAME_LEN));

	/* ... Energiehistogramm aktualisieren ... */
	mx_histogram_update(cfg->ehists[c], le);

	/* ... und Maximum (95%-Qantil) auf 0 normieren ... */
	le = le - (mx_histogram_invprob_le(cfg->ehists[c], V1_4_EN_HIST_PROBHIGH)
			+ cfg->ehists[c]->resolution / 2.0);

	/* ... Hamming-Fenstern ... */
	dsp_window_hamming(w, pre, V1_1_FRAME_LEN);
	
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
	dsp_dct(C[c], e + 1, V1_1_N_FILTERS, V1_1_N_BASEFEATURES);

	/* ... C0 durch log(Energy) ersetzen ... */
	C[c][0] = le;

	/* ... Kanaladaption OHNE Energienormierung ... */
	dsp_channel_plain(cfg->channels[c], C[c]);
	}

	/* ... Gesamtmerkmalsvektor erzeugen ... */
	for (i = 0; i < V1_1_N_BASEFEATURES; i++)
		/** features[i] = (C[0][i] + C[1][i]) / 2.0; **/
		features[i] = dsp_log((exp(C[0][i]) + exp(C[1][i])) / 2.0);
#ifdef USETHISGARBAGE
{
int c, j;
for (c = 0; c < V2_4_N_CHANNELS; c++) {
	printf("C[%d][0-4] = ", c);
	for(j = 0; j <= 4; j++) printf("%g ", C[c][j]);
	printf("\n");
	}

printf("beamC[0-4] = ");
	for(i = 0; i <= 4; i++) printf("%g ", features[i]);
	printf("\n");
}
#endif /* USETHISGARBAGE */

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

