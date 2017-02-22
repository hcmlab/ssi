// ev_hnr.c
// author: Thurid Vogt <thurid.vogt@informatik.uni-augsburg.de>
// created: 
// Copyright (C) 2003-9 University of Augsburg, Thurid Vogt
//
// This file strongly depends on code of the phonetics software Praat
// of Paul Boersma and David Weenink, available at http://www.fon.hum.uva.nl/praat/
// under the GNU General Public License
// *************************************************************************************************
//
// This file is part of EmoVoice/SSI developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************


#include "ev_memory.h"
#include "ev_messages.h"

#include "ev_emo.h"

#include "ev_hnr.h"
#include "ev_serien.h"

// @begin_add_johannes
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif
// @end_add_johannes

int getVoicedInterval(pitch_t *p, int nframes, mx_real_t after, mx_real_t *start, mx_real_t *end);
mx_real_t findExtremum (mx_real_t *signal, int nsamples, mx_real_t tmin, mx_real_t tmax);
mx_real_t findMaximumCorrelation (mx_real_t *signal, int nsamples, mx_real_t t1, mx_real_t windowLength, mx_real_t tmin2, mx_real_t tmax2, mx_real_t *tout, mx_real_t *peak);
mx_real_t getMeanPeriod(fextract_series_t *pulses,  mx_real_t pmin, mx_real_t pmax, mx_real_t maximumPeriodFactor);
mx_real_t getHannWindowedRms (mx_real_t *signal, int nsamples, mx_real_t tmid, mx_real_t widthLeft, mx_real_t widthRight);
mx_real_t getShimmer_local (fextract_series_t *peaks, mx_real_t pmin, mx_real_t pmax, mx_real_t maximumAmplitudeFactor);
fextract_series_t* getPeaks (fextract_series_t *pulses, mx_real_t *signal, int nsamples, mx_real_t pmin, mx_real_t pmax, mx_real_t maximumPeriodFactor);
int isPeriod(fextract_series_t *pulses, int i, mx_real_t minimumPeriod, mx_real_t maximumPeriod, mx_real_t maximumPeriodFactor);
mx_real_t getPitchValueAt(pitch_t *p, int nframes, mx_real_t x);
static int _cmp_mx_real(const void *_big, const void *_small);

pitch_t *hnr_create() {
    pitch_t *hnr=pitch_create(AC_HANNING);
    hnr=pitch_hnr_configure(hnr);
    return hnr;
}

mx_real_t *hnr_calc(pitch_t *hnr, dsp_sample_t *signal, int frameLength) {
    int i, nFrames;
    mx_real_t r;
    mx_real_t *hnr_list, *h;

    h=pitch_calc (hnr,signal, frameLength);
	rs_free(h);
	nFrames=hnr->nframes;

    if (nFrames==0)
		return NULL;

    hnr_list = (mx_real_t *) rs_malloc(nFrames*sizeof(mx_real_t),"list of Harmonics-to-Noise ratio values");

    for (i=0;i<nFrames;i++) {
		if (hnr->frame_candidates[i]-> candidates[0].F == 0)
		    hnr_list[i] = -200;
		else {
		    r = hnr->frame_candidates[i]->candidates[0].R;
		    hnr_list[i] = r <= 1e-15 ? -150 : r > 1 - 1e-15 ? 150 : 10 * log10 (r / (1 - r));
		}
    }
    return hnr_list;
}

