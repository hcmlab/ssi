// OSCons.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/09/21 
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
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

#pragma once

#ifndef SSI_OPENSMILE_TOOLS_H
#define SSI_OPENSMILE_TOOLS_H

#include "SSI_Cons.h"

// following code taken from openSMILE 1.0.1, smileTypes.hpp and smileUtil.c
// http://opensmile.sourceforge.net/

/* opensmile internal types */
#define FLOAT_DMEM_FLOAT  0
#define FLOAT_DMEM_DOUBLE  1

// this defines the float type used throughout the data memory, either 'float' or 'double'
#define FLOAT_DMEM  ssi_real_t
#define FLOAT_DMEM_NUM  FLOAT_DMEM_FLOAT // this numeric constant MUST equal the float type set above ...
                                           // 0 = float, 1 = double:
// this defines the int type used throughout the data memory, either 'short', 'int' or 'long'
#define INT_DMEM    int

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

/*======= rounding functions ==========*/

/* check if number is power of 2 (positive or negative) */
SSI_INLINE long smileMath_isPowerOf2(long x)
{
	if (x==1) return 1;  // 1 is a power of 2
	if (((x&1) == 0)&&(x != 0)) { // only even numbers > 1
		x=x>>1;
		while ((x&1) == 0) { x=x>>1;  }
		return ((x==1)||(x==-1));
	}
	return 0;
}

/* round to nearest power of two */
SSI_INLINE long smileMath_roundToNextPowOf2(long x)
{
	// round x up to nearest power of 2
	unsigned long int flng = (unsigned long int)x;
	unsigned long int fmask = 0x8000;
	while ( (fmask & flng) == 0) { fmask = fmask >> 1; }
	// fmask now contains the MSB position
	if (fmask > 1) {
		if ( (fmask>>1)&flng ) { flng = fmask<<1; }
		else { flng = fmask; }
	} else {
		flng = 2;
	}

	return (long)flng;
}

/* round up to next power of 2 */
SSI_INLINE long smileMath_ceilToNextPowOf2(long x)
{
	long y = smileMath_roundToNextPowOf2(x);
	if (y<x) y *= 2;
	return y;
}

/* round down to next power of two */
SSI_INLINE long smileMath_floorToNextPowOf2(long x)
{
	long y = smileMath_roundToNextPowOf2(x);
	if (y>x) y /= 2;
	return y;
}


/*******************************************************************************************
***********************=====   Sort functions   ===== **************************************
*******************************************************************************************/

/** inplace quicksort algorithms **/

/* QuickSort algorithm for a float array with nEl elements */
SSI_INLINE void smileUtil_quickSort_float(float *arr, long nEl)
{
#ifdef MAX_LEVELS
#undef MAX_LEVELS
#endif
#define  MAX_LEVELS  300

	float piv;
	long beg[MAX_LEVELS], end[MAX_LEVELS],swap;
	long i=0, L, R;

	beg[0]=0; end[0]=nEl;
	while (i>=0) {
		L=beg[i]; R=end[i]-1;
		if (L<R) {
			piv=arr[L]; 
			while (L<R) {
				while (arr[R]>=piv && L<R) R--; if (L<R) arr[L++]=arr[R];
				while (arr[L]<=piv && L<R) L++; if (L<R) arr[R--]=arr[L]; }
			arr[L]=piv; beg[i+1]=L+1; end[i+1]=end[i]; end[i++]=L;
			if (end[i]-beg[i]>end[i-1]-beg[i-1]) {
				swap=beg[i]; beg[i]=beg[i-1]; beg[i-1]=swap;
				swap=end[i]; end[i]=end[i-1]; end[i-1]=swap;
			}
		} else { i--; }
	}
}

/* QuickSort algorithm for a double array with nEl elements */
SSI_INLINE void smileUtil_quickSort_double(double *arr, long nEl)
{
#ifndef MAX_LEVELS
#define MAX_LEVELS  300
#endif

	double piv;
	long beg[MAX_LEVELS], end[MAX_LEVELS],swap;
	long i=0, L, R;

	beg[0]=0; end[0]=nEl;
	while (i>=0) {
		L=beg[i]; R=end[i]-1;
		if (L<R) {
			piv=arr[L];
			while (L<R) {
				while (arr[R]>=piv && L<R) R--; if (L<R) arr[L++]=arr[R];
				while (arr[L]<=piv && L<R) L++; if (L<R) arr[R--]=arr[L]; }
			arr[L]=piv; beg[i+1]=L+1; end[i+1]=end[i]; end[i++]=L;
			if (end[i]-beg[i]>end[i-1]-beg[i-1]) {
				swap=beg[i]; beg[i]=beg[i-1]; beg[i-1]=swap;
				swap=end[i]; end[i]=end[i-1]; end[i-1]=swap;
			}
		} else { i--; }
	}
}

/* QuickSort algorithm for a FLOAT_DMEM array with nEl elements */
SSI_INLINE void smileUtil_quickSort_FLOATDMEM(FLOAT_DMEM *arr, long nEl)
{
#ifndef MAX_LEVELS
#define MAX_LEVELS  300
#endif

	FLOAT_DMEM piv;
	long beg[MAX_LEVELS], end[MAX_LEVELS],swap;
	long i=0, L, R;

	beg[0]=0; end[0]=nEl;
	while (i>=0) {
		L=beg[i]; R=end[i]-1;
		if (L<R) {
			piv=arr[L];
			while (L<R) {
				while (arr[R]>=piv && L<R) R--; if (L<R) arr[L++]=arr[R];
				while (arr[L]<=piv && L<R) L++; if (L<R) arr[R--]=arr[L]; }
			arr[L]=piv; beg[i+1]=L+1; end[i+1]=end[i]; end[i++]=L;
			if (end[i]-beg[i]>end[i-1]-beg[i-1]) {
				swap=beg[i]; beg[i]=beg[i-1]; beg[i-1]=swap;
				swap=end[i]; end[i]=end[i-1]; end[i-1]=swap;
			}
		} else { i--; }
	}
}

