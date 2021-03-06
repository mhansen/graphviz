/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include	<sfio/sfhdr.h>
#include	<stddef.h>

/*	Fill the buffer of a stream with data.
**	If n < 0, sffilbuf() attempts to fill the buffer if it's empty.
**	If n == 0, if the buffer is not empty, just return the first byte;
**		otherwise fill the buffer and return the first byte.
**	If n > 0, even if the buffer is not empty, try a read to get as
**		close to n as possible. n is reset to -1 if stack pops.
**
**	Written by Kiem-Phong Vo
*/

/**
 * @param f fill the read buffer of this stream
 * @param n see above
 */
int _sffilbuf(Sfio_t * f, int n)
{
    ssize_t r;
    int first, local, rcrv, rc, justseek;

    SFMTXSTART(f, -1);

    GETLOCAL(f, local);

    /* any peek data must be preserved across stacked streams */
    rcrv = f->mode & (SF_RC | SF_RV | SF_LOCK);
    rc = f->getr;

    justseek = f->bits & SF_JUSTSEEK;
    f->bits &= (unsigned short)~SF_JUSTSEEK;

    for (first = 1;; first = 0, (f->mode &= ~SF_LOCK)) {	/* check mode */
	if (SFMODE(f, local) != SF_READ && _sfmode(f, SF_READ, local) < 0)
	    SFMTXRETURN(f, -1);
	SFLOCK(f, local);

	/* current extent of available data */
	if ((r = f->endb - f->next) > 0) {	/* on first iteration, n is amount beyond current buffer;
						   afterward, n is the exact amount requested */
	    if ((first && n <= 0) || (!first && n <= r) ||
		(f->flags & SF_STRING))
		break;

	    /* try shifting left to make room for new data */
	    if (!(f->bits & SF_MMAP) && f->next > f->data &&
		n > (f->size - (f->endb - f->data))) {
		memcpy(f->data, f->next, (size_t)r);
		f->next = f->data;
		f->endb = f->data + r;
	    }
	} else if (!(f->flags & SF_STRING) && !(f->bits & SF_MMAP))
	    f->next = f->endb = f->endr = f->data;

	if (f->bits & SF_MMAP)
	    r = n > 0 ? n : f->size;
	else if (!(f->flags & SF_STRING)) {
	    r = f->size - (f->endb - f->data);	/* available buffer */
	    if (n > 0) {
		if (r > n && f->extent < 0 && (f->flags & SF_SHARE))
		    r = n;	/* read only as much as requested */
		else if (justseek && (size_t)n <= f->iosz && f->iosz <= f->size)
		    r = f->iosz;	/* limit buffer filling */
	    }
	}

	/* SFRD takes care of discipline read and stack popping */
	f->mode |= rcrv;
	f->getr = rc;
	if ((r = SFRD(f, f->endb, r, f->disc)) >= 0) {
	    r = f->endb - f->next;
	    break;
	}
    }

    SFOPEN(f, local);

    rcrv = (n == 0) ? (r > 0 ? (int) (*f->next++) : EOF) : (int) r;

    SFMTXRETURN(f, rcrv);
}
