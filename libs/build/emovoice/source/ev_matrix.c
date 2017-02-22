/**
 * Datei:	matrix.c
 * Autor:	Gernot A. Fink
 *		(nach einem alten Modul vom 26.2.1990 mit Anregungen
 *		 von Ernst-Guenther Schukat-Talamazzini und Franz Kummert)
 * Datum:	17.4.1998 (14.7.1997)
 *
 * Beschreibung:	Routinen zur Matrixmanipulation, naemlich:
 *			Erzeugen/Loeschen, Invertieren, Multiplizieren, 
 *			Eigenwerte/Eigenvektoren berechnen
 **/

#include "ev_memory.h"
#include "ev_messages.h"
#include "ev_io.h"

#include <stdio.h>

#define MX_KERNEL 
#include "ev_basics.h"
#include "ev_matrix.h"
#include "ev_eigen.h"

#define LOCAL_SMALL_PIVOT	0.00001
#define LOCAL_EPSILON		(1e-10)
#define LOCAL_GAUSSEPS          (1e-5)


static void _matrix_normline(mx_matrix_t mat, 
			     int cols,
			     int line, 
			     mx_real_t factor);
static void _matrix_sublines(mx_matrix_t mat, 
			     int cols,
			     int line, 
			     int subline, 
			     mx_real_t factor);
static void _matrix_swaplines(mx_matrix_t mat, 
			      int line1, 
			      int line2);


/**
 * mx_matrix_getcov(cov, data, mean, no_smpl, dim)
 *	berechnet die 'dim'x'dim'-dimensionale Kovarianzmatrix 'cov' fuer 
 *	gegebene Daten 'data' ('no_smpl'-viele), benutzt wenn vorhanden
 *	'mean' als Mittelwertvektor sonst eigene Berechnung
 **/
void mx_matrix_getcov(mx_matrix_t *cov,
		      mx_vector_t *data,
		      mx_vector_t mean, /* means in all dims */
		      int no_smpl, 
		      int dim)		      
{
	int i, j, n;
	mx_vector_t _mean = NULL;
	mx_matrix_t mu_mut = mx_matrix_create(dim, dim);

	/* ggf. Ergebnismatrix erzeugen */
	if(!*cov)
		*cov = mx_matrix_create(dim, dim);
	if(mean) {
		for(n=0;n<no_smpl;n++)
			mx_vector_mult_add(cov, data[n], data[n], dim, dim);  /* BUG!!! */
		mx_vector_mult(&mu_mut, mean, mean, dim, dim);
	} else {
		_mean = mx_vector_create(dim);
		for(n=0;n<no_smpl;n++) {
			mx_vector_mult_add(cov, data[n], data[n], dim, dim);   /* BUG */
			mx_vector_add(&_mean, data[n], _mean, dim);
		}
		mx_vector_scale(&_mean, _mean, (mx_real_t)(1.00/(mx_real_t)no_smpl), dim);
		mx_vector_mult(&mu_mut, _mean, _mean, dim, dim);
		mx_vector_destroy(_mean);
	}
	mx_matrix_scale(cov, *cov, (mx_real_t)(1.00/(mx_real_t)no_smpl), dim, dim);
	mx_matrix_sub(cov, *cov, mu_mut, dim, dim);
	mx_matrix_destroy(mu_mut, dim);
}

/**
* mx_matrix_create(rows, cols)
*	Erzeugt eine 'rows'x'cols'-Matrix
**/
mx_matrix_t mx_matrix_create(int rows, int cols)
{
	mx_matrix_t mat;
	int i;

	mat = rs_malloc(rows * sizeof(mx_real_t *), "matrix");
	for (i = 0; i < rows; i++)
		mat[i] = rs_calloc(cols, sizeof(mx_real_t), "matrix row");
	return(mat);
}

