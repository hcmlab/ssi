/**
* Datei:	vad.c
* Autor:	Gernot A. Fink
* Datum:	30.8.2000
*
* Beschreibung:	"voice activity detection", d.h. Klassifikation von
*		Signalabschnitten nach Sprache bzw. Pause in konservativer
*		Weise (d.h. Sprachabschnitte duerfen Pausen enthalten aber
*		nicht umgekehrt) in verschiedenen Versionen:
*
*	1.0:	Entscheidungsmerkmal ist die mittelwertbereinigte, unnormierte
*		logarithmische Signalenergie, die mittels eines Histogramms
*		zeitlich begrenzter Ausdehnung auf den Bereich [0.0 .. 1.0]
*		normiert wird.
*
*		Die potentielle Sprachframes werden gespeichert, sobald
*		die histogramm-relative Signalenergie V1_0_VA_THRESH_START
*		ueberschreitet. Wird auch V1_0_VA_THRESH_VOICE ueberschritten,
*		liegt Sprache vor, solange V1_0_VA_THRESH_STOP nicht laenger
*		als fuer insgesamt V1_0_VA_GAP_LEN unterschritten wird.
*
*		Da das Verfahren nur bei hinreichend positivem Signal-Rausch-
*		Abstand anwendbar ist, wird bei Unterschreiten von V1_0_SNR_MIN
*		in der Dynamik des Energiehistogramms fuer Pause entschieden.
**/

#include <stdio.h>
#include <string.h>
//#include <unistd.h>

#include "ev_memory.h"
#include "ev_messages.h"


#include "ev_vad.h"

/* Konstantendefinitionen fuer ... */
/* ... voice activity detection V1.0: */
#define V1_0_PREEMPH_A		1.0

#define V1_0_EHIST_MIN		0
#define V1_0_EHIST_MAX		10
#define V1_0_EHIST_RES		0.5
#define V1_0_EHIST_LEN		1000

#define V1_0_EHIST_BUCKET_HIGH	0.5	/* Buckets > 50% ggf. nicht fuellen! */

#define V1_0_PROB_LOW		0.05
#define V1_0_PROB_HIGH		0.95

#define V1_0_BUFFER_LEN		500

#define V1_0_SNR_MIN		15
#define V1_0_VA_THRESH_START	0.2
#define V1_0_VA_THRESH_VOICE	0.8
#define V1_0_VA_THRESH_STOP	0.5
#define V1_0_VA_GAP_LEN		20

static int _dsp_vad_calc_1_0(dsp_sample_t *voice,
				dsp_vad_t *vad, dsp_sample_t *samples);
static dsp_vad_state_t _dsp_vad_newstate_1_0(dsp_vad_t *vad, mx_real_t va);

dsp_vad_t *dsp_vad_create(int version, int frame_len)
	{
	dsp_vad_t *vad;

	/* Parameter pruefen ... */
	if (frame_len <= 0)
		return(NULL);

	/* ... speziellen Datenbereich erzeugen ... */
	vad = rs_malloc(sizeof(dsp_vad_t), "VAD configuration data");

	/* ... und gemaess Version fuellen ... */
	vad->version = version;
	vad->frame_len = frame_len;
	vad->state = dsp_vad_no_decision;
	vad->n_no_va_frames = 0;

	vad->signal = rs_malloc(sizeof(dsp_sample_t) * vad->frame_len,
				"VAD frame buffer");
		/* optionale Eintraege - noch nicht verfuegbar! */
	vad->sigbuf = NULL;
	vad->ehist = NULL;

	switch(vad->version) {
		case DSP_MK_VERSION(1, 0):
			/* ... Signalpuffer hinzufuegen ... */
			vad->sigbuf = dsp_delay_create(V1_0_BUFFER_LEN,
					sizeof(dsp_sample_t), vad->frame_len);

			/* ... und begrenztes Energiehistogramm erzeugen */
			vad->ehist = mx_histogram_create(
				V1_0_EHIST_MIN, V1_0_EHIST_MAX, V1_0_EHIST_RES);
			mx_histogram_limit_set(vad->ehist, V1_0_EHIST_LEN);
			vad->last_idx = -1;

			break;
		default:
			dsp_vad_destroy(vad);
			return(NULL);
		}

	return(vad);
	}

