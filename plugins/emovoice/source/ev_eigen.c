/**
 * Datei:	        eigen.c
 * Autor:	        Thomas Ploetz, Tue Aug  7 17:08:59 2001
 * Time-stamp:          <04/07/27 15:11:15 tploetz>
 *
 * Beschreibung:	Routinen zur Eigenwertberechnung
 **/

#include "ev_memory.h"
#include "ev_messages.h"

#include <stdio.h>

#define MX_KERNEL 
#include "ev_matrix.h"

#define LOCAL_EPSILON		(1e-5)
#define VONMISES_THRESH		0.1
#define MAXITER                 50000

#ifndef M_PI
#define M_PI   3.14159265
#endif
#ifndef M_PI_2
#define M_PI_2   1.57079
#endif
#ifndef M_PI_4
#define M_PI_4   0.78539
#endif

typedef struct
{
	mx_real_t val;
	int org_pos;
} ew_sort_t;

#define __mfill(mp,_dim,r,c,exp)  \
	{					\
	for ((r)=0; (r)<(_dim); (r)++)		\
		for ((c)=0; (c)<(_dim); (c)++)	\
			(mp)[r][c]=(exp);	\
	}

/* Prototypes */
static mx_real_t _matrix_pivot(mx_matrix_t a, int dim, int *pr, int *pc);
static void _matrix_mrot(mx_matrix_t a, int dim, int p, int q, mx_real_t phi);
static void _matrix_mrotr(mx_matrix_t a, int dim, int p, int q, mx_real_t phi);
static int _ew_sort_f(const void *a, const void *b);

static int _matrix_deflate(mx_matrix_t *result, 
			   mx_matrix_t A, mx_vector_t v, 
			   mx_real_t lambda, int dim);
static int _vonMises_Iteration(mx_vector_t *result, 
			       mx_real_t *lambda, mx_matrix_t A, 
			       mx_vector_t startvec,
			       mx_real_t threshold, int maxiter, int dim);
static int _vonMises_Step(mx_vector_t *vneu, mx_real_t *lambda, mx_matrix_t A, 
			  mx_vector_t vold, int dim);

/****************************************** Start ****************************/

/**
 * mx_matrix_eigenv_n(_ew, _ev, _a, dim, n)
 *      berechnet die ersten 'n' Eigenwrte bzw. -vektoren von '_a'
 *      und gibt diese in '_ew' bzw. '_ev' zurueck
 *      gibt die (auf 1 normierten) Eigenvektoren spaltenweise in 'ev' 
 *	zurueck, dabei Sortierung nach Eigenwerten
 **/
