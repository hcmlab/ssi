// AudioLpc.cpp
// author: Ionut Damian <damian@hcm-lab.de>
// created: 2012/10/16 
// Copyright (C) 2007-12 University of Augsburg, Ionut Damian
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

// based on code of openSMILE 1.0.1
// http://opensmile.sourceforge.net/

/*F******************************************************************************
 *
 * openSMILE - open Speech and Music Interpretation by Large-space Extraction
 *       the open-source Munich Audio Feature Extraction Toolkit
 * Copyright (C) 2008-2009  Florian Eyben, Martin Woellmer, Bjoern Schuller
 *
 *
 * Institute for Human-Machine Communication
 * Technische Universitaet Muenchen (TUM)
 * D-80333 Munich, Germany
 *
 *
 * If you use openSMILE or any code from openSMILE in your research work,
 * you are kindly asked to acknowledge the use of openSMILE in your publications.
 * See the file CITING.txt for details.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ******************************************************************************E*/

#define LPC_METHOD_ACF   0
//#define LPC_METHOD_ACF2   1
#define LPC_METHOD_BURG   5

#define SIGN_CHANGE(a,b) (((a)*(b))<0.0)
#define LPC_SCALING  1.f
#define FREQ_SCALE 1.0
//#define ANGLE2X(a) (approx_cos(a))
#define X2ANGLE(x) (acos(x))

#define LSP_DELTA1 .2
#define LSP_DELTA2 .05

#include "AudioLpc.h"
//#include "AudioTools.h"
#include "fftXg.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


namespace ssi {

AudioLpc::AudioLpc (const ssi_char_t *file)
	: _file (0),
	  p(0),
	  saveRefCoeff(false), acf(NULL),
	  lpCoeff(NULL), lastLpCoeff(NULL), refCoeff(NULL),
	  burgB1(NULL), burgB2(NULL), burgAA(NULL),
	  lSpec(NULL), _ip(NULL), _w(NULL), latB(NULL) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

AudioLpc::~AudioLpc () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void AudioLpc::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {		

	method = 0;
	if (strcmp(_options.method,"acf") == 0) { method = LPC_METHOD_ACF; }
	//else if (strcmp(_options.method,"acf2") == 0) { method = LPC_METHOD_ACF2; }
	else if (strcmp(_options.method,"burg") == 0) { method = LPC_METHOD_BURG; }

	p = _options.p;
	saveLPCoeff = _options.saveLPCoeff; 
	lpGain = _options.lpGain; 
	saveRefCoeff = _options.saveRefCoeff; 
	residual = _options.residual; 
	forwardRes = _options.forwardFilter;  
	lpSpectrum = _options.lpSpectrum; 
	lpSpecDeltaF = _options.lpSpecDeltaF; 
	lpSpecBins = _options.lpSpecBins; 

	latB = (ssi_real_t*)calloc(1,sizeof(ssi_real_t)*p);

	if(saveLPCoeff) {
		lpCoeff = (ssi_real_t*)calloc(1,sizeof(ssi_real_t)*(p+1));
		//lastLpCoeff = (ssi_real_t*)calloc(1,sizeof(ssi_real_t)*(p));
	}

	if(saveRefCoeff)
		refCoeff = (ssi_real_t*)calloc(1,sizeof(ssi_real_t)*p);
}

void AudioLpc::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	long Nsrc = stream_in.num;
	ssi_real_t *src = ssi_pcast (ssi_real_t, stream_in.ptr);

	long Ndst = stream_out.dim;
	ssi_real_t *dst = ssi_pcast (ssi_real_t, stream_out.ptr);
	
	// following code taken from openSMILE 1.0.1, lpc.cpp
	// http://opensmile.sourceforge.net/

	long myN = Ndst;
	if (residual) myN = Ndst - Nsrc;

	long expectedN = 0;
	if (saveRefCoeff) expectedN += p;
	if (saveLPCoeff) expectedN += p;
	if (lpGain) expectedN += 1;
	if (lpSpectrum) expectedN += lpSpecBins;
	if (_options.lsp) expectedN += p;
	if (myN != expectedN) {
		ssi_wrn("Ndst(-Nsrc) (=%i) <> expected value (%i) ! something is wrong.. the program might crash!",myN,expectedN);
	}