/**
* mx_matrix_dup(src, rows, cols)
*	Erzeugt eine Kopie der 'rows'x'cols'-Matrix 'src'
**/
mx_matrix_t mx_matrix_dup(mx_matrix_t src, int rows, int cols)
{
	mx_matrix_t mat;
	int i, j;
	
	mat = mx_matrix_create(rows, cols);
	for (i = 0; i < rows; i++)
		for (j = 0; j < cols; j++)
			mat[i][j] = src[i][j];
	return(mat);
}

mx_matrix_t mx_matrix_copy(mx_matrix_t *dest, mx_matrix_t src,
		int rows, int cols)
	{
        int i, j;        
 
	/* first check parameters ... */ 
        if (! dest || !src || rows <= 0 || cols <= 0)
		return(NULL);

	/* ... evtl. create destination matrix ... */
	if (!*dest)
		*dest = mx_matrix_create(rows, cols);

	/* ... and copy data from source ... */
	for (i = 0; i < rows; i++)
		for (j = 0; j < cols; j++)
			(*dest)[i][j] = src[i][j];

	return(*dest);
	}

int mx_matrix_zero(mx_matrix_t m, int rows, int cols)
	{
	int i, j;

	/* first check parameters ... */
	if (!m || rows <= 0 || cols <= 0)
		return(-1);

	/* ... set all matrix elements to zero ... */
	for (i = 0; i < rows; i++)
		for (j = 0; j < cols; j++)
			m[i][j] = 0;

	return(0);
	}

/**
* mx_matrix_destroy(mat, rows)
*	Loescht eine mit 'mx_matrix_create()' erzeugte 'rows'x?-Matrix
**/
void mx_matrix_destroy(mx_matrix_t mat, int rows)
{
	int i;

	if (!mat)
		return;
	for (i = 0; i < rows; i++)
		rs_free(mat[i]);
	rs_free(mat);
}


/**
* mx_matrix_mul(&AxB, A, B, rows1, cols1, cols2)
*	Multipliziert die 'rows1'x'cols1' Matrix 'A' mit der
*	'cols1'x'cols2' Matrix 'B' und liefert das 'rows1'x'cols2'
*	Ergebnis in '&AxB'.
**/
void mx_matrix_mul(mx_matrix_t *AxB_p,
		   mx_matrix_t A, mx_matrix_t B, int rows1, int cols1, int cols2)
{
	mx_matrix_t AxB;
	int i, j, k;

	/* ggf. Ergebnis-Matrix erzeugen ... */
	if (*AxB_p == NULL)
		*AxB_p = mx_matrix_create(rows1, cols2);
	AxB = *AxB_p;
	/* ... und Multiplikation durchfuehren */
	for (i = 0; i < rows1; i++)
		for (j = 0; j < cols2; j++) {
			AxB[i][j] = 0.0;
			for (k = 0; k < cols1; k++)
				AxB[i][j] += A[i][k] * B[k][j];
		}
}

/**
* mx_matrix_mul(&AxB, A, B, rows1, cols1, cols2)
*	Multipliziert die 'rows1'x'cols1' Matrix 'A' mit der
*	'cols1'x'cols2' Matrix 'B' und liefert das 'rows1'x'cols2'
*	Ergebnis in '&AxB'.
*       Das Ergebnis wird zur evtl. uebergebenen Matrix 'AxB_p' 
*       addiert
**/
void mx_matrix_mul_add(mx_matrix_t *AxB_p,
		       mx_matrix_t A, mx_matrix_t B, int rows1, int cols1, int cols2)
{
	mx_matrix_t AxB;
	int i, j, k;

	/* ggf. Ergebnis-Matrix erzeugen ... */
	if (*AxB_p == NULL)
		*AxB_p = mx_matrix_create(rows1, cols2);
	AxB = *AxB_p;
	/* ... und Multiplikation durchfuehren */
	for (i = 0; i < rows1; i++)
		for (j = 0; j < cols2; j++) {
			for (k = 0; k < cols1; k++)
				AxB[i][j] += A[i][k] * B[k][j];
		}
}