int mx_matrix_eigenv_n(mx_real_t **_ew, mx_matrix_t *_ev, mx_matrix_t _a, 
		       int dim, int n)
	{
	mx_real_t *ew;
	mx_matrix_t a, ev, __a, calcW;
	mx_vector_t startvec = mx_vector_create(dim);
	int i, j;
	int maxiter = MAXITER;
	mx_real_t threshold = VONMISES_THRESH;
	int n_ew = n;
	ew_sort_t *ew_sort = NULL;

	if (n_ew > dim) {
		rs_warning("# requested eigenvects (%d) larger than dim (%d), "
			   "bounding to %d",
			   n_ew, dim, dim);
		n_ew = dim;
		}

	/* ggf. Ergebnisvektor erzeugen */
	if(!*_ew)
		*_ew = mx_vector_create(n_ew);
	ew = *_ew;

	/* Rechenmatrix erzeugen */
	calcW = mx_matrix_create(dim, dim);

	a = _a;
	__a = mx_matrix_create(dim, dim);

	startvec[0] = 1;
	for (j = 1; j < dim; j++)
		startvec[j] = j;

	for (i = 0; i < n_ew; i++) {
		/* ... i-ter Eigenvektor ... */
		if(!_vonMises_Iteration(&(calcW[i]),
					&(ew[i]),
					a,
					startvec,
					threshold,
					maxiter,
					dim))
			break;

		/* ... Originalmatrix bzgl. i-tem Eigen{wert,vektor} 
		   deflatieren ... */
		_matrix_deflate(&__a,
				a,
				calcW[i],
				ew[i],
				dim);

		a = __a;
	}

	if (i < n_ew) {
		rs_warning("vonMises-Iteration aborted after %d eigenvals", i);
		n_ew = i;
	}

	/* ... cleanup ... */
	mx_matrix_destroy(__a, dim);
	mx_vector_destroy(startvec);

	/* ... Eigenvektoren normieren ... */
	for (i = 0; i < n_ew; i++)
		mx_vector_norm(&(calcW[i]), calcW[i], dim);

	/* ... spaltenweises return! ... */
	a = mx_matrix_dup(calcW, dim, dim);
	mx_matrix_transp(&calcW, a, dim, dim);
	
	/* ... sortieren ... */
	ew_sort = rs_malloc(dim*sizeof(ew_sort_t), "ew_sort");
	for(i = 0; i < n_ew; i++) {
		ew_sort[i].val = ew[i];
		ew_sort[i].org_pos = i;
	}
	qsort(ew_sort, n_ew, sizeof(ew_sort_t), _ew_sort_f);

	mx_matrix_destroy(a, dim);
	a = mx_matrix_dup(calcW, dim, dim);

	for (i = 0; i < dim; i++)
		for(j = 0; j < n_ew; j++)
			calcW[i][j] = a[i][ew_sort[j].org_pos];
	for(i = 0; i < n_ew; i++)
		ew[i] = ew_sort[i].val;

	mx_matrix_destroy(a, dim);
	free(ew_sort);

	/* ggf. Ergebnismatrix erzeugen */
	if(!*_ev) 
		*_ev = mx_matrix_create(dim, n_ew);
	ev = *_ev;

	for (i = 0; i < dim; i++)
		for (j = 0; j < n_ew; j++)
			ev[i][j] = calcW[i][j];

	mx_matrix_destroy(calcW, dim);

	return(n_ew);
	}


/**
 * mx_matrix_eigenv(ew, ev, _a, dim)
 *	berechnet Eigenwerte 'ew' und Eigenvektoren 'ev' der 'dim' dimensionalen
 *	matrix '_a' (Jacoby-Rot)
 *      gibt die (auf 1 normierten) Eigenvektoren spaltenweise in 'ev' 
	zurueck, dabei Sortierung nach Eigenwerten
 **/
int mx_matrix_eigenv(mx_real_t **_ew, mx_matrix_t *_ev, mx_matrix_t _a, int dim)
	{
	int r,c,p,q,i,j;
	mx_real_t phi, *ew;
	mx_matrix_t a, ev;
	ew_sort_t *ew_sort = NULL;

	/* ggf. Ergebnismatrix erzeugen */
	if(!*_ev) 
		*_ev = mx_matrix_create(dim, dim);
	ev = *_ev;
	/* ggf. Ergebnisvektor erzeugen */
	if(!*_ew)
		*_ew = mx_vector_create(dim);
	ew = *_ew;
	a = mx_matrix_dup(_a, dim, dim);

	__mfill(ev,dim,r,c,(r == c) ? 1 : 0);

	for (p=1; p<dim-1; p++)
		for (q=p+1; q<dim; q++) { 
			if(a[p-1][p] == 0) {	    
				phi = (a[p-1][q] < 0) ? 
					M_PI_2 : ((a[p-1][q] == 0) ? 
						  0 : -M_PI_2);
				rs_warning("mx_matrix_eigenw(): zero value in "
					   "Jacobi-Rot, bounding phi to "
					   "default! (%f/%f/%f)\n", 
					a[p-1][p],
					a[p-1][q],
					phi);
			} else
				phi=atan(-a[p-1][q]/a[p-1][p]);
			_matrix_mrot(a,dim,p,q,phi);
			_matrix_mrotr(ev,dim,p,q,phi);
		}			
	while (_matrix_pivot(a,dim,&p,&q) > LOCAL_EPSILON) {
		/*       fprintf(stderr, "pivot: %d / %d\n", p ,q); */
		/* emergency break; atan is defined in [-Pi/2...Pi/2]
		   set phi manually */
		if(a[q][q] == a[p][p]) {
			phi = (a[p][q] < 0) ? 
				-M_PI_4 : ((a[p][q] == 0) ? 
					   0 : M_PI_4);
				rs_warning("mx_matrix_eigenw(): zero value in "
					   "Jacobi-Rot, bounding phi to "
					   "default! (%f/%f/%f/%f)\n", 
				a[p][q],
				a[q][q],
				a[p][p],
				phi);
		} else
			/* see Numerical Recipes 11.1.8 and cot->tan */
			phi=0.5*atan(2.0*a[p][q]/(a[q][q]-a[p][p]));
		_matrix_mrot(a,dim,p,q,phi);
		_matrix_mrotr(ev,dim,p,q,phi);
	}
	for(i=0;i<dim;i++)
		ew[i] = (fabs(a[i][i]) < LOCAL_EPSILON) ? 0.00 : (a[i][i]);

	/* sort ew/ev */
	ew_sort = rs_malloc(dim*sizeof(ew_sort_t), "ew_sort");
	for(i=0;i<dim;i++) {
		ew_sort[i].val = ew[i];
		ew_sort[i].org_pos = i;
	}
	qsort(ew_sort, dim, sizeof(ew_sort_t), _ew_sort_f);
	mx_matrix_destroy(a, dim);
	a = mx_matrix_dup(ev, dim, dim);
	for(i=0;i<dim;i++)
		for(j=0;j<dim;j++)
			ev[i][j] = a[i][ew_sort[j].org_pos];
	for(i=0;i<dim;i++)
		ew[i] = ew_sort[i].val;
	mx_matrix_destroy(a, dim);

	free(ew_sort);
	return(dim);
	}


