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

#include <color.h>
#include <common/colorprocs.h>

#ifdef __cplusplus
extern "C" {
#endif

void rgb2hex(float r, float g, float b, char *cstring, const char* opacity);
  /* dimension of cstring must be >=7 */

char* hue2rgb(double hue, char *color);

#ifdef __cplusplus
}
#endif
