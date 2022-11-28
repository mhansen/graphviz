/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <sparse/general.h>
#include <math.h>
#include <sparse/SparseMatrix.h>
#include <sfdpgen/spring_electrical.h>
#include <sfdpgen/post_process.h>
#include <sfdpgen/sparse_solve.h>
#include <time.h>
#include <sfdpgen/uniform_stress.h>
#include <stdbool.h>

/* uniform stress solves the model:

\Sum_{i<->j} (||x_i-x_j||-1)^2 + alpha * \Sum_{i!=j} (||x_i-x_j||^2-M)^2

This is somewhat similar to the binary stress model

*/

static UniformStressSmoother UniformStressSmoother_new(SparseMatrix A,
                                                       double alpha, double M) {
  UniformStressSmoother sm;
  int i, j, k, m = A->m, *ia = A->ia, *ja = A->ja, *iw, *jw, *id, *jd;
  int nz;
  double *d, *w, *a = A->a;
  double diag_d, diag_w, dist, epsilon = 0.01;

  assert(SparseMatrix_is_symmetric(A, false));

  sm = MALLOC(sizeof(struct StressMajorizationSmoother_struct));
  sm->data = NULL;
  sm->scheme = SM_SCHEME_UNIFORM_STRESS;
  sm->lambda = NULL;
  sm->data = MALLOC(sizeof(double)*2);
  ((double*) sm->data)[0] = alpha;
  ((double*) sm->data)[1] = M;
  sm->data_deallocator = free;
  sm->tol_cg = 0.01;
  sm->maxit_cg = (int)sqrt((double) A->m);

  /* Lw and Lwd have diagonals */
  sm->Lw = SparseMatrix_new(m, m, A->nz + m, MATRIX_TYPE_REAL, FORMAT_CSR);
  sm->Lwd = SparseMatrix_new(m, m, A->nz + m, MATRIX_TYPE_REAL, FORMAT_CSR);

  if (!(sm->Lw) || !(sm->Lwd)) {
    StressMajorizationSmoother_delete(sm);
    return NULL;
  }

  iw = sm->Lw->ia; jw = sm->Lw->ja;
  id = sm->Lwd->ia; jd = sm->Lwd->ja;
  w = sm->Lw->a;
  d = sm->Lwd->a;
  iw[0] = id[0] = 0;

  nz = 0;
  for (i = 0; i < m; i++){
    diag_d = diag_w = 0;
    for (j = ia[i]; j < ia[i+1]; j++){
      k = ja[j];
      if (k != i){
	dist = MAX(fabs(a[j]), epsilon);
	jd[nz] = jw[nz] = k;
	w[nz] = -1/(dist*dist);
	w[nz] = -1.;
	d[nz] = w[nz]*dist;
	diag_w += w[nz];
	diag_d += d[nz];
	nz++;
      }
    }
    jd[nz] = jw[nz] = i;
    w[nz] = -diag_w;
    d[nz] = -diag_d;
    nz++;

    iw[i+1] = nz;
    id[i+1] = nz;

  }

  sm->Lw->nz = nz;
  sm->Lwd->nz = nz;

  return sm;
}


void UniformStressSmoother_delete(UniformStressSmoother sm){

  StressMajorizationSmoother_delete(sm);

}

static double UniformStressSmoother_smooth(UniformStressSmoother sm, int dim, double *x, int maxit_sm) {

  return StressMajorizationSmoother_smooth(sm, dim, x, maxit_sm, 0.001);

}

static SparseMatrix get_distance_matrix(SparseMatrix A, double scaling){
  /* get a distance matrix from a graph, at the moment we just symmetrize the matrix. At the moment if the matrix is not real,
   we just assume distance of 1 among edges. Then we apply scaling to the entire matrix */
  SparseMatrix B;
  double *val;
  int i;

  if (A->type == MATRIX_TYPE_REAL){
    B = SparseMatrix_symmetrize(A, false);
  } else {
    B = SparseMatrix_get_real_adjacency_matrix_symmetrized(A);
  }
  val = B->a;
  if (scaling != 1) for (i = 0; i < B->nz; i++) val[i] *= scaling;
  return B;
}

extern void scale_to_box(double xmin, double ymin, double xmax, double ymax, int n, int dim, double *x);

void uniform_stress(int dim, SparseMatrix A, double *x) {
  UniformStressSmoother sm;
  double lambda0 = 10.1, M = 100, scaling = 1.;
  int maxit = 300, samepoint = TRUE, i, k, n = A->m;
  SparseMatrix B = NULL;

  /* just set random initial for now */
  for (i = 0; i < dim*n; i++) {
    x[i] = M*drand();
  }

  /* make sure x is not all at the same point */
  for (i = 1; i < n; i++){
    for (k = 0; k < dim; k++) {
      if (fabs(x[0*dim+k] - x[i*dim+k]) > MACHINEACC){
	samepoint = FALSE;
	i = n;
	break;
      }
    }
  }

  if (samepoint){
    srand(1);
#ifdef DEBUG_PRINT
    fprintf(stderr,"input coordinates to uniform_stress are the same, use random coordinates as initial input");
#endif
    for (i = 0; i < dim*n; i++) x[i] = M*drand();
  }

  B = get_distance_matrix(A, scaling);
  assert(SparseMatrix_is_symmetric(B, false));

  sm = UniformStressSmoother_new(B, 1000000*lambda0, M);
  UniformStressSmoother_smooth(sm, dim, x, maxit);
  UniformStressSmoother_delete(sm);

  sm = UniformStressSmoother_new(B, 10000*lambda0, M);
  UniformStressSmoother_smooth(sm, dim, x, maxit);
  UniformStressSmoother_delete(sm);

  sm = UniformStressSmoother_new(B, 100*lambda0, M);
  UniformStressSmoother_smooth(sm, dim, x, maxit);
  UniformStressSmoother_delete(sm);

  sm = UniformStressSmoother_new(B, lambda0, M);
  UniformStressSmoother_smooth(sm, dim, x, maxit);
  UniformStressSmoother_delete(sm);

  scale_to_box(0,0,7*70,10*70,A->m,dim,x);;

  SparseMatrix_delete(B);

}
