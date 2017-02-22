/* Konstantendefinitionen fuer ... */
/* ... Merkmalsberechnung V1.4: */
#include "ev_mfcc_1_4.h"

/* ... und zusaetzlich fuer V1.5: */
#define V1_5_FFT_LEN            1024
#define V1_5_N_FILTERS         34
#define V1_5_N_FRESOLUTION	((mx_real_t)V1_1_SAMPLERATE / V1_5_FFT_LEN)

#define	V1_5_EN_HIST_RES	0.25 /*Geht besser bei Maskierung*/

/* ... Vorwaertsmaskierung ... */
#define V1_5_DB_MULTIPLIKATOR 10

#define V1_5_DB_SPRACHE       ((mx_real_t) 50 - ((V1_5_EN_HIST_RES / 2.0) * V1_5_DB_MULTIPLIKATOR))   /*Wenn mit Maskierung und HIST_RES 0.25 + 1024 FFT*/ 
