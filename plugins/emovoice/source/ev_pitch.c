// ev_pitch.c
// author: Thurid Vogt <thurid.vogt@informatik.uni-augsburg.de>
// created: 
// Copyright (C) 2003-9 University of Augsburg, Thurid Vogt
//
// This file strongly depends on code of the phonetics software Praat
// of Paul Boersma and David Weenink, available at http://www.fon.hum.uva.nl/praat/
// under the GNU General Public License
//
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
#include "ev_matrix.h"

#include "ev_pitch.h"
#include "ev_efeatures.h"

// @begin_add_johannes
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif
#ifndef M_LN2
#define M_LN2       0.69314718055994530942
#endif
// @end_add_johannes

#define NUM_PEAK_INTERPOLATE_NONE  0
#define NUM_PEAK_INTERPOLATE_PARABOLIC  1
#define NUM_PEAK_INTERPOLATE_CUBIC  2
#define NUM_PEAK_INTERPOLATE_SINC70  3
#define NUM_PEAK_INTERPOLATE_SINC700  4

#define NUM_goldenSection  0.618033988749895
#define ZEPS 1.0e-10 

#define SILENCE_THRESHOLD        0.03    /* threshold for analyzing frames as voiceless */
#define VOICING_THRESHOLD        0.45    /* threshold for analyzing frames as voiceless */
#define OCTAVE_COST              0.01    /* to favour higher fundamental frequencies */
#define OCTAVE_JUMP_COST         0.35    /* for transition cost in path finder between two voiced frames */
#define VOICED_UNVOICED_COST     0.14    /* transition cost between voiced and unvoiced frame in path finder */

const mx_real_t dx=0.0000625;

// @begin_remove_johannes
//int fitInFrame (mx_real_t windowDuration, mx_real_t timeStep,int *numberOfFrames, mx_real_t *startTime, mx_real_t x1, int frameLength);
// @end_remove_johannes
mx_real_t interpolate_sinc (mx_real_t y [], int nx, mx_real_t x, int maxDepth);
mx_real_t minimize_brent (mx_real_t (*f) (mx_real_t x, void *closure), mx_real_t a, mx_real_t b, void *closure, mx_real_t tol, mx_real_t *fx);
mx_real_t improveMaximum (mx_real_t *y, int nx, int ixmid, int interpolation, mx_real_t *ixmid_real);
//pitch_frame_t* pathFinder (pitch_frame_t frame_candidates[], mx_real_t ceiling, mx_real_t maxnCandidates, int nFrames, mx_real_t dt);
pitch_frame_t* pathFinder(pitch_t *pitch, int nFrames);

static mx_real_t improve_evaluate (mx_real_t x, void *closure) {
	struct improve_params *me = closure;
	mx_real_t y = interpolate_sinc (me-> y_d, me-> ixmax, x, me-> depth);
	return me-> isMaximum ? - y : y;
}



pitch_t *pitch_create (pitch_method_t method) {
	pitch_t *pitch;

	pitch = (pitch_t *) rs_malloc(sizeof(pitch_t),"Pitch configuration data");
	pitch->minimumPitch = MINPITCH; 
	pitch->maximumPitch = MAXPITCH;
	pitch->maxnCandidates = 4; /* maximum number of candidates per frame */
	pitch->method = method; /* method to compute pitch, change manually */
	if (method < FCC_NORMAL)
		pitch->periodsPerWindow = 3; /* 3 for Pitch, 6 for HNR, change manually */
	else
		pitch->periodsPerWindow= 1;
	pitch->dt = pitch->periodsPerWindow / pitch->minimumPitch / 4.0;   /* local frame shift */ 
	pitch->last_frame = NULL;

	pitch->silence_threshold=SILENCE_THRESHOLD;
	pitch->voicing_threshold=VOICING_THRESHOLD;
	pitch->octave_cost=OCTAVE_COST;
	pitch->octave_jump_cost=OCTAVE_JUMP_COST;
	pitch->voiced_unvoiced_cost=VOICED_UNVOICED_COST;
	pitch->nframes=0;

	return pitch;
}

pitch_t *pitch_hnr_configure(pitch_t *pitch) {
	pitch->periodsPerWindow=6;
	pitch->dt=0.01;
	pitch->maximumPitch=0.5*SAMPLERATE;
	pitch->maxnCandidates=15;//pitch->maximumPitch/pitch->minimumPitch;

	pitch->silence_threshold=0.1;
	pitch->voicing_threshold=0;
	pitch->octave_cost=0;
	pitch->octave_jump_cost=0;
	pitch->voiced_unvoiced_cost=0;

	return pitch;
}