/**
* mx_matrix_mulv(&Axv, A, v, rows, cols)
*	Multipliziert die 'rows'x'cols' Matrix 'A' mit dem
*	'cols'-dimensionalen Vektor 'v' und liefert den 'rows'-dimensionalen
*	Ergebnisvektor in '&Axv'.
**/
void mx_matrix_mulv(mx_vector_t *Axv_p,
		    mx_matrix_t A, mx_vector_t v, int rows, int cols)
{
	mx_vector_t Axv;
	int i, j, k;

	/* ggf. Ergebnisvector erzeugen ... */
	if (*Axv_p == NULL)
		*Axv_p = mx_vector_create(rows);
	Axv = *Axv_p;
	/* ... und Multiplikation durchfuehren */
	for (i = 0; i < rows; i++) {
		Axv[i] = 0.0;
		for (j = 0; j < cols; j++)
			Axv[i] += A[i][j] * v[j];
	}
}

/**
* mx_matrix_muls(&As, A, s, rows, cols)
*	Multipliziert die 'rows'x'cols' Matrix 'A' mit dem Skalar 's'
*	und liefert die 'rows'x'cols' Ergebnis-Matrix in '&As'.
**/
void mx_matrix_muls(mx_matrix_t *As, mx_matrix_t A, mx_real_t s,
		int rows, int cols)
	{
	int i, j;

	/* first check parameters ... */
	if (!A || rows <= 0 || cols <= 0)
		return;

	/* ... evtl. create result matrix ... */
	if (!As)
		*As = mx_matrix_create(rows, cols);

	/* ... and multiply all matrix elements by 's' */
	for (i = 0; i < rows; i++)
		for (j = 0; j < cols; j++)
			(*As)[i][j] = A[i][j] * s;
	}

static void _matrix_normline(mx_matrix_t mat, int cols,
			     int line, mx_real_t factor)
{
	int j;

	for (j = 0; j < cols; j++)
		mat[line][j] /= factor;
}

static void _matrix_sublines(mx_matrix_t mat, int cols,
			     int line, int subline, mx_real_t factor)
{
	int j;
	
	for (j = 0; j < cols; j++)
		mat[line][j] -= mat[subline][j] * factor;
}

static void _matrix_swaplines(mx_matrix_t mat, int line1, int line2)
{
	mx_vector_t swap;

	swap = mat[line1];
	mat[line1] = mat[line2];
	mat[line2] = swap;
}

