/**
* File:		mfcc_1_1.h
* Author:	Gernot A. Fink
* Date:		26.4.2004
*
* Description:	Baseline definitions for MFCC version 1.1
**/

#ifndef __DSP_MFCC_1_1_H_INCLUDED__
#define __DSP_MFCC_1_1_H_INCLUDED__

/*
 * Constant Definitions
 */
#define V1_1_SAMPLERATE		16000
#define V1_1_N_CHANNELS		1
#define V1_1_FRAME_LEN		256
#define V1_1_FRAME_SHIFT	160
#define V1_1_N_FRESOLUTION	((mx_real_t)V1_1_SAMPLERATE / V1_1_FRAME_LEN)
#define V1_1_MIN_FREQ		(3 * V1_1_N_FRESOLUTION)
#define V1_1_MAX_FREQ		(V1_1_SAMPLERATE / 2)
#define V1_1_N_FILTERS		31
#define V1_1_W_LENGTH		5
#define V1_1_N_BASEFEATURES	(1 + 12)
#define V1_1_N_FEATURES		(3 * V1_1_N_BASEFEATURES)

#endif /* __DSP_MFCC_1_1_H_INCLUDED__ */