	if (p<0) {
		ssi_wrn("p<0! something is wrong...");
		p=0;
	}

	long i;
	ssi_real_t *dst0 = dst;
	ssi_real_t gain = 0.0;

	//for (i=0; i<Nsrc; i++) {
	//lpc[i] = src[lpcCoeffIdx+i];
	//}printf("----\n");

	if (saveRefCoeff) {
		//calcLpc(const ssi_real_t *x, long Nsrc, ssi_real_t * lpc, long nCoeff, ssi_real_t *refl) 
		gain = calcLpc(src, Nsrc, lpCoeff, p, refCoeff);
		if (saveLPCoeff) {
			for (i=0; i<p; i++) {
				dst[i] = lpCoeff[i];
				dst[i+p] = refCoeff[i];
			}
			dst += 2*p;
		} else {
			for (i=0; i<p; i++) {
				dst[i] = refCoeff[i];
			}
			dst += p;
		}    
	} else {
		if (saveLPCoeff || residual || lpSpectrum || lpGain) {
			gain = calcLpc(src, Nsrc, lpCoeff, p, refCoeff);
			//gain = calcLpc(acf, p, lpCoeff, NULL);
		}
		if (saveLPCoeff) {
			for (i=0; i<p; i++) {
				dst[i] = lpCoeff[i];
			}
			dst += p;
		}
	}

	if(_options.lsp)
	{
		if (saveLPCoeff && p > 0) 
		{
			long nLpc = p;

			/* LPC to LSPs (x-domain) transform */
			int roots;
			roots = lpc_to_lsp (lpCoeff, nLpc, dst, 10, (ssi_real_t)LSP_DELTA1);
			if (roots!=nLpc) {
				roots = lpc_to_lsp (lpCoeff, nLpc, dst, 10, (ssi_real_t)LSP_DELTA2);  // nLpc was Nsrc
				if (roots!=nLpc) {
					int i;
					for (i=roots;i<nLpc;i++) {
						dst[i]=0.0;
					}
				}
			}
		}
	}

	if (lpGain) {
		//printf("gain: %f\n",gain);
		dst[0] = gain;
		dst++;
	}

	if (lpSpectrum) {
		/*
			we compute the lp spectrum by zero padding and fft of the lp coefficients
			the number of 0's we pad with determines our frequency resolution
		*/
			// config parameters: lpSpecDeltaF & lpSpecBins (N or -1 for = nLpc)
		//double fftN = (1.0/T) / lpSpecDeltaF;
		if (lSpec == NULL) lSpec = (FLOAT_TYPE_FFT*)malloc(sizeof(FLOAT_TYPE_FFT) * lpSpecBins * 2);

		// create padded vector
		for (i=0; i<lpSpecBins*2; i++) {
			lSpec[i] = 0.0;
		}
		lSpec[0] = 1.0;
		for (i=1; i<=p; i++) {
			lSpec[i] = (FLOAT_TYPE_FFT)lpCoeff[i-1];
		}

		// transform
		if (_ip==NULL) _ip = (int *)calloc(1,sizeof(int)*(lpSpecBins*2+2));
		if (_w==NULL) _w = (FLOAT_TYPE_FFT *)calloc(1,sizeof(FLOAT_TYPE_FFT)*(lpSpecBins*2*5)/4+2);
		//perform FFT
		rdft(lpSpecBins*2, 1, lSpec, _ip, _w);

		// compute magnitude
		int n=0;
		*(dst++) = (ssi_real_t)fabs( lSpec[0] ); /* DC part */
		for (i=2; i<(lpSpecBins-1)*2; i += 2) {
			// save in output vector
			*(dst++) = (ssi_real_t)sqrt( lSpec[i]*lSpec[i] + lSpec[i+1]*lSpec[i+1] );
		}
		*(dst++) = (ssi_real_t)fabs( lSpec[1] ); /* Nyquist freq. */
	}