fextract_series_t *pulses_calc(pitch_t *p, int nframes, mx_real_t *signal, int nsamples){
	int i, x=0;
	mx_real_t globalPeak=signal[0];
	mx_real_t t = 0;
	mx_real_t peak;
	double addedRight = -1e300;
	fextract_series_t *pulses = series_create(MAX_SERIES);
	

	for (i=1;i<nsamples;i++) {
		mx_real_t this = fabs(signal[i]);
		if (this > globalPeak)
			globalPeak=this;		
	}
	/*
	 * Cycle over all voiced intervals.
	 */
    for (;;) {
		mx_real_t tleft, tright, tmiddle, f0middle, tmax, tsave;
		x++;
		if (! getVoicedInterval (p, nframes, t, & tleft, & tright)) 
			break;

		/*
		 * Go to the middle of the voice stretch.
		 */
		tmiddle = (tleft + tright) / 2;
		f0middle = getPitchValueAt(p,nframes,tmiddle);

		/*
		 * Our first point is near this middle.
		 */
		tmax = findExtremum (signal, nsamples, tmiddle - 0.5 / f0middle, tmiddle + 0.5 / f0middle);
		series_add(pulses,tmax);

		//find pulses backward
		tsave = tmax;
		for (;;) {
			mx_real_t f0 , correlation, last_tmax;
			f0 = getPitchValueAt(p,nframes,tmax);
			if (f0==-1)
				break;
            #if _WIN32||_WIN64
            #else
            //fix arm hang
            if (f0==0.0f)
                break;
            #endif
			last_tmax=tmax;
			correlation = findMaximumCorrelation (signal, nsamples, tmax, 1.0 / f0, tmax - 1.25 / f0, tmax - 0.8 / f0, & tmax, & peak);
			if (correlation == -1 || tmax >last_tmax) /*break*/ 
				tmax -= 1.0 / f0;   /* This one period will drop out. */
			if (tmax < tleft) {
				if (correlation > 0.7 && peak > 0.023333 * globalPeak && tmax - addedRight > 0.8 / f0) {
					series_add(pulses,tmax);
				}
				break;
			}
			if (correlation > 0.3 && (peak == 0.0 || peak > 0.01 * globalPeak)) {
				if (tmax - addedRight > 0.8 / f0) {   /* Do not fill in a short originally unvoiced interval twice. */
					series_add(pulses,tmax);
				}
			}
		}
		
		//find pulses forward
		tmax = tsave;
		for (;;) {
			mx_real_t f0 , correlation, last_tmax;
			f0 = getPitchValueAt(p,nframes,tmax);
			if (f0==-1)
				break;

            #if _WIN32||_WIN64
            #else
            //fix arm hang
            if (f0==0.0f)
                break;
            #endif

            last_tmax=tmax;
			correlation = findMaximumCorrelation (signal, nsamples, tmax, 1.0 / f0, tmax + 0.8 / f0, tmax + 1.25 / f0, & tmax, & peak);
			if (correlation == -1 || tmax <last_tmax) /*break*/ 
				tmax += 1.0 / f0;

			if (tmax > tright) {
				if (correlation > 0.7 && peak > 0.023333 * globalPeak) {
					series_add(pulses,tmax);
					addedRight = tmax;
				}


				break;
			}
			if (correlation > 0.3 && (peak == 0.0 || peak > 0.01 * globalPeak)) {
					series_add(pulses,tmax);
					addedRight = tmax;
			}


		}
		t = tright;

	}

	qsort(pulses->series,pulses->nSeries,sizeof(mx_real_t),_cmp_mx_real);
	return pulses;
	
	
}


mx_real_t getJitter(fextract_series_t *pulses, mx_real_t maximumPitch, mx_real_t minimumPitch) {
	mx_real_t pmin =0.8/maximumPitch, pmax=1.25/minimumPitch, maximumPeriodFactor=1.3, sum=0.0;
	int imin=0, imax=pulses->nSeries-1, numberOfPeriods=pulses->nSeries-1, i;

	if (numberOfPeriods < 2) 
		return -1;
	for (i = imin + 1; i < imax; i ++) {
		mx_real_t p1 = pulses->series [i] - pulses->series [i - 1], p2 = pulses->series [i + 1] - pulses->series [i];
		mx_real_t intervalFactor = p1 > p2 ? p1 / p2 : p2 / p1;
		if (pmin == pmax || (p1 >= pmin && p1 <= pmax && p2 >= pmin && p2 <= pmax && intervalFactor <= maximumPeriodFactor)) {
			sum += fabs (p1 - p2);
		} else {
			numberOfPeriods --;
		}
	}
	
	if (numberOfPeriods < 2) 
		return -1;
	return sum / (numberOfPeriods - 1) / getMeanPeriod (pulses, pmin, pmax, maximumPeriodFactor);
}


mx_real_t getShimmer(fextract_series_t *pulses, mx_real_t *signal, int n_samples, mx_real_t maxPitch, mx_real_t minPitch) {
	mx_real_t pmin =0.8/maxPitch, pmax=1.25/minPitch;
	mx_real_t maximumPeriodFactor=1.3, maximumAmplitudeFactor=1.6;
	mx_real_t result;
	fextract_series_t *peaks=NULL;

	peaks = getPeaks(pulses,signal,n_samples,pmin,pmax,maximumPeriodFactor);	
	
	if (peaks == NULL) 
		return -1;
	result = getShimmer_local (peaks, pmin, pmax, maximumAmplitudeFactor);

	series_destroy(peaks);
	return result;
}

