/**
* Datei:	prob.c
* Autor:	Gernot A. Fink
* Datum:	14.10.1997
*
* Beschreibung:	Parameter und Algorithmen fuer (log.) Wahrscheinlichkeiten
**/

#define MX_KERNEL
#include "ev_prob.h"

#ifndef M_LN2 
#define M_LN2 0.69314718055994530942 
#endif 

mx_prob_t mx_prob_low =		MX_PROB_LOW;
mx_logprob_t mx_logprob_low =	MX_LOGPROB_LOW;

mx_prob_t mx_prob_ignore =	MX_PROB_IGNORE;

/**
* mx_logprob_add(p1, p2)
*	berechnet aus 'p1' = -log(a) und 'p2' = -log(b) das Ergebnis -log(a + b)
*	gemaess der "Kingsbury-Rayner-Formel":
*			      
*			    -(-log(a) - -log(b))
*	-log(a + b) = -log(e			+ 1) + -log(b)
*
*	Dieses Vorgehen wurde aus ISADORA uebernommen und scheint numerisch
*	stabiler bzw. effizienter zu sein, als die komplette Delogarithmierung
*	und anschliessende Logarithmierung.
*
*	vgl.:
*		N. G. Kingsbury, P. J. W. Rayner: "Digital Filtering Using
*			Logarithmic Arithmetic", Electronical Letters,
*			Bd. 7, 1971, S. 56-58.
*
*	oder auch z.B.:
*		E. G. Schukat-Talamazzini: Automatische Spracherkennung,
*			Vieweg, 1995, S. 135.
*		K.-F. Lee: Automatic Speech Recognition -
*			The Development of the SPHINX System,
*			Kluwer Academic Publishers, Boston, 1989, S. 29.
**/
mx_logprob_t mx_logprob_add(mx_logprob_t p1, mx_logprob_t p2)
	{
	mx_logprob_t psmall, pbig;

	/* zunaechst Argumente groessenmaessig ordnen ... */
	if (p1 < p2) {
		psmall = p1;
		pbig = p2;
		}
	else	{
		psmall = p2;
		pbig = p1;
		}

	/*
	 * Eine Berechnung ist in folgenden Faellen NICHT notwendig:
	 */
	/* ... falls das kleinere zu klein ist ... */
	if (pbig >= MX_LOGPROB_MIN)
		return(psmall);
	/* ... bzw. die Groessendifferenz zu gross ... */
	if ((psmall - pbig) < -MX_LOGREAL_MAX)
		return(psmall);

	/* ... sonst berechnen */
	return(-log1p(exp(-(psmall - pbig))) + pbig);
	}
