/**
* File:		score.c
* Author:	Gernot A. Fink
* Date:		12.2.2004 
*
* Description:	Methods for handling general scores
*
* History:	based on 'md/score.c'
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ev_memory.h"
#include "ev_messages.h"

#define MX_KERNEL
#include "ev_prob.h"
#include "ev_score.h"

#define LOCAL_SCORE_BLOCK	64
#define LOCAL_SCORELIST_BLOCK	32

/*
 * Local Definitions
 */
/* definition of texts for score types (see 'mx_stype_t') */
char *mx_stype_text[] = {
	"undefined",
	"probability",
	"log-probability",
	"cost",
	"distance"
	};

/* check given type id for validity ... */
#define _stype_isvalid(st)	\
	(!((st) < 0) &&	\
	 !((st) >= sizeof(mx_stype_text)/sizeof(mx_stype_text[0])))

	/*
	 * creating, destroying, and manipulation of score lists
	 */
mx_scorelist_t *mx_scorelist_create(void)
	{
	mx_scorelist_t *scorelist;
	int i;

	/* create emtpy container ... */
	scorelist = rs_malloc(sizeof(mx_scorelist_t), "head of score list");

	/* ... score types are yet undefined ... */
	scorelist->type = mx_stype_Undefined;

	/* ... list of scores is empty ... */
	scorelist->max_scores = 0;
	scorelist->n_scores = 0;
	scorelist->score = NULL;

	/* ... initialize hints */
	mx_bitmask_clear(scorelist->hint);

	return(scorelist);
	}

mx_scorelist_t *mx_scorelist_create_by_type(mx_stype_t type)
	{
	mx_scorelist_t *scorelist;

	scorelist = mx_scorelist_create();

	scorelist->type = type;

	return(scorelist);
	}

int mx_scorelist_resize(mx_scorelist_t *scorelist, int max_scores)
	{
	int i;

	/* first check parameters ... */
	if (!scorelist || max_scores <= 0)
		return(-1);

	/* can't resize below number of scores used ... */
	if (max_scores < scorelist->n_scores)
		return(-1);

	/* if resizing to length 0 ... */
	if (max_scores == 0) {
		/* ... clear existing score list ... */
		scorelist->max_scores = 0;
		scorelist->n_scores = 0;	/* should have been 0 anyway! */
		rs_free(scorelist->score);
		scorelist->score = NULL;
		}
	else	{
		/* ... resize list of available scores ... */
		scorelist->max_scores = max_scores;

		scorelist->score = rs_realloc(scorelist->score,
					scorelist->max_scores *
						sizeof(mx_sscore_t),
					"list of scores");

		/* ... and initialize unused entries to {-1, ?} */
		for (i = scorelist->n_scores; i < scorelist->max_scores; i++)
			scorelist->score[i].id = -1;
		}

	return(scorelist->max_scores);
	}


mx_scorelist_t *mx_scorelist_dup(mx_scorelist_t *src)
	{
	mx_scorelist_t *dest;
	int i;

	/* first check parameters ... */
	if (!src)
		return(NULL);

	/* ... create new container ... */
	dest = mx_scorelist_create_by_type(src->type);

	/* ... adjust size ... */
	mx_scorelist_resize(dest, src->max_scores);

	/* ... and fill in score data */
	dest->n_scores = src->n_scores;
	memcpy(dest->score, src->score, dest->n_scores * sizeof(mx_sscore_t));
	memcpy(dest->hint, src->hint, sizeof(mx_bitmask_t));

	return(dest);
	}

int mx_scorelist_copy(mx_scorelist_t **destp, mx_scorelist_t *src)
	{
	int i;

	/* first check parameters ... */
	if (!src || !destp)
		return(-1);

	/* ... eventually create duplicate container ... */
	if (!*destp) {
		*destp = mx_scorelist_dup(src);
		}
	else	{
		/* ... check size of '*destp' ... */
		if ((*destp)->max_scores < src->max_scores)
			mx_scorelist_resize((*destp), src->max_scores);

		/* ... rewind '*destp' ... */
		mx_scorelist_rewind(*destp);

		/* ... and fill in score data */
		for (i = 0; i < src->n_scores; i++)
			mx_scorelist_addscore(*destp,
				src->score[i].id, src->score[i].score);
		}

	return(0);
	}

