/**
* Autor:	Robert Haschke & Gernot A. Fink
* Datum:	10.11.1997 & 1998-2000
*
* Beschreibung:	Kanaladaption durch Mittelwertbereinigung des Cepstrums
**/

#include <stdio.h>
#include <math.h>

#include "ev_real.h"

#include "ev_memory.h"
#include "ev_messages.h"


#include "ev_channel.h"

dsp_channel_t *dsp_channel_create(int dim)
	{
	dsp_channel_t *pData;

	// begin_change_thurid
	//int i, j;
	// end_change_thurid

	pData = rs_malloc(sizeof (dsp_channel_t), "channel data");
	pData->pMeans = rs_calloc(dim, sizeof(mx_real_t), "channel mean");

	pData->nDim = dim;
	pData->nTime = 0;
	pData->nTime_Min = 0;
	pData->nTime_Max = 0;

	/* obsolete Energie-Dynamik-Adaption (nur mfcc v1.2) */
	pData->iEnergy = -1;
	pData->rPercent = 0;
	pData->fMin[1] =  1.0e20;
	pData->fMax[1] = -1.0e20;
	pData->nUsed[0] = pData->nUsed[1] = 0;
	pData->rDecay = 0;

	return(pData);
	}

int dsp_channel_configure(dsp_channel_t *ch,
		int t, int t_min, int t_max, mx_real_t *mean)
	{
	int i;

	/* first check parameters ... */
	if (!ch || t_min < 0 || t_max < t_min)
		return(-1);

	/* ... save time limits for mean adaptation ... */
	ch->nTime = t;
	ch->nTime_Min = t_min;
	ch->nTime_Max = t_max;

	/* ... and eventually initialize mean vector */
	if (mean) {
		for (i = 0; i < ch->nDim; i++)
			ch->pMeans[i] = mean[i];
		}

	return(0);
	}

dsp_channel_t *dsp_channel_init (int nDim, int iEnergy, 
				 int nTime_Min, int nTime_Max,
				 mx_real_t rDecay, mx_real_t rPercent,
				 mx_real_t *rParameters)
	{
	dsp_channel_t *pData;

	// begin_change_thurid
	int j;
	// end_change_thurid

	pData = rs_malloc(sizeof (dsp_channel_t), "channel data");
	/*** pData->pMeans = dsp_vector_create (nDim); ***/
	pData->pMeans = rs_calloc(nDim, sizeof(mx_real_t), "channel mean");

	pData->nDim = nDim;
	pData->nTime_Min = nTime_Min;
	pData->nTime_Max = nTime_Max;

	pData->iEnergy = iEnergy;
	pData->rPercent = rPercent;
	pData->fMin[1] =  1.0e20;
	pData->fMax[1] = -1.0e20;
	pData->nUsed[0] = pData->nUsed[1] = 0;
	pData->rDecay = rDecay;

	/* eventuell sollte man den Zeitschritt nTime wieder auf Null setzen,
	 damit (ausgehend von den aktuellen Mittelwerten) neu adaptiert wird,
	 immerhin beginnt ja ein neuer Turn ... */

	/* restliche Daten mit uebergebenen Optionen initialisieren */
if (rParameters) {
	/* aktueller Zeitschritt, default min/max energy level */
	pData->nTime = rParameters[0];
	pData->fMin[0] = rParameters[1];
	pData->fMax[0] = rParameters[2];

	/* aktuelle Mittelwerte des Cepstrums uebernehmen */
	for (j = 0; j < nDim; j++)
		pData->pMeans[j] = rParameters[3 + j];
	}
else	{
	pData->nTime = 0.0;
	pData->fMin[0] = 0.0;
	pData->fMax[0] = 0.0;

	for (j = 0; j < nDim; j++)
		pData->pMeans[j] = 0.0;
	}

	return pData;
	}

int dsp_channel_close (dsp_channel_t *pData)
	{
	int i;

	/* aktuelle Informationen ausgeben */
	printf (" %d %g %g ", pData->nTime, pData->fMin[0], pData->fMax[0]);
	for (i=0; i < pData->nDim; i++)
		printf ("%g ", pData->pMeans[i]);
	printf ("\n");

	return 0;
	}

/* Gewichtung der Cepstralwerte zur Mittelwertsbestimmung:
           t < t_min : 1 / t_min
   t_min < t < t_max : 1 / t
   t_max < t         : 1 / t_max
*/
#define weight(t,min,max)	((t < min) ? 1/(mx_real_t)min : \
			 (t > max) ? 1/(mx_real_t)max : 1/(mx_real_t)t)

/* Gewichtung der neuen Mininma/Maxima gegenueber den
   alten, sicheren Werten soll sigmoid mit der Zahl x
   der Aenderungen erfolgen: -> tanh 
   Folgende Funktion nimmt Werte zwischen 0 und 0.2 an,
   nach 15 Updates, ist der Nulldurchgang bei 0.1 erreicht */
#define en_weight(x) (0.1*(1+tanh(0.2*(double)(x-15.0))))

