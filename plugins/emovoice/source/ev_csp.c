/** 
 *	@file	csp.c 
 *	@author	Christian Plahl 
 *	@date	Dez. 2004 
 * 
 *	@brief	Methods for Cross phase Power Spectrum (CSP) calculation. 
 */ 
 
#include <string.h> 
#include <time.h>

#include "ev_memory.h" 
#include "ev_csp.h" 
 
/*	dsp_csp_data_create (n_channels, n_elements) */ 
/**	Create a new dsp_csp_data_t struct. 
 * 
 *	Use this function to create and initialize a new dsp_csp_data_t 
 *	structure. NULL will be returned when the parameters are not valid. 
 *	Valid parameters are: 
 *	- n_channels > 0 
 *	- n_elements > 0 
 * 
 *	@param	n_channels	number of channel for the buffer. 
 *	@param	n_elements	number of max elements for each channel. 
 * 
 *	@return	pointer to the new created structure or NULL. 
 */ 
dsp_csp_data_t* dsp_csp_data_create (int n_channels, int n_elements) 
{ 
	dsp_csp_data_t* data = NULL; 
	int channel; 
 
	/* first check parameters ... */ 
	if ((n_channels < 1) || (n_elements < 1)) 
		return (NULL); 
 
	/* ... allocate memory for csp_data element ... */ 
	data = rs_malloc (sizeof(dsp_csp_data_t), "csp-data element"); 
 
	/* ... and initialize it */ 
	(data->n_channels) = n_channels; 
	(data->max_len)    = n_elements; 
	(data->n_len)      = rs_malloc ((n_channels * sizeof(int)), "csp-data element"); 
	(data->data)       = rs_malloc ((n_channels * sizeof(mx_real_t*)), "csp-data element"); 
 
	for (channel = 0; channel < n_channels; channel++) { 
		(data->n_len)[channel] = 0; 
		(data->data) [channel] = rs_malloc ((n_elements * sizeof(mx_real_t)), "csp-data element"); 
	}	 
	 
	return (data); 
} 
 
/*	dsp_csp_data_destroy (csp_data) */ 
/**	Destroy a dsp_csp_data_t structure. 
 * 
 *	Use this function to destroy a dsp_csp_data_t structure 
 *	and free the used memory. 
 *	If the pointer is not valid (NULL) \ref DSP_ERROR will be returned. 
 * 
 *	@param	csp_data	pointer to the structure to destroy. 
 *	 
 *	@return	\ref DSP_SUCCESS if detroyed successfully, \ref DSP_ERROR else. 
 */ 
int dsp_csp_data_destroy (dsp_csp_data_t* csp_data) 
{ 
	int channel; 
 
	/* first check parameters ... */ 
	if (csp_data == NULL) 
		return (DSP_ERROR); 
 
	/* ... then invalidate and destroy csp-data element */ 
	if (csp_data->n_len != NULL) 
		rs_free (csp_data->n_len); 
 
	if (csp_data->data != NULL) { 
		for (channel = 0; channel < (csp_data->n_channels); channel++)  
			if (csp_data->data[channel] != NULL) 
				rs_free (csp_data->data[channel]); 
		rs_free (csp_data->data); 
	} 
 
	/* free dsp_csp_data_t structure */ 
	memset (csp_data, -1, sizeof(dsp_csp_data_t)); 
	rs_free (csp_data); 
	 
	return (DSP_SUCCESS); 
} 
 
 
/*	dsp_csp_create (n_channels, fft_length, n_samples, engy_len) */ 
/**	Create a new dsp_csp_t structure. 
 * 
 *	This function creates and initialize a dsp_csp_t structure. 
 *	NULL will be returned when the parameters are not valid. 
 *	Valid parameters are: 
 *	- n_channels > 1 
 *	- fft_length > 0 
 *	- n_samples > fft_length 
 *	- engy_len > 1 
 * 
 *	Use \ref dsp_csp_configure for the rest of configuration for 
 *	this stucture, \ref dsp_csp_destroy to destroy the structure 
 *	at the end. 
 *	This function allocates all neccessary memory for the  
 *	structure. If you want to change same of the parameters 
 *	you have to create a new dsp_csp_t structure. 
 * 
 *	@param	n_channels	number of channel for the buffer 
 *	@param	fft_length	length for the fft calculation 
 *	@param	n_samples	number of samples to hold in the buffer 
 *	@param	engy_len	number of energy values for each channel 
 * 
 *	@return	pointer to the new allocated structure or NULL 
 */ 
