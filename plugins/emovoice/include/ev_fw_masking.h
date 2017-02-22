/**
* Datei:	fw_masking.h
* Autor:	Sascha Wendt
* Datum:	9.4.2001
*
* Beschreibung:	Definitionen fuer Datenstrukturen und Vorwaertsmaskierung
*	
**/

#ifndef __DSP_FORWARD_MASKING_H_INCLUDED__
#define __DSP_FORWARD_MASKING_H_INCLUDED__

#include "ev_real.h"

typedef struct tone_t {
  mx_real_t delta_spl;
  int   decay_b;
  int   decay_a;
}tone_t;

typedef struct dsp_fwm_param_t{
  mx_real_t* attack;
  mx_real_t* release;
  mx_real_t* slope;
  mx_real_t* threshold;
  mx_real_t* hoerschwelle;

  /* Speicherung der Toene */
  tone_t **toene_vektor; 
  int *last_tones;

  /* Speicherung der Masken */
  tone_t **masken_vektor;
  int *last_masks;

  /* Speicherung des jeweils letzten Tones fuer jede Frequenzgruppe */
  mx_real_t *last_ins;

  /* Neuer Ton in Frequenzgruppe? Ja => probe_on = 1, sonst 0 */
  int *probes_on;
}dsp_fwm_param_t;


void dsp_masking(mx_real_t *maskiert, mx_real_t *spektrum, int n_channel, dsp_fwm_param_t* params);

/*mx_real_t fw_masking_new(int channel, mx_real_t in, mx_real_t attack, mx_real_t release, mx_real_t slope, mx_real_t threshold, mx_real_t hoerschwelle);*/

dsp_fwm_param_t* dsp_init_forward_masking(mx_real_t *mtf, int n_channel);

mx_real_t dsp_calc_energy(mx_real_t* channels, int n_channel);

void dsp_set_minima(dsp_fwm_param_t* params, mx_real_t *spektrum, int n_channel);

void dsp_apply_minima(dsp_fwm_param_t* params, mx_real_t *spektrum, int n_channel);

#endif /*  __DSP_FORWARD_MASKING_H_INCLUDED__ */