/**
* mx_matrix_invert(&inv, mat, dim)
*	Berechnet die Inverse 'inv' der 'dim'x'dim'-Matrix mat und liefert
*	den Logarithmus der Determinante det('inv') sofern die
*	Matrix 'mat' positiv definit ist, d.h. nur positive Eigenwerte besitzt.
*
*	BEACHTE:
*		Nach diesem Algorithmus ist JEDE Matrix invertierbar,
*		da evtl. verschwindende Pivot-Elemente durch ein sehr
*		kleines Epsilon ersetzt werden :-)
*		Leider sind die entstehenden Inversen in der Regel NICHT
*		besonders gut, d.h. A * Inv(A) ergibt nicht -- nicht eimal
*		naeherungsweise -- die Einheitsmatrix :-(
**/
mx_real_t mx_matrix_invert(mx_matrix_t *inv_p, mx_matrix_t mat, int dim)
{
	static mx_Matrix_t Mat = {-1, -1, NULL};

	mx_matrix_t inv;
	int eigenvalues_are_positive = 1;
	int i, j, p, pivot;
	mx_real_t logdet = 0.0;
	mx_real_t diag, factor;

	/* ggf. temporaere Kopie der zu invertierenden Matrix erzeugen ... */
	if (Mat.rows != dim) {
		mx_matrix_destroy(Mat.elems, Mat.rows);
		Mat.elems = mx_matrix_create(dim, dim);
		Mat.rows = Mat.cols = dim;
	}

	/* ggf. Inverse Matrix erzeugen ... */
	if (*inv_p == NULL)
		*inv_p = mx_matrix_create(dim, dim);
	inv = *inv_p;

	/* zu invertierende Matrix duplizieren ... */
	/** mx_matrix_dup(Mat.elems, mat); **/
	for (i = 0; i < dim; i++)
		for (j = 0; j < dim; j++)
			Mat.elems[i][j] = mat[i][j];

	/* ... und Inverse als Einheitsmatrix vorbesetzen */
	for (i = 0; i < dim; i++)
		for (j = 0; j < dim; j++)
			inv[i][j] = (i == j) ? 1 : 0;

	/* Alle Spalten der Ausgangsmatrix sukzessive umformen ... */
	for (i = 0; i < dim; i++) {
		/* ... dazu betragsmaessig groesstes Element in noch nicht
		   umgeformten Zeilen als Pivot-Element auswaehlen ... */
		pivot = i;
		/**********
			   for (p = pivot + 1; p < dim; p++)
			   if (fabs(Mat.elems[pivot][i]) < fabs(Mat.elems[p][i]))
			   pivot = p;
		**********/

		/* ... durch Zeilenvertauschung auf Diagonale bringen ... */
		if (pivot != i) {
			_matrix_swaplines(Mat.elems, i, pivot);
			_matrix_swaplines(inv, i, pivot);
		}
		diag = Mat.elems[i][i];

		/* ... ggf. korrigieren ... */
		if (fabs(diag) < LOCAL_SMALL_PIVOT) {
			diag = LOCAL_SMALL_PIVOT * ((diag < 0) ?  -1 : 1);

			rs_warning("pivot #%d (=%g) adjusted in %dx%d inversion - results may be unusable!",
				   i, Mat.elems[i][i], dim, dim);

		}

		/* ... log(det(inv)) aktualisieren falls Matrix positiv
		   definit ist, d.h. nur Eigenwerte groesser 0 besitzt
		   (diese entstehen waehrend der Umformung auf der
		   Hauptdiagonalen der Ursprungsmatrix) ... */
		if (eigenvalues_are_positive)
			if (diag > 0)
				logdet -= mx_log(diag);
			else	{
				rs_warning("can't calculate log-determinant with non-positive eigenvalue %g!", diag);
				eigenvalues_are_positive = 0;
				logdet = MX_REAL_MAX;
			}

		/* ... Zeile des Pivots normieren (Pivot <- 1.0) ... */
		_matrix_normline(Mat.elems, dim, i, diag);
		_matrix_normline(inv, dim, i, diag);

		/* ... und diese zur Loeschung des i-ten Elements der
		   anderen Zeilen verwenden ... */
		for (j = 0; j < dim; j++) {
			if (j == i)
				continue;

			factor = Mat.elems[j][i];
			_matrix_sublines(Mat.elems, dim, j, i, factor);
			_matrix_sublines(inv, dim, j, i, factor);
		}
	}

	/* 	for(i=0;i<dim;i++) */
	/* 		for(j=0;j<dim;j++) */
	/* 			inv[i][j] = (fabs(inv[i][j]) < LOCAL_EPSILON) ? 0.00 : inv[i][j]; */

	// begin_add_johannes
	mx_matrix_destroy (Mat.elems, dim);
	// end_add_johannes

	return(logdet);
}

/**
 * mx_matrix_gausselim(result, A, rows, cols)
 *      fuehrt eine Gausselimination der uebergebenen Matrix 'A' ['rows'x'cols']
 *      durch und gibt das Ergebnis (obere Dreiecksmatrix) in 'result' zurueck
 *      'result' wird _hier_ alloziert!
 **/