void mx_scorelist_destroy(mx_scorelist_t *scorelist)
	{
	/* first check parameters ... */
	if (!scorelist)
		return;

	/* ... free and invalidate data structures ... */
	if (scorelist->score)
		rs_free(scorelist->score);
	memset(scorelist, -1, sizeof(mx_scorelist_t));
	rs_free(scorelist);
	}

int mx_scorelist_reset(mx_scorelist_t *scorelist)
	{
	int i;

	/* first check parameters ... */
	if (!scorelist)
		return(-1);

	/* ... clear number of scores actually used (including hints) */
	scorelist->n_scores = 0;
	mx_bitmask_clear(scorelist->hint);

	/* ... clear all available entries to {-1, ?} */
	for (i = 0; i < scorelist->max_scores; i++)
		scorelist->score[i].id = -1;

	return(0);
	}

int mx_scorelist_rewind(mx_scorelist_t *scorelist)
	{
	int i;

	/* first check parameters ... */
	if (!scorelist)
		return(-1);

	/* ... clear number of scores actually used (including hints) */
	scorelist->n_scores = 0;
	mx_bitmask_clear(scorelist->hint);

	return(0);
	}

int mx_scorelist_addscore(mx_scorelist_t *scorelist, int id, mx_real_t score)
	{
	/* first check parameters ... */
	if (!scorelist || id < 0)
		return(-1);

	/* ... eventually expand list of scores ... */
	if (scorelist->n_scores >= scorelist->max_scores)
		mx_scorelist_resize(scorelist,
				scorelist->n_scores + LOCAL_SCORELIST_BLOCK);

	/* ... create new score entry ... */
	scorelist->score[scorelist->n_scores].id = id;
	scorelist->score[scorelist->n_scores].score = score;

	/* ... evtl. set hint ... */
	if (scorelist->type == mx_stype_Probability) {
		/* ... only for probabilities exceeding 'mx_prob_ignore' */
		if (score > mx_prob_ignore)
			mx_bitmask_set(scorelist->hint, id);
		}
	else	{
		/* ... always for non-probability type scores */
		mx_bitmask_set(scorelist->hint, id);
		}

	scorelist->n_scores++;

	return(scorelist->n_scores);
	}

	/*
	 * creating, destroying, and manipulation of score sets
	 */
mx_scoreset_t *mx_scoreset_create(void)
	{
	mx_scoreset_t *scoreset;
	int i;

	/* create emtpy container ... */
	scoreset = rs_malloc(sizeof(mx_scoreset_t), "head of list set");

	/* ... list types are yet undefined ... */
	scoreset->type = mx_stype_Undefined;

	/* ... set of lists is empty ... */
	scoreset->max_lists = 0;
	scoreset->n_lists = 0;
	scoreset->list = NULL;

	return(scoreset);
	}

mx_scoreset_t *mx_scoreset_create_by_type(mx_stype_t type)
	{
	mx_scoreset_t *scoreset;

	scoreset = mx_scoreset_create();

	scoreset->type = type;

	return(scoreset);
	}

int mx_scoreset_resize(mx_scoreset_t *scoreset, int max_lists)
	{
	int i;

	/* first check parameters ... */
	if (!scoreset || max_lists <= 0)
		return(-1);

	/* can't resize below number of lists used ... */
	if (max_lists < scoreset->n_lists)
		return(-1);

	/* if resizing to length 0 ... */
	if (max_lists == 0) {
		/* ... clear existing list set ... */
		scoreset->max_lists = 0;
		scoreset->n_lists = 0;	/* should have been 0 anyway! */
		rs_free(scoreset->list);
		scoreset->list = NULL;
		}
	else	{
		/* ... resize set of available lists ... */
		scoreset->max_lists = max_lists;

		scoreset->list = rs_realloc(scoreset->list,
					scoreset->max_lists *
						sizeof(mx_sscore_t),
					"set of score lists");

		/* ... and initialize unused entries to NULL */
		for (i = scoreset->n_lists; i < scoreset->max_lists; i++)
			scoreset->list[i] = NULL;
		}

	return(scoreset->max_lists);
	}

