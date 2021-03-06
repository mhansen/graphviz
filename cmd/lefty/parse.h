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

/* Lefteris Koutsofios - AT&T Labs Research */

typedef struct Psrc_t {
    int flag;
    char *s;
    FILE *fp;
    int tok;
    int lnum;
} Psrc_t;

void Pinit (void);
void Pterm (void);
Tobj Punit (Psrc_t *);
Tobj Pfcall (Tobj, Tobj);
Tobj Pfunction (char *, int);