mx_real_t getShimmer_local (fextract_series_t *peaks, mx_real_t pmin, mx_real_t pmax, mx_real_t maximumAmplitudeFactor) {
	int i, numberOfPeaks = 0;
	mx_real_t numerator = 0.0, denominator = 0.0;

	for (i = 2; i < peaks->nSeries; i +=2) {
		mx_real_t p = peaks->series [i] - peaks->series [i - 2];
		if (pmin == pmax || (p >= pmin && p <= pmax)) {
			mx_real_t a1 = peaks->series [i - 1], a2 = peaks->series [i+1] ;
			mx_real_t amplitudeFactor = a1 > a2 ? a1 / a2 : a2 / a1;
			if (amplitudeFactor <= maximumAmplitudeFactor) {
				numerator += fabs (a1 - a2);
				numberOfPeaks ++;
			}
		}
	}
	if (numberOfPeaks < 1) 
		return -1;
	numerator /= numberOfPeaks;
	numberOfPeaks = 0;
	for (i = 2; i < peaks->nSeries; i +=2) {
		denominator += peaks->series [i+1];
		numberOfPeaks ++;
	}
	denominator /= numberOfPeaks;
	if (denominator == 0.0) 
		return -1;
	return numerator / denominator;
}


fextract_series_t* getPeaks (fextract_series_t *pulses, mx_real_t *signal, int nsamples, mx_real_t pmin, mx_real_t pmax, mx_real_t maximumPeriodFactor) {
	int i, imin=0, imax=pulses->nSeries-1, numberOfPeaks=pulses->nSeries;
	fextract_series_t *peaks;

	if (numberOfPeaks < 3) 
		return NULL;
	peaks  = series_create(MAX_SERIES);

	for (i = imin + 1; i < imax; i ++) {
		mx_real_t p1 = pulses->series [i] - pulses->series [i - 1];
		mx_real_t p2 = pulses->series [i + 1] - pulses->series [i];
		mx_real_t intervalFactor = p1 > p2 ? p1 / p2 : p2 / p1;
		if (pmin == pmax || (p1 >= pmin && p1 <= pmax && p2 >= pmin && p2 <= pmax && intervalFactor <= maximumPeriodFactor)) {
			mx_real_t peak = getHannWindowedRms (signal, nsamples, pulses->series [i], 0.2 * p1, 0.2 * p2);
			if (peak != MX_REAL_MAX && peak > 0.0)  {
				series_add(peaks,pulses->series[i]);
				series_add(peaks,peak);	
			}
		}
	}
	return peaks;
}


mx_real_t getHannWindowedRms (mx_real_t *signal, int nsamples, mx_real_t tmid, mx_real_t widthLeft, mx_real_t widthRight) {
	mx_real_t sumOfSquares = 0.0, windowSumOfSquares = 0.0;
	int i, imin, imax;
	imin = ceil((tmid-widthLeft)*SAMPLERATE);
	imax = floor((tmid+widthRight)*SAMPLERATE);
	if (imax-imin < 3)
		return -1;

	for (i = imin; i <= imax; i ++) {
		mx_real_t x1 = 1.0/(2*SAMPLERATE);
		mx_real_t t = x1+i *1.0 / SAMPLERATE;
		mx_real_t width = t < tmid ? widthLeft : widthRight;
		mx_real_t windowPhase = (t - tmid) / width;   /* in [-1 .. 1] */
		mx_real_t window = 0.5 + 0.5 * cos (M_PI * windowPhase);   /* Hann */
		mx_real_t windowedValue = signal [i] * window;
		sumOfSquares += windowedValue * windowedValue;
		windowSumOfSquares += window * window;
	}
	return sqrt (sumOfSquares / windowSumOfSquares);
}


mx_real_t getMeanPeriod(fextract_series_t *pulses,  mx_real_t minimumPeriod, mx_real_t maximumPeriod, mx_real_t maximumPeriodFactor) {
	long imin=0, imax=pulses->nSeries-1, numberOfPeriods, i;
	double sum = 0.0;

	numberOfPeriods = pulses->nSeries-1;
	if (numberOfPeriods < 1) 
		return -1;
	for (i = imin; i < imax; i ++) {
		if (isPeriod (pulses, i, minimumPeriod, maximumPeriod, maximumPeriodFactor)) {
			sum += pulses->series [i + 1] - pulses ->series [i];   /* This interval counts as a period. */
		} else {
			numberOfPeriods --;   /* This interval does not count as a period. */
		}
	}
	return numberOfPeriods > 0 ? sum / numberOfPeriods : -1;

}

