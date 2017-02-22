/**
* Datei:	dwt.c
* Autor:	Thomas Ploetz, Thu Jul 15 09:01:49 2004
* Time-stamp:	<05/12/13 11:54:22 tploetz>
*
* Beschreibung:	Berechnung der diskreten Wavelettransformation
**/
#include <math.h>

#include "ev_real.h"

#include "ev_memory.h"
#include "ev_messages.h"


#include "ev_dsp.h"
#include "ev_dwt.h"

#define DSPDWT_EPS	1e-20

/* Prototypes */
static mx_real_t *_filter(mx_real_t *signal, 
			  mx_real_t *filter,
			  int len_signal,
			  int len_filter);

/**
 * dsp_dwt(u, l_u, v, l_v, signal, len_signal, A, len_A, D, len_D, level)
 *	fast discrete wavelet analysis of given 'signal' up to 'level'
 *	the decomposition is performed using the mallat-algorithm and the given
 *	filters 'A' (lowpass) of length 'len_A' and 'D' (highpass) of length 
 *	'len_D'
 *	output: the approximation coefficient(s) of the level given (u) and 
 *	all wavelet coefficients up to the level given (v)
 *	l_u contains the number of approximation coefficients
 *	l_v contains the length of the wavelet coefficient vectors
 *	(concatenated in v)
 *	CAUTION: only valid for signal lengths beeing a power of 2!
 **/
int dsp_dwt(mx_real_t **u,
	    int *l_u,
	    mx_real_t ***v,
	    int **l_v,
	    mx_real_t *_signal,
	    int _len_signal,
	    mx_real_t *A,
	    int len_A,
	    mx_real_t *D,
	    int len_D,
	    int level) {
  int i,j,l;

  mx_real_t *_u = NULL;

  mx_real_t *_u_i_2 = NULL;
  int *_l_v = NULL;

  mx_real_t **_v = NULL;
  mx_real_t *u_i = NULL;
  mx_real_t *v_i = NULL;
  mx_real_t *_v_i_2 = NULL;

  int len_signal;
  mx_real_t *signal = NULL;
  int signal_extended = 0;

  if (!_signal || !A || !D || _len_signal <= 0 || len_A <= 0 ||
      len_D <= 0 || level < 1)
    return(-1);

  len_signal = pow(2, ceil(log(_len_signal)/log(2)));
  if (len_signal > _len_signal) {
    rs_warning("extending signal to power of 2 for DWT (%d -> %d)!",
	       _len_signal, len_signal);
    signal = rs_calloc(len_signal, sizeof(mx_real_t),
		       "extended signal");
    for (i = 0; i < _len_signal; i++)
      signal[i] = _signal[i];
    signal_extended = 1;
  } else
    signal = _signal;
  
  _v = rs_malloc(level * sizeof(mx_real_t*),
		 "_v-array");
  _u_i_2 = rs_malloc(len_signal * sizeof(mx_real_t),
		     "u_i_2");
  _v_i_2 = rs_malloc(len_signal * sizeof(mx_real_t),
		     "v_i_2");
  _l_v = rs_malloc(level * sizeof(int), 
		   "# of coefficients vector");
  
  /* wavelet crime: start with the original signal as first approximation 
     --> u(0) */
  for (i = 0; i < len_signal; i++)
    _u_i_2[i] = signal[i];
  _u = _u_i_2;

  *l_u = len_signal;

  /* subband coding for all levels */
  for (l = 0; l < level; l++) {

    /* conv. current approx. with lowpass A */
    if (!(u_i = _filter(_u, A, *l_u, len_A)))
      return(-1);
    
    /* conv. with highpass D */
    if (!(v_i = _filter(_u, D, *l_u, len_D)))
      return(-1);

    /* downsampling by 2 (both) */
    for (i = 1, j = 0 ; i < *l_u; i+=2, j++) {
      _u_i_2[j] = u_i[i];
      _v_i_2[j] = v_i[i];
    }
    
    *l_u /= 2;

    /*  save the wavelet coefficients of current level */
    _v[l] = rs_malloc(*l_u * sizeof(mx_real_t),
		      "_v");    
    for (i = 0; i < *l_u; i++)
      _v[l][i] = _v_i_2[i];

    /* save the number of wavelet coefficients of the current level */
    _l_v[l] = *l_u;

    /* level based cleanup */
    if (v_i)
      rs_free(v_i);
    if (u_i)
      rs_free(u_i);
    u_i = NULL;
    v_i = NULL;

    /* the approximation coefficients of the present level are
       the input of the next level */
    _u = _u_i_2;
  }

  /* cleanup */
  if (_v_i_2)
    rs_free(_v_i_2);
  
  *u = _u;
  *v = _v;
  *l_v = _l_v;

  if (signal_extended)
    rs_free(signal);

  return(l);
}

/**
 * dsp_idwt(u, l_u, v, l_v, A, len_A, D, len_D, level)
 *	fast discrete wavelet synthesis using given filterpair 'A' (length 
 *	'len_A') -- lowpass -- and 'D' (length 'len_D' -- highpass --
 *	the composition is performed using the mallat-algorithm at the 
 *	given level 'level'
 **/