	if (residual) {
		if (forwardRes) { // apply forward LPC filter (recursive)
			for (i=0; i<Nsrc; i++) {
			dst[i] = smileDsp_invLattice(refCoeff, latB, p, src[i]);
			}
		} else { // apply inverse LPC filter (this yields the actual residual)
			// alternative: lattice filter with reflection coefficients:
			for (i=0; i<Nsrc; i++) {
			dst[i] = smileDsp_lattice(refCoeff, latB, p, src[i], NULL);
			}
		}

		lastGain = gain;
	}
}

// return value: gain
ssi_real_t AudioLpc::calcLpc(const ssi_real_t *x, long Nsrc, ssi_real_t * lpc, long nCoeff, ssi_real_t *refl)
{
	ssi_real_t gain = 0.0;
	if (method == LPC_METHOD_ACF) {
		if (acf == NULL) acf = (ssi_real_t *)malloc(sizeof(ssi_real_t)*(nCoeff+1));
		smileDsp_autoCorr(x, Nsrc, acf, nCoeff+1);
		smileDsp_calcLpcAcf(acf, lpc, nCoeff, &gain, refl);
	} else if (method == LPC_METHOD_BURG) {
		smileDsp_calcLpcBurg(x, Nsrc, lpc, nCoeff, &gain, &burgB1, &burgB2, &burgAA);
		if (refl != NULL) ssi_wrn("computation of reflection coefficients with Burg's LPC method is not yet implemented!");
	}
	return gain;
}

/*---------------------------------------------------------------------------*\

	FUNCTION....: lpc_to_lsp()

	AUTHOR......: David Rowe
	DATE CREATED: 24/2/93

	This function converts LPC coefficients to LSP
	coefficients.

\*---------------------------------------------------------------------------*/

int AudioLpc::lpc_to_lsp (const ssi_real_t *a, int lpcrdr, ssi_real_t *freq, int nb, ssi_real_t delta)
/*  float *a                    lpc coefficients                        */
/*  int lpcrdr                  order of LPC coefficients (10)          */
/*  float *freq                 LSP frequencies in the x domain         */
/*  int nb                      number of sub-intervals (4)             */
/*  float delta                 grid spacing interval (0.02)            */


