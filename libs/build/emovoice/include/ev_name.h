/**
* Datei:	name.h
* Autor:	Gernot A. Fink
* Datum:	5.8.1997
*
* Beschreibung:	Definitionen fuer Namensverwaltung
**/

#ifndef __RS_NAME_H_INCLUDED__
#define __RS_NAME_H_INCLUDED__

char *rs_name_create(char *s);
char *rs_name_prepare(int len);
void rs_name_destroy(char *name);

#endif /* __RS_NAME_H_INCLUDED__ */
