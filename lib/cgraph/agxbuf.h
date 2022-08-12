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

#include <cgraph/alloc.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* Extensible buffer:
 *  Malloc'ed memory is never released until agxbfree is called.
 */
typedef struct {
  char *buf;           // start of buffer
  char *ptr;           // next place to write
  char *eptr;          // end of buffer
  int stack_allocated; // false if buffer is malloc'ed
} agxbuf;

/* agxbinit:
 * Initializes new agxbuf; caller provides memory.
 * Assume if init is non-null, hint = sizeof(init[])
 */
static inline void agxbinit(agxbuf *xb, unsigned int hint, char *init) {
  if (init != NULL) {
    xb->buf = init;
    xb->stack_allocated = 1;
  } else {
    if (hint == 0) {
      hint = BUFSIZ;
    }
    xb->stack_allocated = 0;
    xb->buf = (char *)gv_calloc(hint, sizeof(char));
  }
  xb->eptr = xb->buf + hint;
  xb->ptr = xb->buf;
  *xb->ptr = '\0';
}

/* agxbfree:
 * Free any malloced resources.
 */
static inline void agxbfree(agxbuf *xb) {
  if (!xb->stack_allocated)
    free(xb->buf);
}

/* agxbpop:
 * Removes last character added, if any.
 */
static inline int agxbpop(agxbuf *xb) {
  int c;
  if (xb->ptr > xb->buf) {
    c = *xb->ptr--;
    return c;
  } else
    return -1;
}

/* agxbmore:
 * Expand buffer to hold at least ssz more bytes.
 */
static inline void agxbmore(agxbuf *xb, size_t ssz) {
  size_t cnt = 0;   // current no. of characters in buffer
  size_t size = 0;  // current buffer size
  size_t nsize = 0; // new buffer size
  char *nbuf;       // new buffer

  size = (size_t)(xb->eptr - xb->buf);
  nsize = size == 0 ? BUFSIZ : (2 * size);
  if (size + ssz > nsize)
    nsize = size + ssz;
  cnt = (size_t)(xb->ptr - xb->buf);
  if (!xb->stack_allocated) {
    nbuf = (char *)gv_recalloc(xb->buf, size, nsize, sizeof(char));
  } else {
    nbuf = (char *)gv_calloc(nsize, sizeof(char));
    memcpy(nbuf, xb->buf, cnt);
    xb->stack_allocated = 0;
  }
  xb->buf = nbuf;
  xb->ptr = xb->buf + cnt;
  xb->eptr = xb->buf + nsize;
}

/* support for extra API misuse warnings if available */
#ifdef __GNUC__
#define PRINTF_LIKE(index, first) __attribute__((format(printf, index, first)))
#else
#define PRINTF_LIKE(index, first) /* nothing */
#endif

/* agxbprint:
 * Printf-style output to an agxbuf
 */
static inline PRINTF_LIKE(2, 3) int agxbprint(agxbuf *xb, const char *fmt,
                                              ...) {
  va_list ap;
  size_t size;
  int result;

  va_start(ap, fmt);

  // determine how many bytes we need to print
  {
    va_list ap2;
    int rc;
    va_copy(ap2, ap);
    rc = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);
    if (rc < 0) {
      va_end(ap);
      return rc;
    }
    size = (size_t)rc + 1; // account for NUL terminator
  }

  // do we need to expand the buffer?
  {
    size_t unused_space = (size_t)(xb->eptr - xb->ptr);
    if (unused_space < size) {
      size_t extra = size - unused_space;
      agxbmore(xb, extra);
    }
  }

  // we can now safely print into the buffer
  result = vsnprintf(xb->ptr, size, fmt, ap);
  assert(result == (int)(size - 1) || result < 0);
  if (result > 0) {
    xb->ptr += (size_t)result;
  }

  va_end(ap);
  return result;
}

#undef PRINTF_LIKE

/* agxbput_n:
 * Append string s of length ssz into xb
 */
static inline size_t agxbput_n(agxbuf *xb, const char *s, size_t ssz) {
  if (ssz == 0) {
    return 0;
  }
  if (xb->ptr + ssz > xb->eptr)
    agxbmore(xb, ssz);
  memcpy(xb->ptr, s, ssz);
  xb->ptr += ssz;
  return ssz;
}

/* agxbput:
 * Append string s into xb
 */
static inline size_t agxbput(agxbuf *xb, const char *s) {
  size_t ssz = strlen(s);

  return agxbput_n(xb, s, ssz);
}

/* agxbputc:
 * Add character to buffer.
 *  int agxbputc(agxbuf*, char)
 */
static inline int agxbputc(agxbuf *xb, char c) {
  if (xb->ptr >= xb->eptr) {
    agxbmore(xb, 1);
  }
  *xb->ptr++ = c;
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
  return xb->ptr;
}

/* agxbstart:
 * Return pointer to beginning of buffer.
 */
static inline char *agxbstart(agxbuf *xb) { return xb->buf; }

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
static inline char *agxbnext(agxbuf *xb) { return xb->ptr; }

/* agxbdisown:
 * Disassociate the backing buffer from this agxbuf and return it. The buffer is
 * NUL terminated before being returned. If the agxbuf is using stack memory,
 * this will first copy the data to a new heap buffer to then return. If you
 * want to temporarily access the string in the buffer, but have it overwritten
 * and reused the next time, e.g., agxbput is called, use agxbuse instead of
 * agxbdisown.
 */
static inline char *agxbdisown(agxbuf *xb) {
  char *buf;

  if (xb->stack_allocated) {
    // the buffer is not dynamically allocated, so we need to copy its contents
    // to heap memory

    buf = gv_strndup(xb->buf, agxblen(xb));

  } else {
    // the buffer is already dynamically allocated, so terminate it and then
    // take it as-is
    agxbputc(xb, '\0');
    buf = xb->buf;
  }

  // reset xb to a state where it is usable
  xb->buf = xb->ptr = xb->eptr = NULL;
  xb->stack_allocated = 0;

  return buf;
}