void mx_matrix_gausselim(mx_matrix_t *_result,
			 mx_matrix_t A,
			 int rows,
			 int cols)
{
	int i, j, k, l, m;
	mx_matrix_t result = mx_matrix_dup(A, rows, cols);
	mx_vector_t tmp;
	int singular = 1;
	mx_real_t pivot, L;
	mx_real_t max_piv;
	int max_index=-1;

	for (i = 0; i < rows-1; i++) {
		max_index = i;
		max_piv = result[i][i];

		/* groesstes Element in Restspalte als Pivotelement (totale Pivotisierung) */
		for (l = i+1; l < rows; l++)
			if (max_piv < result[l][i]) {
				max_piv = result[l][i];
				max_index = l;
			}
		
		/* Zeilen tauschen */
		pivot = result[max_index][i];
		if (!pivot)
			rs_error("Matrix is singular ...");
		tmp = result[max_index];
		result[max_index] = result[i];
		result[i] = tmp;

		for (j = i+1; j < rows; j++) {
			L = result[j][i] / pivot;			
			for (k = 0; k < cols; k++) {
				result[j][k] = result[j][k] - ( L * result[i][k]);
			}
		}
	}

	/* numerische Stabilitaet ... */
	for (i = 0; i < rows; i++)
		for (j = 0; j < i; j++)
			result[i][j] = (fabs(result[i][j]) < LOCAL_GAUSSEPS) ? 0.00 : result[i][j];

	*_result = result;
	return;
}

/**
 * mx_matrix_linsolve(_x, A, b, dim)
 *      loest das lineare Gleichungssystem A * _x = b mit 'A' als 
 *      'dim' x 'dim' - dimensionale Koeffizientenmatrix und 'b' als
 *      'dim'-dimensionaler Loesungsvektor. Das Ergebnis wird in '_x'
 *      als 'dim'-dimensionaler Vektor zurueckgegeben. Es wird das
 *      Verfahren der Gausselimination benutzt. Der Rueckgabewert gilt
 *      als Erfolgsindikator: 0 - Fehler / != 0 - Erfolg
 **/
int mx_matrix_linsolve(mx_vector_t *_x,
		       mx_matrix_t A,
		       mx_vector_t b,
		       int dim) {

	int r,c;
	mx_matrix_t gauss;
	mx_matrix_t Ab = mx_matrix_create(dim, dim+1);
	mx_vector_t x;
	mx_vector_t b2 = mx_vector_create(dim);
	mx_real_t zaehler;

	/* ggf. Ergebnisvektor erzeugen */
	if(!*_x)
		*_x = mx_vector_create(dim);
	x = *_x;

	/* erweiterte A-Matrix bauen */
	for (r = 0; r < dim; r++) {
		for (c = 0; c < dim; c++)
			Ab[r][c] = A[r][c];
		Ab[r][dim] = b[r];
	}

	/* Vorwaertselimination */
	mx_matrix_gausselim(&gauss,
			    Ab,
			    dim,
			    dim + 1);

	/* neues b extrahieren */
	for (r = 0; r < dim; r++)
		b2[r] = gauss[r][dim];

	/* Rueckwaertseinsetzung fuer W */
	for (r = dim-1; r>= 0; r--) {
		x[r] = 0.0;
		zaehler = 0.0;
		for (c = dim-1; c >= r+1; c--)
			zaehler += gauss[r][c] * x[c];
		x[r] = (b2[r] - zaehler) / gauss[r][r];
	}

	/* cleanup */
	mx_matrix_destroy(gauss, dim);
	mx_matrix_destroy(Ab, dim);
	mx_vector_destroy(b2);

	return(1);
}


/**
 * mx_matrix_scale(result, matrix, scalar, rows, cols)
 *	Skaliert die 'rows'x'cols'-dimensionale Matrix 'matrix' komponentenweise
 *	mit 'scalar' in 'result'
 **/
void mx_matrix_scale(mx_matrix_t 	*_result,
		     mx_matrix_t 	matrix, 
		     mx_real_t 		scalar,		     
		     int 		rows,
		     int		cols) 
{
	register int i;					
	register int j;
	mx_matrix_t result;

	/* ggf. Ergebnismatrix erzeugen */
	if(!*_result)
		*_result = mx_matrix_create(rows, cols);
	result = *_result;
	for(i=0;i<rows;i++)
		for(j=0;j<cols;j++)
			result[i][j]=scalar*matrix[i][j]; 
}

