/**
* Datei:	io.c
* Autor:	Gernot A. Fink
* Datum:	29.7.1997
*
* Beschreibung:	Ein-/Ausgabefunktionen 
**/

#include <string.h>
#include <ctype.h>

#include "ev_memory.h"
#include "ev_messages.h"

/** #include "ev_basics.h" **/
#include "ev_io.h"

static _line_len = RS_LINE_LEN_DEFAULT;

static size_t _fread_padded(char *ptr, size_t size, size_t n,
		size_t pad, FILE *fp);
static size_t _fwrite_padded(char *ptr, size_t size, size_t n,
		size_t pad, FILE *fp);

char *rs_line_read(FILE *fp, char comment_char)
	{
	static char *line = NULL;

	char *cp;
	int len, i;

	/* evtl. Zeilenpuffer erzeugen ... */
	if (line == NULL)
		line = rs_malloc(_line_len * sizeof(char), "line buffer");

	/* ... und solange noch Zeilen gelesen werden koennen ... */
	while (fgets(line, _line_len, fp) != NULL) {
		/* ... ggf. '\n' am Zeilenende loeschen ... */
		while (line[(len = strlen(line)) - 1] != '\n') {
			/* ... bei Pufferueberlauf ggf. erweitern ... */
			rs_warning("line longer than %d characters read - expanding!",
				_line_len);

			/* ... falls ueberlange TEXT-Zeile ... */
			for (i = 0; i < len; i++)
				if (!(isprint(line[i]) || isspace(line[i])))
					rs_error("non-printable character '0x%02x' encountered in excess text line!", line[i]);

			_line_len += RS_LINE_LEN_DEFAULT;
			line = rs_realloc(line, _line_len * sizeof(char),
					"extended line buffer");

			if (fgets(line + len, _line_len - len, fp) == NULL) {
				len++;
				break;
				}
			}
		line[len - 1] = '\0';

		/* ... ggf. Kommentare ueberspringen ... */
		if (comment_char && line[0] == comment_char)
			continue;

		/* ... und am Zeilenende loeschen falls nicht "escaped" ... */
		if (comment_char) {
			cp = line + 1;
			while (cp = strrchr(cp, comment_char)) {
				if (cp[-1] != '\\') {
					*cp = '\0';
					break;
					}
				}
			}

		/* ... und Zeilen mit Inhalt zurueckgeben */
		for (cp = line; *cp; cp++)
			if (!isspace(*cp))
				return(line);
		}

	/* ... sonst Dateiende signalisieren */
	return(NULL);
	}

int rs_line_is_empty(char *line)
	{
	while (*line)
		if (!isspace(*line++))
			return(0);
	return(1);
	}

char *rs_line_skipwhite(char *line)
	{
	while (isspace(*line))
		line++;
	return(line);
	}

size_t rs_line_setlength(size_t len)
	{
	size_t old_len = _line_len;

	_line_len = len;

	return(old_len);
	}

size_t rs_line_getlength(void)
	{
	return(_line_len);
	}

int rs_fskipwhite(FILE *fp, char comment_char)
	{
	int c;

	while ((c = fgetc(fp)) != EOF) {
		if (c == comment_char) {
			while ((c = fgetc(fp)) != EOF)
				if (c == '\n')
					break;
			}
		else if (!isspace(c)) {
			ungetc(c, fp);
			break;
			}
		}

	return(c);
	}

rs_IO *rs_fopen(char *name, char *mode)
	{
	FILE *fp;
	rs_IO *io;
	int mode_read, mode_mapped;
	void *buf;
	int buf_size;

	/* Lese-/Schreibmodus bestimmen ... */
	if (strchr(mode, 'w') || strchr(mode, 'a'))
		mode_read = 0;
	else if (strchr(mode, 'r'))
		mode_read = 1;
	else	rs_error("illegal mode '%s' for 'rs_fopen()'!", mode);

	/* ... ggf. ge-"mapped" (nur lesend) ... */
	if (strchr(mode, 'm')) {
		if (mode_read)
			mode_mapped = 1;
		else	{
			mode_mapped = 0;
			rs_warning("mapped file IO not supported for writing!");
			}
		}
	else	mode_mapped = 0;

	/* ... Spezialnamen '-' umsetzen ... */
	if (strcmp(name, "-") == 0) {
		if (mode_mapped)
			rs_error("mapped file IO not supported for stdin/out!");

		fp = (mode_read) ? stdin : stdout;
		}
	else	fp = fopen(name, (mode_read) ? "r" : "w");
	if (!fp)
		return(NULL);
	
	if (!mode_mapped) {
		io = rs_malloc(sizeof(rs_IO), "IO data");
		io->type = rs_iotype_file;
		io->base_size = RS_BINIO_BASE_SIZE_DEFAULT;
		io->fp = fp;
		}
	else	{
		fseek(fp, 0, SEEK_END);
		buf_size = ftell(fp);
		rewind(fp);

		buf = rs_malloc(buf_size, "mapped file buffer");
		fread(buf, buf_size, 1, fp);

		io = rs_mopen(buf, buf_size, "r");
		}

	return(io);
	}

