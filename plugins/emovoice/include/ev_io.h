/**
* Datei:	io.h
* Autor:	Gernot A. Fink
* Datum:	29.7.1997
*
* Beschreibung:	Definitionen fuer Ein-/Ausgabefunktionen 
**/

#ifndef __RS_IO_H_INCLUDED__
#define __RS_IO_H_INCLUDED__

#include <stdio.h>

/*
 * line oriented IO
 */
#define RS_LINE_LEN_DEFAULT	8192

char *rs_line_read(FILE *fp, char comment_char);
int rs_line_is_empty(char *line);
char *rs_line_skipwhite(char *line);

size_t rs_line_setlength(size_t len);
size_t rs_line_getlength(void);

int rs_fskipwhite(FILE *fp, char comment_char);

/*
 * binary IO
 */

#define	RS_BINIO_BASE_SIZE_DEFAULT	(sizeof(int))

typedef enum {rs_iotype_mem, rs_iotype_file} rs_iotype_t;

typedef struct {
	char *base;
	int pos;
	int size;
	} rs_memIO;

typedef struct {
	rs_iotype_t type;
	int base_size;
	rs_memIO mem;
	FILE *fp;
	} rs_IO;

rs_IO *rs_fopen(char *name, char *mode);
rs_IO *rs_mopen(void *base, int size, char *mode);
void rs_close(rs_IO *io);

int rs_bread(void *ptr, int size, int n_elems, rs_IO *io);
int rs_bwrite(rs_IO *io, void *ptr, int size, int n_elems);

#endif /* __RS_IO_H_INCLUDED__ */