/* Reverse the order in an array of elements, i.e. swap first and last element, etc. */
SSI_INLINE void smileUtil_reverseOrder_FLOATDMEM(FLOAT_DMEM *arr, long nEl)
{
	long i; int range;
	FLOAT_DMEM tmp;
	if (nEl % 2 == 0) {
		range = nEl>>1;
	} else {
		range = (nEl-1)>>1;
	}
	for (i=0; i<nEl>>1; i++) {
		tmp = arr[i];
		arr[i] = arr[nEl-i-1];
		arr[nEl-i-1] = tmp;
	}
}

/*======= math functions ==========*/

#define MIN( a, b ) ((a < b) ? a : b)
#define MAX( a, b ) ((a > b) ? a : b)

SSI_INLINE double smileMath_log2(double x)
{
	return log(x)/log(2.0);
}

/* sinc function (modified) : (sin 2pi*x) / x */
SSI_INLINE double smileDsp_lcSinc(double x)
{
	double y = M_PI * x;
	return sin(y)/(y);
}

/* sinc function : (sin x) / x  */
SSI_INLINE double smileDsp_sinc(double x)
{
	return sin(x)/(x);
}

// u: is optional pointer to a workspace pointer (smileMath_spline will allocate
//    a vector there if the pointer pointed to is NULL
//    the calling code is responsible of freeing this memory with free() at any 
//    later time which seems convenient (i.e. at the end of all calculations)
SSI_INLINE int smileMath_spline (const double *x, const double *y, long n, double yp1, double ypn, double *y2, double **workspace)
{
	double p, qn, sig, un, *u;
	long i,k;

	if (workspace!=NULL) u = *workspace;
	if (u==NULL) u = (double*)malloc(sizeof(double)*(n-1));

	if (yp1 > 0.99e30) {
		y2[0] = u[0] = 0.0;
	} else {
		y2[0] = -0.5;
		u[0] = (3.0 / (x[1] - x[0])) * ((y[1] - y[0]) / (x[1] - x[0]) - yp1);
	}

	for (i=1; i < n-1; i++) { // for (i=2; i <= n-1; i++) {
		sig = (x[i] - x[i-1]) / (x[i+1] - x[i-1]);
		p = sig * y2[i-1] + 2.0;
		y2[i] = (sig - 1.0) / p;
		u[i] = (y[i+1] - y[i]) / (x[i+1] - x[i]) - (y[i] - y[i-1]) / (x[i] - x[i-1]);
		u[i] = (6.0 * u[i] / (x[i+1] - x[i-1]) - sig * u[i-1]) / p;
	}

	if (ypn > 0.99e30) {
		qn = un = 0.0;
	} else {
		qn = 0.5;
		un = (3.0 / (x[n-1] - x[n-2])) * (ypn - (y[n-1] - y[n-2]) / (x[n-1] - x[n-2]));
	}

	y2[n-1] = (un - qn * u[n-2]) / (qn * y2[n-2] + 1.0);
	for (k=n-2; k >= 0; k--) { // for (k=n-1; k >= 1; k--) {
		y2[k] = y2[k] * y2[k+1] + u[k];
	}

	if (workspace == NULL) {
		free(u);
	} else {
		*workspace = u;
	}

	return 1;
}

/* spline interpolation
Given arrays xa[1..n] and ya[1..n] containing a tabulated function,
i.e., y[i] = f(x[i]), with x[1] < x[2] < ... < x[n], and given the
array y2a[1..n] which is the output of NUMspline above, and given
a value of x, this routine returns an interpolated value y.
*/
SSI_INLINE int smileMath_splint (double xa[], double ya[], double y2a[], long n, double x, double *y)
{
	long klo, khi, k;
	double h, b, a;

	klo = 1; khi = n;
	while (khi-klo > 1)
	{
		k = (khi + klo) >> 1;
		if (xa[k-1] > x) khi = k;
		else klo = k;
	}
	khi--; klo--;
	h = xa[khi] - xa[klo];
	if (h == 0.0) {
		printf("smileMath: splint() : bad input value (h=0)\n");
		return 0;
	}
	a = (xa[khi] - x) / h;
	b = (x - xa[klo]) / h;
	*y = a * ya[klo] + b * ya[khi]+((a * a * a - a) * y2a[klo] +
		(b * b * b - b) * y2a[khi]) * (h * h) / 6.0;
	return 1;
}

