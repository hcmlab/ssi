/**
* Datei:	bitmask.h
* Autor:	Gernot A. Fink
* Datum:	3.4.1998
*
* Beschreibung:	Definition von Bit-Masken und dazugehoerigen Operationen
**/

#ifndef __MX_BITMASK_H_INCLUDED__
#define __MX_BITMASK_H_INCLUDED__

#include <limits.h>

#define MX_MASK_BITS	(sizeof(long) * CHAR_BIT)
/*** #define MX_MASK_BITS	BITS(long) **/
#define MX_N_MASKS	4

typedef long mx_bitmask_t[MX_N_MASKS];

#define mx_bitmask_clear(m)	((m)[0] = (m)[1] = (m)[2] = (m)[3] = 0L)
#define mx_bitmask_set(m, n)	((m)[((n) / MX_MASK_BITS) % MX_N_MASKS] |= \
				 (1L << ((n) % MX_MASK_BITS)))
#define mx_bitmask_compat(m1, m2)	\
				((m1)[0] & (m2)[0] || (m1)[1] & (m2)[1] || \
				 (m1)[2] & (m2)[2] || (m1)[3] & (m2)[3])
#define mx_bitmask_incompat(m1, m2)	\
				(!((m1)[0] & (m2)[0]) && !((m1)[1] & (m2)[1]) && \
				 !((m1)[2] & (m2)[2]) && !((m1)[3] & (m2)[3]))
#endif /* __MX_BITMASK_H_INCLUDED__ */