/**
 * _matrix_deflate(result, A, v, lambda, dim)
 *     Deflation der Matrix 'A' ['dim' x 'dim'] mit Vektor 'v' ['dim']
 *     und float 'a'; Ergebnis in 'result' ['dim' x 'dim']
 *     Verfahren: leicht erweitertes Hotelling (anstelle der Skalierung
 *     von vvT mit -lambda erfolgt diese mit -lambda/vTv)
 **/
static int _matrix_deflate(mx_matrix_t *result, mx_matrix_t A, 
			   mx_vector_t v, mx_real_t lambda, int dim)
	{
	mx_matrix_t defl = mx_matrix_create(dim, dim);
	mx_matrix_t v_vT = mx_matrix_create(dim, dim);
	mx_real_t nenner;

	mx_vector_scalprod(&nenner, v, v, dim);
	mx_vector_mult(&v_vT, v, v, dim, dim);
	nenner = -lambda / nenner;
	mx_matrix_scale(&defl, v_vT, nenner, dim, dim);
	mx_matrix_add(result, A, defl, dim, dim);
	mx_matrix_destroy(defl, dim);
	mx_matrix_destroy(v_vT, dim);	

	return(1);
	}

/**
 * _vonMises_Iteration(result, lambda, A, startvec, threshold, maxiter, dim)
 *     eine komplette von Mises-Iteration auf der Matrix 'A' ['dim' x 'dim']
 *     ergibt den zum groessten Eigenwert 'lambda' gehoerenden Eigenvektor 
 *     'result' ['dim'] bei Vorgabe der Konvergenzschwelle 'threshold'
 **/
static int _vonMises_Iteration(mx_vector_t *result, mx_real_t *lambda, 
			       mx_matrix_t A, mx_vector_t startvec,
			       mx_real_t threshold, int maxiter, int dim)
	{
 	int iter = 0, i;
	mx_real_t dist, _dist;
	mx_vector_t oldvec = mx_vector_create(dim);

	for (i = 0; i < dim; i++)
		oldvec[i] = startvec[i];

	do {
		dist = _dist = 0.00;
		if (_vonMises_Step(result, lambda, A, oldvec, dim) <= 0) {
			dist = MX_REAL_MAX;
			break;
		}

		for (i = 0; i < dim; i++) {
			_dist = fabs((*result)[i]) - fabs(oldvec[i]);
			dist +=  _dist * _dist;
		}
		dist = sqrt(dist);

		for (i = 0; i < dim; i++) {
			oldvec[i] = (*result)[i];
		}
		
		if (dist < threshold) 
			break;
	} while (iter++ < maxiter);

	if (dist > threshold) {
		rs_warning("Abort in vonMises-Iteration (%f)", dist);
		mx_vector_destroy(oldvec);
		return(0);
		}

	mx_vector_destroy(oldvec);
	return(1);
	}

