/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at http://www.graphviz.org/
 *************************************************************************/

#pragma once

    typedef void *gl_data;

    typedef struct {
	uint64_t used;	/* number of elements in the list */
	uint64_t size;	/* number of elements that the list can hold */
	gl_data *data;		/* pointer to first element */
    } generic_list_t;

    extern generic_list_t new_generic_list(uint64_t size);
    extern void add_to_generic_list(generic_list_t * list,
					       gl_data element);
    extern void free_generic_list(generic_list_t * list);