mx_scoreset_t *mx_scoreset_dup(mx_scoreset_t *src)
	{
	mx_scoreset_t *dest;
	int i;

	/* first check parameters ... */
	if (!src)
		return(NULL);

	/* ... create new container ... */
	dest = mx_scoreset_create_by_type(src->type);

	/* ... adjust size ... */
	mx_scoreset_resize(dest, src->n_lists);

	/* ... and fill in list data */
	dest->n_lists = src->n_lists;
	for (i = 0; i < dest->n_lists; i++)
		dest->list[i] = mx_scorelist_dup(src->list[i]);

	return(dest);
	}

int mx_scoreset_copy(mx_scoreset_t **destp, mx_scoreset_t *src)
	{
	mx_scoreset_t *dest;
	int i;

	/* first check parameters ... */
	if (!src || !destp)
		return(-1);

	/* ... eventually create duplicate container ... */
	if (!*destp) {
		*destp = mx_scoreset_dup(src);
		}
	else	{
		/* ... check size of '*destp' ... */
		if ((*destp)->n_lists != src->n_lists)
			return(-1);

		/* ... and fill in list data */
		for (i = 0; i < (*destp)->n_lists; i++)
			mx_scorelist_copy(&((*destp)->list[i]), src->list[i]);
		}

	return(0);
	}

void mx_scoreset_destroy(mx_scoreset_t *scoreset)
	{
	int i;

	/* first check parameters ... */
	if (!scoreset)
		return;

	/* ... free and invalidate data structures ... */
	if (scoreset->list) {
		for (i = 0; i < scoreset->max_lists; i++)
			if (scoreset->list[i])
				mx_scorelist_destroy(scoreset->list[i]);
		rs_free(scoreset->list);
		}
	memset(scoreset, -1, sizeof(mx_scoreset_t));
	rs_free(scoreset);
	}

int mx_scoreset_reset(mx_scoreset_t *scoreset)
	{
	int i;

	/* first check parameters ... */
	if (!scoreset)
		return(-1);

	/* ... reset all score lists used */
	for (i = 0; i < scoreset->n_lists; i++)
		mx_scorelist_reset(scoreset->list[i]);

	return(0);
	}

int mx_scoreset_rewind(mx_scoreset_t *scoreset)
	{
	int i;

	/* first check parameters ... */
	if (!scoreset)
		return(-1);

	/* ... rewind all score lists used */
	for (i = 0; i < scoreset->n_lists; i++)
		mx_scorelist_rewind(scoreset->list[i]);

	return(0);
	}

int mx_scoreset_addlist(mx_scoreset_t *scoreset, mx_scorelist_t *list)
	{
	/* first check parameters ... */
	if (!scoreset || !list)
		return(-1);

	/* ... eventually expand list of score lists ... */
	if (scoreset->n_lists >= scoreset->max_lists)
		mx_scoreset_resize(scoreset,
				scoreset->n_lists + LOCAL_SCORELIST_BLOCK);

	/* ... create new list entry ... */
	scoreset->list[scoreset->n_lists] = list;
	scoreset->n_lists++;

	return(scoreset->n_lists);
	}

	/*
	 * Functions for handling IO between internal score representation
	 * and external representation
	 *
	 * NOTE: Binary IO supports probability and log-probability type
	 *	score data only!
	 */

static mx_real_t _xscore2score(mx_xscore_size_t xs, mx_stype_t type)
	{
	mx_real_t score;

	/* first check parameters ... */
	if (!xs)
		return(-1);
	if (!_stype_isvalid(type))
		rs_error("can't convert score of invalid type %d!",
			type);

	switch (type) {
		case mx_stype_Probability:
			score = (mx_real_t)(xs) / 10000.0;
			break;
		case mx_stype_LogProbability:
			score = (mx_real_t)(xs) / 100.0;
			break;
		default:
			rs_error("can't convert %s-type scores from external format!",
				mx_stype_text[type]);
		}

	return(score);
	}