dsp_csp_t *dsp_csp_create (int n_channels, int fft_length, int n_samples, int engy_len) 
{ 
	int channel; 
	dsp_csp_t *csp		= NULL; 
 
	/* first check parameters ... */ 
	if ((n_channels < 2) || (fft_length < 1) || (engy_len < 1) 
			|| (n_samples < fft_length)) 
		return (NULL); 
 
  	/* ... allocate memory for csp-calc element ... */ 
	csp 				= rs_malloc (sizeof(dsp_csp_t), "csp-calc element"); 
 
	/* ... and initialize some global values here ... */ 
	(csp->n_samples)	= n_samples; 
	(csp->n_channels)	= n_channels; 
 
	/* ... initialize signal elements ... */ 
	(csp->buf_len)		= (n_samples * n_channels); 
	(csp->buf)			= rs_malloc ((csp->buf_len) * sizeof(dsp_sample_t), "csp-signal element"); 
	(csp->buf_head) 	= 0; 
	(csp->buf_tail) 	= 0; 
 
	/* ... initialize fft elements ... */ 
	(csp->fft_len)		= fft_length; 
	(csp->hwin)			= dsp_window_hanning_create (fft_length); 
	(csp->buf_fft)		= rs_malloc (n_channels * sizeof(mx_complex_t*), "csp-fft element"); 
    (csp->buf_ifft)		= rs_malloc (n_channels * sizeof(mx_complex_t*), "csp-ifft element"); 
	for (channel = 0; channel < n_channels; channel++) { 
		(csp->buf_fft) [channel] = rs_malloc (fft_length * sizeof(mx_complex_t), "csp-fft element"); 
		(csp->buf_ifft)[channel] = rs_malloc (fft_length * sizeof(mx_complex_t), "csp-ifft element"); 
	} 
 
	/* ... initialize energy elements ... */ 
/*	(csp->m_channels)	= (csp->n_channels) * ((csp->n_channels) -1) / 2; /* gauss formula */  
	(csp->m_channels)	= (csp->n_channels) -1;
	(csp->engy_len)		= engy_len; 
	(csp->energy)		= rs_malloc ((csp->m_channels) * sizeof(mx_real_t*), "csp-energy element"); 
	for (channel = 0; channel < (csp->m_channels); channel++)  
		(csp->energy)[channel] = rs_malloc ((csp->engy_len) * sizeof(mx_real_t), "csp-energy element"); 
 
	/* ... be ready, but invalid status */	 
	(csp->status)		= DSP_CSP_STATUS_INVALID; 
	return (csp); 
} 
 
/*	dsp_csp_destroy (csp) */ 
/**	Destroy a dsp_csp_t structure. 
 * 
 *	Destroys a dsp_csp_t structure and frees the used memory. 
 *	If the pointer is not valid (NULL) \ref DSP_ERROR will be returned. 
 * 
 *	@param	csp	pointer to the structure to destroy 
 * 
 *	@return	\ref DSP_SUCCESS if detroyed, \ref DSP_ERROR else. 
 */ 
int dsp_csp_destroy (dsp_csp_t *csp) 
{ 
	int channel; 
 
	/* first check parameters ... */ 
	if (csp == NULL) 
		return (DSP_ERROR); 
 
	/* ... then invalidate and destroy csp-calc element */ 
	/* signal */ 
	if (csp->buf != NULL) 
		rs_free (csp->buf); 
 
	/* ... free FFT- structure ... */ 
	if (csp->buf_fft != NULL) { 
		for (channel = 0; channel < (csp->n_channels); channel++)  
			if (csp->buf_fft[channel] != NULL) 
				rs_free (csp->buf_fft[channel]); 
		rs_free (csp->buf_fft); 
	} 
	if (csp->buf_ifft != NULL) { 
		for (channel = 0; channel < (csp->n_channels); channel++)  
			if (csp->buf_ifft[channel] != NULL) 
				rs_free (csp->buf_ifft[channel]); 
		rs_free (csp->buf_ifft); 
	} 
 
	/* ... frees hanning window ... */ 
	if (csp->hwin != NULL) { 
		rs_free (csp->hwin); 
	}	 
	 
	/* ... frees Energy- structure ... */ 
	if (csp->energy != NULL) { 
		for (channel = 0; channel < (csp->m_channels); channel++)  
			if (csp->energy[channel] != NULL) 
				rs_free (csp->energy[channel]); 
		rs_free (csp->energy); 
	} 
 
	/* ... and at last the csp- structure itself */ 
	memset (csp, -1, sizeof(dsp_csp_t)); 
	rs_free (csp); 
 
	return (DSP_SUCCESS); 
} 
 
/*	dsp_csp_configure (csp, shift, method, sr, fmin, fmax) */ 
/**	configure the dsp_csp_t structure. 
 * 
 *	configure will be initialize the rest of global values 
 *	and will set all other neccessary information to work with 
 *	this structure. Use this function after using \ref dsp_csp_create 
 *	and before using any other function, except \ref dsp_csp_destroy. 
 * 
 *	@param	csp	pointer to the dsp_csp_t structure. 
 *	@param	shift	global shift in samples for next calculation step. 
 *	@param	method	method for CSP calculation, use DSP_CSP_METHOD_NORMAL, 
 *		DSP_CSP_METHOD_SHIFT or DSP_CSP_METHOD_SLIDING. 
 *	@param	sr	sampling rate in kHz. 
 *	@param	fmin lower frequency bound in kHz. 
 *	@param	fmax upper frequency bound in kHz. 
 * 
 *	@return	\ref DSP_SUCCESS if all goes right, \ref DSP_ERROR else. 
 */ 
int dsp_csp_configure (dsp_csp_t *csp, int shift, int method, int sr, float fmin, float fmax) 
{ 
	// begin_change_thurid
	//int channel; 
	// end_change_thurid
 
	/* first check parameters ... */ 
	if (csp == NULL) 
		return (DSP_ERROR); 
 
	/* store (other) global parameters here ...*/ 
	(csp->shift)		= shift; 
	(csp->use_method)	= method; 
	(csp->samplingrate) = sr; 
	(csp->min_filter)	= fmin; 
	(csp->max_filter)	= fmax; 
	 
	/* ... number of calc iteration since last call to configure ... */ 
	(csp->iter)			= 0; 
	 
	/* ... and declare CSP element as valid */ 
	(csp->status)		= DSP_CSP_STATUS_VALID; 
 
	return (DSP_SUCCESS); 
} 
 
/*	dsp_csp_reset (csp) */ 
/**	reset configuration. 
 * 
 *	reset the configuration of the dsp_csp_t structure. You have to 
 *	use \ref dsp_csp_configure again to get a valid dsp_csp_t structure. 
 * 
 *	@param	csp	pointer to the dsp_csp_t structure. 
 * 
 *	@return	\ref DSP_SUCCESS if all goes right, \ref DSP_ERROR else. 
 */ 
