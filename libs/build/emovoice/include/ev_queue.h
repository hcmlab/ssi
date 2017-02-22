/**
* Datei:	queue.h
* Autor:	Gernot A. Fink
* Datum:	28.7.1998
*
* Beschreibung:	Definitionen fuer Warteschlange
**/

#ifndef __RS_QUEUE_H_INCLUDED__
#define __RS_QUEUE_H_INCLUDED__

/* Eintrag in den Warteschlange ... */
typedef struct rs_queue_elem {
	struct rs_queue_elem *next;
	void *data;
	} rs_queue_elem_t;

/* Warteschlange ... */
typedef struct {
	int n_elems;
	rs_queue_elem_t *head;
	rs_queue_elem_t *tail;
	} rs_queue_t;

/*
 * Funktionsprototypen 
 */
rs_queue_t *rs_queue_create(void);
void rs_queue_destroy(rs_queue_t *queue);

void *rs_queue_push(rs_queue_t *queue, void *data);
void *rs_queue_pop(rs_queue_t *queue);
void *rs_queue_top(rs_queue_t *queue);

#define rs_queue_empty(q)	((q)->n_elems == 0)

#endif /* __RS_QUEUE_H_INCLUDED__ */
