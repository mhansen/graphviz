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

int sfputc(Sfio_t * f, int c)
{
  return f->next >= f->endw ? _sfflsbuf(f, (int)((unsigned char)(c)))
                            : (int)(*f->next++ = (unsigned char)(c));
}