int dsp_csp_reset (dsp_csp_t *csp) 
{ 
	/* first check parameters ... */ 
	if (csp == NULL) 
		return (DSP_ERROR); 
 
	/* ... reset head and tail pointers ... */ 
	(csp->buf_head)	= (csp->buf_tail) = 0; 
	 
	/* ... reset configure parameters (use dsp_csp_configure to set) ... */ 
	(csp->shift)	= (csp->use_method) = (csp->samplingrate) 
					= (csp->min_filter) = (csp->max_filter)	= 0; 
 
	/* ... and declare status as invalid => use dsp_csp_configure for valid data */ 
	(csp->status)	= DSP_CSP_STATUS_INVALID; 
 
	return (DSP_SUCCESS); 
} 
 
/*	dsp_csp_push (csp, sample, n_samples) */ 
/**	Push n sample to buffer. 
 * 
 *	This function pushes n samples to the intern buffer to have new data for the 
 *	next csp calculation step. The process sequence is as follows: 
 *	- test if possible to push data 
 *	- push data in buffer (if necessary) 
 *	- set buffer-head-pointer (if necessary)  
 * 
 *	@param	csp	pointer to the dsp_csp_t structure 
 *	@param	sample	pointer to the raw data to push (length = n_channels * n_samples) 
 *	@param	n_samples	how many samples to copy 
 * 
 *	@return	\ref DSP_SUCCESS on success, otherwise \ref DSP_ERROR or \ref DSP_ERROR_DATA_INVALID 
 * 
 *	@warning	\ref DSP_ERROR will be returned if pushing is not possible. 
 *		Try \ref dsp_csp_calc at first and then try \ref dsp_csp_push again. 
 *	@warning	\ref DSP_ERROR_DATA_INVALID will be returned if the dsp_csp_t structure 
 *		is not valid. Use \ref dsp_csp_configure for valid data. 
 */ 
int dsp_csp_push (dsp_csp_t *csp, dsp_sample_t *sample, int n_samples) 
{ 
	int i; 
	int iPush; 
	int iNewHead; 
	int iBufPos; 
	 
	/* check if structure is valid to use it */ 
	if ((csp->status) == DSP_CSP_STATUS_VALID) { 
		 
		/* check if buffer is long enough to hold the n_samples ...*/ 
		iNewHead = ((csp->buf_head) < (csp->buf_tail)) ? (csp->buf_tail) : ((csp->buf_tail) + (csp->buf_len)); 
		iPush = iNewHead > (csp->buf_head) + ((csp->n_channels) * n_samples); 
	 
		/* ... and push data (if you can) */ 
		if (iPush != 0) { 
			/* copy samples */ 
			for (i = 0; i < (n_samples * (csp->n_channels)); i++) { 
				iBufPos = ((csp->buf_head) + i) % (csp->buf_len); 
				(csp->buf)[iBufPos] = sample[i]; 
			} 
		 
			/* set new head- pointer */ 
			(csp->buf_head) = (((csp->buf_head) + ((csp->n_channels) * n_samples)) % (csp->buf_len)); 
		} else { 
			/* error because cannot push data, free buffer elements to short */ 
/*			 rs_msg("Cannot push data into the buffer, try 'dsp_csp_calc' first!"); */ 
			return (DSP_ERROR); 
		} 
		 
		return (DSP_SUCCESS); 
	} else  
		return (DSP_ERROR_DATA_INVALID); 
} 
 
/*	dsp_csp_calc_Normal (csp) */ 
/**	Cross power Spectrum phase estimation. 
 * 
 *	This Cross power Spectrum phase (CSP) estimation go ahead as follow: 
 *	- extract signal and multiply it with a hanning windows. 
 *	- transform the signal by using a fourier transformation. 
 *	- use a band pass filter on the frequencies. 
 *	- calculate the correlation (\f[ FT_1 * FT_2^{*} \f]). 
 *	- transform back, using an inverse fourier transformation. 
 *	- estimate the energy values. 
 * 
 *	for more information see: 
 *	D. Giuliani and M. Omologo and P. Svaizer, 
 *	Talker Localization and Speech Recognition Using a Microphone Array and 
 *	a Cross-Powerspectrum Phase Analysis,  
 *	ICSLP, Vol. 3, p. 1243--1246, year 1994 
 *	(url = citeseer.nj.nec.com/giuliani94talker.html) 
 * 
 *	@param	csp	pointer to the dsp_csp_t structure. 
 * 
 *	@return	\ref DSP_SUCCESS on success.	 
 */ 
