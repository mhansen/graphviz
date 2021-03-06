/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include "toolboxcallbacks.h"
#include "viewport.h"

#include "gltemplate.h"
#include <glcomp/glutils.h>
#include "glmotion.h"





void btnToolZoomOut_clicked(GtkWidget * widget, gpointer user_data)
{
    glmotion_zoom_inc(0);
}

void btnToolZoomFit_clicked(GtkWidget * widget, gpointer user_data)
{

    float z, GDX, SDX, GDY, SDY;
    (view->active_camera >= 0)
	? (z = view->cameras[view->active_camera]->r) : (z =
							 view->zoom * -1);

    GDX = (view->bdxRight / z - view->bdxLeft / z);
    SDX = (view->clipX2 - view->clipX1);
    GDY = (view->bdyTop / z - view->bdyBottom / z);
    SDY = (view->clipY2 - view->clipY1);

    if ((SDX / GDX) <= (SDY / GDY)) {
	(view->active_camera >= 0) ?
	    (view->cameras[view->active_camera]->r =
	     view->cameras[view->active_camera]->r / (SDX /
						      GDX)) : (view->zoom =
							       view->zoom /
							       (SDX /
								GDX));
    } else {
	(view->active_camera >= 0) ?
	    (view->cameras[view->active_camera]->r =
	     view->cameras[view->active_camera]->r / (SDY /
						      GDY)) : (view->zoom =
							       view->zoom /
							       (SDY /
								GDY));

    }
    btnToolFit_clicked(NULL, NULL);
}

void btnToolFit_clicked(GtkWidget * widget, gpointer user_data)
{
    float scx, scy, gcx, gcy, z;



    (view->active_camera >= 0)
	? (z = view->cameras[view->active_camera]->r) : (z =
							 view->zoom * -1);





    gcx =
	view->bdxLeft / z + (view->bdxRight / z -
			     view->bdxLeft / z) / (float) (2.0);
    scx = view->clipX1 + (view->clipX2 - view->clipX1) / (float) (2.0);
    gcy =
	view->bdyBottom / z + (view->bdyTop / z -
			       view->bdyBottom / z) / (float) (2.0);
    scy = view->clipY1 + (view->clipY2 - view->clipY1) / (float) (2.0);



    if (view->active_camera >= 0) {

	view->cameras[view->active_camera]->targetx += (gcx - scx);
	view->cameras[view->active_camera]->targety += (gcx - scy);
    } else {
	view->panx += (gcx - scx);
	view->pany += (gcy - scy);
    }
    view->Topview->fitin_zoom = view->zoom;

    glexpose();
}

void on_btnActivateGraph_clicked(GtkWidget * widget, gpointer user_data)
{
    int graphId;
    graphId = gtk_combo_box_get_active(view->graphComboBox);
    /* fprintf (stderr, "switch to graph %d\n",graphId); */
    switch_graph(graphId);
}
