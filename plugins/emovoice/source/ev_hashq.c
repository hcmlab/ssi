/**
* Datei:	hashq.c
* Autor:	Gernot A. Fink
* Datum:	13.8.1990
*
* Beschreibung:	Modul zur Erzeugung und Verwaltung einer allgemeinen
*		Hashtabelle mit verketteten Listen (Queues)
*
* letzte wichtige Aenderung:
*	28.07.1997, Gernot A. Fink:	Umstellung auf ANSI-C
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ev_memory.h"
#include "ev_hashq.h"

static void *memdup(void *src, size_t size);

static void _hashq_del(rs_hashq_elem_t **elem_p);
static int _hashq_csize(rs_hashq_elem_t *h_elem);

static void *memdup(void *src, size_t size)
	{
	void *dest;
	
	dest = rs_malloc(size, "hash-queue element contents");

	memcpy(dest, src, size);
	return(dest);
	}
	
rs_hashq_t *rs_hashq_create(size_t t_size, size_t e_size,
		unsigned int (*idx)(void *),
		int (*compare)(void *, void *),
		int mode)
	{
	unsigned int i;
	
	rs_hashq_t *hq;
	
	hq = rs_malloc(sizeof(rs_hashq_t), "hash-queue");
		/* allocate table entries initialized with NULL */
	hq->table = rs_calloc(t_size, sizeof(char *), "hash-queue table");
	
	hq->t_size = t_size;
	hq->e_size = e_size;
	hq->idx = idx;
	hq->compare = compare;
	hq->mode = mode;
	
	return(hq);
	}

void rs_hashq_destroy(rs_hashq_t *hq)
	{
	if (!hq)
		return;

	rs_free(hq->table);
	rs_free(hq);
	}

void *rs_hashq_put(rs_hashq_t *hq, void *elem)
	{
	unsigned int idx;
	rs_hashq_elem_t **elem_p;
	
	idx = hq->idx(elem) % hq->t_size;
	elem_p = hq->table + idx;
	
	while (*elem_p) {
		if (hq->compare((*elem_p)->elem,elem) == 0)
			return((*elem_p)->elem);
		elem_p = &((*elem_p)->next);
		}
	
	*elem_p = rs_malloc(sizeof(rs_hashq_elem_t), "hash-queue element");

	if (hq->mode & RS_HQ_NODUP)
		(*elem_p)->elem = elem;
	else
		(*elem_p)->elem = memdup(elem, hq->e_size);
	(*elem_p)->next = NULL;

	return((*elem_p)->elem);
	}

void *rs_hashq_get(rs_hashq_t *hq, void *elem)
	{
	unsigned int idx;
	rs_hashq_elem_t **elem_p;
	
	idx = hq->idx(elem) % hq->t_size;
	elem_p = hq->table + idx;
	
	while (*elem_p) {
		if (hq->compare((*elem_p)->elem,elem) == 0)
			return((*elem_p)->elem);
		elem_p = &((*elem_p)->next);
		}

	return(NULL);
	}

static void _hashq_del(rs_hashq_elem_t **elem_p)
	{
	rs_hashq_elem_t *tmp;
	
	tmp = (*elem_p)->next;

	free((*elem_p)->elem);
	free(*elem_p);

	*elem_p = tmp;
	}

int rs_hashq_del(rs_hashq_t *hq, void *elem)
	{
	unsigned int idx;
	rs_hashq_elem_t **elem_p, *tmp;
	
	idx = hq->idx(elem) % hq->t_size;
	elem_p = hq->table + idx;
	
	while (*elem_p) {
		if (hq->compare((*elem_p)->elem,elem) == 0) {
			_hashq_del(elem_p);
			return(0);
			}
		elem_p = &((*elem_p)->next);
		}

	return(-1);
	}

int rs_hashq_flush(rs_hashq_t *hq)
	{
	unsigned int i;
	rs_hashq_elem_t **elem_p;
	
	for (i = 0; i < hq->t_size; i++) {
		elem_p = hq->table+i;
		while (*elem_p)
			_hashq_del(elem_p);
		}

	return(0);
	}
	
void rs_hashq_free(rs_hashq_t *hq)
	{
	rs_hashq_flush(hq);

	free(hq->table);	
	free(hq);
	}
	
static int _hashq_csize(rs_hashq_elem_t *h_elem)
	{
	int csize = 0;
	
	while (h_elem) {
		csize++;
		h_elem = h_elem->next;
		}
	return(csize);
	}

void rs_hashq_stat(rs_hashq_t *hq,
		int *t_size, int *min_chain, int *max_chain, int *tot_elem)
	{
	int min,max,tot,csize,i;
	
	max = min = tot = _hashq_csize(hq->table[0]);
	for (i = 1; i < hq->t_size; i++) {
		csize = _hashq_csize(hq->table[i]);
		tot += csize;
		if (csize > max)
			max = csize;
		if (csize < min)
			min = csize;
		}
	
	*t_size = hq->t_size;
	*min_chain = min;
	*max_chain = max;
	*tot_elem = tot;
	}

int rs_hashq_forall(rs_hashq_t *hq, int (*func)(void *))
	{
	unsigned i;
	rs_hashq_elem_t **elem_p;
	
	for (i = 0; i < hq->t_size; i++) {
		elem_p = hq->table+i;
	
		while (*elem_p) {
			(*func)((*elem_p)->elem);
			elem_p = &((*elem_p)->next);
			}
		}
	
	return(0);
	}

unsigned int stridx(char *s)
	{
	unsigned int idx;
	
	idx = 0;
	while (*s) 
		idx = 2*idx + *s++;
	return(idx);
	}
	
#ifdef TEST
unsigned idx(char *s)
	{
	return(*s);
	}
	
main()
	{
	rs_hashq_t *hq;
	char entry[20],cmd;
	int t_size,min,max,tot;
	
	hq = rs_hashq_create(11,20,idx,strcmp, RS_HQ_DEFAULT);
	
	while (!feof(stdin)) {
		if (scanf("%c%s",&cmd,entry) != 2)
			break;
		while (getchar() != '\n');
		switch (cmd) {
			case 'a' :
				rs_hashq_put(hq,entry);
				break;
			case 'f' :
				if (rs_hashq_get(hq,entry) == NULL)
					printf("not found!\n");
				else	printf("found\n");
				break;
			case 'd' :
				rs_hashq_del(hq,entry);
				break;
			case 's' :
				rs_hashq_stat(hq,&t_size,&min,&max,&tot);
				printf("hashq-statistics:\nt:%d min:%d max:%d tot:%d\n",
					t_size,min,max,tot);
				break;				
			case 'n' :
				rs_hashq_free(hq);
				hq = rs_hashq_create(11,20,idx,strcmp, RS_HQ_DEFAULT);
				break;
			default :
				printf("illegal command!\n");
			}
		}
	}
#endif