void dsp_vad_destroy(dsp_vad_t *vad)
	{
	/* Parameter pruefen ... */
	if (!vad)
		return;

	/* ... Datenbereich unbrauchbar machen und freigeben */
	if (vad->signal)
		rs_free(vad->signal);
	if (vad->sigbuf)
		dsp_delay_destroy(vad->sigbuf);
	if (vad->ehist)
		mx_histogram_destroy(vad->ehist);
	memset(vad, -1, sizeof(dsp_vad_t));
	rs_free(vad);
	}

void dsp_vad_reset(dsp_vad_t *vad)
	{
	/* Parameter pruefen ... */
	if (!vad)
		return;

	/* ... und gemaess Version ruecksetzen ... */
	vad->state = dsp_vad_no_decision;
	vad->n_no_va_frames = 0;

	switch(vad->version) {
		case DSP_MK_VERSION(1, 0):
			dsp_delay_flush(vad->sigbuf);

			mx_histogram_reset(vad->ehist);
			vad->last_idx = -1;

			break;
		default:
			return;
		}
	}

/**
* dsp_vad_calc(&voice[], vad, samples[])
*	Fuehrt eine "voice activity detection" durch. Dazu wird in
*	in Abhaengigkeit vom internen in 'vad' gepeichterten Zustand
*	sowie der neuen Signaldaten 'samples[]' ein Zustandsuebergang
*	durchgefuehrt und die Signaldaten im internen Puffer gespeichert.
*
*	Aus diesem Puffer wird anschliessend ggf. wieder ein Signalframe
*	entnommen, der i.A. um einige Frames verzoegert wurde.
*
*	Liefert 1, falls ein Sprachframe aus dem Puffer extrahiert wurde,
*	0 bei einem Pausenframe und -1, falls keine Daten entnommen werden
*	konnten bzw. im Fehlerfalle.
**/
int dsp_vad_calc(dsp_sample_t *voice, dsp_vad_t *vad, dsp_sample_t *samples)
	{
	dsp_vad_state_t mark;

	/* Parameter pruefen ... */
	if (!vad)
		return(-1);

	/* ... falls neue Daten vorliegen ... */
	if (samples) {
		/* ... Zustandsaenderung gemaess Version berechnen ... */
		switch(vad->version) {
			case DSP_MK_VERSION(1, 0):
				_dsp_vad_calc_1_0(voice, vad, samples);
				break;
			default:
				return(-1);
			}
		}

	/* ... ggf. verzoegerten Frame zur Verarbeitung zurueckgeben ... */
	if (dsp_delay_topm(NULL, &mark, vad->sigbuf) < 0)
		return(-1);

	/* ... Pause ... */
	if (mark <= dsp_vad_silence) {
		dsp_delay_pop(voice, vad->sigbuf);
		return(0);
		}
	/* ... evtl. beginnender Sprachabschnitt ... */
	else if (mark == dsp_vad_starting)
		return(-1);
	/* ... bzw. Sprache */
	else	{
		dsp_delay_pop(voice, vad->sigbuf);
		return(1);
		}
	}