mx_real_t *pitch_calc (pitch_t *cfg, dsp_sample_t *signal, int frameLength) {
	mx_real_t dt = cfg->dt, periodsPerWindow = cfg->periodsPerWindow;
	mx_real_t minimumPitch = cfg->minimumPitch, maximumPitch = cfg->maximumPitch;
	int maxnCandidates = cfg->maxnCandidates, method = cfg->method;
	mx_complex_t *frame = NULL, *windowR = NULL;
	mx_real_t duration, t1, x1=0.5/SAMPLERATE;  // x1 koennte eigentlich auch gleich 0 sein??
	mx_real_t dt_window, globalPeak, interpolation_depth=0;   
	mx_real_t *r = NULL, *window = NULL;
	mx_real_t *amplitude=NULL; /* ??? Window length in seconds. */
	mx_real_t* pitchList=NULL;
	int i, j, last=0;
	int nsamp_window, halfnsamp_window;   /* Number of samples per window. */
	int minimumLag, maximumLag;
	int iframe, nsampFFT=0, *imax = NULL;
	int nsamp_period, halfnsamp_period, nFrames;   
	/* Number of samples in longest period. */
	int brent_ixmax, brent_depth=0;
	pitch_frame_t **frame_candidates=NULL;

	switch (method) {
		case AC_HANNING:
			brent_depth = NUM_PEAK_INTERPOLATE_SINC70;
			interpolation_depth = 0.5;
			break;
		case AC_GAUSS:
			periodsPerWindow *= 2;   /* Because Gaussian window is twice as long. */
			brent_depth = NUM_PEAK_INTERPOLATE_SINC700;
			interpolation_depth = 0.25;   /* Because Gaussian window is twice as long. */
			break;
		case FCC_NORMAL:
			brent_depth = NUM_PEAK_INTERPOLATE_SINC70;
			interpolation_depth = 1.0;
			break;
		case FCC_ACCURATE:
			brent_depth = NUM_PEAK_INTERPOLATE_SINC700;
			interpolation_depth = 1.0;
			break;
	}
    
	duration = dx * frameLength;
	if (minimumPitch < periodsPerWindow / duration) {
		rs_warning ("For this sound segment, the parameter 'minimum pitch' may not be less than %.2g Hz or the segment length (%g s) is too small.", periodsPerWindow / duration, duration);
	/*    if (periodsPerWindow/duration <= maxMinPitch)
		minimumPitch = periodsPerWindow/duration;
		else */ 
		nFrames=0;
		return NULL;
	}
    
    /*
	* Determine the number of samples in the longest period.
	* We need this to compute the local mean of the sound (looking one period in both directions),
	* and to compute the local peak of the sound (looking half a period in both directions).
    */
	nsamp_period = floor (1 / dx / minimumPitch);
	halfnsamp_period = nsamp_period / 2 + 1;
    
	if (maximumPitch > 0.5 / dx) 
		maximumPitch = 0.5 / dx;
    
    /*
	* Determine window length in seconds and in samples.
    */
	dt_window = periodsPerWindow / minimumPitch;
	nsamp_window = floor (dt_window / dx) +1 ;
	halfnsamp_window = nsamp_window / 2 ;
	if (halfnsamp_window < 2)
		rs_error ("Window too short.");
    
    /*
	* Determine the minimum and maximum lags.
    */
	minimumLag = floor (1 / dx / maximumPitch);
	if (minimumLag < 2) 
		minimumLag = 2;
	maximumLag = floor (nsamp_window / periodsPerWindow) + 2;
	if (maximumLag > nsamp_window) 
		maximumLag = nsamp_window;
    
    /*
	* Determine the number of frames.
	* Fit as many frames as possible symmetrically in the total duration.
	* We do this even for the forward cross-correlation method,
	* because that allows us to compare the two methods.
    */
	if (! fitInFrame (method >= FCC_NORMAL ? 1 / minimumPitch + dt_window : dt_window, dt, &nFrames, & t1,x1,frameLength))
		goto end;
	if (method >=FCC_NORMAL)
		nFrames=nFrames-1;

	amplitude= (mx_real_t *) rs_malloc(frameLength*sizeof(mx_real_t),"real signal data");

	/* Step 1: compute global absolute peak for determination of silence threshold. */
	{
		mx_real_t mean = 0.0;
		globalPeak = 0;
		for (i = 0; i < frameLength; i ++) {
			amplitude[i]=signal[i]*(1.0f/32768);
			mean += amplitude [i];
		}
		mean /= frameLength;
		for (i = 0; i < frameLength; i ++) {
			mx_real_t damp = amplitude [i] - mean;
			if (fabs (damp) > globalPeak)
				globalPeak = fabs (damp);
		}
		if (globalPeak == 0.0) {
			return NULL;
		}
	}
    
    
	if (method >= FCC_NORMAL) {   /* For cross-correlation analysis. */
		/*
		* Create buffer for cross-correlation analysis.
		*/
		frame = (mx_complex_t *) rs_malloc (sizeof(mx_complex_t) * nsamp_window, "Frame");
		brent_ixmax = nsamp_window * interpolation_depth;
	}
	else {   /* For autocorrelation analysis. */
	
		/*
		* Compute the number of samples needed for doing FFT.
		* To avoid edge effects, we have to append zeroes to the window.
		* The maximum lag considered for maxima is maximumLag.
		* The maximum lag used in interpolation is nsamp_window * interpolation_depth.
		*/
		nsampFFT = 1;
		while (nsampFFT < nsamp_window * (1 + interpolation_depth))
			nsampFFT *= 2;
		/*
		* Create buffers for autocorrelation analysis.
		*/
		frame = (mx_complex_t *) rs_malloc(sizeof(mx_complex_t) * nsampFFT, "Frame");
		windowR = (mx_complex_t *) rs_malloc(sizeof(mx_complex_t) * nsampFFT, "WindowR");
		window = (mx_real_t *) rs_malloc(sizeof(mx_real_t) * nsamp_window, "WindowR");
    
		/*
		* A Gaussian or Hanning window is applied against phase effects.
		* The Hanning window is 2 to 5 dB better for 3 periods/window.
		* The Gaussian window is 25 to 29 dB better for 6 periods/window.
		*/
		if (method == AC_GAUSS) {   /* Gaussian window. */
			mx_real_t imid = 0.5 * (nsamp_window + 1), edge = exp (-12.0);
			for (i = 0; i < nsamp_window; i ++)
				window [i] = (exp (-48.0 * (i - imid) * (i - imid) / (nsamp_window + 1) / (nsamp_window + 1)) - edge) / (1 - edge);
	    
		}
		else {   /* Hanning window. */
			for (i = 0; i < nsamp_window; i ++)
				window [i] = 0.5 - 0.5 * cos (i * 2 * M_PI / (nsamp_window + 1));
		}
      
		/*
		* Compute the normalized autocorrelation of the window.
		*/
		for (i = 0; i < nsamp_window; i ++) {
			mx_re(windowR [i]) = window [i];
			mx_im(windowR [i]) = 0.0;
		}
		for (i=nsamp_window; i<nsampFFT; i++)
			mx_re(windowR [i]) = mx_im(windowR [i]) = 0.0;
	
		dsp_xfft (windowR, nsampFFT, 0);    /* Complex spectrum. */
		mx_re(windowR [0]) *= mx_re(windowR [0]);   /* DC component. */
		mx_re(windowR [1]) *= mx_re(windowR [1]);   /* Nyquist frequency. */
		for (i = 2; i < nsampFFT-1; i += 2) {
			mx_re(windowR [i]) = mx_re(windowR [i]) * mx_re(windowR [i]) + mx_re(windowR [i+1]) * mx_re(windowR [i+1]);
			mx_re(windowR [i + 1]) = 0.0;   /* Power spectrum: square and zero. */
		}
		dsp_xfft (windowR, nsampFFT, 1);    /* Autocorrelation. */
		for (i = 1; i < nsamp_window; i ++) {
			mx_re(windowR [i]) /= mx_re(windowR [0]);   /* Normalize. */
		}
		mx_re(windowR [0]) = 1.0;   /* Normalize. */
	
		brent_ixmax = nsamp_window * interpolation_depth;
	}

	if (! (r= (mx_real_t *) rs_malloc ( (2*nsamp_window+1)*sizeof(mx_real_t),"autocorrelation array")) || ! (imax = (int *) rs_malloc(sizeof(int)* maxnCandidates,"imax"))) {
		goto end;
	}


	if (cfg->last_frame) {
		frame_candidates= (pitch_frame_t **) rs_malloc((nFrames+1) * sizeof(pitch_frame_t*),"list of candidates");
		for (i=0;i<nFrames+1;i++)
			frame_candidates[i]= (pitch_frame_t *) rs_malloc(sizeof(pitch_frame_t),"pitch candidate");
		last=1;
	}
	else {
		frame_candidates= (pitch_frame_t **) rs_malloc(nFrames * sizeof(pitch_frame_t *),"list of candidates");
		for (i=0;i<nFrames;i++)
			frame_candidates[i]= (pitch_frame_t *) rs_malloc(sizeof(pitch_frame_t),"pitch candidate");
	}

    	
	for (iframe = 0; iframe < nFrames; iframe ++) {
		pitch_frame_t* thisFrame = frame_candidates [iframe];
		mx_real_t t = t1+iframe * dt;
		mx_real_t localMean, localPeak;
		int leftSample = (int) floor ((t - x1)/ dx)+1;
		int rightSample = leftSample + 1;
		int startSample, endSample;
	
		thisFrame->candidates= (pitch_candidate_t *) rs_malloc(maxnCandidates*sizeof(pitch_candidate_t),"a candidate for a frame");
	
		/*
		* Compute the local mean; look one longest period to both sides.
		*/
		localMean = 0.0;
		startSample = rightSample - nsamp_period;
		endSample = leftSample + nsamp_period;
		if (startSample < 1 )
			rs_error("Startsample must be greater than 0");
		if(endSample > frameLength )
			rs_error(" end sample must be in frame!");
		for (i = startSample; i <= endSample; i ++)
			localMean += amplitude [i];
		localMean /= 2 * nsamp_period;
    
		/*
		* Copy a window to a frame and subtract the local mean.
		* We are going to kill the DC component before windowing.
		*/
		startSample = rightSample - halfnsamp_window;
		endSample = leftSample + halfnsamp_window;
	
		if (startSample < 1 || endSample > frameLength )
			rs_error("Startsample must be greater than 0 and end sample must be in frame!");
		if (method >= FCC_NORMAL) {
			for (j = 0, i = startSample-1; j < nsamp_window; j ++) {
				mx_re(frame [j]) = (amplitude [i ++] - localMean);
				mx_im(frame[j]) = 0.0;
			}
		}
		else {
			for (j = 0, i = startSample-1; j < nsamp_window; j ++) {
				mx_re(frame [j]) = (amplitude [i ++] - localMean) * window [j];
				mx_im(frame[j]) = 0.0;
			}
			for (j = nsamp_window; j < nsampFFT; j ++)
				mx_re(frame [j]) = mx_im(frame[j]) = 0.0;
		}

		/*
		* Compute the local peak; look half a longest period to both sides.
		*/
		localPeak = 0;
		if ((startSample = halfnsamp_window + 1 - halfnsamp_period) < 1)
			startSample = 1;
		if ((endSample = halfnsamp_window + halfnsamp_period) > nsamp_window)
			endSample = nsamp_window;
		for (j = startSample-1; j < endSample; j ++)
			if (fabs (mx_re(frame [j])) > localPeak)
				localPeak = fabs (mx_re(frame [j]));
		thisFrame->intensity = localPeak > globalPeak ? 1 : localPeak / globalPeak;
	

		/*
		* Compute the correlation into the array 'r'.
		*/
		if (method >= FCC_NORMAL) {
			mx_real_t startTime = t - (1 / minimumPitch + dt_window) / 2;
			mx_real_t sumx2 = 0, sumy2 = 0;   /* Sum of squares. */
			long localSpan = maximumLag + nsamp_window, localMaximumLag, offset;
			if ((startSample = (int) floor(startTime/dx)+1) < 1)
				startSample = 1;
			if (localSpan > frameLength + 1 - startSample)
				localSpan = frameLength + 1 - startSample;
			localMaximumLag = localSpan - nsamp_window;
			offset = startSample - 1;
			for (i = 0; i < nsamp_window; i ++) {
				mx_real_t x = amplitude [offset + i];
				sumx2 += x * x;
			}
//    	if (method >=FCC_NORMAL && iframe==*nFrames -1) {
// 			return NULL;
//    	}
			sumy2 = sumx2;   /* At zero lag, these are still equal. */
			r [nsamp_window] = 1.0;
			for (i = 1; i <= localMaximumLag; i ++) {
				mx_real_t *x = ((mx_real_t *) amplitude) + offset;
				mx_real_t *y = x + i;
				mx_real_t product = 0.0;
				sumy2 += y [nsamp_window] * y [nsamp_window] - y [0] * y [0];
				for (j = 0; j < nsamp_window; j ++)
					product += x [j] * y [j];
				r [nsamp_window - i] = r [nsamp_window + i] = product / sqrt (sumx2 * sumy2);
			}
		}
		else {
	  	  //hier koennte man Zeit optimieren...
	  	  /*
			* The FFT of the autocorrelation is the power spectrum.
		  */
      
			dsp_xfft (frame, nsampFFT, 0);    /* Complex spectrum. */
	    
			mx_re(frame [0]) *= mx_re(frame [0]);   /* DC component? */
			mx_re(frame [1]) *= mx_re(frame [1]);   /* Nyquist frequency. */
			for (i = 2; i < nsampFFT-1; i += 2) {
				mx_re(frame [i]) = mx_re(frame [i]) * mx_re(frame [i]) + mx_re(frame [i+1]) * mx_re(frame [i+1]);
				mx_re(frame [i + 1]) = 0.0;   /* Power spectrum: square and zero. */
			}
	    
			dsp_xfft (frame, nsampFFT, 1);    /* Autocorrelation. */
	
		    /*
			* Normalize the autocorrelation to the value with zero lag,
			* and divide it by the normalized autocorrelation of the window.
		    */
			r [nsamp_window] = 1.0;
			for (i = 1; i <= brent_ixmax; i ++) {
				r [nsamp_window - i] = r [nsamp_window + i] = mx_re(frame [i]) / (mx_re(frame [0]) * mx_re(windowR [i]));
			}
		}
	

		/*
		* Register the first candidate, which is always present: voicelessness.
		*/
		thisFrame->nCandidates = 1;
		thisFrame->candidates[0].F = 0.0;   /* Voiceless: always present. */
		thisFrame->candidates[0].R = 0.0;

		/*
		* Shortcut: absolute silence is always voiceless.
		* Go to next frame.
		*/
		if (localPeak == 0)
			continue;
    
		/*
		* Find the strongest maxima of the correlation of this frame,
		* and register them as candidates.
		*/
		imax [0] = 0;
		for (i = 2; i < maximumLag && i < brent_ixmax; i ++)
			if (r [nsamp_window + i] > 0.5 * cfg->voicing_threshold && /* Not too unvoiced? */ r [nsamp_window + i] > r [nsamp_window + i-1] && r [nsamp_window + i] >= r [nsamp_window + i+1])   /* Maximum? */ {
			int place = 0;
	
				/*
			* Use parabolic interpolation for first estimate of frequency,
			* and sin(x)/x interpolation to compute the strength of this frequency.
				*/
			mx_real_t dr = 0.5 * (r [nsamp_window + i+1] - r [nsamp_window + i-1]);
			mx_real_t d2r = 2 * r [nsamp_window + i] - r [nsamp_window + i-1] - r [nsamp_window + i+1];
			mx_real_t frequencyOfMaximum = 1 / dx / (i + dr / d2r);
			long offset = - brent_ixmax-1;
			mx_real_t strengthOfMaximum = interpolate_sinc (r + nsamp_window + offset, brent_ixmax - offset, 1 / dx / frequencyOfMaximum - offset, 30);
			/*: r [i] + 0.5 * dr * dr / d2r; */
			/* High values due to short windows are to be reflected around 1. */
			if (strengthOfMaximum > 1.0)
				strengthOfMaximum = 1.0 / strengthOfMaximum;
				/*
			* Find a place for this maximum.
				*/
			if (thisFrame->nCandidates < maxnCandidates) { /* Is there still a free place? */
				place = thisFrame->nCandidates ++;
			}
			else {
				/* Try the place of the weakest candidate so far. */
				mx_real_t weakest = 2;
				int iweak;
				for (iweak = 1; iweak < maxnCandidates; iweak ++) {
					/* High frequencies are to be favoured */
					/* if we want to analyze a perfectly periodic signal correctly. */
					mx_real_t localStrength = thisFrame->candidates[iweak].R - cfg->octave_cost * log (minimumPitch / thisFrame->candidates[iweak].F)/M_LN2;
					if (localStrength < weakest) {
						weakest = localStrength;
						place = iweak;
					}
				}
				/* If this maximum is weaker than the weakest candidate so far, give it no place. */
				if (strengthOfMaximum - cfg->octave_cost* log (minimumPitch / frequencyOfMaximum)/M_LN2 <= weakest)
					place = -1;
			}
			if (place+1) {   /* Have we found a place for this candidate? */
				thisFrame->candidates[place].F = frequencyOfMaximum;
				thisFrame->candidates[place].R = strengthOfMaximum;
				imax [place] = i;
			}
			}

	    	
		/*
			* Second pass: for extra precision, maximize sin(x)/x interpolation ('sinc').
		*/
			/*if (method & 1)*/
			for (i = 1; i < thisFrame->nCandidates; i ++) {
				mx_real_t xmid, ymid;
				int offset = - brent_ixmax - 1;
				ymid = improveMaximum ( r + nsamp_window + offset, brent_ixmax - offset, imax [i] - offset, brent_depth, & xmid);
				xmid += offset;
				thisFrame->candidates[i].F = 1.0 / dx / xmid;
				if (ymid > 1.0)
					ymid = 1.0 / ymid;
				thisFrame->candidates[i].R = ymid;
			}
	}   /* Next frame. */
    
		
    /*  for (i=0;i<*nFrames;i++) {
	fprintf(stderr,"p[%d]: ",i);
	for (j=0;j<frame_candidates[i].nCandidates;j++)
	fprintf(stderr,"c[%d].F=%g, c[%d].R=%g; ",j,frame_candidates[i].candidates[j].F,j,frame_candidates[i].candidates[j].R);
	fprintf(stderr,"\n");
}
    */
	cfg->frame_candidates=frame_candidates;

	if (cfg->last_frame) {
		for (i= nFrames;i>0;i--)
			frame_candidates[i]=frame_candidates[i-1];
		frame_candidates[0]=cfg->last_frame;
		/* 	cfg->last_frame = pathFinder (frame_candidates, maximumPitch, maxnCandidates, *nFrames + 1, dt); */
		cfg->last_frame = pathFinder (cfg, nFrames + 1);

		pitchList= (mx_real_t *) rs_malloc(nFrames*sizeof(mx_real_t),"Pitch List");
	
		for (i=1;i<nFrames+1;i++)
			pitchList[i-1]=frame_candidates[i]->candidates[0].F;
	}
	else {
		/* 	cfg->last_frame =  pathFinder (frame_candidates, maximumPitch, maxnCandidates, *nFrames, dt); */
		cfg->last_frame =  pathFinder (cfg, nFrames);
		pitchList= (mx_real_t *) rs_malloc(nFrames*sizeof(mx_real_t),"Pitch List");
	
		for (i=0;i<nFrames;i++) {
			pitchList[i]=frame_candidates[i]->candidates[0].F;
		}
	}
    
	cfg->nframes=nFrames;
    
	end:
    
			if (window)
			rs_free (window);
	if (windowR)
		rs_free (windowR);
	if (r)
		rs_free (r);
	if (imax)
		rs_free (imax);
	if (frame)
		rs_free (frame);
	if (amplitude)
		rs_free(amplitude);

	return pitchList;
}

