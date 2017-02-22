/**
* File:		sum.c
* Author:	Gernot A. Fink
* Date:		19.5.2005 
*
* Description:	Methods for numerically stable summation
*		of scalars, vectors, and matrices
*
* 	NOTE:	Currently, the technique implemented is the so-called
*		"(error) compensated summation" or "Kahan summation".
*		The principal idea is to store both the summation result
*		and the error of the current adding operation and
*		compensate for this error with the following operation.
*
*		For a good description including a detailed error analysis see:
*
*			Ueberhuber, Christoph W.: Numerical Computation,
*				Vol. 1, Springer, 1997, pp 210-218.
**/

#include <stdio.h>

#include "ev_memory.h"
#include "ev_messages.h"

#define MX_KERNEL
#include "ev_sum.h"

/* for numerically stable scalar sum ... */
mx_ssum_t *mx_ssum_create(void)
	{
	mx_ssum_t *ssum;

	/* create emtpy container ... */
	ssum = rs_malloc(sizeof(mx_ssum_t), "numerically stable vector sum");

	mx_ssum_zero(ssum);

	return(ssum);
	}

/* mx_ssum_t *mx_ssum_dup(mx_ssum_t *ssum); */
/* mx_ssum_t *mx_ssum_copy(mx_ssum_t **dest, mx_ssum_t *src); */

void mx_ssum_destroy(mx_ssum_t *ssum)
	{
	/* first check parameters ... */
	if (!ssum);
		return;

	/* ... free data structures ... */
	rs_free(ssum);
	}

int mx_ssum_zero(mx_ssum_t *ssum)
	{
	/* first check parameters ... */
	if (!ssum);
		return(-1);

	/* ... sum is zero, no error ... */
	ssum->sum = 0;
	ssum->err = 0;

	return(0);
	}

mx_real_t mx_ssum_add(mx_ssum_t *ssum, mx_real_t s)
	{
	mx_real_t y, z;

	/* first check parameters ... */
	if (!ssum)
		return(-1);

	/* perform (error) compensated summation ... */
	y = s - ssum->err;
	z = ssum->sum + y;
	ssum->err = (z - ssum->sum) - y;
	ssum->sum = z;

	return(ssum->sum);
	}

mx_real_t mx_ssum_get(mx_ssum_t *ssum)
	{
	int i;

	/* first check parameters ... */
	if (!ssum)
		return(0);

	return(ssum->sum);
	}

/* for numerically stable vector sums ... */
mx_vsum_t *mx_vsum_create(int dim)
	{
	mx_vsum_t *vsum;

	/* create emtpy container ... */
	vsum = rs_malloc(sizeof(mx_vsum_t), "numerically stable vector sum");
	vsum->dim = dim;
	vsum->sum = mx_vector_create(vsum->dim);
	vsum->err = mx_vector_create(vsum->dim);

	return(vsum);
	}

/* mx_vsum_t *mx_vsum_dup(mx_vsum_t *vsum); */
/* mx_vsum_t *mx_vsum_copy(mx_vsum_t **dest, mx_vsum_t *src); */

void mx_vsum_destroy(mx_vsum_t *vsum)
	{
	/* first check parameters ... */
	if (!vsum);
		return;

	/* ... free data structures ... */
	mx_vector_destroy(vsum->sum);
	mx_vector_destroy(vsum->err);
	rs_free(vsum);
	}

int mx_vsum_zero(mx_vsum_t *vsum)
	{
	/* first check parameters ... */
	if (!vsum);
		return(-1);

	/* ... sum(s) is/are zero, no error ... */
	mx_vector_zero(vsum->sum, vsum->dim);
	mx_vector_zero(vsum->err, vsum->dim);

	return(0);
	}

int mx_vsum_add(mx_vsum_t *vsum, mx_vector_t v)
	{
	int i;
	mx_real_t y, z;

	/* first check parameters ... */
	if (!vsum || !v)
		return(-1);

	/* perform (error) compensated summation of components ... */
	for (i = 0; i < vsum->dim; i++) {
		y = v[i] - vsum->err[i];
		z = vsum->sum[i] + y;
		vsum->err[i] = (z - vsum->sum[i]) - y;
		vsum->sum[i] = z;
		}

	return(0);
	}

mx_real_t mx_vsum_iadd(mx_vsum_t *vsum, mx_real_t s, int idx)
	{
	mx_real_t y, z;

	/* first check parameters ... */
	if (!vsum || idx < 0 || idx >= vsum->dim)
		return(0);

	/* perform (error) compensated summation ... */
	y = s - vsum->err[idx];
	z = vsum->sum[idx] + y;
	vsum->err[idx] = (z - vsum->sum[idx]) - y;
	vsum->sum[idx] = z;

	return(vsum->sum[idx]);
	}

