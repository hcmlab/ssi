/**
* Datei:	memory.c
* Autor:	Gernot A. Fink
* Datum:	29.7.1997
*
* Beschreibung:	Speicherverwaltung inkl. Fehlerbehandlung
**/

#include <stdio.h>
#include <stdlib.h>

#include "ev_messages.h"
#include "ev_memory.h"

void *rs_malloc(size_t size, char *msg)
	{
	void *pointer = NULL;

	if (size > 0) {
		pointer = malloc(size);

		if (pointer == NULL)
			rs_error("out of memory for %s!", msg);
		}
	else if (size < 0)
		rs_error("illegal memory size (%d bytes) for %s!",
			size, msg);

	return(pointer);
	}

void *rs_realloc(void *pointer, size_t size, char *msg)
	{
#ifdef NO_RALLOC_OF_VOID
	if (pointer == NULL)
		return(rs_malloc(size, msg));
#endif

	if (size >= 0) {
		pointer = realloc(pointer, size);

		if (pointer == NULL && size > 0)
			rs_error("can't grow memory for %s!", msg);
		}
	else	rs_error("illegal memory resizing (%d bytes) for %s!",
			size, msg);

	return(pointer);
	}

void *rs_calloc(size_t num_of_elts, size_t elt_size, char *msg)
	{
	void *pointer = NULL;

	if (num_of_elts > 0 && elt_size > 0) {
		pointer = calloc(num_of_elts, elt_size);

		if (pointer == NULL)
			rs_error("out of memory for %s!", msg);
		}
	else if (num_of_elts < 0 || elt_size < 0)
		 rs_error("can't allocate %dx%d bytes for %s!",
			num_of_elts, elt_size, msg);

	return(pointer);
	}

void rs_free(void *pointer)
	{
	if (pointer != NULL)
		free(pointer);
	else
		rs_warning("trying to free a null-pointer");
	}
