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

#include <sfdpgen/post_process.h> 

typedef StressMajorizationSmoother UniformStressSmoother;

#define UniformStressSmoother_struct StressMajorizationSmoother_struct

void UniformStressSmoother_delete(UniformStressSmoother sm);

void uniform_stress(int dim, SparseMatrix A, double *x);