/* peak enhancement in a linear magnitude spectrum */
SSI_INLINE int smileDsp_specEnhanceSHS (double *a, long n)
{
	long i, j, nmax = 0, *posmax;
	posmax = (long *)calloc(1,sizeof(long)*( (n + 1) / 2 + 1/*?*/ ));
	if ( (n < 2) || (posmax == NULL)) return 0;

	if (a[0] > a[1]) posmax[nmax++] = 0;

	for (i=1; i < n-1; i++) {  // for (i=2; i <= n-1; i++) {
		if (a[i] > a[i-1] && a[i] >= a[i+1]) {
			posmax[nmax++] = i; // ++nmax] = i;
		}
	}

	if (a[n-1] > a[n-2]) posmax[nmax++] = n-1;

	if (nmax == 1) {
		for (j=0; j <= posmax[1]-3; j++) a[j] = 0; // for (j=1; j <= posmax[1]-3; j++) a[j] = 0;
		for (j=posmax[1]+3; j < n; j++) a[j] = 0; // for (j=posmax[1]+3; j <= n; j++) a[j] = 0;
	}
	else {
		for (i=1; i < nmax; i++) { // for (i=2; i <= nmax; i++) {
			for (j=posmax[i-1]+3; j <= posmax[i]-3; j++) a[j] = 0;
		}
	}

	free(posmax);
	return 1;
}

/* smooth a magnitude spectrum (linear) */
SSI_INLINE void smileDsp_specSmoothSHS (double *a, long n)
{
	double ai, aim1 = 0; long i;
	for (i=0; i < n-1; i++) { // for (i=1; i <= n-1; i++)
		ai = a[i]; a[i] = (aim1 + 2.0 * ai + a[i+1]) / 4.0; aim1 = ai;
	}
}

// constructs a parabola from three points (parabolic interpolation)
// returns: peak x of parabola, and optional (if not NULL) the y value of the peak in *y and the steepness in *_a
SSI_INLINE double smileMath_quadFrom3pts(double x1, double y1, double x2, double y2, double x3, double y3, double *y, double *_a)
{
	double den = x1*x1*x2 + x2*x2*x3 + x3*x3*x1 - x3*x3*x2 - x2*x2*x1 - x1*x1*x3;
	if (den != 0.0) {
		double a = (y1*x2 + y2*x3 + y3*x1 - y3*x2 - y2*x1 - y1*x3)/den;
		double b = (x1*x1*y2 + x2*x2*y3 + x3*x3*y1 - x3*x3*y2 - x2*x2*y1 - x1*x1*y3) / den;
		double c = (x1*x1*x2*y3 + x2*x2*x3*y1 + x3*x3*x1*y2 - x3*x3*x2*y1 - x2*x2*x1*y3 - x1*x1*x3*y2) / den;
		if (a != 0.0) {
			double x;
			if (_a != NULL) *_a = a;
			x = -b/(2.0*a);
			if (y!=NULL) *y = c - a*x*x;
			return x;
		} 
	} 

	// fallback to peak picking if we can't construct a parabola
	if (_a!=NULL) *_a = 0.0;
	if ((y1>y2)&&(y1>y3)) { if (y!=NULL) *y = y1; return x1; }
	else if ((y2>y1)&&(y2>y3)) { if (y!=NULL) *y = y2; return x2; }
	else if ((y3>y1)&&(y3>y2)) { if (y!=NULL) *y = y3; return x3; }

	// fallback to keep compiler happy.. this will only happen if all input values are equal:
	if (y!=NULL) *y = y1; return x1;
}


/*
median of vector x
(workspace can be a pointer to an array of N FLOAT_DMEMs which is used to sort the data in x without changing x)
(if workspace is NULL , the function will allocate and free the workspace internally)
*/
SSI_INLINE FLOAT_DMEM smileMath_median(const FLOAT_DMEM *x, long N, FLOAT_DMEM *workspace)
{
	long i;
	FLOAT_DMEM median=0.0;
	FLOAT_DMEM *tmp = workspace;
	if (tmp == NULL) tmp = (FLOAT_DMEM*)malloc(sizeof(FLOAT_DMEM)*N);
	if (tmp==NULL) return 0.0;
	for (i=0; i<N; i++) { tmp[i] = x[i]; }
	//memcpy(tmp, x, sizeof(FLOAT_DMEM)*N);

	smileUtil_quickSort_FLOATDMEM(tmp,N);
	if (N&1) { // easy median for odd N
		median = tmp[N>>1];
	} else { // median as mean of the two middle elements for even N
		median = (FLOAT_DMEM)0.5 * (tmp[N/2]+tmp[N/2-1]);
	}
	if (workspace == NULL) free(tmp);
	return median;
}

/*
median of vector x
(workspace can be a pointer to an array of 2*N (!) FLOAT_DMEMs which is used to sort the data in x without changing x)
(if workspace is NULL , the function will allocate and free the workspace internally)
THIS function should return the original vector index of the median in workspace[0] (and workspace[1] if N is even), to use this functionality you must provide a workspace pointer!
*/
SSI_INLINE FLOAT_DMEM smileMath_medianOrdered(const FLOAT_DMEM *x, long N, FLOAT_DMEM *workspace)
{
	long i,j;
	long oi0=0, oi1=0;
	FLOAT_DMEM median=0.0;
	FLOAT_DMEM *tmp = workspace;
	if (tmp == NULL) tmp = (FLOAT_DMEM*)malloc(sizeof(FLOAT_DMEM)*2*N);
	if (tmp==NULL) return 0.0;


	for (i=0; i<N; i++) { tmp[i] = x[i]; }
	//memcpy(tmp, x, sizeof(FLOAT_DMEM)*N);

	for (i=0; i<N; i++) {
		tmp[N+i] = (FLOAT_DMEM)i;
	}

	// we cannot use quicksort, since it doesn't preserve the original indexing
	//smileUtil_quickSort_FLOATDMEM(tmp,N);
	for (i=0; i<N; i++) {
		for (j=i+1; j<N; j++) {
			if (tmp[i] > tmp[j]) { //swap data and indicies
				FLOAT_DMEM t = tmp[i]; // swap data
				tmp[i] = tmp[j];
				tmp[j] = t;
				t = tmp[i+N]; // swap indicies
				tmp[i+N] = tmp[j+N];
				tmp[j+N] = t;
			}
		}
	}

	if (N&1) { // easy median for odd N
		median = tmp[N>>1];
		tmp[0] = tmp[N+(N>>1)];
	} else { // median as mean of the two middle elements for even N
		median = (FLOAT_DMEM)0.5 * (tmp[N>>1]+tmp[(N>>1)-1]);
		tmp[0] = tmp[N+(N>>1)-1];
		tmp[1] = tmp[N+(N>>1)];
	}
	if (workspace == NULL) free(tmp);
	return median;
}