{
	ssi_real_t temp_xr,xl,xr,xm=0;
	ssi_real_t psuml,psumr,psumm,temp_psumr/*,temp_qsumr*/;
	int i,j,m,flag,k;
	ssi_real_t *Q = NULL;                   /* ptrs for memory allocation           */
	ssi_real_t *P = NULL;
	ssi_real_t *Q16 = NULL;         /* ptrs for memory allocation           */
	ssi_real_t *P16 = NULL;
	ssi_real_t *px;                   /* ptrs of respective P'(z) & Q'(z)     */
	ssi_real_t *qx;
	ssi_real_t *p;
	ssi_real_t *q;
	ssi_real_t *pt;                   /* ptr used for cheb_poly_eval()
								whether P' or Q'                        */
	int roots=0;                /* DR 8/2/94: number of roots found     */
	flag = 1;                   /*  program is searching for a root when,
								1 else has found one                    */
	m = lpcrdr/2;               /* order of P'(z) & Q'(z) polynomials   */

	/* Allocate memory space for polynomials */
	Q = (ssi_real_t*) calloc(1,sizeof(ssi_real_t)*(m+1));
	P = (ssi_real_t*) calloc(1,sizeof(ssi_real_t)*(m+1));

	/* determine P'(z)'s and Q'(z)'s coefficients where
	  P'(z) = P(z)/(1 + z^(-1)) and Q'(z) = Q(z)/(1-z^(-1)) */

	px = P;                      /* initialise ptrs                     */
	qx = Q;
	p = px;
	q = qx;

	

	*px++ = LPC_SCALING;
	*qx++ = LPC_SCALING;
	for(i=0;i<m;i++) {
	   *px++ = (a[i]+a[lpcrdr-1-i]) - *p++;
	   *qx++ = (a[i]-a[lpcrdr-1-i]) + *q++;
	}
	px = P;
	qx = Q;
	for(i=0;i<m;i++) {
	   *px = 2**px;
	   *qx = 2**qx;
	   px++;
	   qx++;
	}

	px = P;                     /* re-initialise ptrs                   */
	qx = Q;

	/* now that we have computed P and Q convert to 16 bits to
	   speed up cheb_poly_eval */

	P16 = P;
	Q16 = Q;

	/*
	for (i=0;i<m+1;i++) {
	   P16[i] = P[i];
	   Q16[i] = Q[i];
	}
*/

	/* Search for a zero in P'(z) polynomial first and then alternate to Q'(z).
	Keep alternating between the two polynomials as each zero is found  */

	xr = 0;                     /* initialise xr to zero                */
	xl = FREQ_SCALE;                    /* start at point xl = 1                */

	for(j=0;j<lpcrdr;j++){
		if(j&1)                 /* determines whether P' or Q' is eval. */
			pt = Q16;
		else
			pt = P16;

		psuml = cheb_poly_eva(pt,xl,m);   /* evals poly. at xl    */
		flag = 1;
		while(flag && (xr >= -FREQ_SCALE)){
		   ssi_real_t dd;
		   /* Modified by JMV to provide smaller steps around x=+-1 */

		   dd=delta*((ssi_real_t)1.0-(ssi_real_t)0.9*xl*xl);
		   if (fabs(psuml)<.2)
			  dd *= (ssi_real_t)0.5;

		   xr = xl - dd;                          /* interval spacing     */
			psumr = cheb_poly_eva(pt,xr,m);/* poly(xl-delta_x)    */
			temp_psumr = psumr;
			temp_xr = xr;

   /* if no sign change increment xr and re-evaluate poly(xr). Repeat til
	sign change.
	if a sign change has occurred the interval is bisected and then
	checked again for a sign change which determines in which
	interval the zero lies in.
	If there is no sign change between poly(xm) and poly(xl) set interval
	between xm and xr else set interval between xl and xr and repeat till
	root is located within the specified limits                         */

			if(SIGN_CHANGE(psumr,psuml))
			{
				roots++;

				psumm=psuml;
				for(k=0;k<=nb;k++){
					xm = (ssi_real_t)0.5*(xl+xr);            /* bisect the interval  */
					psumm=cheb_poly_eva(pt,xm,m);
					/*if(psumm*psuml>0.)*/
					if(!SIGN_CHANGE(psumm,psuml))
					{
						psuml=psumm;
						xl=xm;
					} else {
						psumr=psumm;
						xr=xm;
					}
				}

			   /* once zero is found, reset initial interval to xr      */
			   if (xm>1.0) { xm = 1.0; } /* <- fixed a possible NAN issue here...? */
			   else if (xm<-1.0) { xm = -1.0; }
//               else if (!finite(xm)) { xm = 1.0; }
			   freq[j] = X2ANGLE(xm);
			   xl = xm;
			   flag = 0;                /* reset flag for next search   */
			}
			else{
				psuml=temp_psumr;
				xl=temp_xr;
			}
		}
	}

	if (P!=NULL) free(P);
	if (Q!=NULL) free(Q);

	return(roots);
}

/*---------------------------------------------------------------------------*\

   FUNCTION....: cheb_poly_eva()

   AUTHOR......: David Rowe
   DATE CREATED: 24/2/93

   This function evaluates a series of Chebyshev polynomials

\*---------------------------------------------------------------------------*/

ssi_real_t AudioLpc::cheb_poly_eva(ssi_real_t *coef, ssi_real_t x, int m)
{
	int k;
	ssi_real_t b0, b1, tmp;

	/* Initial conditions */
	b0=0; /* b_(m+1) */
	b1=0; /* b_(m+2) */

	x*=2;

	/* Calculate the b_(k) */
	for(k=m;k>0;k--)
	{
		tmp=b0;                           /* tmp holds the previous value of b0 */
		b0=x*b0-b1+coef[m-k];    /* b0 holds its new value based on b0 and b1 */
		b1=tmp;                           /* b1 holds the previous value of b0 */
	}

	return(-b1+(ssi_real_t)0.5*x*b0+coef[m]);
}

