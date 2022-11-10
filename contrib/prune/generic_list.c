/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at http://www.graphviz.org/
 *************************************************************************/

#include <cgraph/alloc.h>
#include <stdlib.h>

#include "generic_list.h"

#define DFLT_SIZE 100

generic_list_t new_generic_list(size_t size) {
    generic_list_t list = {0};
    if (size != 0) {
	list.data = gv_calloc(size, sizeof(gl_data));
    }
    list.size = size;
    return list;
}

void free_generic_list(generic_list_t * list)
{
    if (list->size > 0) {
	free(list->data);
    }
}

void add_to_generic_list(generic_list_t *list, gl_data element) {
    size_t new_size;

    if (list->size == list->used) {
	if (list->size == 0) {
	    new_size = DFLT_SIZE;
	} else {
	    new_size = list->size * 2;
	}
	gl_data *new_data = gv_recalloc(list->data, list->size, new_size,
	                                sizeof(gl_data));
	list->data = new_data;
	list->size = new_size;
    }
    list->data[list->used++] = element;
}
