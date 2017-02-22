/**
 *	@file	csp.h
 *	@author	Christian Plahl
 *	@date	Dez. 2004
 *
 *	@brief	Definitions for Cross Power Spektrum Phase estimation (CSP).
 */

#ifndef __DSP_CSP_H_INCLUDED__
#define __DSP_CSP_H_INCLUDED__

#include "ev_dsp.h"
#include "ev_sdft.h"
#include "ev_errors.h"

/*
 * Constants
 */
/* status of a csp- element ... */
#define DSP_CSP_STATUS_INVALID	-1 /**< invalid data state */
#define DSP_CSP_STATUS_VALID	0 /**< valid data state */

/* method to use for csp- estimation */
#define DSP_CSP_METHOD_NORMAL	1
#define DSP_CSP_METHOD_SHIFT	2
#define DSP_CSP_METHOD_SLIDING	3

/*	------------ Data Types ------------*/

/**	a dsp_csp_data struct.
 *
 *	This structure is for handling types of results after the
 *	Cross power Spectrum Phase (CSP) estimation. The data will be stored
 *	in the 'data' buffer of max size 'n_channels' * 'max_len'.
 *	The real number of entries for each channel in the 'data' buffer
 *	should be stored in the 'n_len' array.
 */
typedef struct {
	int n_channels; /**< number of channels in the buffer */
	int max_len; /**< number of elements for each channel */
	int* n_len; /**< for each channel the real length */
	mx_real_t** data; /**< the data buffer, dimension: 'n_channels' * 'max_len' */
} dsp_csp_data_t;


/**	a dsp_csp struct.
 *
 *	This structure is for handling the Cross power Spectrum Phase (CSP) estimation.
 *	When you want to use it, first create the structure with \ref dsp_csp_create
 *	and then configure with the \ref dsp_csp_configure function.
 *	Use \ref dsp_csp_destroy to free your structure at the end.
 */
typedef struct {
	int n_samples; /**< number of samples in the buffer */
	int n_channels; /**< number of channels in the buffer */
	int shift; 	/**< shift in samples for next calculation */
	int use_method; /**< method for the CSP estimation, possible values are
		DSP_CSP_METHOD_NORMAL, DSP_CSP_METHOD_SHIFT or DSP_CSP_METHOD_SLIDING */
	int iter; /**< number of iteration of calculation after last call to configure */
	int samplingrate; /**< sampling rate in kHz */
	float min_filter; /**< lower frequency in kHz for the band pass filter */
	float max_filter; /**< upper frequency in kHz for the band pass filter */

	int buf_len; /**< length of the signalbuffer (n_samples * n_channels) */
	dsp_sample_t *buf; /**< signal buffer (circle) */
	int buf_head; /**< start of the signal data */
	int buf_tail; /**< end ot the signal data */
	
	int fft_len; /**< length of the fft buffer */
	mx_real_t* hwin; /**< hanning window of size 'fft_len' */
    mx_complex_t **buf_fft; /**< complex memory of size 'fft_len' for each channels */
    mx_complex_t **buf_ifft; /**< complex memory of size 'fft_len' for each channels */

	int m_channels; /**< number of energy vectors */
	int engy_len; 	/**< length of one energy vector */
	mx_real_t **energy; /**< energydata for each channel of length 'engy_len' */

/*	int *peak_len; 	/**< length of the peak vector for each channel*/
/*	mx_real_t **peaks; /**< peakdata for each channel */

	int status;		/**< status of the structure, can be
			'DSP_CSP_STATUS_VALID' or 'DSP_CSP_STATUS_INVALID' */
} dsp_csp_t;

/*
 * Function Prototypes
 */
dsp_csp_data_t* dsp_csp_data_create(int n_channels, int n_elements);
int dsp_csp_data_destroy(dsp_csp_data_t* csp_data);


dsp_csp_t *dsp_csp_create (int n_channels, int fft_length, int n_samples, int engy_len);
int dsp_csp_destroy (dsp_csp_t *csp);

int dsp_csp_reset (dsp_csp_t *csp);
int dsp_csp_configure (dsp_csp_t *csp, int shift, int method, int sr, float fmin, float fmax);

int dsp_csp_push (dsp_csp_t *csp, dsp_sample_t *sample, int n_samples);
int dsp_csp_calc (dsp_csp_data_t** delays, dsp_csp_data_t* energys, dsp_csp_t *csp, mx_real_t threshold);
int dsp_csp_calc_Shift (dsp_csp_t *csp);
int dsp_csp_calc_Normal (dsp_csp_t *csp);
int dsp_csp_calc_Sliding (dsp_csp_t *csp);
void dsp_csp_peak_detect (dsp_csp_data_t* delays, dsp_csp_t *csp, mx_real_t threshold);

void dsp_csp_data2fft (dsp_csp_t* csp, int signal, int channel1, int channel2, int window);
int dsp_csp_check4filter (dsp_csp_t* csp, int bandnumber);

#endif /* __DSP_CSP_H_INCLUDED__ */
