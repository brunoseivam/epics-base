/* flex - tool to generate fast lexical analyzers */

/*  Copyright (c) 1990 The Regents of the University of California. */
/*  All rights reserved. */

/*  This code is derived from software contributed to Berkeley by */
/*  Vern Paxson. */

/*  The United States Government has rights in this work pursuant */
/*  to contract no. DE-AC03-76SF00098 between the United States */
/*  Department of Energy and the University of California. */

/*  This file is part of flex. */

/*  Redistribution and use in source and binary forms, with or without */
/*  modification, are permitted provided that the following conditions */
/*  are met: */

/*  1. Redistributions of source code must retain the above copyright */
/*     notice, this list of conditions and the following disclaimer. */
/*  2. Redistributions in binary form must reproduce the above copyright */
/*     notice, this list of conditions and the following disclaimer in the */
/*     documentation and/or other materials provided with the distribution. */

/*  Neither the name of the University nor the names of its contributors */
/*  may be used to endorse or promote products derived from this software */
/*  without specific prior written permission. */

/*  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR */
/*  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED */
/*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR */
/*  PURPOSE. */

#include "flexdef.h"

/* global buffers. */
struct Buf userdef_buf;		/* for user #definitions triggered by cmd-line. */
struct Buf defs_buf;		/* for #define's autogenerated. */
struct Buf yydmap_buf;		/* string buffer to hold yydmap elements */


/* Append a "%s" formatted string to a string buffer */
struct Buf *buf_prints (struct Buf *buf, const char *fmt, const char *s)
{
	char   *t;

	t = flex_alloc (strlen (fmt) + strlen (s) + 1);
	sprintf (t, fmt, s);
	buf = buf_strappend (buf, t);
	flex_free (t);
	return buf;
}


/* Appends n characters in str to buf. */
struct Buf *buf_strnappend (buf, str, n)
     struct Buf *buf;
     const char *str;
     int n;
{
	buf_append (buf, str, n + 1);

	/* "undo" the '\0' character that buf_append() already copied. */
	buf->nelts--;

	return buf;
}

/* Appends characters in str to buf. */
struct Buf *buf_strappend (buf, str)
     struct Buf *buf;
     const char *str;
{
	return buf_strnappend (buf, str, strlen (str));
}

/* appends "#define str def\n" */
struct Buf *buf_strdefine (buf, str, def)
     struct Buf *buf;
     const char *str;
     const char *def;
{
	buf_strappend (buf, "#define ");
	buf_strappend (buf, " ");
	buf_strappend (buf, str);
	buf_strappend (buf, " ");
	buf_strappend (buf, def);
	buf_strappend (buf, "\n");
	return buf;
}

/* create buf with 0 elements, each of size elem_size. */
void buf_init (buf, elem_size)
     struct Buf *buf;
     size_t elem_size;
{
	buf->elts = (void *) 0;
	buf->nelts = 0;
	buf->elt_size = elem_size;
	buf->nmax = 0;
}

/* frees memory */
void buf_destroy (buf)
     struct Buf *buf;
{
	if (buf && buf->elts)
		flex_free (buf->elts);
	buf->elts = (void *) 0;
}


/* appends ptr[] to buf, grow if necessary.
 * n_elem is number of elements in ptr[], NOT bytes.
 * returns buf.
 * We grow by mod(512) boundaries.
 */

struct Buf *buf_append (buf, ptr, n_elem)
     struct Buf *buf;
     const void *ptr;
     int n_elem;
{
	int     n_alloc = 0;

	if (!ptr || n_elem == 0)
		return buf;

	/* May need to alloc more. */
	if (n_elem + buf->nelts > buf->nmax) {

		/* exact amount needed... */
		n_alloc = (n_elem + buf->nelts) * buf->elt_size;

		/* ...plus some extra */
		if (((n_alloc * buf->elt_size) % 512) != 0
		    && buf->elt_size < 512)
			n_alloc +=
				(512 -
				 ((n_alloc * buf->elt_size) % 512)) /
				buf->elt_size;

		if (!buf->elts)
			buf->elts =
				allocate_array (n_alloc, buf->elt_size);
		else
			buf->elts =
				reallocate_array (buf->elts, n_alloc,
						  buf->elt_size);

		buf->nmax = n_alloc;
	}

	memcpy ((char *) buf->elts + buf->nelts * buf->elt_size, ptr,
		n_elem * buf->elt_size);
	buf->nelts += n_elem;

	return buf;
}

/* vim:set tabstop=8 softtabstop=4 shiftwidth=4: */