int dsp_csp_calc_Normal (dsp_csp_t *csp) 
{ 
	int sample, channel; 
	int buf_pos, fft_pos, shift; 
	float frequency; 
	mx_real_t a, b, x, y, u, v; /* complex values */ 
	 
	/* extract signal, multiplying with window and convert into complex structure ... */ 
	dsp_csp_data2fft (csp, 0, 0, ((csp->n_channels) -1), 1); 
 
	/* ... calculates complex values for every channel */ 
	for (sample = 0; sample < (csp->fft_len); sample++) { 
 
		/* use band pass filter, 1 = filter, 0 = calculation */ 
		if (dsp_csp_check4filter (csp, sample) == 0) { 
 
			/* reference channel! ... */ 
			x = mx_re(csp->buf_fft[0][sample]); 
			y = mx_im(csp->buf_fft[0][sample]); 
 
			/* ... all other channels */ 
			for (channel = 1; channel < (csp->n_channels); channel++) { 
				/* real and imaginary part of the sample ... */ 
				u = mx_re(csp->buf_fft[channel][sample]); 
				v = mx_im(csp->buf_fft[channel][sample]); 
 
				/* ... CSP- calculation (FFT1 * [(FFT2)complex conjugate]) ... */ 
				a = (x*u + y*v) / (sqrt (x*x + y*y) * sqrt (u*u + v*v)); 
				b = (y*u - x*v) / (sqrt (x*x + y*y) * sqrt (u*u + v*v)); 
 
				/* save values for backtransformation */ 
				mx_re(csp->buf_ifft[channel -1][sample]) = a; 
				mx_im(csp->buf_ifft[channel -1][sample]) = b; 
			} 
		} else { 
			/* band pass filter is activ => value = 0.0 */ 
			for (channel = 1; channel < (csp->n_channels); channel++) { 
				mx_re(csp->buf_ifft[channel -1][sample]) = 0.0; 
				mx_im(csp->buf_ifft[channel -1][sample]) = 0.0; 
			} 
		} 
	} 
	
	/* ... backtransformation for every channel by ifft ... */ 
	for (channel = 0; channel < ((csp->n_channels) -1); channel++) 
		dsp_xfft((csp->buf_ifft[channel]), (csp->fft_len), 1); 
	 
	shift = (int) ((csp->engy_len) / 2); 
	 
	/* ... and calculates energyvalues + norm, because ifft doesn't do this */ 
	for (channel = 0; channel < ((csp->n_channels) -1); channel++) 
		for (sample = -shift; sample <= shift; sample++) { 
			/* value of 'sample' is negativ, but indexes are positiv => short conversion ... */ 
			buf_pos = (sample + (csp->fft_len)) % (csp->fft_len); 
	 
			/* ... get comples values ... */ 
			a = mx_re(csp->buf_ifft[channel][buf_pos]); 
			b = mx_im(csp->buf_ifft[channel][buf_pos]); 
			 
			/* ... and calculates energies + norm */ 
/*			csp->energy[channel][sample + shift] = sqrt(a * a + b * b) / (float) (csp->fft_len); */
			csp->energy[channel][-sample + shift] = sqrt(a * a + b * b) / (float) (csp->fft_len); 
		} 
	
	return (DSP_SUCCESS); 
} 
 
/*	dsp_csp_calc_Shift (csp) */ 
/**	Cross power Spectrum phase estimation. 
 * 
 *	This Cross power Spectrum phase (CSP) estimation go ahead as follow: 
 *	- for the first channel do 
 *		- extract signal 
 *		- multiply signal with a hanning windows 
 *		- transformate signal using a fourier transformation 
 *		. 
 *	- for each enery value to calculate do 
 *		- extract signal of other channel (with offset) 
 *		- multiply signal with a hanning windows 
 *		- transform signal using a fourier transformation 
 *		- use a band pass filter (on frequencies) 
 *		- calculate only the real part of the value for \f[ fft_1 * fft_2^{*} \f] 
 *		- use backtransformation to get energy value 
 * 
 *	@param	csp	pointer to the dsp_csp_t structure. 
 * 
 *	@return	\ref DSP_SUCCESS on success.	 
 */ 
int dsp_csp_calc_Shift (dsp_csp_t *csp) 
{ 
	int sample, channel; 
	int buf_pos, fft_pos; 
	int shift; 
	mx_real_t a, b, x, y, u, v; /* complex values */ 
	 
	/*	set energyvalues to zero for better calculation (there will be a summation at the end). */ 
	for (channel = 0; channel < (csp->m_channels); channel++) 
		for (sample = 0; sample < (csp->engy_len); sample++) 
			(csp->energy)[channel][sample] = 0.0; 
 
	/*	extract signal, multipling with window and convert into complex structure  
	 *	only for referenz buffer (channel 0)! shift for startsamples is 
	 *	((csp->engy_len) / 2). Also do a fft- transformation for channel 0. */ 
	dsp_csp_data2fft (csp, (int) ((csp->engy_len) / 2), 0, 0, 1); 
	 
	/* ... and now calcs energys */ 
	for (shift = 0; shift < (csp->engy_len); shift++) { 

		/* extract signal data and do the fft transformation ... */ 
		dsp_csp_data2fft (csp, shift, 1, ((csp->n_channels) -1), 1); 
 
		/* ... calculates energyvalues for every channel ... */ 
		for (sample = 0; sample < (csp->fft_len); sample++) { 
 
			/* use band pass filter, 1 = filter, 0 = calculation */ 
			if (dsp_csp_check4filter (csp, sample) == 0) { 
				 
				/* first channel is reference channel! */ 
				x = mx_re(csp->buf_fft[0][sample]); 
				y = mx_im(csp->buf_fft[0][sample]); 
	 
				for (channel = 1; channel < (csp->n_channels); channel++) { 
					/* real and imaginary part of sample 'sample' */ 
					u = mx_re(csp->buf_fft[channel][sample]); 
					v = mx_im(csp->buf_fft[channel][sample]); 
	 
					/* csp- calculation (\[ fft_1 * fft_2^{*} \]) */ 
					a = (x*u + y*v) / (sqrt (x*x + y*y) * sqrt (u*u + v*v)); 

#ifndef DSP_CSP_SLOW
					/* save and sum value as backtransformation */ 
					(csp->energy[channel -1][shift]) += a; /* 2*a */
				}
			}
		}
#else
					b = (y*u - x*v) / (sqrt (x*x + y*y) * sqrt (u*u + v*v)); 

					/* save values for backtransformation */ 
					mx_re(csp->buf_ifft[channel -1][sample]) = a; 
					mx_im(csp->buf_ifft[channel -1][sample]) = b; 
				} 
			} else { 
				/* band pass filter is activ => value = 0.0 */ 
				for (channel = 1; channel < (csp->n_channels); channel++) { 
					mx_re(csp->buf_ifft[channel -1][sample]) = 0.0; 
					mx_im(csp->buf_ifft[channel -1][sample]) = 0.0; 
				} 
			} 
		}
		
		/* ... backtransformation for every channel by ifft ... */ 
		for (channel = 0; channel < ((csp->n_channels) -1); channel++) 
			dsp_xfft((csp->buf_ifft[channel]), (csp->fft_len), 1); 

		/* ... aus Elem 0 hole energy */ 
		for (channel = 0; channel < ((csp->n_channels) -1); channel++) {
			a = mx_re(csp->buf_ifft[channel][0]);
			b = mx_im(csp->buf_ifft[channel][0]);
			(csp->energy[channel][shift]) = sqrt (a * a + b * b);
		}