static int _dsp_vad_calc_1_0(dsp_sample_t *voice,
			dsp_vad_t *vad, dsp_sample_t *samples)
	{
	int i, idx, activity = 0;
	mx_real_t e_sum, e_sum2, le, lemin, lemax, va;
	mx_real_t snr;
	dsp_vad_state_t newstate, mark;

	/* Praeemphase ausfuehren ... */
	dsp_preemph(vad->signal, samples, vad->frame_len, V1_0_PREEMPH_A, 0);

	/* ... und mittelwertbereinigte Signalenergie berechnen ... */
	e_sum = e_sum2 = 0;
	for (i = 0; i < vad->frame_len; i++) {
		e_sum += vad->signal[i];
		e_sum2 += dsp_sqr(vad->signal[i]);
		}
	le = dsp_log10(e_sum2 / vad->frame_len -
			dsp_sqr(e_sum / vad->frame_len));

	/* ... ggf. Energiehistogram aktualisieren ... */
	idx = mx_histogram_val2idx(vad->ehist, le);
	if (!(idx >= 0 && idx == vad->last_idx &&
	      mx_histogram_prob(vad->ehist, le) > V1_0_EHIST_BUCKET_HIGH))
		mx_histogram_update(vad->ehist, le);
	vad->last_idx = idx;

	/* ... Histogrammdynamik ermitteln ... */
	lemin = mx_histogram_invprob_le(vad->ehist, V1_0_PROB_LOW) -
				vad->ehist->resolution / 2;
	lemax = mx_histogram_invprob_le(vad->ehist, V1_0_PROB_HIGH) +
				vad->ehist->resolution / 2;

	/* ... Signalrauschabstand schaetzen und ... */
	snr = 10 * (lemax - lemin);

	/* ... Entscheidungsparameter fuer voice activity berechnen ... */
	if (snr >= V1_0_SNR_MIN) 
		va = (le - lemin) / (lemax - lemin);
	else	va = 0.0;

	newstate = _dsp_vad_newstate_1_0(vad, va);

	/* ... ggf. bei best. Zustandsuebergaengen Daten manipulieren ... */
	/* ... falls 'starting -> no_decision/silence/voice' ... */
	if (vad->state == dsp_vad_starting && newstate != dsp_vad_starting) {
		/*
		 * ... alle mit Marke 'starting' gespeicherten Signalframes
		 * nach neu berechmetem Zustand ummarkieren
		 */
		for (i = 0; i < vad->sigbuf->length; i++) {
			if (dsp_delay_accessm(NULL, &mark, vad->sigbuf, i) < 0)
				break;

			if (mark != dsp_vad_starting)
				break;

			dsp_delay_mark(vad->sigbuf, i, newstate);
			}
		}

	/* ... und neu berechneten Zustand uebernehmen ... */
	vad->state = newstate;

	/* ... aktuellen Frame IMMER speichern ... */
	dsp_delay_pushm(vad->sigbuf, samples, vad->state);

	return(vad->state);
	}

static dsp_vad_state_t _dsp_vad_newstate_1_0(dsp_vad_t *vad, mx_real_t va)
	{
	dsp_vad_state_t state;

	/* ... je nach internem Zustand entscheiden ... */
	state = vad->state;
	switch (state) {
		case dsp_vad_no_decision:
		case dsp_vad_silence:
			if (va > V1_0_VA_THRESH_VOICE)
				state = dsp_vad_voice;
			else if (va > V1_0_VA_THRESH_START)
				state = dsp_vad_starting;
			break;

		case dsp_vad_starting:
			if (va <= V1_0_VA_THRESH_START)
				state = dsp_vad_silence;
			else if (va > V1_0_VA_THRESH_VOICE)
				state = dsp_vad_voice;
			break;

		case dsp_vad_voice:
			if (va <= V1_0_VA_THRESH_VOICE) {
				state = dsp_vad_stopping;
				vad->n_no_va_frames = 0;
				}
			break;

		case dsp_vad_stopping:
			if (va > V1_0_VA_THRESH_VOICE)
				state = dsp_vad_voice;
			else if (va <= V1_0_VA_THRESH_STOP)
				vad->n_no_va_frames++;

			if (vad->n_no_va_frames > V1_0_VA_GAP_LEN)
				state = dsp_vad_starting;
			
			break;

		default:
			rs_error("internal VAD data corrupted!");
		}

	return(state);
	}
