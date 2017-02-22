/**
 * Datei:	vector.c
 * Autor:	org: 		Gernot A. Fink
 *		erweitert: 	Thomas Ploetz
 * Datum:	Thomas Ploetz, Wed May 24 12:02:28 2000
 *
 * Beschreibung:	Routinen zur Vektormanipulation, naemlich:
 *			Erzeugen/Loeschen, Skalieren, Skalar-/Vektorprodukt /
 *			avg bestimmen / Addition / Subtraktion 
 **/

#include <stdio.h>

#include "ev_memory.h"
#include "ev_messages.h"

#define MX_KERNEL
#include "ev_basics.h"
#include "ev_vector.h"
#include "ev_matrix.h"

/**
 * mx_vector_create(dim)
 *	Erzeugt einen Vektor der Laenge 'dim'
 **/
mx_vector_t mx_vector_create(int dim)
{
	mx_vector_t vec;

	vec = rs_calloc(dim, sizeof(mx_real_t), "vector");
	return(vec);
}

int mx_vector_zero(mx_vector_t v, int dim)
	{
	int i;

	/* first check parameters ... */
	if (!v || dim <= 0)
		return(-1);

	/* ... set vector components to zero ... */
	for (i = 0; i < dim; i++)
		v[i] = 0;

	return(0);
	}

/**
 * mx_vector_destroy(vec)
 *	Loescht einen Vektor
 **/
void mx_vector_destroy(mx_vector_t vec)
{
	rs_free(vec);
}

/**
 * mx_vector_dup(vec, dim)
 *	dupliziert den übergebenen 'dim'-dimensionalen
 *	Vektor 'vec' und gibt ihn zurück
 **/
mx_vector_t mx_vector_dup(mx_vector_t vec, int dim) {
        int i;
        mx_vector_t v =  NULL;
  
        if (!vec || dim <= 0)
		return(NULL);

	v = mx_vector_create(dim);
	for (i = 0; i < dim; i++)
		v[i] = vec[i];

	return(v);
}

/**
 * mx_vector_copy(&dest, src, dim)
 *	kopiert die den übergebenen 'dim'-dimensionalen Vektor 'src'
 *	nach 'dest' (der ggf. angelegt wird)und gibt diesen zurück
 **/
mx_vector_t mx_vector_copy(mx_vector_t *dest, mx_vector_t src, int dim)
	{
        int i;
 
	/* first check parameters ... */ 
        if (! dest || !src || dim <= 0)
		return(NULL);

	/* ... evtl. create destination vector ... */
	if (!*dest)
		*dest = mx_vector_create(dim);

	/* ... and copy data from source ... */
	for (i = 0; i < dim; i++)
		(*dest)[i] = src[i];

	return(*dest);
	}

int mx_vector_fprint(FILE *fp, mx_vector_t src, int dim)
	{
	int i, n_elems = 0;        
	
	if (!fp || !src || dim <= 0)
		return(-1);

	/* Header ausgeben */
	fprintf(fp, "%c %d-dimensional vector\n",
		MX_COMMENT_CHAR, dim);

	/* Vektor ausgeben */
	for (i = 0; i < dim; i++) {
		fprintf(fp, "%g%c",
			src[i], (i < (dim - 1)) ? ' ' : '\n');
		n_elems++;
		}

	return(n_elems);
	}

int mx_vector_fscan(mx_vector_t *dest, int *dim, FILE *fp)
	{
	rs_error("sorry - not implemented, yet.");
	}


/**
 * mx_vector_scale(result, vector, scalar, dim)
 *	Skaliert einen 'dim'-dimensionalen Vektor 'vector' 
 *	komponentenweise mit 'scalar' in 'result'
 **/
void mx_vector_scale(mx_vector_t 	*_result,
		     mx_vector_t 	vector, 
		     mx_real_t 		scalar,
		     int dim) 
{
	register int i;	
	mx_vector_t result;

	/* ggf. Ergebnisvektor erzeugen */
	if(!*_result)
		*_result = mx_vector_create(dim);
	result = *_result;
	for(i=0;i<dim;i++)   		
		result[i]=scalar*vector[i]; 
}

/**
 * mx_vector_norm(unit, vector, dim)
 *	Berechnet den Betrag des 'dim'-dimensionalen Vektors 'vector'
 *	und liefert diesen als Rueckgabewert.
 *
 *	Sofern der Betrag 'norm' nicht verschwindet, wird der Eingabevektor
 *	ggf. (d.h. falls 'unit != NULL') auf Betrag 1 normiert und der
 *	normierte Einheitsvektor 'unit' als Ergebnis geliefert.
 **/
mx_real_t mx_vector_norm(mx_vector_t *unit, mx_vector_t vector, int dim) 
	{
	int i;
	mx_real_t norm = 0;

	/* ggf. Ergebnisvektor erzeugen */
	if(unit && !*unit)
		*unit = mx_vector_create(dim);

	/* ... Vektornorm berechnen ... */
	for(i = 0; i < dim; i++)
		norm += vector[i] * vector[i];
	norm = sqrt(norm);

	/* ... sofern normierter Vektor zu berechnen ist ... */
	if (unit) {
		/* ... Betrag pruefen ... */
		if (norm < MX_REAL_LOW)
			rs_error("can't normalize vanishing vector!");

		/* ... und Einheitsvektor erzeugen */
		for(i = 0; i < dim; i++)
			(*unit)[i] = vector[i] / norm;
		}

	/* ... und Vektorbetrag als Ergebnis liefern */
	return(norm);
	}

