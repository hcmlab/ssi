/**
* Datei:	memory.h
* Autor:	Gernot A. Fink
* Datum:	29.7.1997
*
* Beschreibung:	Definitionen fuer Speicherverwaltung inkl. Fehlerbehandlung
**/

#ifndef __RS_MEMORY_H_INCLUDED__
#define __RS_MEMORY_H_INCLUDED__

#include <stdlib.h>

void *rs_malloc(size_t size, char *msg);
void *rs_realloc(void *pointer, size_t size, char *msg);
void *rs_calloc(size_t num_of_elts, size_t elt_size, char *msg);

void rs_free(void *pointer);

#endif /* __RS_MEMORY_H_INCLUDED__ */