#endif

		/* ... and norm all energyvalues because a fft does it too */ 
		for (channel = 0; channel < (csp->m_channels); channel++) 
			(csp->energy)[channel][shift] = (float) ((csp->energy)[channel][shift]) / (float) (csp->fft_len); 
	} 
 
	return (DSP_SUCCESS); 
} 
 
/*	dsp_csp_calc_Sliding (csp) */ 
/**	Cross power Spectrum phase estimation. 
 * 
 *	This Cross power Spectrum phase (CSP) estimation go ahead as follow: 
 *	- for the first channel do: 
 *		- extract signal (NO windowing). 
 *		- transform signal using a fourier transformation. 
 *		. 
 *	- for each enery value to calculate do: 
 *		- on first time: 
 *			- extract signal of the other channel (with offset). 
 *			- transform signal using a fourier transformation. 
 *			. 
 *		- on other times: 
 *			- use a sliding fourier transformation (put new and old sample in 
 *				and get the new spectrum out). 
 *			. 
 *		- use a band pass filter (on frequencies). 
 *		- calculate only the real part of the value for \f[ fft_1 * fft_2^{*} \f]. 
 *		- use a backtransformation to get the energy value. 
 * 
 *	@param	csp	pointer to the dsp_csp_t structure. 
 * 
 *	@return	\ref DSP_SUCCESS on success.	 
 */ 
int dsp_csp_calc_Sliding (dsp_csp_t *csp) 
{ 
	int sample, channel; 
	int buf_pos, shift; 
	int old_sample, new_sample; 
	int left, right; 
	mx_real_t a, x, y, u, v; /* complex values */ 
	 
	/*	set energyvalues to zero for better calculation (sum) */ 
	for (channel = 0; channel < (csp->m_channels); channel++) 
		for (sample = 0; sample < (csp->engy_len); sample++) 
			(csp->energy)[channel][sample] = 0.0; 
 
	/*	extract signal, !NO! multipling with window and convert into complex structure  
	 *	only for reference buffer! shift for startsamples is ((csp->engy_len) / 2). 
	 *	also do a fft- transformation for channel 0. 
	 */ 
	dsp_csp_data2fft (csp, (int) ((csp->engy_len) / 2), 0, 0, 0); 
 
	/* copy signal and fft for the first time of other channels ... */ 
	dsp_csp_data2fft (csp, 0, 1, ((csp->n_channels) -1), 0); 
 
	/* ... and now calcs energys */ 
	for (shift = 0; shift < (csp->engy_len); shift++) { 
 
		/* position of start of the samples for next calculation (sliding dft) ... */ 
		if (shift) { 
			old_sample = ((csp->buf_tail) + ((csp->n_channels) * (shift -1))) % (csp->buf_len); 
			new_sample = ((csp->buf_tail) + ((csp->n_channels) * (shift -1 + (csp->fft_len)))) % (csp->buf_len); 
			 
			/* sliding fft with new sample (forward) */ 
			for (channel = 1; channel < (csp->n_channels); channel++) 
				dsp_rsdft((csp->buf_fft[channel]), (csp->fft_len), 
						(mx_real_t) (csp->buf[old_sample + channel]), 
						(mx_real_t) (csp->buf[new_sample + channel]), 0); 
		} 
		 
		/* ... calculates energyvalues for every channel ... */ 
		for (sample = 0; sample < (csp->fft_len); sample++) { 
 
			/* check for using band pass filter, 1 = filter, 0 = calculation */ 
			if (dsp_csp_check4filter (csp, sample) == 0) { 
			 
				/* calcs window index, use '%' (mod) because of ring buffer ... */ 
				left  = (sample -1 + (csp->fft_len)) % (csp->fft_len); 
				right = (sample +1 + (csp->fft_len)) % (csp->fft_len); 
			 
				/* ... first channel (reference channel)! */ 
				x = 0.5 * mx_re( csp->buf_fft[0][sample] ) 
						- 0.25 * mx_re( csp->buf_fft[0][left] ) 
						- 0.25 * mx_re( csp->buf_fft[0][right] ); 
				y = 0.5 * mx_im( csp->buf_fft[0][sample] ); 
						- 0.25 * mx_im( csp->buf_fft[0][left] ); 
						- 0.25 * mx_im( csp->buf_fft[0][right] );	 
					 
				/* for all other channels */ 
				for (channel = 1; channel < (csp->n_channels); channel++) { 
					/* real and imaginary part of sample 'sample' */ 
					u = 0.5 * mx_re( csp->buf_fft[channel][sample] ); 
						- 0.25 * mx_re( csp->buf_fft[channel][left] ) 
						- 0.25 * mx_re( csp->buf_fft[channel][right] ); 
					v = 0.5 * mx_im( csp->buf_fft[channel][sample] ); 
						- 0.25 * mx_im( csp->buf_fft[channel][left] ); 
						- 0.25 * mx_im( csp->buf_fft[channel][right] );	 
		 
					/* csp- calculation (\[ fft_1 * fft_2^{*} \]) */ 
					a = ( x*u + y*v ) / ( sqrt( x*x + y*y ) * sqrt( u*u + v*v ) ); 
 
					/* save and sum values as backtransformation (only one value)*/ 
					(csp->energy)[channel -1][shift] += a; /* 2*a */ 
				} 
			} 
		} 
		 
		/* ... and norm all energyvalues because a fft does it too */ 
		for (channel = 0; channel < (csp->m_channels); channel++) 
			(csp->energy)[channel][shift] = (csp->energy)[channel][shift] / 
					(float) (csp->fft_len); 
	} 
		 
	return (DSP_SUCCESS); 
} 
 