/**
 * _vonMises_Step(vneu, lambda, A, vold, dim)
 *	ein 'von Mises'-Schritt auf 'A' ['dim' x 'dim'] mit dem
 *	bisherigen Ergebnisvektor 'vold' ['dim']; Ergebnis in 'vneu' ['dim'] 
 *	bzw. 'lambda'
 **/
static int _vonMises_Step(mx_vector_t *vneu, mx_real_t   *lambda, 
			  mx_matrix_t A, mx_vector_t vold, int dim)
	{
	int i;
	
	mx_matrix_mulv(vneu, A, vold, dim, dim);
	*lambda = -MX_REAL_MAX;
	for (i = 0; i < dim; i++)
		*lambda = (*lambda > (*vneu)[i]) ? *lambda : (*vneu)[i];
	if (!*lambda)
		return(-1);
	mx_vector_scale(vneu, *vneu, 1/ *lambda, dim);
		
	return(1);
	}
			  
/**
 * _matrix_pivot(a,dim,pr,pc)
 *	Hilfsfunktion; liefert das groesste Element der symmetrischen 
 *	'dim'x'dim' Matrix 'a' als Pivotelement als Indizes in 'pr' und 'pc'
 **/
static mx_real_t _matrix_pivot(mx_matrix_t a, int dim, int *pr, int *pc)
	{
	int r,c;
	mx_real_t pv;
	
	pv=0;
	*pr=*pc=0;
	/* obere Dreiecksmatrix nach groesstem Element absuchen */
	for (r=0; r<dim-1; r++)
		for (c=r+1; c<dim; c++)
			if (fabs(a[r][c]) > pv) {
				pv=fabs(a[r][c]);
				*pr=r;
				*pc=c;
			}
	return(pv);
	}

/**
 * _matrix_mrot(a, dim, p , q, phi)
 *     Hilfsfunktion; NR
 **/
static void _matrix_mrot(mx_matrix_t a, int dim, int p, int q, mx_real_t phi)
	{
	int i,j;
	mx_real_t aip,aiq,sinphi,cosphi,tanphi,tau;
	mx_real_t apj,aqj;
	
	/* see Numerical Recipes 11.1.4 */
	sinphi=sin(phi);
	cosphi=cos(phi);
	tanphi=tan(phi);
	tau=tanphi/2.0;

	/* ORG GERNOT */
	/* see Numerical Recipes 11.1.4 */
	sinphi=sin(phi);
	cosphi=cos(phi);
	for (i=0; i<dim; i++) {
		aip=a[i][p];
		aiq=a[i][q];
		a[i][p]=aip*cosphi-aiq*sinphi;
		a[i][q]=aip*sinphi+aiq*cosphi;
	}
	for (j=0; j<dim; j++) {
		apj=a[p][j];
		aqj=a[q][j];
		a[p][j]=apj*cosphi-aqj*sinphi;
		a[q][j]=apj*sinphi+aqj*cosphi;
	}
	}

/**
 * _matrix_mrotr(a, dim, p, q, phi)
 *     Hilfsfunktion
 **/
static void _matrix_mrotr(mx_matrix_t a, int dim, int p, int q, mx_real_t phi)
	{
	int i;
	mx_real_t aip,aiq,sinphi,cosphi;
	
	sinphi=sin(phi);
	cosphi=cos(phi);
	/* see Numerical Recipes 11.1.24 */
	for (i=0; i<dim; i++) {
		aip=a[i][p];
		aiq=a[i][q];
		a[i][p]=aip*cosphi-aiq*sinphi;
		a[i][q]=aip*sinphi+aiq*cosphi;
	}
	}


/**
 * _ew_sort_f(a,b)
 *	Sortierroutine fuer qsort der Eigenwerte
 **/
static int _ew_sort_f(const void *a, const void *b) 
	{
	ew_sort_t *A = (ew_sort_t*)a;
	ew_sort_t *B = (ew_sort_t*)b;

	if(A->val < B->val)
		return(1);
	if(A->val > B->val)
		return(-1);
	return(0);
	}

/************************************ EOF eigen.c ****************************/