int fitInFrame (mx_real_t windowDuration, mx_real_t timeStep, int *numberOfFrames, mx_real_t *startTime, mx_real_t x1, int frameLength) {
    
	mx_real_t frameDuration /* duration of the whole segment */, offsetDuration /* duration from start of first window to start of last window in segment*/, midTime;
	if (windowDuration <= 0.0 || timeStep <= 0.0)
		rs_error("Window Duration and Time Step must be greater 0!");
  
	frameDuration = dx * frameLength;
	if (windowDuration > frameDuration) {
		rs_warning ("Sound segment shorter than window length."); 
		return 0;
	}
	*numberOfFrames = floor ((frameDuration - windowDuration) / timeStep) + 1;
  
	if (*numberOfFrames < 1)
		rs_error("Number of frames must be greater 0!");
	midTime = x1 - 0.5 * dx + 0.5 * frameDuration;
	offsetDuration = *numberOfFrames * timeStep;
	*startTime = midTime - 0.5 * offsetDuration + 0.5 * timeStep;
	return 1;
}

mx_real_t interpolate_sinc (mx_real_t y [], int nx, mx_real_t x, int maxDepth) { 
	int ix, midleft = floor (x), midright = midleft + 1, left, right; 
	mx_real_t result = 0.0, a, halfsina, aa, daa; 
    
	/* simple cases */
	if (nx < 1) 
		return HUGE_VAL; 
	if (x > nx) 
		return y [nx]; 
	if (x < 1) 
		return y [1]; 
	if (x == midleft) 
		return y [midleft]; 
	/* 1 < x < nx && x not integer: interpolate. */ 
	if (maxDepth > midright - 1) 
		maxDepth = midright - 1; 
	if (maxDepth > nx - midleft) 
		maxDepth = nx - midleft; 
	if (maxDepth <= 0) 
		return y [(int) floor (x + 0.5)]; 
	if (maxDepth == 1) 
		return y [midleft] + (x - midleft) * (y [midright] - y [midleft]); 
	if (maxDepth == 2) { 
		mx_real_t yl = y [midleft], yr = y [midright]; 
		mx_real_t dyl = 0.5 * (yr - y [midleft - 1]), dyr = 0.5 * (y [midright + 1] - yl); 
		mx_real_t fil = x - midleft, fir = midright - x; 
		return yl * fir + yr * fil - fil * fir * (0.5 * (dyr - dyl) + (fil - 0.5) * (dyl + dyr - 2 * (yr - yl))); 
	}
    
	/* rest */
	left = midright - maxDepth, right = midleft + maxDepth; 
	a = M_PI * (x - midleft); 
	halfsina = 0.5 * sin (a); 
	aa = a / (x - left + 1); 
	daa = M_PI / (x - left + 1); 
	for (ix = midleft; ix >= left; ix --) { 
		mx_real_t d = halfsina / a * (1.0 + cos (aa)); 
		result += y [ix] * d; 
		a += M_PI;	 
		aa += daa;	 
		halfsina = - halfsina; 
	} 
	a = M_PI * (midright - x); 
	halfsina = 0.5 * sin (a); 
	aa = a / (right - x + 1); 
	daa = M_PI / (right - x + 1); 
	for (ix = midright; ix <= right; ix ++) { 
		mx_real_t d = halfsina / a * (1.0 + cos (aa)); 
		result += y [ix] * d; 
		a += M_PI;	 
		aa += daa; 
		halfsina = - halfsina; 
	} 
	return result; 
}


