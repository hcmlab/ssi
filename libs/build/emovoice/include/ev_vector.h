/**
 * Datei:	vector.h
 * Autor:	org: 		Gernot A. Fink
 *		erweitert: 	Thomas Ploetz
 * Datum:	Thomas Ploetz, Wed May 24 12:07:03 2000
 *
 * Beschreibung:	Definitionen von Datenstrukturen zur Vektormanipulation
 **/

#ifndef _MX_VECTOR_H_INCLUDED_
#define _MX_VECTOR_H_INCLUDED_

#include <stdio.h>

#ifdef MX_KERNEL
#include "ev_real.h"
#else
#include "ev_real.h"
#endif

typedef mx_real_t *mx_vector_t;		/* Vektor ohne und ... */
typedef struct {
	int dim;
	mx_vector_t elems;
	} mx_Vector_t;			/* ... mit Dimensionsinformation */

/* protos */
mx_vector_t mx_vector_create(int dim);
mx_vector_t mx_vector_dup(mx_vector_t vec, int dim);
mx_vector_t mx_vector_copy(mx_vector_t *dest, mx_vector_t src, int dim);
int mx_vector_fprint(FILE *fp, mx_vector_t src, int dim);
int mx_vector_fscan(mx_vector_t *dest, int *dim, FILE *fp);

int mx_vector_zero(mx_vector_t vec, int dim);
void mx_vector_destroy(mx_vector_t vec);
mx_real_t mx_vector_norm(mx_vector_t *unit, mx_vector_t vector, int dim);
void mx_vector_scalprod(mx_real_t   *result, 
			mx_vector_t vector1, 
			mx_vector_t vector2, 			
			int dim);
void mx_vector_sub(mx_vector_t *_result,
		   mx_vector_t vector1, 
		   mx_vector_t vector2, 		   
		   int dim);
void mx_vector_mult(mx_vector_t **_result,
		    mx_vector_t vector1, 
		    mx_vector_t vector2, 		    
		    int dim,
		    int dim2);
void mx_vector_mult_add(mx_vector_t **_result,
			mx_vector_t vector1, 
			mx_vector_t vector2, 			
			int dim,
			int dim2);
void mx_vector_scale(mx_vector_t 	*_result,
		     mx_vector_t 	vector, 
		     mx_real_t 		scalar,		     
		     int dim);
void mx_vector_add(mx_vector_t *_result,
		   mx_vector_t vector1, 
		   mx_vector_t vector2, 		   
		   int dim);
void mx_vector_getavg(mx_vector_t *_avg,
		      mx_vector_t *vectors,		      
		      int dim,
		      int count);
mx_real_t mx_vector_edist2(mx_vector_t v1, mx_vector_t v2, int dim);

#endif /* _MX_VECTOR_H_INCLUDED_ */
