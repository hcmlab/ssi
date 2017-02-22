/**
* Datei:	matrix.h
* Autor:	Gernot A. Fink
* Datum:	17.4.1998
*
* Beschreibung:	Definitionen von Datenstrukturen zur Matrixmanipulation
**/

#ifndef _MX_MATRIX_H_INCLUDED_
#define _MX_MATRIX_H_INCLUDED_

#include <stdio.h>

#ifdef MX_KERNEL
#include "ev_real.h"
#include "ev_vector.h"
#include "ev_eigen.h"
#else
#include "ev_real.h"
#include "ev_vector.h"
#include "ev_eigen.h"
#endif

typedef mx_real_t **mx_matrix_t;	/* Matrix ohne und ... */
typedef struct {
	int rows, cols;
	mx_matrix_t elems;
	} mx_Matrix_t;			/* ... mit Dimensionsinformation */

/**
* Funktionsprototypen
**/

mx_matrix_t mx_matrix_create(int rows, 
			     int cols);
mx_matrix_t mx_matrix_dup(mx_matrix_t src,
			int rows, int cols);
mx_matrix_t mx_matrix_copy(mx_matrix_t *dest, mx_matrix_t src,
			int rows, int cols);
int mx_matrix_fprint(FILE *fp, mx_matrix_t mat,
		     int rows, int cols,
		     char *comment);
int mx_matrix_fscan(mx_matrix_t *mat,
		    int *rows, int *cols,
		    FILE *fp);
int mx_matrix_zero(mx_matrix_t m, int rows, int cols);
void mx_matrix_destroy(mx_matrix_t mat, 
		       int rows);

void mx_matrix_mul(mx_matrix_t *AxB_p,
		   mx_matrix_t A, 
		   mx_matrix_t B, 
		   int rows1, 
		   int cols1, 
		   int cols2);
void mx_matrix_mul_add(mx_matrix_t *AxB_p,
		       mx_matrix_t A, 
		       mx_matrix_t B, 
		       int rows1, 
		       int cols1, 
		       int cols2);
void mx_matrix_mulv(mx_vector_t *Axv_p,
		    mx_matrix_t A, 
		    mx_vector_t v, 
		    int rows, 
		    int cols);
void mx_matrix_muls(mx_matrix_t *As_p, mx_matrix_t A, mx_real_t s,
			int rows, int cols);
void mx_matrix_sub(mx_matrix_t 	*result,
		   mx_matrix_t 	matrix1, 
		   mx_matrix_t 	matrix2, 		   
		   int		rows,
		   int		cols);
void mx_matrix_add(mx_matrix_t 	*result,
		   mx_matrix_t 	matrix1, 
		   mx_matrix_t 	matrix2, 		   
		   int		rows,
		   int		cols);
void mx_matrix_wadd(mx_matrix_t	*aA_plus_bB,
		   mx_real_t a, mx_matrix_t A,
		   mx_real_t b, mx_matrix_t B,
		   int rows, int cols);
void mx_matrix_transp(mx_matrix_t *result,
		      mx_matrix_t matrix,		      
		      int rows, 
		      int cols);
mx_real_t mx_matrix_invert(mx_matrix_t *inv_p, 
			   mx_matrix_t mat, 
			   int dim);
void mx_matrix_gausselim(mx_matrix_t *_result,
			 mx_matrix_t A,
			 int rows,
			 int cols);
int mx_matrix_linsolve(mx_vector_t *_x,
		       mx_matrix_t A,
		       mx_vector_t b,
		       int dim);

void mx_matrix_scale(mx_matrix_t 	*result,
		     mx_matrix_t 	matrix, 
		     mx_real_t 		scalar,		     
		     int 		rows,
		     int		cols);

void mx_matrix_getcov(mx_matrix_t *cov,
		      mx_vector_t *data,
		      mx_vector_t mean, /* means in all dims */
		      int no_smpl, 
		      int dim);

mx_real_t mx_matrix_mdist(mx_vector_t v1, 
			  mx_vector_t v2, 
			  mx_matrix_t C, 
			  mx_vector_t diag_v,
			  int dim,
			  int diag,
			  int inv);

#endif /* _MX_MATRIX_H_INCLUDED_ */