/*
  Closely modelled after the netlib code by Oleg Keselyov.
*/
mx_real_t minimize_brent (mx_real_t (*f) (mx_real_t x, void *closure), mx_real_t a, mx_real_t b, void *closure, mx_real_t tol, mx_real_t *fx) {
	mx_real_t x, v, fv, w, fw;
	const mx_real_t golden = 1 - NUM_goldenSection;
	int iter, itermax = 2; //itermax zwischen 2 und 65
    
	if (tol <= 0 || a >= b)
		rs_error("invalid in minimize brent");
   
	/* First step - golden section */
   
	v = a + golden * (b - a);
	fv = (*f)(v, closure);
	x = v;  w = v;
	*fx = fv;  fw = fv;
   
	for(iter = 1; iter <= itermax; iter++) {
		mx_real_t range = b - a;
		mx_real_t middle_range = (a + b) / 2;
		mx_real_t tol_act=tol*fabs(x)+ZEPS; /* geaendert gemaess "Numerical recipes in C", urspruenglich: mx_real_t tol_act = sqrt_epsilon * fabs(x) + tol / 3;*/
	//     mx_real_t tol_act= ZEPS*fabs(x) + tol/3;
		mx_real_t new_step; /* Step at this iteration */
     
		if (fabs(x - middle_range) + range / 2 <= 2 * tol_act) {
			return x;
		}
     
		/* Obtain the golden section step */
		new_step = golden * (x < middle_range ? b - x : a - x);
	
		/* Decide if the parabolic interpolation can be tried	*/
		if (fabs(x - w) >= tol_act) {
	    /*
			Interpolation step is calculated as p/q; 
			division operation is delayed until last moment.
	    */
			mx_real_t p, q, t;
       
			t = (x - w) * (*fx - fv);
			q = (x - v) * (*fx - fw);
			p = (x - v) * q - (x - w) * t;
			q = 2 * (q - t);
	    
			if( q > 0) {
				p = -p;
			}
			else {
				q = -q;
			}
	    
	    /*
			If x+p/q falls in [a,b], not too close to a and b,
			and isn't too large, it is accepted.
			If p/q is too large then the golden section procedure can
			reduce [a,b] range.
	    */
	    
			if( fabs (p) < fabs (new_step * q) && p > q * (a - x + 2 * tol_act) && p < q * (b - x - 2 * tol_act)) {
				new_step = p / q;
			}
		}
     
		/* Adjust the step to be not less than tolerance. */
		if (fabs(new_step) < tol_act) {
			new_step = new_step > 0 ? tol_act : - tol_act;
		}

		/* Obtain the next approximation to min	and reduce the enveloping range */
		{
			mx_real_t t = x + new_step;	/* Tentative point for the min	*/
			mx_real_t ft = (*f)(t, closure);
       
	    /*
			If t is a better approximation, reduce the range so that
			t would fall within it. If x remains the best, reduce the range
			so that x falls within it.
	    */
			if (ft <= *fx) {
				if( t < x ) {
					b = x;
				}
				else {
					a = x;
				}
		
				v = w;  w = x;  x = t;
				fv = fw;  fw = *fx;  *fx = ft;
			}
			else {        		             
				if (t < x) {
					a = t;
				}                   
				else {
					b = t;
				}
		
				if (ft <= fw || w == x) {
					v = w; w = t;
					fv = fw; fw = ft;
				}
				else 
					if (ft <= fv || v == x || v == w) {
					v = t;
					fv=ft;
					}
			}
		}
	}
    // rs_warning ("minimize_brent: maximum number of iterations (%d) exceeded.", itermax);
	return x;
}