/* helper functions */

/*
Implementation of an inverse lattice filter
This function processed a single value per call
k: coefficients
*b : temporary work area, initialise with 0 at the beginning! (size: sizeof(FLOAT_DMEM)*M )
M: order (number of coefficients)
out: e(n) input sample
return value: f(M) = filter "input" s(n)
*/
ssi_real_t AudioLpc::smileDsp_invLattice(ssi_real_t *k, ssi_real_t *b, int M, ssi_real_t out)
{
	int i;
	ssi_real_t fM;
	ssi_real_t last = b[M - 1];
	/* initialisation */
	fM = out;
	for (i = M - 1; i>0; i--) {
		fM -= k[i] * b[i - 1];
		b[i] = k[i] * fM + b[i - 1];
	}
	b[M - 1] = last;
	fM = fM - k[0] * b[M - 1];
	b[0] = k[0] * fM + b[M - 1];
	b[M - 1] = fM;
	/* return resulting f0 */
	return fM;
}


/*
Implementation of a lattice filter
This function processed a single value per call
k: coefficients
*b : temporary work area, initialise with 0 at the beginning! (size: sizeof(FLOAT_DMEM)*M )
M: order (number of coefficients)
in: s(n) input sample
*bM : optional b(M) result
return value: f(M) = filter output
*/
ssi_real_t AudioLpc::smileDsp_lattice(ssi_real_t *k, ssi_real_t *b, int M, ssi_real_t in, ssi_real_t *bM)
{
	int i;
	ssi_real_t f0, f1, b0, b1;
	/* initialisation */
	b0 = f0 = in;
	for (i = 0; i<M; i++) {
		f1 = f0 + k[i] * b[i];
		b1 = k[i] * f0 + b[i];
		b[i] = b0; // store b[n-1]
		// save old coefficients for next iteration:
		f0 = f1;
		b0 = b1;
	}
	/* return resulting b (optional) */
	if (bM != NULL) *bM = b1;
	/* return resulting f */
	return f1;
}

/* autocorrelation in the time domain (used for LPC autocorrelation method) */
void AudioLpc::smileDsp_autoCorr(const ssi_real_t *x, const int n, ssi_real_t *acf, int lag)
{
	int i;
	while (lag) {
		acf[--lag] = 0.0;
		for (i = lag; i < n; i++) {
			acf[lag] += x[i] * x[i - lag];
		}
	}
}

/* LPC analysis via acf (=implementation of Durbin recursion)*/
int AudioLpc::smileDsp_calcLpcAcf(ssi_real_t * r, ssi_real_t *a, int p, ssi_real_t *gain, ssi_real_t *k)
{
	int i, m;
	ssi_real_t e;
	int errF = 1;
	ssi_real_t k_m;
	ssi_real_t *al;

	if (a == NULL) return 0;
	if (r == NULL) return 0;

	if ((r[0] == 0.0) || (r[0] == -0.0)) {
		for (i = 0; i < p; i++) a[i] = 0.0;
		return 0;
	}

	al = (ssi_real_t*)malloc(sizeof(ssi_real_t)*(p));

	/* Initialisation, Eq. 158 */
	e = r[0];

	/* The iterations: m=1..p (here), Eq. 159 */
	for (m = 1; m <= p; m++) {
		/* Eq. 159 (a) */
		ssi_real_t sum = (ssi_real_t)1.0 * r[m];
		for (i = 1; i<m; i++) {
			sum += a[i - 1] * r[m - i];
		}
		k_m = ((ssi_real_t)-1.0 / e) * sum;

		// save reflection coefficient
		if (k != NULL) k[m - 1] = k_m;

		/* Eq. 159 (b) */
		a[m - 1] = k_m;

		for (i = 1; i <= m / 2; i++) {
			ssi_real_t x = a[i - 1];
			a[i - 1] += k_m * a[m - i - 1];
			if ((i < (m / 2)) || ((m & 1) == 1)) a[m - i - 1] += k_m * x;
		}

		// update the error:
		e *= ((ssi_real_t)1.0 - k_m*k_m);
		if (e == 0.0) {
			for (i = m; i <= p; i++) {
				a[i] = 0.0;
				if (k != NULL) k[m] = 0.0;
			}
			break;
		}
	}

	free(al);

	if (gain != NULL) *gain = e;
	return 1;
}