int isPeriod(fextract_series_t *pulses, int ileft, mx_real_t minimumPeriod, mx_real_t maximumPeriod, mx_real_t maximumPeriodFactor) {
	/*
	 * This function answers the question: is the interval from point 'ileft' to point 'ileft+1' a period?
	 */
	int iright = ileft + 1;
	/*
	 * Period condition 1: both 'ileft' and 'iright' have to be within the point process.
	 */
	if (ileft < 0 || iright > pulses->nSeries-1) {
		return 0;
	} else {
		/*
		 * Period condition 2: the interval has to be within the boundaries, if specified.
		 */
		if (minimumPeriod == maximumPeriod) {
			return 1;   /* All intervals count as periods, irrespective of absolute size and relative size. */
		} else {
			mx_real_t interval = pulses->series [iright] - pulses->series [ileft];
			if (interval <= 0.0 || interval < minimumPeriod || interval > maximumPeriod) {
				return 0;
			} else 
				if (fabs(maximumPeriodFactor) ==MX_REAL_MAX || maximumPeriodFactor < 1.0) {
					return 1;
			} else {
				/*
				 * Period condition 3: the interval cannot be too different from both of its neigbours, if any.
				 */
				mx_real_t previousInterval = ileft <= 1 ? MX_REAL_MAX : pulses->series [ileft] - pulses->series [ileft - 1];
				mx_real_t nextInterval = iright >= pulses->nSeries-1 ? MX_REAL_MAX : pulses->series [iright + 1] - pulses->series [iright];
				mx_real_t previousIntervalFactor = fabs (previousInterval) !=MX_REAL_MAX && previousInterval > 0.0 ? interval / previousInterval : MX_REAL_MAX;
				mx_real_t nextIntervalFactor = fabs (nextInterval) != MX_REAL_MAX && nextInterval > 0.0 ? interval / nextInterval : MX_REAL_MAX;
				if (fabs (previousIntervalFactor) == MX_REAL_MAX && fabs (nextIntervalFactor) == MX_REAL_MAX) {
					return 1;   /* No neighbours: this is a period. */
				}
				if (fabs (previousIntervalFactor) != MX_REAL_MAX && previousIntervalFactor > 0.0 && previousIntervalFactor < 1.0) {
					previousIntervalFactor = 1.0 / previousIntervalFactor;
				}
				if (fabs (nextIntervalFactor) != MX_REAL_MAX && nextIntervalFactor > 0.0 && nextIntervalFactor < 1.0) {
					nextIntervalFactor = 1.0 / nextIntervalFactor;
				}
				if (fabs (previousIntervalFactor) != MX_REAL_MAX && previousIntervalFactor > maximumPeriodFactor &&
					fabs (nextIntervalFactor) != MX_REAL_MAX && nextIntervalFactor > maximumPeriodFactor)
				{
					return 0;
				}
			}
		}
	}
	return 1;
}


mx_real_t findExtremum (mx_real_t *signal, int nsamples, mx_real_t tmin, mx_real_t tmax) {
	int imin = tmin*SAMPLERATE, imax= ceil(tmax*SAMPLERATE), iind, i;
	mx_real_t iextremum, extremum;

	if (imin < 1) 
		imin = 1;
	if (imax > nsamples) 
		imax = nsamples;
	
	extremum=fabs(signal[imin-1]);
	iind=imin-1;
	for (i=imin;i<imax;i++) {	
		mx_real_t val = fabs(signal[i]);
		if (val > extremum) {
			extremum=val;
			iind=i;
		}
	}
	return 1.0*iind/SAMPLERATE;
	
	iextremum= iind + 0.5 * (signal [iind + 1] - signal [iind - 1]) / (2 * signal [iind] - signal [iind - 1] - signal [iind + 1]);
	if (iextremum)
		return (imin - 1 + iextremum - 1)/SAMPLERATE;
	else
		return (tmin + tmax) / 2;

}

int getVoicedInterval(pitch_t *p, int nframes, mx_real_t after, mx_real_t *start, mx_real_t *end) {
	int i;
	mx_real_t dt = p->dt;
	
	i=(after/dt)+1;
	while (i<nframes) {
		if (p->frame_candidates[i]->candidates[0].F!=0)
			break;
		i++;
	}

	if (i>=nframes)
		return 0;	

	*start=i*dt-0.5*dt;

	i++;
	while (i<nframes) {
		if (p->frame_candidates[i]->candidates[0].F==0)
			break;
		i++;
	}

	if (i>=nframes)
		i=nframes-1;	

	*end=i*dt+0.5*dt;
	return 1;
}