/* dsp_csp_calc (delays, energys, csp, threshold) */ 
/*	calculation of the delays in radian from a signal. 
 * 
 *	Calculation of the time delay in radian from a signal. 
 *	You can choose one of 3 different version for the calculation of 
 *	the energys. Set the version with the \ref dsp_csp_configure function. 
 *	Values for the different method are: 
 *	- DSP_CSP_METHOD_NORMAL, see \ref dsp_csp_calc_Normal for more information 
 *	- DSP_CSP_METHOD_SHIFT, see \ref dsp_csp_calc_Shift for details. 
 *	- DSP_CSP_METHOD_SLIDING, see \ref dsp_csp_calc_Sliding for an overview. 
 * 	 
 * 	The process sequence is as follow: 
 * 	- check for valid data structure. 
 * 	- check if it is possible to do a calculation step. 
 *	- calculate the energy values. 
 *	- write values out if necessary. 
 * 	- detect peaks in the energy values. 
 * 	- estimate the time delays from the peaks. 
 * 	- set data tail pointer forward. 
 * 	 
 * 	@param	delays	memory for the delays in radian. 
 * 	@param	energys	memory for the energy, if set to NULL, no energy will be returned. 
 * 	@param	csp	pointer	to the dsp_csp_t structure. 
 * 	@param	threshold	threshold for peakfinding 
 * 	 
 * 	@return	number of calculation since the last call of configure, 0 when  
 *		calculation isn't possible or an errorcode. 
 * 	 
 * 	@warning	return 0 when no calculation is possible. Try \ref dsp_csp_push to 
 *		have data for the next calculaion step. 
 *	@warning	\ref DSP_ERROR_DATA_INVALID will be returned if the dsp_csp_t 
 *		structure isn't valid. Use \ref dsp_csp_configure to have a valid structure. 
 *	@warning	\ref DSP_ERROR will be returned if the dimension in the dsp_csp_data_t 
 *		structure to write the delays (in frame number) out, are not as needed. 
 *	@warning	No error will be returned if the dimension in the dsp_csp_data_t 
 *		structure to write the energy values out, are not as needed. 
 */ 
int dsp_csp_calc (dsp_csp_data_t** delays, dsp_csp_data_t* energys, dsp_csp_t *csp, mx_real_t threshold) 
{ 
	int iDoCalc; 
	int iHead, iLength; 
	int channel, sample; 
	 
/* first check Parameters ... */
	/* ... check if csp status for calculation is ok ... */ 
	if (csp->status == DSP_CSP_STATUS_INVALID)
		return (DSP_ERROR_DATA_INVALID);
		
	/*  ... check for memory, perhaps do memory allocation ... */ 
	if ((*delays) == NULL) { 
		/* allocate essential memory ... */ 
		(*delays) = dsp_csp_data_create((csp->m_channels), (csp->engy_len)); 
	} else {
	/* ... check for right dimension */ 
		if ( (((*delays)->n_channels) != (csp->m_channels)) && 
				(((*delays)->max_len) != (csp->engy_len)) ) {
			return (DSP_ERROR);
		}
	}

/* calculation part ... */
	/* ... test if calculation is possible => iCalc = 0 (no calc) or iCalc != 0 (do calc) ... */ 
	switch (csp->use_method) { 
		case DSP_CSP_METHOD_NORMAL: { 
			iLength = (csp->n_channels) * (csp->fft_len); 
			break; 
		} 
		case DSP_CSP_METHOD_SHIFT: {} 
		case DSP_CSP_METHOD_SLIDING: { 
			iLength = (csp->n_channels) * ((csp->fft_len) + (csp->engy_len)); 
			break; 
		}
		default : {
			iLength = (csp->n_channels) * (csp->fft_len); 
		} 
	} 
	iHead = ((csp->buf_head) < (csp->buf_tail)) ? ((csp->buf_head) + (csp->buf_len)) : (csp->buf_head); 
	iDoCalc = ((csp->buf_tail) + iLength) <= iHead; 

	/* ... now do the calculation ... */ 
	if (iDoCalc) { 
		/* calculates energy values (3 possibilities) ... */ 
		switch (csp->use_method) { 
			case DSP_CSP_METHOD_NORMAL: {dsp_csp_calc_Normal  (csp); break;} 
			case DSP_CSP_METHOD_SHIFT:	{dsp_csp_calc_Shift   (csp); break;} 
			case DSP_CSP_METHOD_SLIDING:{dsp_csp_calc_Sliding (csp); break;} 
		} 
		 
		/*	... write energy data into the memory if necessary (energys != NULL) ... */ 
		if (energys != NULL) 
			/* write energys out, if dimensions are ok */ 
			if (((energys->n_channels) == (csp->m_channels)) 
					&& ((energys->max_len) == (csp->engy_len))) 
				for (channel = 0; channel < (csp->m_channels); channel++) { 
					(energys->n_len)[channel] = (csp->engy_len); 
					for (sample = 0; sample < (csp->engy_len); sample++) 
						(energys->data)[channel][sample] = (csp->energy[channel][sample]); 
				} 
/*			else {rs_msg ("Dimension to copy energys are not the same, check this first!");} */ 
			 
		/* ... search for peaks ... */ 
		dsp_csp_peak_detect ((*delays), csp, threshold); 
	 
		/* ... set tail-Pointer shift-samples forward for next calculation */ 
		(csp->buf_tail) = ((csp->buf_tail) + ((csp->n_channels) * (csp->shift))) % (csp->buf_len); 
 
	} else { 
/*		 rs_msg ("Calculation not possible! Waiting for new Data!"); */ 
		return 0;
	} 

	/* ... return number of calculations since last call to dsp_csp_configure */ 
	return ++(csp->iter); 
} 
 
