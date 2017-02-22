/**
* File:		score.h
* Author:	Gernot A. Fink
* Date:		12.2.2004 
*
* Description:	Definition of data structures and function prototypes
*		for the representation of general scores
*
* History:	based on 'md/score.h'
**/

#ifndef __MX_SCORE_H_INCLUDED__
#define __MX_SCORE_H_INCLUDED__

#include <stdio.h>

#ifdef MX_KERNEL
#include "ev_real.h"
#include "ev_bitmask.h"
#else
#include "ev_real.h"
#include <mx/bitmask.h>
#endif

/*
 * Data Types
 */

/* type of a score ... */
typedef enum {
	mx_stype_Undefined,		/* undefined */
	mx_stype_Probability,		/* probability (or prob. density) */
	mx_stype_LogProbability,	/* logarithmic probability/density */
	mx_stype_Cost,			/* additive cost */
	mx_stype_Distance		/* distance (defines metric) */
	} mx_stype_t;

extern char *mx_stype_text[];	/* description texts for 'mx_ctype_t' entries */

/* "silent" score (i.e. without type annotation) ... */
typedef struct {
	int id;
	mx_real_t score;
	} mx_sscore_t;

/* compact external representation of scores ... */
	/*
	 * NOTE: For probability-type scores compatible with ISADORA SVQ-format
	 */
typedef short mx_xscore_size_t;		/* i.e. 16 bits integer */
typedef struct {
	mx_xscore_size_t id;
	mx_xscore_size_t score;
	} mx_xscore_t;

/* list of scores, typed ... */
typedef struct {
	mx_stype_t type;		/* type of scores in the list */
	int max_scores;			/* maximum and ... */
	int n_scores;			/* ... total number of scores */
	mx_sscore_t *score;		/* list of "silent" scores */
	mx_bitmask_t hint;		/* "hint" (= bit) for relevant scores */
	} mx_scorelist_t;

/* set of scores (i.e. list of lists), typed ... */
typedef struct {
	mx_stype_t type;		/* type of scores in the set */
	int max_lists;			/* maximum and ... */
	int n_lists;			/* ... total number of score lists */
	mx_scorelist_t **list;		/* list of score lists */
	} mx_scoreset_t;

/*
 * Function Prototypes 
 */
/* for score lists ... */
mx_scorelist_t *mx_scorelist_create(void);
mx_scorelist_t *mx_scorelist_create_by_type(mx_stype_t type);
int mx_scorelist_resize(mx_scorelist_t *scorelist, int max_scores);
mx_scorelist_t *mx_scorelist_dup(mx_scorelist_t *src);
int mx_scorelist_copy(mx_scorelist_t **destp, mx_scorelist_t *src);
void mx_scorelist_destroy(mx_scorelist_t *scorelist);
int mx_scorelist_reset(mx_scorelist_t *scorelist);
int mx_scorelist_rewind(mx_scorelist_t *scorelist);

int mx_scorelist_addscore(mx_scorelist_t *scorelist, int id, mx_real_t score);

/* for score sets ... */
mx_scoreset_t *mx_scoreset_create(void);
mx_scoreset_t *mx_scoreset_create_by_type(mx_stype_t type);
int mx_scoreset_resize(mx_scoreset_t *scoreset, int max_lists);
mx_scoreset_t *mx_scoreset_dup(mx_scoreset_t *src);
int mx_scoreset_copy(mx_scoreset_t **destp, mx_scoreset_t *src);
void mx_scoreset_destroy(mx_scoreset_t *scoreset);
int mx_scoreset_reset(mx_scoreset_t *scoreset);
int mx_scoreset_rewind(mx_scoreset_t *scoreset);

int mx_scoreset_addlist(mx_scoreset_t *scoreset, mx_scorelist_t *list);

int mx_scoreset_fread(mx_scoreset_t *scoreset, FILE *fp);
int mx_scoreset_fwrite(FILE *fp, mx_scoreset_t *scoreset);
int mx_scoreset_fprint(FILE *fp, mx_scoreset_t *scoreset);

#endif /* __MX_SCORE_H_INCLUDED__ */