mx_real_t findMaximumCorrelation (mx_real_t *amp, int nsamples, mx_real_t t1, mx_real_t windowLength, mx_real_t tmin2, mx_real_t tmax2, mx_real_t *tout, mx_real_t *peak) {
	mx_real_t maximumCorrelation = -1.0, r1 = 0.0, r2 = 0.0, r3 = 0.0, r1_best=0, r3_best=0, ir=0;
	mx_real_t halfWindowLength = 0.5 * windowLength;
	int i1, i2, ileft2;
	int ileft1 = (t1 - halfWindowLength)*SAMPLERATE+0.5;
	int iright1 = (t1 + halfWindowLength)*SAMPLERATE+0.5;
	int ileft2min = (tmin2 - halfWindowLength)*SAMPLERATE;
	int ileft2max = ceil((tmax2 - halfWindowLength)*SAMPLERATE);
	
	*peak = 0.0;   /* Default. */
	for (ileft2 = ileft2min; ileft2 <= ileft2max; ileft2 ++) {
		mx_real_t norm1 = 0.0, norm2 = 0.0, product = 0.0, localPeak = 0.0;
		for (i1 = ileft1, i2 = ileft2; i1 <= iright1; i1 ++, i2 ++) {
			if (i1 < 1 || i1 > nsamples || i2 < 1 || i2 > nsamples) 
				continue;
			norm1 += amp [i1] * amp [i1];
			norm2 += amp [i2] * amp [i2];
			product += amp [i1] * amp [i2];
			if (fabs (amp [i2]) > localPeak)
				localPeak = fabs (amp [i2]);
		}
		r1 = r2;
		r2 = r3;
		r3 = product ? product / (sqrt (norm1 * norm2)) : 0.0;
		if (r2 > maximumCorrelation && r2 >= r1 && r2 >= r3) {
			r1_best = r1;
			maximumCorrelation = r2;
			r3_best = r3;
			ir = ileft2 - 1;
			*peak = localPeak;  
		}
	}
	/*
	 * Improve the result by means of parabolic interpolation.
	 */
	if (maximumCorrelation > -1.0) {
		double d2r = 2 * maximumCorrelation - r1_best - r3_best;
		if (d2r != 0.0) {
			double dr = 0.5 * (r3_best - r1_best);
			maximumCorrelation += 0.5 * dr * dr / d2r;
			ir += dr / d2r;
		}
		*tout = t1 + (ir - ileft1)/SAMPLERATE;
	}
    if(maximumCorrelation&&(windowLength==1.0f))
    {
        return 0.0f;
    }
	
	return maximumCorrelation;
}

mx_real_t getPitchValueAt(pitch_t *p, int nframes, mx_real_t x) {
	mx_real_t ireal, phase, fnear, ffar;
	int ileft, inear, ifar;

	if (x <0 || x >=nframes*p->dt)
		return -1;

	ireal = x/p->dt;
	ileft = floor (ireal);
	phase = ireal - ileft;

	if (phase < 0.5) {
		inear = ileft, ifar = ileft + 1;
	} 
	else {
		ifar = ileft, inear = ileft + 1;
		phase = 1.0 - phase;
	}
	if (inear < 0 || inear >= nframes) 
		return -1;   /* X out of range? */
	fnear = p->frame_candidates[inear]->candidates[0].F;
	if (fnear == MX_REAL_MAX) 
		return -1;   /* Function value not defined? */
	if (ifar < 0 || ifar >=nframes) 
		return fnear;   /* At edge? Extrapolate. */
	ffar = p->frame_candidates[inear]->candidates[0].F;
	if (ffar == MX_REAL_MAX) 
		return fnear;   /* Neighbour undefined? Extrapolate. */
	return fnear + phase * (ffar - fnear);   /* Interpolate. */
}

static int _cmp_mx_real(const void *_big, const void *_small) {
  mx_real_t *small = (mx_real_t *) _small, *big =(mx_real_t *) _big;
  
   if (!*small && !*big)
    return(0);
  else
    if (!*small)
      return(-1);
    else
      if (!*big)
      return(1);

   if (*big == *small)
     return (0);
   else
     if (*big > *small)
       return(1);
     else 
       return (-1);
}