/***** vector math *******/

/* compute euclidean norm of given vector x */
SSI_INLINE FLOAT_DMEM smileMath_vectorLengthEuc(FLOAT_DMEM *x, long N)
{
	long i; FLOAT_DMEM norm = 0.0;
	for (i=0; i<N; i++) norm += x[i]*x[i];
	return (FLOAT_DMEM)sqrt(norm);
}

/* compute L1 norm (absolute sum) of given vector x */
SSI_INLINE FLOAT_DMEM smileMath_vectorLengthL1(FLOAT_DMEM *x, long N)
{
	long i; FLOAT_DMEM norm = 0.0;
	for (i=0; i<N; i++) norm += (FLOAT_DMEM)fabs(x[i]);
	return norm;
}

/* normalise euclidean length of x to 1 */
SSI_INLINE FLOAT_DMEM smileMath_vectorNormEuc(FLOAT_DMEM *x, long N)
{
	FLOAT_DMEM norm = smileMath_vectorLengthEuc(x,N);
	long i; 
	if (norm > 0.0) for (i=0; i<N; i++) x[i] /= norm;
	return norm;
}

/* normalise vector sum to 1 */
SSI_INLINE FLOAT_DMEM smileMath_vectorNormL1(FLOAT_DMEM *x, long N)
{
	FLOAT_DMEM norm = smileMath_vectorLengthL1(x,N);
	long i; 
	if (norm > 0.0) for (i=0; i<N; i++) x[i] /= norm;
	return norm;
}

/* normalise values of vector x to range [min - max] */
SSI_INLINE void smileMath_vectorNormMax(FLOAT_DMEM *x, long N, FLOAT_DMEM min, FLOAT_DMEM max)
{
	long i;
	FLOAT_DMEM _min=x[0];
	FLOAT_DMEM _max=x[0];
	FLOAT_DMEM scale;
	for (i=0; i<N; i++) {
		if (x[i] < _min) _min = x[i];
		else if (x[i] > _max) _max = x[i];
	}
	if (_max==_min) scale = 1.0;
	else scale = (max-min)/(_max-_min);
	for (i=0; i<N; i++) {
		x[i] = (x[i]-_min)*scale+min;
	}
}

/* compute the arithmetic mean of vector x */
SSI_INLINE FLOAT_DMEM smileMath_vectorAMean(FLOAT_DMEM *x, long N)
{
	long i; FLOAT_DMEM sum = 0.0;
	for (i=0; i<N; i++) sum += x[i];
	return sum / (FLOAT_DMEM)N;
}

/* root of each element in a vector */
SSI_INLINE void smileMath_vectorRoot(FLOAT_DMEM *x, long N)
{
	long i;
	for (i=0; i<N; i++) { if (x[i]>=(FLOAT_DMEM)0.0) x[i]=(FLOAT_DMEM)sqrt(x[i]); }
}

/* root of each element in a vector */
SSI_INLINE void smileMath_vectorRootD(double *x, long N)
{
	long i;
	for (i=0; i<N; i++) { if (x[i]>=0.0) x[i]=sqrt(x[i]); }
}

/**** complex number math ****/

/* absolute value of a complex number */
SSI_INLINE double smileMath_complexAbs(double Re, double Im)
{
  return sqrt (Re*Re + Im*Im);
}

/* compute A/B , store in C */
SSI_INLINE void smileMath_complexDiv(double ReA, double ImA, double ReB, double ImB, double *ReC, double *ImC)
{
	double r, den;
	double R=0,I=0;

	if (fabs (ReB) >= fabs (ImB)) {
		if (ReB != 0.0) {
			r = ImB / ReB;
			den = ReB + r * ImB;
			if (den != 0.0) {
				R = (ReA + ImA*r ) / den; // R = (ReA  + r*ImA ) / den;
				I = (ImA - r*ReA) / den; // I = (ImA * ReB - r * ReA) / den;
			}
		}
	} else {
		if (ImB != 0.0) {
			r = ReB / ImB;
			den = ImB + r * ReB;
			if (den != 0.0) {
				R = (ReA * r + ImA) / den;
				I = (ImA * r - ReA) / den;
			}
		}
	}
	if (ReC != NULL) *ReC = R;
	if (ImC != NULL) *ImC = I;
}

/* fix roots to inside the unit circle */
// ensure all roots are within the unit circle
// if abs(root) > 1  (outside circle)
// then root = 1 / root*
//
// *roots is an array of n complex numbers (2*n doubles)
SSI_INLINE void smileMath_complexIntoUnitCircle(double *roots, int n)
{
	long i;
	for (i=0; i<n; i++) {
		long i2 = i*2;
		// if abs(root) > 1.0 
		if (smileMath_complexAbs(roots[i2],roots[i2+1]) > 1.0) {
			// root = 1.0 / root*
			smileMath_complexDiv(1.0 , 0.0 , roots[i2], -roots[i2+1], &roots[i2], &roots[i2+1]);
		}
	}
}

