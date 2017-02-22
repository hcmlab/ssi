/**
* File:		ssum.h
* Author:	Gernot A. Fink
* Date:		19.5.2005 
*
* Description:	Definition of data structures and function prototypes
*		for numerically stable summation
**/

#ifndef __MX_SUM_H_INCLUDED__
#define __MX_SUM_H_INCLUDED__

#include <stdio.h>

#ifdef MX_KERNEL
#include "ev_real.h"
#include "ev_vector.h"
#include "ev_matrix.h"
#else
#include "ev_real.h"
#include <mx/vector.h>
#include <mx/matrix.h>
#endif

/*
 * Data Types
 */
/* for numerically stable scalar sum ... */
typedef struct {
	int dim;
	mx_real_t sum;
	mx_real_t err;
	} mx_ssum_t;

/* for numerically stable vector sums ... */
typedef struct {
	int dim;
	mx_vector_t sum;
	mx_vector_t err;
	} mx_vsum_t;

/* for numerically stable matrix sums ... */
typedef struct {
	int rows, cols;
	mx_matrix_t sum;
	mx_matrix_t err;
	} mx_msum_t;

/*
 * Function Prototypes 
 */
/* for numerically stable scalar sum ... */
mx_ssum_t *mx_ssum_create(void);
/* mx_ssum_t *mx_ssum_dup(mx_ssum_t *ssum); */
/* mx_ssum_t *mx_ssum_copy(mx_ssum_t **dest, mx_ssum_t *src); */
void mx_ssum_destroy(mx_ssum_t *ssum);
int mx_ssum_zero(mx_ssum_t *ssum);

mx_real_t mx_ssum_add(mx_ssum_t *ssum, mx_real_t s);

mx_real_t mx_ssum_get(mx_ssum_t *ssum);

/* for numerically stable vector sums ... */
mx_vsum_t *mx_vsum_create(int dim);
/* mx_vsum_t *mx_vsum_dup(mx_vsum_t *vsum); */
/* mx_vsum_t *mx_vsum_copy(mx_vsum_t **dest, mx_vsum_t *src); */
void mx_vsum_destroy(mx_vsum_t *vsum);
int mx_vsum_zero(mx_vsum_t *vsum);

int mx_vsum_add(mx_vsum_t *vsum, mx_vector_t v);
mx_real_t mx_vsum_iadd(mx_vsum_t *vsum, mx_real_t s, int idx);
int _mx_vsum_add_cprod(mx_vsum_t *vsum, mx_vector_t v1, mx_vector_t v2);

int mx_vsum_get(mx_vector_t *vp, mx_vsum_t *vsum);
mx_real_t mx_vsum_iget(mx_vsum_t *vsum, int idx);
mx_real_t mx_vsum_iset(mx_vsum_t *vsum, mx_real_t s, int idx);

/* for numerically stable matrix sums ... */
mx_msum_t *mx_msum_create(int rows, int cols);
/* mx_msum_t *mx_msum_dup(mx_msum_t *msum); */
/* mx_msum_t *mx_msum_copy(mx_msum_t **dest, mx_msum_t *src); */
void mx_msum_destroy(mx_msum_t *msum);
int mx_msum_zero(mx_msum_t *msum);

int mx_msum_add(mx_msum_t *msum, mx_matrix_t m);
mx_real_t mx_msum_iadd(mx_msum_t *msum, mx_real_t s, int i, int j);

int mx_msum_get(mx_matrix_t *mp, mx_msum_t *msum);
mx_real_t mx_msum_iget(mx_msum_t *msum, int i, int j);
mx_real_t mx_msum_iset(mx_msum_t *msum, mx_real_t s, int i, int j);

#endif /* __MX_SUM_H_INCLUDED__ */
