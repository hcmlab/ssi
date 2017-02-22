/**
* Datei:	select.c
* Autor:	Gernot A. Fink
* Datum:	3.2.2005
*
* Beschreibung:	Verwaltung von Merkmalsselektionsvorschriften
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>

#include "ev_memory.h"
#include "ev_messages.h"
#include "ev_io.h"

#include "ev_vector.h"

#define FX_KERNEL
#include "ev_select.h"

fx_select_t *fx_select_create(int n_features)
	{
	fx_select_t *select;

	/* first check parameters ... */
	if (n_features <= 0)
		return(NULL);

	/* create emtpy container ... */
	select = rs_malloc(sizeof(fx_select_t), "head of feature selection");

	/* ... fill container and allocate characteristic vector ... */
	select->n_features = n_features;
	select->n_selected = 0;
	select->selected = rs_calloc(n_features, sizeof(int), "feature selection vector");

	return(select);
	}

void fx_select_destroy(fx_select_t *select)
	{
	/* first check parameters ... */
	if (!select)
		return;

	/* ... free and invalidate data structures ... */
	rs_free(select->selected);
	memset(select, -1, sizeof(fx_select_t));
	rs_free(select);
	}

int fx_select_fprint(FILE *fp, fx_select_t *select)
	{
	rs_error("not implemented, yet :-(");
	}

	/*
	 * NOTE: Selection strings can have the following formats
	 *
	 *	1. Index position:	<idx>	e.g. "1" "32"
	 *	2. Index range:	<min>-<max>	e.g. "1-8" "32-77"
	 *	3. Comma or White-space separated list of selections
	 */

int fx_select_sscan(fx_select_t *select, char *s)
	{
	int idx, min, max;
	int status, pos = 0;

	/* first check parameters ... */
	if (!select || !s)
		return(-1);

	/* for all selections in 's' ... */
	do	{
		/* skip leading white space and comma first ... */
		s = rs_line_skipwhite(s);
		if (*s == ',')
			s++;

		/* first check for range selection ... */
		status = sscanf(s, "%d-%d%n", &min, &max, &pos);
		if (status == 2) {
			if (min < 1 || max > select->n_features)
				return(-1);

			for (idx = min - 1; idx <= max - 1; idx++) {
				if (select->selected[idx] == 0)
					select->n_selected++;
				select->selected[idx] = 1;
				}
			}
		else if (sscanf(s, "%d%n", &idx, &pos) == 1) {
			if (idx < 1 || idx > select->n_features)
				return(-1);

			if (select->selected[idx - 1] == 0)
				select->n_selected++;
			select->selected[idx - 1] = 1;

			}
		else	return(-1);

		s += pos;
		}
	while (!rs_line_is_empty(s));

	return(select->n_selected);
	}

int fx_select_apply(fx_feature_t **dest, fx_select_t *select, fx_feature_t *src)
	{
	int i, j;

	/* first check parameters ... */
	if (!select || !src || !dest)
		return(-1);

	/* ... create destination vector, if necessary ... */
	if (!*dest)
		*dest = mx_vector_create(select->n_selected);

	/* ... for all vector compoments ... */
	for (i = 0, j = 0; i < select->n_features; i++) {
		/* ... if current vector compoment is selected ... */
		if (select->selected[i])
			/* ... copy it to destination vector */
			(*dest)[j++] = src[i];
		}

	return(select->n_features);
	}