mx_real_t improveMaximum (mx_real_t *y, int nx, int ixmid, int interpolation, mx_real_t *ixmid_real) { 
	struct improve_params params;
	mx_real_t result;
	int isMaximum=1;
	if (ixmid <= 1) { 
		*ixmid_real = 1; 
		return y [1]; 
	}
	if (ixmid >= nx) { 
		*ixmid_real = nx; 
		return y [nx]; 
	}
	if (interpolation <= NUM_PEAK_INTERPOLATE_NONE) { 
		*ixmid_real = ixmid; 
		return y [ixmid]; 
	}
	if (interpolation == NUM_PEAK_INTERPOLATE_PARABOLIC) {
		mx_real_t dy = 0.5 * (y [ixmid + 1] - y [ixmid - 1]);
		mx_real_t d2y = 2 * y [ixmid] - y [ixmid - 1] - y [ixmid + 1];
		*ixmid_real = ixmid + dy / d2y;
		return y [ixmid] + 0.5 * dy * dy / d2y;
	}
	/* Sinc interpolation. */
	params. y_d = y;
	params. depth = interpolation == NUM_PEAK_INTERPOLATE_SINC70 ? 70 : 700;
	params. ixmax = nx;
	params. isMaximum = isMaximum;
	*ixmid_real = minimize_brent (improve_evaluate, ixmid - 1, ixmid + 1, & params, 1e-10, & result);
	return isMaximum ? - result : result;
}


