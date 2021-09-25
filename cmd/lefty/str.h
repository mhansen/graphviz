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

void Sinit (void);
void Sterm (void);
char *Spath (char *, Tobj);
char *Sseen (Tobj, char *);
char *Sabstract (Tobj, Tobj);
char *Stfull (Tobj);
char *Ssfull (Tobj, Tobj);
char *Scfull (Tobj, int, int);
