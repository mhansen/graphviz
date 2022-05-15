/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#include "config.h"

#ifdef GVDLL
#ifdef EXPORT_AGXBUF
#define AGXBUF_API __declspec(dllexport)
#else
#define AGXBUF_API __declspec(dllimport)
#endif
#endif

#ifndef AGXBUF_API
#define AGXBUF_API /* nothing */
#endif

/* Extensible buffer:
 *  Malloc'ed memory is never released until agxbfree is called.
 */
    typedef struct {
	unsigned char *buf;	/* start of buffer */
	unsigned char *ptr;	/* next place to write */
	unsigned char *eptr;	/* end of buffer */
	int dyna;		/* true if buffer is malloc'ed */
    } agxbuf;

/* agxbinit:
 * Initializes new agxbuf; caller provides memory.
 * Assume if init is non-null, hint = sizeof(init[])
 */
    AGXBUF_API void agxbinit(agxbuf * xb, unsigned int hint,
			 unsigned char *init);

/* support for extra API misuse warnings if available */
#ifdef __GNUC__
  #define PRINTF_LIKE(index, first) __attribute__((format(printf, index, first)))
#else
  #define PRINTF_LIKE(index, first) /* nothing */
#endif

/* agxbprint:
 * Printf-style output to an agxbuf
 */
    AGXBUF_API int agxbprint(agxbuf * xb, const char *fmt, ...)
       PRINTF_LIKE(2, 3);

#undef PRINTF_LIKE

/* agxbput_n:
 * Append string s of length n into xb
 */
    AGXBUF_API size_t agxbput_n(agxbuf * xb, const char *s, size_t n);

/* agxbput:
 * Append string s into xb
 */
    AGXBUF_API size_t agxbput(agxbuf * xb, const char *s);

/* agxbfree:
 * Free any malloced resources.
 */
    AGXBUF_API void agxbfree(agxbuf * xb);

/* agxbpop:
 * Removes last character added, if any.
 */
    AGXBUF_API int agxbpop(agxbuf * xb);

/* agxbmore:
 * Expand buffer to hold at least ssz more bytes.
 */
    AGXBUF_API void agxbmore(agxbuf * xb, size_t ssz);

/* agxbputc:
 * Add character to buffer.
 *  int agxbputc(agxbuf*, char)
 */
static inline int agxbputc(agxbuf * xb, char c) {
  if (xb->ptr >= xb->eptr) {
    agxbmore(xb, 1);
  }
  *xb->ptr++ = (unsigned char)c;
  return 0;
}

/* agxbuse:
 * Null-terminates buffer; resets and returns pointer to data. The buffer is
 * still associated with the agxbuf and will be overwritten on the next, e.g.,
 * agxbput. If you want to retrieve and disassociate the buffer, use agxbdisown
 * instead.
 */
static inline char *agxbuse(agxbuf *xb) {
  (void)agxbputc(xb, '\0');
  xb->ptr = xb->buf;
  return (char *)xb->ptr;
}

/* agxbstart:
 * Return pointer to beginning of buffer.
 */
static inline char *agxbstart(agxbuf *xb) { return (char *)xb->buf; }

/* agxblen:
 * Return number of characters currently stored.
 */
static inline size_t agxblen(const agxbuf *xb) {
  return (size_t)(xb->ptr - xb->buf);
}

/* agxbclear:
 * Resets pointer to data;
 */
static inline void agxbclear(agxbuf *xb) { xb->ptr = xb->buf; }

/* agxbnext:
 * Next position for writing.
 */
static inline char *agxbnext(agxbuf *xb) { return (char *)xb->ptr; }

/* agxbdisown:
 * Disassociate the backing buffer from this agxbuf and return it. The buffer is
 * NUL terminated before being returned. If the agxbuf is using stack memory,
 * this will first copy the data to a new heap buffer to then return. If failure
 * occurs, NULL is returned. If you want to temporarily access the string in the
 * buffer, but have it overwritten and reused the next time, e.g., agxbput is
 * called, use agxbuse instead of agxbdisown.
 */
    AGXBUF_API char *agxbdisown(agxbuf * xb);

#ifdef __cplusplus
}
#endif