/**
 * mx_vector_add(result, vector1, vector2, dim)
 *	Addiert die 'dim'-dimensionalen Vektoren 'vector1' und
 *	'vector2' in 'result'
 **/
void mx_vector_add(mx_vector_t *_result,
		   mx_vector_t vector1, 
		   mx_vector_t vector2, 
		   int dim) 
{	 
	register int i;		 
	mx_vector_t result;

	/* ggf. Ergebnisvektor erzeugen */
	if(!*_result)
		*_result = mx_vector_create(dim);
	result = *_result;
	for(i=0;i<dim;i++)   			 
		result[i]=vector1[i]+vector2[i]; 
}	

/**
 * mx_vector_getavg(avg, vectors, dim, count)
 *	Bestimmt aus den 'count' vielen 'dim'-dimensionalen
 *	Vektoren in 'vectors' den Mittelvektor in 'avg'
 **/
void mx_vector_getavg(mx_vector_t *_avg,
		      mx_vector_t *vectors,		      
		      int dim,
		      int count)
{
	register int i,k;
	mx_vector_t vect, avg;

	/* ggf. Ergebnisvektor erzeugen */
	if(!*_avg)
		*_avg = mx_vector_create(dim);
	avg = *_avg;
	for(i=0;i<dim;i++)
		avg[i]=0.00;
	for(i=0;i<count;i++) {
		vect = vectors[i];
		mx_vector_add(&avg,vect,avg,dim);
	}   
	mx_vector_scale(&avg,avg,(double)1.0/(double)count,dim);
}

/**
 * mx_vector_scalprod(result, vector1, vector2, dim)
 *	Berechnet das Skalarprodukt der 'dim'-dimensionalen Vektoren
 *	'vector1' und 'vector2' in 'result'
 **/
void mx_vector_scalprod(mx_real_t   *result,
			mx_vector_t vector1, 
			mx_vector_t vector2, 			
			int dim)
{	 
	register int i;					 

	*result=0.00;
	for(i=0;i<dim;i++) 				 
		*result+=vector1[i]*vector2[i];
}		

/**
 * mx_vector_sub(result, vector1, vector2, dim)
 *	Berechnet den Differenzvektor aus den 'dim'-dimensionalen
 *	Vektoren 'vector1' und 'vector2' in 'result
 **/
void mx_vector_sub(mx_vector_t *_result,
		   mx_vector_t vector1, 
		   mx_vector_t vector2, 		   
		   int dim) 
{	 
	register int i;					 
	mx_vector_t result;

	/* ggf. Ergebnisvektor erzeugen */
	if(!*_result)
		*_result = mx_vector_create(dim);
	result = *_result;
	for(i=0;i<dim;i++)   			 
		result[i]=vector1[i]-vector2[i]; 
}

/**
 * mx_vector_mult(result, vector1, vector2, dim, dim2)
 *	Berechnet das Vektorprodukt der 'dim'- bzw. 'dim2'-dimensionalen Vektoren
 *	'vector1' und 'vector2' in der 'dim'x'dim2'-dimensionalen Matrix
 *	'result'
 **/
void mx_vector_mult(mx_matrix_t *_result,
		    mx_vector_t vector1, 
		    mx_vector_t vector2, 		    
		    int dim,
		    int dim2)
{
	register int i;
	register int j;
	mx_vector_t *result;

	/* ggf. Ergebnisvektor erzeugen */
	if(!(*_result))
		*_result = mx_matrix_create(dim, dim2);
	result = *_result;

	for(i = 0; i < dim; i++) 
		for(j = 0; j < dim2; j++) 
			result[i][j]=vector1[i]*vector2[j];
}

/**
 * mx_vector_mult_add(result, vector1, vector2, dim, dim2)
 *	Berechnet das Vektorprodukt der 'dim'- bzw. 'dim2'-dimensionalen Vektoren
 *	'vector1' und 'vector2' in der 'dim'x'dim2'-dimensionalen Matrix
 *	'result' - addiert jedoch die Ergebnisse zu den einzelnen Komponenten
 *      der evtl. uebergebenen 'result'-Matrix
 **/
void mx_vector_mult_add(mx_vector_t **_result,
			mx_vector_t vector1, 
			mx_vector_t vector2, 			
			int dim,
			int dim2)
{
	register int i;
	register int j;
	mx_vector_t *result;

	/* ggf. Ergebnisvektor erzeugen */
	if(!*_result) {
		*_result = rs_malloc(dim*sizeof(mx_real_t *), "matrix");
		for (i = 0; i < dim; i++)
			(*_result)[i] = rs_calloc(dim2, sizeof(mx_real_t), "matrix row");
	}
	result = *_result;
	for(i = 0; i < dim; i++) 
		for(j = 0; j < dim2; j++) 
			result[i][j]+=vector1[i]*vector2[j];
}

/**
 * mx_vector_edist2(v1, v2, dim)
 *	Berechnet den quadratischen euklidischen Abstand der 'dim'-dimensionalen
 *	Vektoren 'v1' und 'v2'.
 **/
mx_real_t mx_vector_edist2(mx_vector_t v1, mx_vector_t v2, int dim)
	{
	int i;
	mx_real_t dist2 = 0.0;

	for (i = 0; i < dim; i++)
		dist2 += mx_sqr(v1[i] - v2[i]);

	return(dist2);
	}
