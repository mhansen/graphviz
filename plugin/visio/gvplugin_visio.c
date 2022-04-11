/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <gvc/gvplugin.h>

extern gvplugin_installed_t gvdevice_vdx_types[];
extern gvplugin_installed_t gvrender_vdx_types[];

static gvplugin_api_t apis[] = {
    {API_device, gvdevice_vdx_types},
    {API_render, gvrender_vdx_types},
    {(api_t)0, 0},
};

#ifdef GVDLL
__declspec(dllexport)
#endif
gvplugin_library_t gvplugin_visio_LTX_library = { "visio", apis };