/*	dsp_csp_peak_detect (csp, threshold) */ 
/**	find peaks. 
 * 
 *	find peaks in the energy vector. The magnitude of the peaks must be 
 *	over the threshold. 
 * 
 * 	The process sequence is as follow: 
 *	- search for maximum. 
 *	- check if maximum is over threshold 
 *	- save maximum (for better localisation use parabolic interpolation). 
 *	- search minimum to search for next maximum. 
 *	 
 *	@param	delays	pointer to the output.
 *	@param	csp	pointer to the dsp_csp_t structure. 
 *	@param	threshold	threshold for finding peaks. 
 **/ 
void dsp_csp_peak_detect (dsp_csp_data_t* delays, dsp_csp_t *csp, mx_real_t threshold) 
{ 
	int channel; 
	int iEPos; 
	int iMaxPos; 
	int iPeakSize; /* number of located peaks */ 
	mx_real_t tMax, tTmp; 
	mx_real_t tLeft, tMiddle, tRight; /* variables for the parabollic interpolation */ 
 
	/* searching in every energy-channel for peaks */ 
	for (channel = 0; channel < (csp->m_channels); channel++) { 
		iPeakSize	= 0; 
		iMaxPos		= 0; 
		 
		/* searching for peaks ... */ 
		for (iEPos = 1; iEPos <= (csp->engy_len); iEPos++) { 
			/* searching for the peak (max value) */ 
			if (iEPos < (csp->engy_len)) 
				tTmp = (csp->energy[channel][iEPos]); 
			else 
				tTmp = -100; 
			 
			if (tTmp > (csp->energy[channel][iMaxPos])) { 
				iMaxPos = iEPos; 
			} else { 
				/* looking if max is over the threshold ... */ 
				if ((csp->energy[channel][iMaxPos]) >= threshold) { 
					/*	parabolic interpolation: calculation the difference from founded position, 
					 *	be carefully because of index  = (-1) and (csp->engy_len) 
					 */ 
					tLeft	= (iMaxPos == 0) ? 0.0 : (csp->energy[channel][iMaxPos -1]); 
					tMiddle	= (csp->energy[channel][iMaxPos]); 
					tRight 	= (iMaxPos == ((csp->engy_len) -1)) ? 0.0 : (csp->energy[channel][iMaxPos +1]); 
					tMax	= (tLeft - tRight ) / (2 * (tLeft + tRight - (2 * tMiddle))); 
 
					/* ... locate peak => save it (with parabolic interpolation) */ 
					delays->data[channel][iPeakSize] = iMaxPos + tMax; 
					iPeakSize++; 
				} 
 
				/* searching for minimum, so we can start for searching next peak */ 
				while ((csp->energy[channel][iEPos]) <= (csp->energy[channel][iEPos -1])) { 
					iEPos++; 
 
					/* check for end of buffer */ 
					if (iEPos >= (csp->engy_len)) 
						break; 
				} 
 
				/* check for end of buffer */ 
				if (iEPos >= (csp->engy_len)) 
					break; 

				/* set new maximum, it's just the minimum, will be overridden in the next steps */ 
				iEPos--; 
				iMaxPos = iEPos; 
			} 
		} 
 
		/* ... and set number of located peaks */ 
		delays->n_len[channel] = iPeakSize; 
	} 
} 
 
/*	dsp_csp_data2fft (csp, signal, channel1, channel2, window) */ 
/**	copy signal data into channels. 
 *   
 *  copy signal data from channel1 to channel2 into the buffer for 
 *	the fourier transformation. Multiply the signal data with 
 *	a hanning window if the value is != 0. 
 * 
 *  @param	csp	pointer to the dsp_csp_t structure. 
 *  @param	signal	start of the data in samples from tail of data. 
 *  @param	channel1	first channel. 
 *  @param	channel2	last channel. 
 *  @param	window	window multiplication index (0 = NO windowing, 1 = windowing). 
 **/ 