mx_real_t *dsp_idwt(mx_real_t *u,
		    int l_u,
		    mx_real_t **v,
		    int *l_v,
		    mx_real_t *A,
		    int len_A,
		    mx_real_t *D,
		    int len_D,
		    int level) {
  int i,j,l;

  int l_i;
  int r_sig_l = 0;
  mx_real_t *_u = NULL;
  mx_real_t *_v = NULL;
  mx_real_t *_u_i = NULL;
  mx_real_t *_v_i = NULL;
  mx_real_t *_u_o = NULL;
  mx_real_t *_v_o = NULL;
  mx_real_t *r_signal = NULL;
  int s_len, max_level ;

  if (!A || !D || len_A <= 0 ||
      len_D <= 0 || level < 1)
    return(NULL);

  s_len = len_A + len_D;
  max_level = log(s_len / (log(s_len)/log(2))) / log(2);
  if (level > max_level) {
    rs_warning("inverse DWT impossible at level %d for given filter pair"
	       " (len(A): %d / len(D): %d), decreasing level to %d.",
	       level, len_A, len_D, max_level);
    level = max_level;
  }

  r_sig_l = pow(l_u,2);

  r_signal = rs_calloc(r_sig_l, sizeof(mx_real_t),
		       "r_signal");
  _u = rs_calloc(r_sig_l, sizeof(mx_real_t),
		 "_u-array");
  _v = rs_calloc(r_sig_l, sizeof(mx_real_t),
		 "_v-array");
  _u_o = rs_calloc(r_sig_l, sizeof(mx_real_t),
		   "_u_o-array");
  _v_o = rs_calloc(r_sig_l, sizeof(mx_real_t),
		   "_v_o-array");
  _u_i = rs_calloc(l_u, sizeof(mx_real_t),
		   "_u_i-array");
  _v_i = rs_calloc(l_v[level-1], sizeof(mx_real_t),
		   "_v_i-array");
  
  for (i = 0; i < l_u; i++)
    _u_i[i] = u[i];
  for (i = 0; i < l_v[level - 1]; i++)
    _v_i[i] = v[level - 1][i];
  l_i = l_u;

  /* "inverse subband coding" */
  for (l = level-1; l >= 0; l--) {
    
    /* upsampling by 2 (both) */
    for (i = 0, j = 0; i < l_i; i++, j+= 2) {
      _u[j] = _u_i[i];
      _v[j] = _v_i[i];
    }
    l_i *= 2;

    /* cleanup for current level */
    rs_free(_u_i);
    rs_free(_v_i);
    _u_i = NULL;
    _v_i = NULL;

    /* conv. current approx. with lowpass A */
    if (!(_u_i = _filter(_u, A, l_i, len_A)))
      return(NULL);
    
    /* conv. with highpass D */
    if (!(_v_i = _filter(_v, D, l_i, len_D)))
      return(NULL);

    /* compensate filterig offsets */
    j = 0;
    for (i = (l_i - (len_A - 2)); i < l_i; i++, j++)
      _u_o[j] = _u_i[i];
    for (i = 0; i < l_i - (len_A - 2); i++, j++)
      _u_o[j] = _u_i[i];
    
    j = 0;
    for (i = (l_i - (len_D - 2)); i < l_i; i++, j++)
      _v_o[j] = _v_i[i];
    for (i = 0; i < l_i - (len_D - 2); i++, j++)
      _v_o[j] = _v_i[i];
    
    /* reunite approximation and details */
    for (i = 0; i < l_i; i++) {
      r_signal[i] = ((_u_o[i] < DSPDWT_EPS) ? 0 : _u_o[i])
			+ 
	      ((_v_o[i] < DSPDWT_EPS) ? 0 : _v_o[i]);
    }
    /* reconstruction is the input for the next level */
    for (i = 0; i < l_i; i++) {
      _u_i[i] = r_signal[i];
      if (l > 0)
	_v_i[i] = v[l-1][i];
    }
  }
  
  /* cleanup */
  rs_free(_u_i);
  rs_free(_v_i);
  _u_i = NULL;
  _v_i = NULL;
  rs_free(_u);
  rs_free(_v);
  _u = NULL;
  _v = NULL;
  rs_free(_u_o);
  rs_free(_v_o);
  _u_o = NULL;
  _v_o = NULL;

  return(r_signal);
}

/**
 * _filter(signal, filter, len_signal, len_filter)
 *	filters the given 'signal' of length 'len_signal' using
 *	the given filter of length 'len_filter'
 *	returns the fiiltered signal in original length!
 **/
static mx_real_t *_filter(mx_real_t *signal, 
			  mx_real_t *filter,
			  int len_signal,
			  int len_filter) {
  mx_real_t *y = NULL;
  mx_real_t *yR = NULL;
  mx_real_t *t_signal = NULL;

  mx_real_t x0 = 0.0;
  int i,j;

  if (!signal || !filter || len_signal <= 0 || len_filter <= 0)
    return(NULL);

  t_signal = rs_malloc((len_signal + len_filter) * sizeof(mx_real_t),
		       "t_signal");

  y = rs_malloc((len_signal + len_filter) * sizeof(mx_real_t),
		"y-vector");
  yR = rs_malloc(len_signal * sizeof(mx_real_t),
		"yR-vector");

  for (i = 0; i < len_signal; i++)
    t_signal[i] = signal[i];

  /* continue signal periodically */
  j = -1;
  for (i = len_signal; i < len_signal + len_filter; i++) {
    j = (j >= len_signal) ? 0 : j + 1;
    t_signal[i] = signal[j];
  }

  /* filter signal */
  for (i = 0; i < len_signal + len_filter - 1; i++) {
    x0 = 0;

    for (j = 0; j < len_filter; j++)
      if (i - j >= 0)
	x0 = x0 + t_signal[i - j] * filter[j];
    
    y[i] = x0;
  }

  /* get the effective filter response for given signal */
  j = 0;
  for (i = len_filter - 2; 
       i < len_signal + (len_filter - 1) - 1; 
       i++, j++)
    yR[j] = y[i];

  rs_free(t_signal);
  rs_free(y);

  return(yR);
}
