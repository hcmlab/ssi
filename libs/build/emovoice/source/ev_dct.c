/**
* File:		dct.h
* Author:	Gernot A. Fink
* Date:		15.3.2004 (based on 'costrans.c', originally from 14.4.1997)
*
* Description:	Discrete Cosine Transform (DCT)
**/

#include <math.h>

#include "ev_memory.h"


#include "ev_dct.h"

#ifndef M_PI
#define M_PI   3.14159265
#endif
#ifndef M_PI_2
#define M_PI_2   1.57079
#endif
#ifndef M_PI_4
#define M_PI_4   0.78539
#endif

static void _dsp_dct_init(int n);

static mx_real_t **g = NULL;		/* Parametermatrix */

/**
* _dsp_dct_init(n)
*	Interne Datenstrukturen fuer eine Cosinus-Transformation der Laenge 'n'
*	initialisieren, naemlich:
*	  -	g[]	Parametermatrix
*
*		g[0][j] = sqrt(1/n)
*
*		g[i][j] = sqrt(2/n) * cos(M_PI*i * (j + 0.5) / n)
**/
static void _dsp_dct_init(int n)
	{
	static int last_n = -1;	/* Laenge der letzten Initialisierung */

	int i, j;
	
	if (last_n == n)
		return;		/* Daten koennen weiterverwendet werden */

	/* alte Parametermatrix loeschen ... */
	if (g) {
		for (i = 0; i < last_n; i++) {
			if (g[i])
				free(g[i]);
			}
		free(g);
		}

	/* ... (wieder) allozieren ... */
	g = rs_malloc(n * sizeof(mx_real_t *), "cosine transform");
	for (i = 0; i < n; i++)
		g[i] = rs_malloc(n * sizeof(mx_real_t),
				"cosine transform parameters");

	/* ... und Werte berechnen */
	for (j = 0; j < n; j++)
		g[0][j] = sqrt(1 / (mx_real_t)n);

	for (i = 1; i < n; i++)
		for (j = 0; j < n; j++)
			g[i][j] = sqrt(2 / (mx_real_t)n) *
					cos(M_PI*i * (j + 0.5)/(mx_real_t)n);

	last_n = n;
	}
		
/**
* dsp_dct(C[], f[], n, m)
*	Berechnet die Cosinustransformation des reellen Signals 'f[]' der
*	Laenge 'n' und speichert die ersten 'm' Ergebniskoeffizienten in 'C[]'.
*
*	vgl. z.B.:
*       	R. C. Gonzalez & P. Wintz: Digital Image Processing,
*			Addison-Wesley, (2)1987, S. 121.
**/
void dsp_dct(mx_real_t *C, mx_real_t *f, size_t n, size_t m)
	{
	int i, j;

	/* Berechnung initialisieren ... */
	_dsp_dct_init(n);

	/*
	 * ... und Transformation 'C[]' durch Multiplikation von 'f[]' mit
	 * der Parametermatrix 'g[][]' bestimmen
	 */
	for (i = 0; i < m; i++) {
		C[i] = 0.0;

		for (j = 0; j < n; j++)
			C[i] += f[j] * g[i][j];
		}
	}

/**
* dsp_idct(f[], C[], n, m)
*	Berechnet die inverse Cosinustransformation der Laenge 'n'
*	aus den 'm' Koeffizienten 'C[]' und speichert das reelle
*	Ergebnissignal in 'f[]'.
*
*	vgl. z.B.:
*       	R. C. Gonzalez & P. Wintz: Digital Image Processing,
*			Addison-Wesley, (2)1987, S. 121.
**/
void dsp_idct(mx_real_t *f, mx_real_t *C, size_t n, size_t m)
	{
	int i, j;

	/* Berechnung initialisieren ... */
	_dsp_dct_init(n);

	/*
	 * ... und Transformation 'C[]' durch Multiplikation von 'f[]' mit
	 * der Parametermatrix 'g[][]' bestimmen
	 */
	for (j = 0; j < n; j++) {
		f[j] = C[0] * g[0][0];

		for (i = 1; i < m; i++)
			f[j] += C[i] * g[i][j];
		}
	}

#ifdef TEST

int main(int argc, char **argv)
	{
	/*
	 * for testing use e.g. f[] = 0 1 2 -1 0 1 2 -1
	 */
	mx_real_t *f, *C;
	int n, m;
	int i, status;

	if (argc != 3) {
		rs_error("needs 2 arguments: <n> and <m>.");
		exit(1);
		}
	n = atoi(argv[1]);
	m = atoi(argv[2]);

	/* Speicher allozieren */
	f = rs_malloc(sizeof(mx_real_t) * n, "signal");
	C = rs_malloc(sizeof(mx_real_t) * n, "DCT");

	/* Signal einlesen */
	for (i = 0; i < n; i++) {
		status = scanf("%g", f + i);
		if (status != 1)
			rs_error("can't read %d-th sample.", i);
		}

	printf("f[] =\t\t");
	for (i = 0; i < n; i++)
		printf("%8.2g", f[i]);
	printf("\n");

	dsp_dct(C, f, n, m);

	printf("C[] = DCT{f} =\t");
	for (i = 0; i < n; i++)
		printf("%8.2g", C[i]);
	printf("\n");

	dsp_idct(f, C, n, m);

	printf("f'[]=ICDT{C}=\t");
	for (i = 0; i < n; i++)
		printf("%8.2g", f[i]);
	printf("\n");
	}
#endif