rs_IO *rs_mopen(void *base, int size, char *mode)
	{
	rs_IO *io;

	if (strcmp(mode, "r") == 0) {
		if (!base || size <= 0)
			return(NULL);

		io = rs_malloc(sizeof(rs_IO), "IO data");
		io->type = rs_iotype_mem;
		io->base_size = RS_BINIO_BASE_SIZE_DEFAULT;

		io->mem.base = base;
		io->mem.pos = 0;
		io->mem.size = size;
		}
	else if (strcmp(mode, "w") == 0) {
		io = rs_malloc(sizeof(rs_IO), "IO data");
		io->type = rs_iotype_mem;
		io->base_size = RS_BINIO_BASE_SIZE_DEFAULT;

		if (base) {
			io->mem.base = base;
			io->mem.pos = 0;
			io->mem.size = size;
			}
		else	{
			io->mem.base = NULL;
			io->mem.pos = 0;
			io->mem.size = 0;
			}
		}
	else	rs_error("illegal mode '%s' for 'rs_mopen()'!", mode);

	return(io);
	}

void rs_close(rs_IO *io)
	{
	if (io)	{
		if (io->type == rs_iotype_file)
			fclose(io->fp);
		}
	}

int rs_bread(void *_ptr, int size, int n_elems, rs_IO *io)
	{
	int i;
	char *ptr = (char*)_ptr;

	if (!ptr || !io)
		return(-1);

	switch(io->type) {
		case rs_iotype_mem:
			if (io->base_size > size) {
				for (i = 0; i < n_elems; i++) {
					memcpy(ptr, io->mem.base + io->mem.pos,
							size);
					io->mem.pos += io->base_size;
					ptr += size;
					}
				}
			else if ((size % io->base_size) != 0)
				rs_error("can't convert %d-byte block data from %d-byte padding!",
					size, io->base_size);
			else	{
				memcpy(ptr, io->mem.base + io->mem.pos,
							size * n_elems);
				io->mem.pos += size * n_elems;
				}

			/* ACHTUNG: keine Grenzueberpruefung! */
			break;

		case rs_iotype_file:
			n_elems = _fread_padded(ptr, size, n_elems,
					io->base_size, io->fp);
			break;

		default:
			rs_error("IO type %d not supported by 'rs_bread()'!",
				io->type);
		}

	return(n_elems);
	}

int rs_bwrite(rs_IO *io, void *_ptr, int size, int n_elems)
	{
	char* ptr = (char*)_ptr;

	if (!ptr || !io)
		return(-1);

	switch(io->type) {
		case rs_iotype_mem:
			rs_error("binary write to memory not supported!");

			/* ACHTUNG: keine Grenzueberpruefung! */
			break;

		case rs_iotype_file:
			n_elems = _fwrite_padded(ptr, size, n_elems,
						io->base_size, io->fp);
			break;

		default:
			rs_error("IO type %d not supported by 'rs_bread()'!",
				io->type);
		}

	return(n_elems);
	}

static size_t _fread_padded(char *ptr, size_t size, size_t n,
		size_t pad, FILE *fp)
	{
	int i;

	if (pad > size) {
		for (i = 0; i < n; i++, ptr += size) {
			fread(ptr, size, 1, fp);
			fseek(fp, pad - size, SEEK_CUR);
			}
		}
	else if ((size % pad) != 0)
		rs_error("can't convert %d-byte block data from %d-byte padding!",
			size, pad);
	else	{
		return(fread(ptr, size, n, fp));
		}
	}

static size_t _fwrite_padded(char *ptr, size_t size, size_t n,
		size_t pad, FILE *fp)
	{
	int i;

	if (pad > size) {
		for (i = 0; i < n; i++, ptr += size) {
			fwrite(ptr, size, 1, fp);
			fseek(fp, pad - size, SEEK_CUR);
			}
		}
	else if ((size % pad) != 0)
		rs_error("can't convert %d-byte block data to %d-byte padding!",
			size, pad);
	else	{
		return(fwrite(ptr, size, n, fp));
		}
	}