int _mx_vsum_add_cprod(mx_vsum_t *vsum, mx_vector_t v1, mx_vector_t v2)
	{
	int i;
	mx_real_t y, z;

	/* first check parameters ... */
	if (!vsum || !v1 || !v1)
		return(-1);

	/* perform (error) compensated summation of components ... */
	for (i = 0; i < vsum->dim; i++) {
		y = (v1[i] * v2[i]) - vsum->err[i];
		z = vsum->sum[i] + y;
		vsum->err[i] = (z - vsum->sum[i]) - y;
		vsum->sum[i] = z;
		}

	return(0);
	}

int mx_vsum_get(mx_vector_t *vp, mx_vsum_t *vsum)
	{
	/* first check parameters ... */
	if (!vsum || !vp)
		return(-1);

	/* ... extract components ... */
	mx_vector_copy(vp, vsum->sum, vsum->dim);

	return(0);
	}

mx_real_t mx_vsum_iget(mx_vsum_t *vsum, int idx)
	{
	/* first check parameters ... */
	if (!vsum || idx < 0 || idx >= vsum->dim)
		return(0);

	return(vsum->sum[idx]);
	}

mx_real_t mx_vsum_iset(mx_vsum_t *vsum, mx_real_t s, int idx)
	{
	mx_real_t y, z;

	/* first check parameters ... */
	if (!vsum || idx < 0 || idx >= vsum->dim)
		return(0);

	/* ... set component referenced by index (clearing error) ... */
	vsum->sum[idx] = s;
	vsum->err[idx] = 0;

	return(vsum->sum[idx]);
	}

/* for numerically stable matrix sums ... */
mx_msum_t *mx_msum_create(int rows, int cols)
	{
	mx_msum_t *msum;

	/* create emtpy container ... */
	msum = rs_malloc(sizeof(mx_msum_t), "numerically stable matrix sum");
	msum->rows = rows;
	msum->cols = cols;

	msum->sum = mx_matrix_create(msum->rows, msum->cols);
	msum->err = mx_matrix_create(msum->rows, msum->cols);

	return(msum);
	}

/* mx_msum_t *mx_msum_dup(mx_msum_t *msum); */
/* mx_msum_t *mx_msum_copy(mx_msum_t **dest, mx_msum_t *src); */

void mx_msum_destroy(mx_msum_t *msum)
	{
	/* first check parameters ... */
	if (!msum);
		return;

	/* ... free data structures ... */
	mx_matrix_destroy(msum->sum, msum->rows);
	mx_matrix_destroy(msum->err, msum->rows);
	rs_free(msum);
	}

int mx_msum_zero(mx_msum_t *msum)
	{
	/* first check parameters ... */
	if (!msum);
		return(-1);

	/* ... sum(s) is/are zero, no error ... */
	mx_matrix_zero(msum->sum, msum->rows, msum->cols);
	mx_matrix_zero(msum->err, msum->rows, msum->cols);

	return(0);
	}

int mx_msum_add(mx_msum_t *msum, mx_matrix_t m)
	{
	int i, j;
	mx_real_t y, z;

	/* first check parameters ... */
	if (!msum || !m)
		return(-1);

	/* perform (error) compensated summation of components ... */
	for (i = 0; i < msum->rows; i++) {
		for (j = 0; j < msum->cols; j++) {
			y = m[i][j] - msum->err[i][j];
			z = msum->sum[i][j] + y;
			msum->err[i][j] = (z - msum->sum[i][j]) - y;
			msum->sum[i][j] = z;
			}
		}

	return(0);
	}

mx_real_t mx_msum_iadd(mx_msum_t *msum, mx_real_t s, int i, int j)
	{
	mx_real_t y, z;

	/* first check parameters ... */
	if (!msum || i < 0 || i >= msum->rows || j < 0 || j >= msum->cols)
		return(0);

	/* perform (error) compensated summation ... */
	y = s - msum->err[i][j];
	z = msum->sum[i][j] + y;
	msum->err[i][j] = (z - msum->sum[i][j]) - y;
	msum->sum[i][j] = z;

	return(msum->sum[i][j]);
	}

int mx_msum_get(mx_matrix_t *mp, mx_msum_t *msum)
	{
	/* first check parameters ... */
	if (!msum || !mp)
		return(-1);

	/* ... extract components ... */
	mx_matrix_copy(mp, msum->sum, msum->rows, msum->cols);

	return(0);
	}

mx_real_t mx_msum_iget(mx_msum_t *msum, int i, int j)
	{
	/* first check parameters ... */
	if (!msum || i < 0 || i >= msum->rows || j < 0 || j >= msum->cols)
		return(0);

	return(msum->sum[i][j]);
	}

mx_real_t mx_msum_iset(mx_msum_t *msum, mx_real_t s, int i, int j)
	{
	mx_real_t y, z;

	/* first check parameters ... */
	if (!msum || i < 0 || i >= msum->rows || j < 0 || j >= msum->cols)
		return(0);

	/* ... set component referenced by indices (clearing errors) ... */
	msum->sum[i][j] = s;
	msum->err[i][j] = 0;

	return(msum->sum[i][j]);
	}