int AudioLpc::smileDsp_calcLpcBurg(const ssi_real_t *x, long n, ssi_real_t *a, int m, ssi_real_t *gain, ssi_real_t **burgB1, ssi_real_t **burgB2, ssi_real_t **burgAA)
{
	long i = 1, j; int status = 0;
	ssi_real_t p = 0.0;
	ssi_real_t *aa = NULL;
	ssi_real_t xms = 0.0;
	ssi_real_t *b1 = NULL, *b2 = NULL;

	if (x == NULL) return 0;
	if (a == NULL) return 0;
	if ((n<m) || (m <= 0)) return 0;

	if (burgB1 != NULL) b1 = *burgB1;
	if (b1 == NULL) b1 = (ssi_real_t*)calloc(1, sizeof(ssi_real_t)*n);

	if (burgB2 != NULL) b2 = *burgB2;
	if (b2 == NULL) b2 = (ssi_real_t*)calloc(1, sizeof(ssi_real_t)*n);

	if (burgAA != NULL) aa = *burgAA;
	if (aa == NULL) aa = (ssi_real_t*)calloc(1, sizeof(ssi_real_t)*m);

	/* (3) */
	for (j = 0; j < n; j++) {  // for (j = 1; j <= n; j++) {
		p += x[j] * x[j];
	}

	xms = p / n;
	if (xms <= 0) goto end;

	/* (9) */
	b1[0] = x[0];       // b1[1] = x[1];
	b2[n - 2] = x[n - 1]; // b2[n - 1] = x[n];
	for (j = 1; j < n - 1; j++) { // for (j = 2; j <= n - 1; j++) {
		b1[j] = b2[j - 1] = x[j];
	}

	for (i = 0; i < m; i++) { // for (i = 1; i <= m; i++) {

		/* (7) */
		ssi_real_t num = 0.0, denum = 0.0;
		for (j = 0; j < n - i - 1; j++) {  // for (j = 1; j <= n - i; j++) {  
			num += b1[j] * b2[j];
			denum += b1[j] * b1[j] + b2[j] * b2[j];
		}

		if (denum <= 0) goto end;

		a[i] = (ssi_real_t)2.0 * num / denum;

		/* (10) */
		xms *= (ssi_real_t)1.0 - a[i] * a[i];

		/* (5) */
		for (j = 0; j < i; j++) {  // for (j = 1; j <= i - 1; j++) {    
			a[j] = aa[j] - a[i] * aa[i - j - 1]; //  aa[i-j]
		}

		if (i < m - 1) { // if (i < m) {

			/* (8) */
			/* Watch out: i -> i+1 */
			for (j = 0; j <= i; j++) { // for (j = 1; j <= i; j++) {   
				aa[j] = a[j];
			}
			for (j = 0; j < n - i - 2; j++) { // for (j = 1; j <= n - i - 1; j++)
				b1[j] -= aa[i] * b2[j];
				b2[j] = b2[j + 1] - aa[i] * b1[j + 1];
			}
		}
	}

	status = 1;

end:
	if (burgB1 != NULL) *burgB1 = b1;
	else if (b1 != NULL) free(b1);
	if (burgB2 != NULL) *burgB2 = b2;
	else if (b2 != NULL) free(b2);
	if (burgAA != NULL) *burgAA = aa;
	else if (aa != NULL) free(aa);

	for (j = 0; j < i; j++) a[j] = -a[j]; // invert coefficients for compatibility with ACF method's lpcs
	for (j = i; j < m; j++) a[j] = 0.0;   // pad remaining coefficients with zeroes
	// adjust gain:
	if (gain != NULL) *gain = xms * (ssi_real_t)n;
	return status;
}

}

