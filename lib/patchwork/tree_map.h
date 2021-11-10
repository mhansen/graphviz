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

#include <sparse/SparseMatrix.h>

typedef struct rectangle_struct {
  double x[2];/* center */
  double size[2]; /* total width/height*/
} rectangle;

extern rectangle* tree_map(int n, double *area, rectangle fillrec);

extern rectangle rectangle_new(double x, double y, double width, double height);
