/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "gui.h"
#include <glade/glade.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdk.h>
#include "viewport.h"
#include <common/memory.h>
#include <cgraph/strcasecmp.h>

GladeXML *xml;			//global libglade vars
GtkWidget *gladewidget;

attribute attr[MAXIMUM_WIDGET_COUNT];

void Color_Widget_bg(char *colorstring, GtkWidget * widget)
{
    GdkColor color;
    gdk_color_parse(colorstring, &color);
    gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &color);
    gtk_widget_modify_base(widget, GTK_STATE_NORMAL, &color);
}

void load_graph_properties(Agraph_t * graph)
{
    //dlgOpenGraph , GtkDialog
    gtk_entry_set_text((GtkEntry *)
		       glade_xml_get_widget(xml, "entryGraphFileName"),
		       view->Topview->Graphdata.GraphFileName);
}

void load_attributes(void)
{
    FILE *file;
    char line[BUFSIZ];
    char *ss;
    char *pch;
    int ind = 0;
    int attrcount = 0;
    static char *smyrna_attrs;

    if (!smyrna_attrs) {
	smyrna_attrs = smyrnaPath("attrs.txt");
    }
    //loads attributes from a text file
    file = fopen(smyrna_attrs, "r");
    if (file != NULL) {
	while (fgets(line, sizeof line, file) != NULL) {
	    pch = strtok(line, ",");
	    ind = 0;
	    while (pch != NULL) {
		ss = strdup(pch);
		pch = strtok(NULL, ",");
		switch (ind) {
		case 0:
		    attr[attrcount].Type = ss[0];
		    break;
		case 1:
		    attr[attrcount].Name = strdup(ss);
		    break;
		case 2:
		    attr[attrcount].Default = strdup(ss);
		    break;
		case 3:
		    break;
		case 4:
		    if (strstr(ss, "ALL_ENGINES")) {
			attr[attrcount].Engine[GVK_DOT] = 1;
			attr[attrcount].Engine[GVK_NEATO] = 1;
			attr[attrcount].Engine[GVK_TWOPI] = 1;
			attr[attrcount].Engine[GVK_CIRCO] = 1;
			attr[attrcount].Engine[GVK_FDP] = 1;
		    } else {
			attr[attrcount].Engine[GVK_DOT] =
			    strstr(ss, "DOT") ? 1 : 0;
			attr[attrcount].Engine[GVK_NEATO] =
			    strstr(ss, "NEATO") ? 1 : 0;
			attr[attrcount].Engine[GVK_TWOPI] =
			    strstr(ss, "TWOPI") ? 1 : 0;
			attr[attrcount].Engine[GVK_CIRCO] =
			    strstr(ss, "CIRCO") ? 1 : 0;
			attr[attrcount].Engine[GVK_FDP] =
			    strstr(ss, "FDP") ? 1 : 0;
		    }
		    break;
		default:
		    attr[attrcount].ComboValues =
			RALLOC(attr[attrcount].ComboValuesCount,
			       attr[attrcount].ComboValues, char *);
		    attr[attrcount].ComboValues[attr[attrcount].
						ComboValuesCount] =
			strdup(ss);
		    attr[attrcount].ComboValuesCount++;
		    break;
		}
		ind++;
	    }
	    attrcount++;
	}
	fclose (file);
    }
}
void show_gui_warning(char *str)
{
    Dlg = (GtkMessageDialog *) gtk_message_dialog_new(NULL,
						      GTK_DIALOG_MODAL,
						      GTK_MESSAGE_WARNING,
						      GTK_BUTTONS_OK, "%s", str);

    gtk_dialog_run((GtkDialog *) Dlg);
    gtk_object_destroy((GtkObject *) Dlg);
}



/*
Generic Open File dialog, if a file is selected and return value is 1, else 0
file name is copied to char* filename,which should be allocated before using the function
*/
int openfiledlg(int filtercnt, char **filters, agxbuf * xbuf)
{
    GtkWidget *dialog;
    GtkFileFilter *filter;
    int id, rv;
    filter = gtk_file_filter_new();
    if (filtercnt >= 1) {
	for (id = 0; id < filtercnt; id++) {
	    gtk_file_filter_add_pattern(filter, filters[id]);
	}
    }

    dialog = gtk_file_chooser_dialog_new("Open File",
					 NULL,
					 GTK_FILE_CHOOSER_ACTION_OPEN,
					 GTK_STOCK_CANCEL,
					 GTK_RESPONSE_CANCEL,
					 GTK_STOCK_OPEN,
					 GTK_RESPONSE_ACCEPT, NULL);

    if (filtercnt >= 1)
	gtk_file_chooser_set_filter((GtkFileChooser *) dialog, filter);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
	agxbput(xbuf,
		gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));
	rv = 1;
    } else
	rv = 0;

    gtk_widget_destroy(dialog);
    return rv;
}

int savefiledlg(int filtercnt, char **filters, agxbuf * xbuf)
{
    GtkWidget *dialog;
    GtkFileFilter *filter;
    int id, rv;
    filter = gtk_file_filter_new();
    if (filtercnt >= 1) {
	for (id = 0; id < filtercnt; id++) {
	    gtk_file_filter_add_pattern(filter, filters[id]);
	}
    }

    dialog = gtk_file_chooser_dialog_new("Save File",
					 NULL,
					 GTK_FILE_CHOOSER_ACTION_OPEN,
					 GTK_STOCK_CANCEL,
					 GTK_RESPONSE_CANCEL,
					 GTK_STOCK_OPEN,
					 GTK_RESPONSE_ACCEPT, NULL);

    if (filtercnt >= 1)
	gtk_file_chooser_set_filter((GtkFileChooser *) dialog, filter);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
	agxbput(xbuf,
		gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));
	rv = 1;
    } else
	rv = 0;

    gtk_widget_destroy(dialog);
    return rv;
}

void append_textview(GtkTextView * textv, const char *s, size_t bytes)
{

    GtkTextIter endit;
    GtkTextBuffer *gtkbuf;
    /*get text view buffer */
    gtkbuf = gtk_text_view_get_buffer(textv);
    /*set iterator to the end of the buffer */
    gtk_text_buffer_get_end_iter(gtkbuf, &endit);
    /* insert buf to the end */
    gtk_text_buffer_insert(gtkbuf, &endit, s, (gint)bytes);

}