/*======= window functions ==========*/

/* rectangular window */
SSI_INLINE double * smileDsp_winRec(long _N)
{
	int i;
	double * ret = (double *)malloc(sizeof(double)*_N);
	double * x = ret;
	for (i=0; i<_N; i++) {
		*x = 1.0; x++;
	}
	return ret;
}

/* triangular window (non-zero endpoints) */
SSI_INLINE double * smileDsp_winTri(long _N)
{
	long i;
	double * ret = (double *)malloc(sizeof(double)*_N);
	double * x = ret;
	for (i=0; i<_N/2; i++) {
		*x = 2.0*(double)(i+1)/(double)_N;
		x++;
	}
	for (i=_N/2; i<_N; i++) {
		*x = 2.0*(double)(_N-i)/(double)_N;
		x++;
	}
	return ret;
}

/* powered triangular window (non-zero endpoints) */
SSI_INLINE double *smileDsp_winTrP(long _N)
{
	double *w = smileDsp_winTri(_N);
	double *x = w;
	long n; for (n=0; n<_N; n++) *x = *x * (*(x++));
	return w;
}

/* bartlett (triangular) window (zero endpoints) */
SSI_INLINE double *smileDsp_winBar(long _N)
{ 
	long i;
	double * ret = (double *)malloc(sizeof(double)*_N);
	double * x = ret;
	for (i=0; i<_N/2; i++) {
		*x = 2.0*(double)(i)/(double)(_N-1);
		x++;
	}
	for (i=_N/2; i<_N; i++) {
		*x = 2.0*(double)(_N-1-i)/(double)(_N-1);
		x++;
	}
	return ret;
}

/* hann(ing) window */
SSI_INLINE double *smileDsp_winHan(long _N)
{
	double i;
	double * ret = (double *)malloc(sizeof(double)*_N);
	double * x = ret;
	double NN = (double)_N;
	for (i=0.0; i<NN; i += 1.0) {
		*x = 0.5*(1.0-cos( (2.0*M_PI*i)/(NN-1.0) ));
		x++;
	}
	return ret;
}

/* hamming window */
SSI_INLINE double *smileDsp_winHam(long _N)
{
	double i;
	double * ret = (double *)malloc(sizeof(double)*_N);
	double * x = ret;
	double NN = (double)_N;
	for (i=0.0; i<NN; i += 1.0) {
		/*    *x = 0.53836 - 0.46164 * cos( (2.0*M_PI*i)/(NN-1.0) ); */
		*x = 0.54 - 0.46 * cos( (2.0*M_PI*i)/(NN-1.0) );
		x++;
	}
	return ret;
}

/* half-wave sine window (cosine window) */
SSI_INLINE double *smileDsp_winSin(long _N)
{
	double i;
	double * ret = (double *)malloc(sizeof(double)*_N);
	double * x = ret;
	double NN = (double)_N;
	for (i=0.0; i<NN; i += 1.0) {
		*x = sin( (1.0*M_PI*i)/(NN-1.0) );
		x++;
	}
	return ret;
}

/* Lanczos window */
SSI_INLINE double *smileDsp_winLac(long _N)
{
	double i;
	double * ret = (double *)malloc(sizeof(double)*_N);
	double * x = ret;
	double NN = (double)_N;
	for (i=0.0; i<NN; i += 1.0) {
		*x = smileDsp_lcSinc( (2.0*i)/(NN-1.0) - 1.0 );
		x++;
	}
	return ret;
}

/* gaussian window ...??? */
SSI_INLINE double *smileDsp_winGau(long _N, double sigma)
{
	double i;
	double * ret = (double *)malloc(sizeof(double)*_N);
	double * x = ret;
	double NN = (double)_N;
	double tmp;
	if (sigma <= 0.0) sigma = 0.01;
	if (sigma > 0.5) sigma = 0.5;
	for (i=0.0; i<NN; i += 1.0) {
		tmp = (i-(NN-1.0)/2.0)/(sigma*(NN-1.0)/2.0);
		*x = exp( -0.5 * ( tmp*tmp ) );
		x++;
	}
	return ret;
}

/* Blackman window */
SSI_INLINE double *smileDsp_winBla(long _N, double alpha0, double alpha1, double alpha2)
{
	double i;
	double * ret = (double *)malloc(sizeof(double)*_N);
	double * x = ret;
	double NN = (double)_N;
	double tmp;
	for (i=0.0; i<NN; i += 1.0) {
		tmp = (2.0*M_PI*i)/(NN-1.0);
		*x = alpha0 - alpha1 * cos( tmp ) + alpha2 * cos( 2.0*tmp );
		x++;
	}
	return ret;

}

/* Bartlett-Hann window */
SSI_INLINE double *smileDsp_winBaH(long _N, double alpha0, double alpha1, double alpha2)
{
	double i;
	double * ret = (double *)malloc(sizeof(double)*_N);
	double * x = ret;
	double NN = (double)_N;
	for (i=0.0; i<NN; i += 1.0) {
		*x = alpha0 - alpha1 * fabs( i/(NN-1.0) - 0.5 ) - alpha2 * cos( (2.0*M_PI*i)/(NN-1.0) );
		x++;
	}
	return ret;
}

