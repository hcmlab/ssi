/**
* Datei:	queue.c
* Autor:	Gernot A. Fink
* Datum:	28.7.1998
*
* Beschreibung:	Warteschlangenverwaltung
**/

#include <string.h>

#include "ev_memory.h"

#include "ev_queue.h"

static rs_queue_elem_t *_queue_elem_create(void);
static void _queue_elem_destroy(rs_queue_elem_t *elem);

rs_queue_t *rs_queue_create(void)
	{
	rs_queue_t *queue;

	queue = rs_malloc(sizeof(rs_queue_t), "queue");

	queue->n_elems = 0;
	queue->head = NULL;
	queue->tail = NULL;

	return(queue);
	}

void rs_queue_destroy(rs_queue_t *queue)
	{
	rs_queue_elem_t *elem, *next;

	if (!queue)
		return;

	elem = queue->head;
	while (elem) {
		next = elem->next;
		_queue_elem_destroy(elem);
		elem = next;
		}
	}

static rs_queue_elem_t *_queue_elem_create(void)
	{
	rs_queue_elem_t *elem;

	elem = rs_malloc(sizeof(rs_queue_elem_t), "queue element");

	return(elem);
	}

static void _queue_elem_destroy(rs_queue_elem_t *elem)
	{
	if (elem)
		rs_free(elem);
	}

void *rs_queue_push(rs_queue_t *queue, void *data)
	{
	rs_queue_elem_t *elem;

	if (!queue || !data)
		return(NULL);

	elem = _queue_elem_create();

	elem->next = NULL;
	elem->data = data;

	if (queue->tail)
		queue->tail->next = elem;
	else	queue->head = elem;
	queue->tail = elem;

	queue->n_elems++;

	return(data);		
	}

void *rs_queue_pop(rs_queue_t *queue)
	{
	void *data;
	rs_queue_elem_t *elem;

	if (!queue || !queue->head)
		return(NULL);

	elem = queue->head;
	queue->head = queue->head->next;
	if (!queue->head)
		queue->tail = NULL;
	data = elem->data;
	_queue_elem_destroy(elem);

	queue->n_elems--;

	return(data);
	}

void *rs_queue_top(rs_queue_t *queue)
	{
	if (!queue)
		return(NULL);

	return(queue->head->data);
	}