/**
 * mx_matrix_sub(result, matrix1, matrix2, rows, cols)
 *	Bildet die Differenzmatrix zwischen den 'rows'x'cols'-dimensionalen
 *	Matrizen 'matrix1' und 'matrix2' in 'result' (matrix1 - matrix2)
 **/
void mx_matrix_sub(mx_matrix_t 	*_result,
		   mx_matrix_t 	matrix1, 
		   mx_matrix_t 	matrix2, 		   
		   int		rows,
		   int		cols) 
{	 
	register int i;		
	register int j;		
	mx_matrix_t result;

	/* ggf. Ergebnismatrix erzeugen */
	if(!*_result)
		*_result = mx_matrix_create(rows, cols);
	result = *_result;
	for(i=0;i<rows;i++)
		for(j=0;j<cols;j++)
			result[i][j] = matrix1[i][j] - matrix2[i][j]; 
}

/**
 * mx_matrix_add(result, matrix1, matrix2, rows, cols)
 *	Bildet die Summenmatrix zwischen den 'rows'x'cols'-dimensionalen
 *	Matrizen 'matrix1' und 'matrix2' in 'result'
 **/
void mx_matrix_add(mx_matrix_t 	*_result,
		   mx_matrix_t 	matrix1, 
		   mx_matrix_t 	matrix2, 		   
		   int		rows,
		   int		cols) 
{	 
	register int i;		
	register int j;		
	mx_matrix_t result;

	/* ggf. Ergebnismatrix erzeugen */
	if(!*_result)
		*_result = mx_matrix_create(rows, cols);
	result = *_result;
	for(i=0;i<rows;i++)
		for(j=0;j<cols;j++)
			result[i][j] = matrix1[i][j] + matrix2[i][j]; 
}

/**
* mx_matrix_wadd(&aA_plus_bB, a, A, b, B ,rows, cols)
*	Calculates the weighted sum of the 'rows'x'cols' matrices 'A' and 'B'
*	according to 'aA + bB' and stores the result in '&aA_plus_bB'.
**/
void mx_matrix_wadd(mx_matrix_t *aA_plus_bB,
		mx_real_t a, mx_matrix_t A,
		mx_real_t b, mx_matrix_t B,
		int rows, int cols)
	{	 
	int i, j;

	/* first check parameters ... */
	if (!A || rows <= 0 || cols <= 0)
		return;

	/* ... evtl. create result matrix ... */
	if(!*aA_plus_bB)
		*aA_plus_bB = mx_matrix_create(rows, cols);

	/* ... calculate weighted sum of matrix elements ... */
	for (i = 0; i < rows; i++)
		for (j = 0; j < cols; j++)
			(*aA_plus_bB)[i][j] = a * A[i][j] + b * B[i][j];
	}

/**
 * mx_matrix_transp(result, matrix, rows, cols)
 *	Transponiert die 'rows'x'cols'-dimensionale Matrix 'matrix' in die
 *	'cols'x'rows'-dimensionale Ergebismatrix 'result'
 **/
void mx_matrix_transp(mx_matrix_t *_result,
		      mx_matrix_t matrix,		      
		      int rows, 
		      int cols)
{ 	 
	register int i;					 
	register int j;					 
	mx_matrix_t result;

	/* ggf. Ergebnismatrix erzeugen */
	if(!*_result)
		*_result = mx_matrix_create(cols, rows);
	result = *_result;
	for(i=0;i<rows;i++)					 
		for(j=0;j<cols;j++)				 
			result[j][i] = matrix[i][j];
}