/* Blackman-Harris window */
SSI_INLINE double *smileDsp_winBlH(long _N, double alpha0, double alpha1, double alpha2, double alpha3)
{
	double i;
	double * ret = (double *)malloc(sizeof(double)*_N);
	double * x = ret;
	double NN = (double)_N;
	double tmp;
	for (i=0.0; i<NN; i += 1.0) {
		tmp = (2.0*M_PI*i)/(NN-1.0);
		*x = alpha0 - alpha1 * cos( tmp ) + alpha2 * cos( 2.0*tmp ) - alpha3 * cos( 3.0*tmp );
		x++;
	}
	return ret;
}

/* LPC analysis via acf (=implementation of Durbin recursion)*/
SSI_INLINE int smileDsp_calcLpcAcf(FLOAT_DMEM * r, FLOAT_DMEM *a, int p, FLOAT_DMEM *gain, FLOAT_DMEM *k)
{
  int i,m;
  FLOAT_DMEM e;
  int errF = 1;
  FLOAT_DMEM k_m;
  FLOAT_DMEM *al;

  if (a == NULL) return 0;
  if (r == NULL) return 0;

  if ((r[0] == 0.0)||(r[0] == -0.0)) {
    for (i=0; i < p; i++) a[i] = 0.0;
    return 0;
  }

  al = (FLOAT_DMEM*)malloc(sizeof(FLOAT_DMEM)*(p));
  
  /* Initialisation, Eq. 158 */
  e = r[0];

  /* The iterations: m=1..p (here), Eq. 159 */
  for (m=1; m<=p; m++) {
    /* Eq. 159 (a) */
    FLOAT_DMEM sum = (FLOAT_DMEM)1.0 * r[m];
    for (i=1; i<m; i++) {
      sum += a[i-1] * r[m-i];
    }
    k_m = ( (FLOAT_DMEM)-1.0 / e ) * sum;

    // save reflection coefficient
    if (k != NULL) k[m-1] = k_m;

    /* Eq. 159 (b) */
    a[m-1] = k_m;

    for (i=1; i<=m/2; i++) {
      FLOAT_DMEM x = a[i-1];
      a[i-1] += k_m * a[m-i-1];
      if ((i < (m/2))||((m&1)==1)) a[m-i-1] += k_m * x;
    }

    // update the error:
    e *= ((FLOAT_DMEM)1.0-k_m*k_m);
    if (e==0.0) {
      for (i=m; i<=p; i++) {
        a[i] = 0.0;
        if (k!=NULL) k[m] = 0.0;
      }
      break;
    }
  }

  free(al);

  if (gain != NULL) *gain=e;
  return 1;
}

SSI_INLINE FLOAT_DMEM smileDsp_lpToCeps(const FLOAT_DMEM *lp, int nLp, FLOAT_DMEM lpGain, FLOAT_DMEM *ceps, int firstCC, int lastCC) 
{
  // CHECK: nCeps <= nLp !
  int i,n;
  int nCeps;
  
  if (firstCC < 1) firstCC = 1;
  if (lastCC > nLp) lastCC = nLp;

  nCeps = lastCC-firstCC+1;

  for (n=firstCC; n<=lastCC; n++) {
  //for (n=1; n<=nCeps; n++) {

    double sum=0;
    for (i=1; i<n; i++) { 
      sum += (n-i)*lp[i-1]*ceps[n-i-1]; 
    }

    ceps[n-firstCC] = -(lp[n-firstCC] + (FLOAT_DMEM)(sum / (double)n));
  }

  if (lpGain <= 0.0) { lpGain = (FLOAT_DMEM)1.0; }
  return (FLOAT_DMEM)(-log(1.0/(double)lpGain));
}

/* autocorrelation in the time domain (used for LPC autocorrelation method) */
SSI_INLINE void smileDsp_autoCorr(const FLOAT_DMEM *x, const int n, FLOAT_DMEM *acf, int lag)
{
  int i;
  while (lag) {
    acf[--lag] = 0.0;
    for (i=lag; i < n; i++) {
      acf[lag] += x[i] * x[i-lag];
    }
  }
}

/* compute LPC coefficients with Burg's method (N. Anderson (1978)):
     N. Anderson (1978), "On the calculation of filter coefficients for maximum entropy spectral analysis", in Childers, Modern Spectrum Analysis, IEEE Press, 252-255.
   x : wave samples, 
   n : number of samples
   a : array to hold the coefficients
   m : number of coefficients desired
   gain : optional pointer to FLOAT_DMEM, which will be filled with the computed LPC gain
   burgB1, burgB2, burgAA : pointers to pointers to work area memory, 
     will be initialised automtically on first use, 
     must be freed by calling application on exit ;
     if these pointers are NULL, calcLpcBurg will allocate 
     the work area at the beginning of the function and
     free it at the end of the function 
     (the latter is ineffective if memory allocation is slow)
 */
