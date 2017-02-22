/**
* Datei:	hashq.h
* Autor:	Gernot A. Fink
* Datum:	13.8.1990
*
* Beschreibung:	Definitionen zum Hash-Queue-Modul
*
* letzte wichtige Aenderung:
*	28.07.1997, Gernot A. Fink:	Umstellung auf ANSI-C
**/

#ifndef __RS_HASHQ_H_INCLUDED__
#define __RS_HASHQ_H_INCLUDED__

#include <stdlib.h>

#define RS_HQ_DEFAULT	0
#define RS_HQ_NODUP	1

typedef struct rs_hashq_elem {
	void *elem;
	struct rs_hashq_elem *next;
	} rs_hashq_elem_t;
	
typedef struct {
	rs_hashq_elem_t **table;
	unsigned t_size;			/* size of queued hash table */
	unsigned e_size;			/* size of hash queue element */
	unsigned (* idx)(void *);
	int (* compare)(void *, void*);
	int mode;
	} rs_hashq_t;

/**
* function prototypes
**/
rs_hashq_t *rs_hashq_create(size_t t_size, size_t e_size,
	unsigned int (*idx)(void *), int (*compare)(void *, void *), int mode);
void rs_hashq_destroy(rs_hashq_t *hq);

void *rs_hashq_put(rs_hashq_t *hq, void *elem);
void *rs_hashq_get(rs_hashq_t *hq, void *elem);

int rs_hashq_del(rs_hashq_t *hq, void *elem);
int rs_hashq_flush(rs_hashq_t *hq);
void rs_hashq_free(rs_hashq_t *hq);

void rs_hashq_stat(rs_hashq_t *hq,
	int *t_size, int *min_chain, int *max_chain, int *tot_elem);
int rs_hashq_forall(rs_hashq_t *hq, int (*func)(void *));

unsigned int stridx(char *s);

#endif /* __RS_HASHQ_H_INCLUDED__ */
