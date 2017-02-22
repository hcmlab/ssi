/* From: 
 * Feature Selection Algorithms - A Brief Guide
 * P. Somol, P. Pudil
 * http://ro.utia.cz/fs/fs_guideline.html
*/

#include "ev_select.h"

#include "ev_class.h"

#ifndef SELECT_H_
#define SELECT_H_

// how often should the informational percentage be estimated
#define PERCENTDETAIL  5
#define SECONDS  2

// output types
#define NOTHING 0
#define PERCENT 1
#define STANDARD 2
#define FULL 6
#define GRAPHICS 8

#define DELTAMUL 1.0            // a multiplier of estimated delta
#define DELTAADD 1              // a constant to be added to estimated delta
#define SAFEUP   5e300              /* double overflow limit */
#define SAFEDOWN 5e-300             /* double underflow limit */
#define SAFEXP   1000               /* double exponent limit */
#define FREE(x,p)           { free( x ); x = NULL; }

typedef struct { 
	int subsetsize; 
	int *featureset; 
	double critvalue; 
	char dobavypoctu[20];
	} TSubset;
/* after the procedure ends, this contains:
   subsetsize - final subset size
   featureset - a field containing indexes of selected features (begins by 0)
   critvalue - criterion value corresponding to the selected subset
   dobavypoctu - a string containing length of computation (in form "hh:mm:ss")
*/

// @begin_change_johannes
//int verbose;
extern int ev_verbose;
// @end_change_johannes

int FloatSearch(int n, int d, int delta, int r, TSubset *bset, int detail, int n_classes, fx_select_t *sel, char **file_names, int n_splits, cType classifier, char *cl_param, int float_level);

#endif /*SELECT_H_*/