//pitch_frame_t* pathFinder (pitch_frame_t frame_candidates[], mx_real_t ceiling, mx_real_t maxnCand, int nframes, mx_real_t dt) {
pitch_frame_t* pathFinder(pitch_t *pitch, int nframes) {

	int iframe;
	int icand, icand1, icand2, maxnCandidates = pitch->maxnCandidates, place, i;
	mx_real_t ceiling=pitch->maximumPitch, dt=pitch->dt;
	mx_real_t timeStepCorrection = 0.01 / dt;
	mx_real_t maximum, value, octaveJumpCost=pitch->octave_jump_cost * timeStepCorrection, voicedUnvoicedCost=pitch->voiced_unvoiced_cost* timeStepCorrection;
	mx_matrix_t delta;
	int **psi;
	mx_real_t ceiling2 = 2 * ceiling;
	pitch_frame_t** frame_candidates=pitch->frame_candidates;
	pitch_frame_t* last_frame;
//    pitch_candidate_t last_candidate;

	delta = mx_matrix_create (nframes, maxnCandidates);
	psi = (int **) rs_malloc(nframes * sizeof(int *), "int matrix");
	for (i = 0; i < nframes; i++)
		psi[i] = (int *) rs_calloc(maxnCandidates, sizeof(int), "int matrix row");

	if (! delta || ! psi) 
		goto end;

	for (iframe = 0; iframe < nframes; iframe ++) {
		pitch_frame_t *thisFrame = frame_candidates [iframe];
		mx_real_t unvoicedStrength = pitch->silence_threshold <= 0 ? 0 : 2 - thisFrame->intensity / (pitch->silence_threshold / (1 + pitch->voicing_threshold));
		unvoicedStrength = pitch->voicing_threshold + (unvoicedStrength > 0 ? unvoicedStrength : 0);
		for (icand = 0; icand < thisFrame->nCandidates; icand ++) {
			pitch_candidate_t* candidate = &thisFrame->candidates [icand];
			int voiceless= candidate->F == 0 || candidate->F > ceiling2;
			delta [iframe] [icand] = voiceless ? unvoicedStrength : candidate->R - pitch->octave_cost * log (ceiling / candidate->F)/M_LN2;
		}
	}

	/* Look for the most probable path through the maxima. */
	/* There is a cost for the voiced/unvoiced transition, */
	/* and a cost for a frequency jump. */

	for (iframe = 1; iframe < nframes; iframe ++) {
		pitch_frame_t *prevFrame = frame_candidates [iframe - 1], *curFrame = frame_candidates [iframe];
		mx_real_t *prevDelta = delta [iframe - 1], *curDelta = delta [iframe];
		int *curPsi = psi [iframe];
		for (icand2 = 0; icand2 < curFrame -> nCandidates; icand2 ++) {
			mx_real_t f2 = curFrame -> candidates [icand2]. F;
			maximum = -10;
			place = 0;
			for (icand1 = 0; icand1 < prevFrame -> nCandidates; icand1 ++) {
				mx_real_t f1 = prevFrame -> candidates [icand1]. F, transitionCost;
				int previousVoiceless = f1 <= 0 || f1 >= ceiling2;
				int currentVoiceless = f2 <= 0 || f2 >= ceiling2;
				if (previousVoiceless != currentVoiceless)   /* Voice transition. */
					transitionCost = voicedUnvoicedCost;
				else 
					if (currentVoiceless)   /* Both voiceless. */
						transitionCost = 0;
				else   /* Both voiced; frequency jump. */
					transitionCost = octaveJumpCost * fabs (log (f1 / f2) / M_LN2);
				value = prevDelta [icand1] - transitionCost + curDelta [icand2];
				if (value > maximum) {
					maximum = value;
					place = icand1;
				}
			}
			curDelta [icand2] = maximum;
			curPsi [icand2] = place;  
		}
	}

	/* Find the end of the most probable path. */

	maximum = delta [nframes-1] [place = 0];
	for (icand = 1; icand < frame_candidates [nframes-1]->nCandidates; icand ++)
		if (delta [nframes-1] [icand] > maximum)
			maximum = delta [nframes-1] [place = icand];


	/* Backtracking: follow the path backwards. */

	for (iframe = nframes-1; iframe >= 0; iframe --) {
		pitch_frame_t *frame = frame_candidates [iframe];
		pitch_candidate_t help = frame -> candidates [0];
		frame -> candidates [0] = frame -> candidates [place];
		frame -> candidates [place] = help;
		place = psi [iframe] [place];
	}

	/* Pull formants: devoice frames with frequencies between ceiling and ceiling2. */

	if (ceiling2 > ceiling) for (iframe = nframes-1; iframe >= 0; iframe --) {
		pitch_frame_t *frame = frame_candidates [iframe];
		pitch_candidate_t *winner = & frame -> candidates [0];
		mx_real_t f = winner ->F;
		if (f > ceiling && f <= ceiling2) {
			for (icand = 1; icand < frame -> nCandidates; icand ++) {
				pitch_candidate_t *loser = & frame -> candidates [icand];
				if (loser -> F == 0.0) {
					pitch_candidate_t help = * winner;
					* winner = * loser;
					* loser = help;
					break;
				}
			}
		}
	}
  
end:
	 mx_matrix_destroy(delta,nframes);
for (i = 0; i < nframes; i++)
 rs_free(psi[i]);
rs_free (psi);
return NULL;
last_frame= (pitch_frame_t *) rs_malloc(sizeof(pitch_frame_t),"last pitch frame"); 
last_frame->candidates= (pitch_candidate_t *) rs_malloc(sizeof(pitch_candidate_t),"candidate of last pitch frame");
last_frame->candidates[0]=frame_candidates[nframes-1]->candidates[0]; 
last_frame->intensity = frame_candidates[nframes-1]->intensity; 
last_frame->nCandidates=1;
// wenn last_frame verwendet werden soll, dann auch pitch_frame_candidates_destroy aendern!!
return (last_frame); 
}


void pitch_destroy (pitch_t *p) {
	if (!p)
		rs_error ("Cannot free NULL pointer!");
	if (p->last_frame) {
		if (p->last_frame->candidates) {
			rs_free (p->last_frame->candidates);
			p->last_frame->candidates=NULL;
		}
		rs_free(p->last_frame);
		p->last_frame=NULL;
	}
	if (p->frame_candidates)
		pitch_frame_candidates_destroy(p);
	rs_free (p);
	p=NULL;
}

void pitch_frame_candidates_destroy (pitch_t *p) {
	int i;
	int nframes=p->nframes;

	if (!p || !nframes) 
		return;


	for (i=0;i<nframes;i++){
		if (p->frame_candidates[i]->candidates) {
			rs_free(p->frame_candidates[i]->candidates); 
			p->frame_candidates[i]->candidates=NULL;
		}
		if (p->frame_candidates[i]) {
			rs_free (p->frame_candidates[i]); 
			p->frame_candidates[i]=NULL;
		}
	} 
	rs_free(p->frame_candidates); 
	p->frame_candidates=NULL;

}