/* Modified by Florian Eyben, 2010
 *   integration into openSMILE, array indicies all start at 0 
 *                               instead of 1 (as in original)
 *
 * Original: 
 *   NUMburg function taken from praat source code (NUM2.c / Sound_and_LPC.c)
 *
 * Copyright (C) 1994-2008 David Weenink
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
SSI_INLINE int smileDsp_calcLpcBurg (const FLOAT_DMEM *x, long n, FLOAT_DMEM *a, int m, FLOAT_DMEM *gain, FLOAT_DMEM **burgB1, FLOAT_DMEM **burgB2, FLOAT_DMEM **burgAA)
{
	long i = 1, j; int status = 0;
	FLOAT_DMEM p = 0.0;
	FLOAT_DMEM *aa = NULL;
	FLOAT_DMEM xms = 0.0;
	FLOAT_DMEM *b1 = NULL, *b2 = NULL;

	if (x == NULL) return 0;
	if (a == NULL) return 0;
	if ((n<m)||(m<=0)) return 0;
  
	if (burgB1 != NULL) b1 = *burgB1;
	if (b1 == NULL) b1 = (FLOAT_DMEM*)calloc(1,sizeof(FLOAT_DMEM)*n);

	if (burgB2 != NULL) b2 = *burgB2;
	if (b2 == NULL) b2 = (FLOAT_DMEM*)calloc(1,sizeof(FLOAT_DMEM)*n);

	if (burgAA != NULL) aa = *burgAA;
	if (aa == NULL) aa = (FLOAT_DMEM*)calloc(1,sizeof(FLOAT_DMEM)*m);

	/* (3) */
	for (j = 0; j < n; j++) {  // for (j = 1; j <= n; j++) {
	p += x[j] * x[j];
	}

	xms = p / n;
	if (xms <= 0) goto end;

	/* (9) */
	b1[0] = x[0];       // b1[1] = x[1];
	b2[n - 2] = x[n-1]; // b2[n - 1] = x[n];
	for (j = 1; j < n - 1; j++) { // for (j = 2; j <= n - 1; j++) {
	b1[j] = b2[j - 1] = x[j];
	}

	for (i = 0; i < m; i++) { // for (i = 1; i <= m; i++) {

		/* (7) */
		FLOAT_DMEM num = 0.0, denum = 0.0;
		for (j = 0; j < n - i - 1; j++) {  // for (j = 1; j <= n - i; j++) {  
			num += b1[j] * b2[j];
			denum += b1[j] * b1[j] + b2[j] * b2[j];
		}

		if (denum <= 0) goto end;

		a[i] = (FLOAT_DMEM)2.0 * num / denum;

		/* (10) */
		xms *= (FLOAT_DMEM)1.0 - a[i] * a[i];

		/* (5) */
		for (j = 0; j < i; j++) {  // for (j = 1; j <= i - 1; j++) {    
			a[j] = aa[j] - a[i] * aa[i - j - 1]; //  aa[i-j]
		}

		if (i < m-1) { // if (i < m) {
    
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
	if (gain != NULL) *gain = xms * (FLOAT_DMEM)n;
	return status;
}

/*	
	Implementation of an inverse lattice filter 
	This function processed a single value per call
	k: coefficients
	*b : temporary work area, initialise with 0 at the beginning! (size: sizeof(FLOAT_DMEM)*M ) 
	M: order (number of coefficients)
	out: e(n) input sample
	return value: f(M) = filter "input" s(n)
 */
SSI_INLINE FLOAT_DMEM smileDsp_invLattice(FLOAT_DMEM *k, FLOAT_DMEM *b, int M, FLOAT_DMEM out)
{
	int i;
	FLOAT_DMEM fM;
	FLOAT_DMEM last = b[M-1];
	/* initialisation */
	fM = out;
	for (i=M-1; i>0; i--) {
	fM -= k[i] * b[i-1];
	b[i] = k[i] * fM + b[i-1];
	}
	b[M-1] = last;
	fM = fM - k[0] * b[M-1];
	b[0] = k[0]*fM + b[M-1];
	b[M-1] = fM;
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
SSI_INLINE FLOAT_DMEM smileDsp_lattice(FLOAT_DMEM *k, FLOAT_DMEM *b, int M, FLOAT_DMEM in, FLOAT_DMEM *bM)
{
  int i;
  FLOAT_DMEM f0,f1,b0,b1;
  /* initialisation */
  b0 = f0 = in;
  for (i=0; i<M; i++) {
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


/*******************************************************************************************
***********************=====   Filter functions   ===== **************************************
*******************************************************************************************/

/* allocate workspace (history matrix) for a temporal median filter */
SSI_INLINE FLOAT_DMEM * smileUtil_temporalMedianFilterInit(long N, long T)
{
	FLOAT_DMEM *ws = (FLOAT_DMEM*)calloc(1,sizeof(FLOAT_DMEM)*(N*(T+1)+2+T));
	// NOTE: first two floats of workspace are N and T
	if (ws != NULL) {
		ws[0] = (FLOAT_DMEM)N;
		ws[1] = (FLOAT_DMEM)T;
	}
	return ws;
}

/* allocate workspace (history matrix) for a temporal median filter */
SSI_INLINE FLOAT_DMEM * smileUtil_temporalMedianFilterInitSl(long N, long Ns, long T)
{
	FLOAT_DMEM *ws = (FLOAT_DMEM*)calloc(1,sizeof(FLOAT_DMEM)*(N*(Ns+1)*(T+1)+2+2*T));
	// NOTE: first two floats of workspace are N and T
	if (ws != NULL) {
		ws[0] = (FLOAT_DMEM)(N*(Ns+1));
		ws[1] = (FLOAT_DMEM)T;
	}
	return ws;
}

/* free the temporal median filter workspace and return NULL */
SSI_INLINE FLOAT_DMEM * smileUtil_temporalMedianFilterFree(FLOAT_DMEM *workspace)
{
	if (workspace != NULL) free(workspace);
	return NULL;
}

/*
Perform median filter of each element in frame x (over time) using a history matrix given in *workspace
The workspace must be created with smileUtil_temporalMedianFilterInit.
workspace : ptr el0 el0 el0(t-1)... el0(t) ; ptr el1 el1 el1(t-1) ... el1(t)
*/
SSI_INLINE void smileUtil_temporalMedianFilter(FLOAT_DMEM *x, long N, FLOAT_DMEM *workspace)
{
	long i;
	long _N;
	long Nw;
	long T, T1;
	FLOAT_DMEM *ws;

	if (workspace == NULL) return;
	if (N<=0) return;


	// check for matching N and find minimal _N we will work with
	Nw = (long)workspace[0];
	if (Nw > N) _N = N;
	else _N = Nw;
	T = (long)workspace[1];
	T1 = T+1;

	ws = workspace + Nw*(T+1)+2;

	for (i=0; i<_N; i++) { // apply median filter to each element 0.._N-1
		long ws0 = i*T1+2;

		// add new element to history
		long ptr = (long)(workspace[ws0])+1;
		workspace[ws0+(ptr++)] = x[i];
		if (ptr > T) ptr = 1;
		workspace[ws0] = (FLOAT_DMEM)(ptr-1);

		// compute median and save in vector x
		x[i] = smileMath_median(&(workspace[ws0+1]), T, ws);
	}
}

/*
Perform median filter of each element in frame x (over time) using a history matrix given in *workspace
The workspace must be created with smileUtil_temporalMedianFilterInit.
workspace : ptr el0 el0 el0(t-1)... el0(t) ; ptr el1 el1 el1(t-1) ... el1(t)
**> Filter with slave data (Ns is number of slave elements for each element in x (total size of x thus is N*Ns))
The workspace must be allocated for N*(Ns+1) elements!
*/
SSI_INLINE void smileUtil_temporalMedianFilterWslave(FLOAT_DMEM *x, long N, long Ns, FLOAT_DMEM *workspace)
{
	long i,j;
	long _N;
	long Nw;
	long T, T1;
	FLOAT_DMEM *ws;

	if (workspace == NULL) return;
	if (N<=0) return;


	// check for matching N and find minimal _N we will work with
	Nw = (long)workspace[0];
	if (Nw > N) _N = N;
	else _N = Nw;
	T = (long)workspace[1];
	T1 = T+1;

	ws = workspace + Nw*(T+1)+2;

	for (i=0; i<_N; i++) { // apply median filter to each element 0.._N-1
		long ws0 = i*T1+2;

		// add new element to history
		long ptr = (long)(workspace[ws0])+1;
		workspace[ws0+(ptr++)] = x[i];
		if (ptr > T) ptr = 1;
		workspace[ws0] = (FLOAT_DMEM)(ptr-1);

		// add slave elements to history
		if (Nw >= N*(Ns+1)) {
			for (j=1; j<=Ns; j++) {
				long ws0 = (i+j*Nw/(Ns+1))*T1+2;
				long ptr = (long)(workspace[ws0])+1;
				workspace[ws0+(ptr++)] = x[i+j*N];
				if (ptr > T) ptr = 1;
				workspace[ws0] = (FLOAT_DMEM)(ptr-1);
			}
		}

		// compute median and save in vector x
		x[i] = smileMath_medianOrdered(&(workspace[ws0+1]), T, ws);

		// use indicies in ws to sort slave data (if workspace is large enough)
		if (Nw >= N*Ns) {
			for (j=1; j<=Ns; j++) {
				if (T&1) { // odd
					long ws0 = (i+j*Nw/(Ns+1))*T1+2;
					long ptr = (long)(workspace[ws0]+(FLOAT_DMEM)1.0-ws[0]);
					if (ptr < 1) ptr += T;
					x[i+j*N] = workspace[ws0+ptr]; 
				} else { // even
					long ws0 = (i+j*Nw/(Ns+1))*T1+2;
					long ptr0 = (long)(workspace[ws0]+(FLOAT_DMEM)1.0-ws[0]);
					long ptr1 = (long)(workspace[ws0]+(FLOAT_DMEM)1.0-ws[1]);
					if (ptr0 < 1) ptr0 += T;
					if (ptr1 < 1) ptr1 += T;
					x[i+j*N] = (FLOAT_DMEM)0.5 * (workspace[ws0+ptr0] + workspace[ws0+ptr1]);
				}
			}
		}

	}
}


/*******************************************************************************************
 ***********************=====   Statistics functions   ===== *******************************
 *******************************************************************************************/

/*
Note: the entropy functions compute entropy from a PMF, thus a sequence of values must be converted to a PMF beforehand!
For spectral entropy the normalised spectrum is assumed to be a PMF, thus it is not converted...
*/

/* compute entropy of normalised values (the values will be normalised to represent probabilities by this function) */
SSI_INLINE FLOAT_DMEM smileStat_entropy(FLOAT_DMEM *_vals, long N)
{
  double e = 0.0;
  int i;
  double dn = 0.0;
  FLOAT_DMEM min=0.0;
  double l2 = (double)log(2.0);

  // get sum of values and minimum value
  for (i=0; i<N; i++) {
    dn += (double)_vals[i];
    if (_vals[i] < min) min = _vals[i];
  }
  
  // floor values < 0
  if (min < 0.0) {
    for (i=0; i<N; i++) {
      _vals[i] -= min;
      if (_vals[i] == 0.0) { _vals[i] = (FLOAT_DMEM)0.00001; dn += (double)0.00001; }
      dn -= (double)min;
    }
  }
  if (dn<(FLOAT_DMEM)0.000001) dn = (FLOAT_DMEM)0.000001;

  // normalise sample values and compute entropy
  for (i=0; i<N; i++) {
    double ln = (double)_vals[i] / dn;
    if (ln > 0.0) 
      e += ln * (double)log(ln) / l2;
  }

  return (FLOAT_DMEM)(-e);
}



#endif