void dsp_csp_data2fft (dsp_csp_t* csp, int signal, int channel1, int channel2, int window) 
{ 
	int sample; 
	int channel; 
	int buf_pos; 
	 
	/* set start (position) for copying */ 
	buf_pos = ((csp->buf_tail) + (signal * (csp->n_channels))) % (csp->buf_len); 
	 
	/* extract signal [multipling with window] and convert into complex structure ... */ 
	for (sample = 0; sample < (csp->fft_len); sample++) { 
		 
		/* set real and imaginary part for each channel */	 
		for (channel = channel1; channel <= channel2; channel++) { 
			mx_im(csp->buf_fft[channel][sample])= 0.0; 

			/* copy sample for fft-tranformation*/ 
			mx_re(csp->buf_fft[channel][sample])= (csp->buf[(buf_pos + channel)]); 
#ifndef DSP_CSP_XWINDOW
			/* do the window */
			if (window) mx_re(csp->buf_fft[channel][sample]) *= csp->hwin[sample]; 
#endif
		} 
 
		/* next position for smaple */ 
		buf_pos = (buf_pos + (csp->n_channels)) % (csp->buf_len); 
	} 
	
#ifdef DSP_CSP_XWINDOW
	/* do the window after the extraction of the signal */
	if (window) {
		for (channel = channel1; channel <= channel2; channel++) {
			for (sample = 0; sample < (csp->fft_len); sample++) {
				mx_re(csp->buf_fft[channel][sample])= mx_re(csp->buf_fft[channel][sample]) * (csp->hwin[sample]); 
			}
		}
	}
#endif	

	/* ... transform every channel by fft */ 
	for (channel = channel1; channel <= channel2; channel++) { 
		dsp_xfft((csp->buf_fft[channel]), (csp->fft_len), 0); 
	} 
} 
 
/*	dsp_csp_check4filter (csp, bandnumber) */ 
/**	test for using band filter. 
 *   
 *  tests if to activate the filter or just use normal calculation. 
 *	The boundaries are 'min_filter' and 'max_filter' of the dsp_csp_t 
 *	structure. 
 * 
 *  @param	csp	pointer to the dsp_csp_t structure. 
 *  @param	bandnumber	number of the band after fourier transformation. 
 *   
 *  @return	1 for activation the filter, 0 else. 
 **/ 
int dsp_csp_check4filter (dsp_csp_t* csp, int bandnumber) 
{ 
	/* band pass filter (between max and min) */ 
	float frequency; 
	int retValue = 0; 
 
	/* calculates frequency (symmetrical) */ 
	if (bandnumber <= ((csp->fft_len) /2)) 
		frequency = bandnumber * ((float) (csp->samplingrate) / (csp->fft_len));		 
	else 
		frequency = ((csp->fft_len) - bandnumber -1) * ((float) (csp->samplingrate) / (csp->fft_len)); 
 
	/* test for apply band pass filter */ 
	if ((frequency < (csp->min_filter)) || (frequency  > (csp->max_filter))) 
		retValue = 1; 
	 
	return (retValue); 
} 
 
/* #define TEST */
#ifdef TEST 
/*	main (argc, argv) */ 
/**	a small main programm for testing. 
 *   
 *  This programm test the functionality of the functions above. 
 * 
 *  @param	argc	number of arguments. 
 *  @param	argv	arguments from the console call. first is filename and
 *		second is calculation method.
 *   
 *  @return	\ref DSP_SUCCESS at the end. 
 *   
 **/ 
int main (int argc, char **argv) 
{ 
	/* values for calculation */ 
	int n_channels	= 2; 
	int fft_length	= 2048; 
	int n_samples	= 2 * fft_length +1; 
	int engy_len	= 29; 
 
	int shift		= fft_length; 
	int method		= atoi(argv[2]); 
	int sr			= 16;  /* in kHz */ 
	float fmin		= 0.1; /* in kHz */ 
	float fmax		= 4.5; /* in kHz */ 
 
	dsp_sample_t sig_buf[n_channels * fft_length]; 
	dsp_csp_t* csp = NULL; 
	dsp_csp_data_t* csp_peak = NULL; 
	dsp_csp_data_t* csp_energy = NULL; 
	 
	FILE* source_fp; 
 
	int count; 
	int channel, sample; 
 
	/* time stamp */
	struct timeval t_start, t_stop; /* 	gettimeofday */

	/* open File ... */ 
	source_fp = fopen (argv[1], "r"); 
	if (! source_fp) 
		rs_error("can't open '%s'!", argv[1]); 
 
	/* ... allocate Memory ... */ 
	csp = dsp_csp_create(n_channels, fft_length, n_samples, engy_len); 
	dsp_csp_configure (csp, shift, method, sr, fmin, fmax); 
	 
	/* ... do the calculation ... */ 
	while (1) { 
		count = fread(sig_buf, sizeof(dsp_sample_t), (n_channels * fft_length), source_fp); 
		dsp_csp_push(csp, sig_buf, fft_length); 
		if (count ==  (n_channels * fft_length)) { 
			while (count > 0) { 
				gettimeofday (&t_start, NULL);
				count = dsp_csp_calc (&csp_peak, NULL, csp, 0.1); 
				gettimeofday (&t_stop, NULL);
				 
				printf("\nPrinten der Energywerte, %d", count);
				
				if (count != 0) { 
					/* time stamp output */
					printf("\n Zeit zur Berechnung: %4.2f \n", (float) rs_timeval_diff (&t_start, &t_stop));
					
					/* ... write Energys out, write peaks out, write delays out! */ 
					printf("Energys ...:\n"); 
					for (channel = 0; channel < (csp->m_channels); channel++) { 
						for (sample = 0; sample < (csp->engy_len); sample++) { 
							printf("%1.4f    ", csp->energy[channel][sample]); 
						} 
						printf("\n... Peaks/ Delays:\n"); 
						for (sample = 0; sample < (csp_peak->n_len[channel]); sample++) { 
							printf(" %1.4f ", csp_peak->data[channel][sample]); 
						} 
						printf("\n"); 
					} 
				}
			} 
		} else { 
			break; 
		} 
	} 
	 
	/* ... destroy, and close file */ 
	fclose(source_fp); 
	dsp_csp_destroy(csp); 
	dsp_csp_data_destroy(csp_peak); 
	dsp_csp_data_destroy(csp_energy); 
	 
	/* ENDE! */ 
	return (DSP_SUCCESS); 
} 
#endif /* TEST */ 
