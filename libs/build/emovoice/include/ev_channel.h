/**
* Datei:	channel.h
* Autor:	Robert Haschke
* Datum:	10.11.1997
*
* Beschreibung:	Definitionen fuer Datenstrukturen und globalen
*		Kanaladaption durch Mittelwertbereinigung des Cepstrums
**/

#ifndef __DSP_CHANNEL_H_INCLUDED__
#define __DSP_CHANNEL_H_INCLUDED__

#include "ev_real.h"

/**
* Datentypen
**/

typedef struct {
	int nDim;		/* Dimension der Merkmalsvektoren */
	int nTime;		/* ! Zeitzaehler */
	int nTime_Min;
	int nTime_Max;

	mx_real_t *pMeans;	/* ! Mittelwerte der Cepstren */

	/* Dynamikbereich-Adaption */
	int iEnergy;		/* Index der Energiekomponente */
	mx_real_t rPercent;	/* percentage of energy dynamics to use for
				   adaption */
	mx_real_t fMax[2];	/* ! default and actual maximal energy */
	mx_real_t fMin[2];	/* ! default and actual minimal energy */
				/* default values are used for actual range */
	int        nUsed[2];
	mx_real_t rDecay;
	} dsp_channel_t;

/**
* Funktionsprototypen
**/

dsp_channel_t *dsp_channel_create(int nDim);
int dsp_channel_configure(dsp_channel_t *ch,
		int t, int t_min, int t_max, mx_real_t *mean);
dsp_channel_t *dsp_channel_init (int nDim, int iEnergy, 
				 int nTime_Min, int nTime_Max,
				 mx_real_t rDecay, mx_real_t rPercent,
				 mx_real_t *rParameters);
void dsp_channel(dsp_channel_t *pData, mx_real_t *features);
void dsp_channel_plain(dsp_channel_t *pData, mx_real_t *features);
void dsp_channel_apply(dsp_channel_t *pData, mx_real_t *features, int update);

void dsp_channel_in(dsp_channel_t *pData, mx_real_t *features);
void dsp_channel_out(dsp_channel_t *pData, mx_real_t *features);

int dsp_channel_close(dsp_channel_t *pData);

#endif /* __DSP_CHANNEL_H_INCLUDED__ */