static mx_xscore_size_t _score2xscore(mx_real_t score, mx_stype_t type)
	{
	mx_xscore_size_t xs;

	/* first check parameters ... */
	if (!score)
		return(-1);
	if (!_stype_isvalid(type))
		rs_error("can't convert score of invalid type %d!",
			type);

	switch (type) {
		case mx_stype_Probability:
			xs = score * 10000.0;
			break;
		case mx_stype_LogProbability:
			xs = ((score < 650) ? (score) * 100.0 : 65535);
			break;
		default:
			rs_error("can't convert %s-type scores to external format!",
				mx_stype_text[type]);
		}

	return(xs);
	}

int mx_scoreset_fread(mx_scoreset_t *scoreset, FILE *fp)
	{
	int i, j, tot_scores = 0;
	mx_scorelist_t *scorelist;
	mx_xscore_size_t n;
	mx_xscore_t xscore;

	if (!scoreset || !fp)
		return(-1);

	mx_scoreset_reset(scoreset);

	if (scoreset->n_lists > 1) {
		if (fread(&n, sizeof(n), 1, fp) != 1)
			return(0);
		if (n != scoreset->n_lists)
			rs_error("number of codebooks %d does not match"
				"external score data %d!",
				n, scoreset->n_lists);
		}

	for (i = 0; i < scoreset->n_lists; i++) {
		scorelist = scoreset->list[i];

		if (fread(&n, sizeof(n), 1, fp) != 1) {
			if (scoreset->n_lists > 1)
				rs_error("can't read # of scores!");
			else	return(0);
			}
		if (n <= 0)
			rs_error("illegal # of scores %d read!", n);

		for (j = 0; j < n; j++) {
			if (fread(&xscore, sizeof(xscore), 1, fp) != 1)
				rs_error("can't read external score!");
			if (xscore.id < 0)
				rs_error("illegal id %d external score data!",
					xscore.id);

			mx_scorelist_addscore(scorelist,
				xscore.id,
				_xscore2score(xscore.score, scorelist->type));
			}

		tot_scores += scorelist->n_scores;
		}

	return(tot_scores);
	}

int mx_scoreset_fwrite(FILE *fp, mx_scoreset_t *scoreset)
	{
	int i, j, tot_scores = 0;
	mx_scorelist_t *scorelist;
	mx_xscore_size_t n;
	mx_xscore_t xscore;

	if (!scoreset || !fp)
		return(-1);

	if (scoreset->n_lists > 1) {
		n = scoreset->n_lists;
		if (fwrite(&n, sizeof(n), 1, fp) != 1)
			rs_error("can't write # of codebooks!");
		}

	for (i = 0; i < scoreset->n_lists; i++) {
		scorelist = scoreset->list[i];

		n = scorelist->n_scores;
		if (fwrite(&n, sizeof(n), 1, fp) != 1)
			rs_error("can't write # of scores!");
		
		for (j = 0; j < scorelist->n_scores; j++) {
			xscore.id = scorelist->score[j].id;
			xscore.score = _score2xscore(scorelist->score[j].score,
						scoreset->type);

			if (fwrite(&xscore, sizeof(xscore), 1, fp) != 1)
				rs_error("can't write score data!");
			}

		tot_scores += scorelist->n_scores;
		}

	return(tot_scores);
	}

int mx_scoreset_fprint(FILE *fp, mx_scoreset_t *scoreset)
	{
	int i, j, n_chars = 0;
	mx_scorelist_t *scorelist;

	for (i = 0; i < scoreset->n_lists; i++) {
		scorelist = scoreset->list[i];

		n_chars += fprintf(fp, "#%4d: [%3d] ", i, scorelist->n_scores);

		for (j = 0; j < scorelist->n_scores; j++)
			n_chars += fprintf(fp,
				(scorelist->type == mx_stype_Probability) ?
						"%03d(%1.2f) " : "%03d(%g) ",
					scorelist->score[j].id,
					scorelist->score[j].score);

		n_chars += fprintf(fp, "\n");
		}

	return(n_chars);
	}