void dsp_channel(dsp_channel_t *pData, mx_real_t *features)
	{
	int iEnergy = pData->iEnergy;
	mx_real_t rDiff;
	mx_real_t rAlpha = weight (pData->nTime, 
					pData->nTime_Min, pData->nTime_Max);
	int i;

	for (i = 0; i < pData->nDim; i++)
		if (i != iEnergy) {
			/* Mittelwert-Bestimmung (Update) */
			pData->pMeans[i] = rAlpha * features[i] + (1-rAlpha) * pData->pMeans[i];
			/* Mittelwert-Bereinigung */
			features[i] = features[i] - pData->pMeans[i];
			}

	/* Energie-Dynamik-Adaption */
	/* adapt maximum and minimum */
	rDiff = pData->fMax[0] - pData->fMin[0];
	if (features[iEnergy] < pData->fMin[1]) {
		pData->fMin[1] = features[iEnergy];
		pData->nUsed[0]++;
		}
	/* falls der Energiewert in der unteren Haelfte der Dynamik liegt,
	   wird er langsam (exp) wieder runtergefahren... */
	else if (features[iEnergy] < pData->fMin[0] + 0.5 * rDiff)
		pData->fMin[1] += rDiff * pData->rDecay;

	if (features[iEnergy] > pData->fMax[1]) {
		pData->fMax[1] = features[iEnergy];
		pData->nUsed[1]++;
		}
	/* falls der Energiewert in der oberen Haelfte der Dynamik liegt,
	   wird er langsam (exp) wieder erhoeht */ 
	else if (features[iEnergy] > pData->fMin[0] + 0.5 * rDiff)
		pData->fMax[1] -= rDiff * pData->rDecay;

	/* adapt energy default min/max values */
	pData->fMin[0] = pData->fMin[0] + 
		en_weight (pData->nUsed[0]) * (pData->fMin[1] - pData->fMin[0]);
	pData->fMax[0] = pData->fMax[0] + 
		en_weight (pData->nUsed[1]) * (pData->fMax[1] - pData->fMax[0]);
	
	/* Falls der Energiewert oberhalb von rPercent der Dynamik liegt,
	   wird der Zeit-Parameter nTime erhoeht, der die Gewichtung der
	   Cepstral-Werte bei der Mittelwertsbestimmung bestimmt */
	if (features[iEnergy] > pData->fMin[0] + 
			pData->rPercent * (pData->fMax[0] - pData->fMin[0]))
		pData->nTime++;

	/* aktuellen Energiewert auf Dynamik normieren */
	features[iEnergy] = (features[iEnergy] - pData->fMin[0]) / rDiff;
	}

void dsp_channel_plain(dsp_channel_t *pData, mx_real_t *features)
	{
	int iEnergy = pData->iEnergy;
	// begin_change_thurid
	//mx_real_t rDiff;
	// end_change_thurid
	mx_real_t rAlpha = weight (pData->nTime, 
					pData->nTime_Min, pData->nTime_Max);
	int i;

	for (i = 0; i < pData->nDim; i++)
		if (i != iEnergy) {
			/* Mittelwert-Bestimmung (Update) */
			pData->pMeans[i] = rAlpha * features[i] + (1-rAlpha) * pData->pMeans[i];
			/* Mittelwert-Bereinigung */
			features[i] = features[i] - pData->pMeans[i];
			}
	}

void dsp_channel_apply(dsp_channel_t *pData, mx_real_t *features, int update)
	{
	int iEnergy = pData->iEnergy;
	// begin_change_thurid
	//mx_real_t rDiff;
	// end_change_thurid
	mx_real_t rAlpha = weight (pData->nTime, 
					pData->nTime_Min, pData->nTime_Max);
	int i;

	for (i = 0; i < pData->nDim; i++)
		if (i != iEnergy) {
			/* Mittelwert-Bestimmung (Update) */
			if (update)
				pData->pMeans[i] = rAlpha * features[i] + (1-rAlpha) * pData->pMeans[i];
			/* Mittelwert-Bereinigung */
			features[i] = features[i] - pData->pMeans[i];
			}
	}

void dsp_channel_in(dsp_channel_t *pData, mx_real_t *features)
	{
	int iEnergy = pData->iEnergy;
	// begin_change_thurid
	//mx_real_t rDiff;
	// end_change_thurid
	mx_real_t rAlpha = weight (pData->nTime, 
					pData->nTime_Min, pData->nTime_Max);
	int i;

	    
	for (i = 0; i < pData->nDim; i++)
		if (i != iEnergy) 
		  {
		    /* Mittelwert-Bestimmung (Update) */
		    pData->pMeans[i] = rAlpha * features[i] + (1-rAlpha) * pData->pMeans[i];
		  }
	
	}

void dsp_channel_out(dsp_channel_t *pData, mx_real_t *features)
	{
	int iEnergy = pData->iEnergy;
	// begin_change_thurid
	//mx_real_t rDiff;
	// end_change_thurid
	mx_real_t rAlpha = weight (pData->nTime, 
					pData->nTime_Min, pData->nTime_Max);
	int i;

	    
	for (i = 0; i < pData->nDim; i++)
		if (i != iEnergy) 
		  {
		    /* Mittelwert-Bereinigung */
		    features[i] = features[i] - pData->pMeans[i];
		  }

	}