int mx_matrix_fscan(mx_matrix_t *mat, int *rows, int *cols, FILE *fp) {
        int i, j, n_elems = 0;
	int lpos = 0;
	char *line, *lp;

	if (!fp)
		return(-1);
	
	
	/* Dimensionen extrahieren */
	if (!(line = rs_line_read(fp, MX_COMMENT_CHAR)))
		rs_error("Corrupt data file for matrix.");
	if (sscanf(line, "%d\t%d", rows, cols) != 2)
		rs_error("Corrupt data file for matrix ('%s').",
			 line);
	if(*mat) {
		rs_warning("Memory leak detected: Given matrix contains data!");
		rs_free(*mat);
		*mat = NULL;
	}
	*mat = mx_matrix_create(*rows, *cols);
	
	for (i = 0; i < *rows; i++) {
		if (!(line = rs_line_read(fp, MX_COMMENT_CHAR)))
			rs_error("Corrupt data file for matrix.");
		lp = line;
		lpos = 0;
		for (j = 0; j < *cols; j++) 
			if (sscanf(lp, "%g%n", &((*mat)[i][j]), &lpos) != 1)
				rs_error("Corrupt data file for matrix "
					 "(line %d: '%s')",
					 i, line);
			else {
				n_elems++;
				lp += lpos;
			}
	}
	
	return(n_elems);
	}

int mx_matrix_fprint(FILE *fp, mx_matrix_t mat, int rows, int cols,
		     char *comment) {
	int i, j, n_elems = 0;        
	
	if (!fp || !mat || rows <= 0 || cols <= 0)
		return(-1);

	/* Header ausgeben */
	if (comment)
		fprintf(fp, "%c %s\n",
			MX_COMMENT_CHAR, comment);
	fprintf(fp, "%c matrix: rows x cols\n",
		MX_COMMENT_CHAR);
	fprintf(fp, "%d\t%d\n", rows, cols);

	/* Matrix ausgeben */
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			fprintf(fp, "%g\t", mat[i][j]);
			n_elems++;
		}
		fprintf(fp, "\n");
	}

	return(n_elems);
	}

/**
 * mx_matrix_mdist(v1, v2, C, diag_v, dim, inv)
 *	Berechnet den Mahalanobis Abstands der 'dim'-dimensionalen Vektoren 
 *	'v1' und 'v2', wobei 'C' die 'dim'x'dim'-dimensionale 
 *	Kovarianzmatrix und 'diag_v' ggf. die Diagonale der
 *	Kovarianz darstellt. 'diag' gibt an, ob die 'C' oder 'diag_v' benutzt
 *	wird (das jeweils andere wird dann _nicht_ betrachtet und kann daher
 *	NULL sein)
 *	'inv' gibt an, ob es sich bei 'C' bzw. 'diag_C' um die bereits
 *	invertierte Matrix bzw. deren Diagonale handelt
 **/
mx_real_t mx_matrix_mdist(mx_vector_t v1, 
			  mx_vector_t v2, 
			  mx_matrix_t C, 
			  mx_vector_t diag_v,
			  int dim,
			  int diag,
			  int inv) {
  mx_matrix_t invC = NULL;
  mx_vector_t invDiag = NULL;
  mx_vector_t xy = NULL;
  mx_vector_t t2 = NULL;
  mx_real_t dist = MX_REAL_MAX;
  int i, j;

  if (!v1 || !v2)
    rs_error("Invalid parameters for mahalanobis distance calculation!");

  if (!inv)
	  if (!diag)
		  mx_matrix_invert(&invC, C, dim);
	  else {
		  invDiag = mx_vector_create(dim);
		  for (i = 0; i < dim; i++)
			  invDiag[i] = 
				  (diag_v[i] == 0.0) ? 0.0 : (1.0 / diag_v[i]) ;
	  }
  else 
	  if (!diag)
		  invC = C;
	  else
		  invDiag = diag_v;

  mx_vector_sub(&xy, v1, v2, dim);
  if (!diag)
	  mx_matrix_mulv(&t2, invC, xy, dim, dim);
  else {
	  t2 = mx_vector_create(dim);
	  for (i = 0; i < dim; i++)
		  t2[i] = invDiag[i] * xy[i];
  }

  mx_vector_scalprod(&dist, xy, t2, dim);
  dist = sqrt(dist);

  if (!inv)
	  if (!diag)
		  mx_matrix_destroy(invC, dim);
	  else
		  mx_vector_destroy(invDiag);
  mx_vector_destroy(xy);
  mx_vector_destroy(t2);

  return(dist);
}
