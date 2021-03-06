/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include "config.h"

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <gvc/gvplugin_loadimage.h>
#include "gvplugin_gdiplus.h"

#include <windows.h>
#include <gdiplus.h>

#include "FileStream.h"

using namespace Gdiplus;

static void gdiplus_freeimage(usershape_t *us) {
  delete reinterpret_cast<Image*>(us->data);
}

static Image* gdiplus_loadimage(GVJ_t * job, usershape_t *us)
{
    assert(job);
    assert(us);
    assert(us->name);

    if (us->data && us->datafree != gdiplus_freeimage) {
	     us->datafree(us);        /* free incompatible cache data */
	     us->data = nullptr;
	     us->datafree = nullptr;
	}
    
    if (!us->data) { /* read file into cache */
		if (!gvusershape_file_access(us))
			return nullptr;

		/* create image from the usershape file */
		/* NOTE: since Image::FromStream consumes the stream, we assume FileStream's lifetime should be shorter than us->f... */
		IStream *stream = FileStream::Create(us->name, us->f);
		us->data = Image::FromStream (stream);
		
		/* clean up */
		if (us->data)
			us->datafree = gdiplus_freeimage;
		stream->Release();
			
		gvusershape_file_release(us);
    }
    return reinterpret_cast<Image*>(us->data);
}

static void gdiplus_loadimage_gdiplus(GVJ_t * job, usershape_t *us, boxf b, bool)
{
	/* get the image from usershape details, then blit it to the context */
	Image *image = gdiplus_loadimage(job, us);
	if (image) {
		auto g = reinterpret_cast<Graphics*>(job->context);
		g->DrawImage(image, RectF(b.LL.x, b.LL.y, b.UR.x - b.LL.x, b.UR.y - b.LL.y));
	}
}

static gvloadimage_engine_t engine = {
    gdiplus_loadimage_gdiplus
};

gvplugin_installed_t gvloadimage_gdiplus_types[] = {
	{FORMAT_BMP, "bmp:gdiplus", 8, &engine, nullptr},
	{FORMAT_GIF, "gif:gdiplus", 8, &engine, nullptr},
	{FORMAT_JPEG, "jpe:gdiplus", 8, &engine, nullptr},
	{FORMAT_JPEG, "jpeg:gdiplus", 8, &engine, nullptr},
	{FORMAT_JPEG, "jpg:gdiplus", 8, &engine, nullptr},
	{FORMAT_PNG, "png:gdiplus", 8, &engine, nullptr},
	{0, nullptr, 0, nullptr, nullptr}
};
