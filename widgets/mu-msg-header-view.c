/*
** Copyright (C) 2011 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 3, or (at your option) any
** later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software Foundation,
** Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
**
*/

#include "mu-msg-header-view.h"
#include <mu-str.h>

/* 'private'/'protected' functions */
static void mu_msg_header_view_class_init (MuMsgHeaderViewClass *klass);
static void mu_msg_header_view_init       (MuMsgHeaderView *obj);
static void mu_msg_header_view_finalize   (GObject *obj);

/* list my signals  */
enum {
	/* MY_SIGNAL_1, */
	/* MY_SIGNAL_2, */
	LAST_SIGNAL
};

struct _MuMsgHeaderViewPrivate {
	GtkWidget *_table;
};
#define MU_MSG_HEADER_VIEW_GET_PRIVATE(o)      (G_TYPE_INSTANCE_GET_PRIVATE((o), \
                                                MU_TYPE_MSG_HEADER_VIEW, \
                                                MuMsgHeaderViewPrivate))
/* globals */
static GtkVBoxClass *parent_class = NULL;

/* uncomment the following if you have defined any signals */
/* static guint signals[LAST_SIGNAL] = {0}; */

G_DEFINE_TYPE (MuMsgHeaderView, mu_msg_header_view, GTK_TYPE_VBOX);

static void
mu_msg_header_view_class_init (MuMsgHeaderViewClass *klass)
{
	GObjectClass *gobject_class;
	gobject_class = (GObjectClass*) klass;

	parent_class            = g_type_class_peek_parent (klass);
	gobject_class->finalize = mu_msg_header_view_finalize;

	g_type_class_add_private (gobject_class, sizeof(MuMsgHeaderViewPrivate));

	/* signal definitions go here, e.g.: */
/* 	signals[MY_SIGNAL_1] = */
/* 		g_signal_new ("my_signal_1",....); */
/* 	signals[MY_SIGNAL_2] = */
/* 		g_signal_new ("my_signal_2",....); */
/* 	etc. */
}

static void
mu_msg_header_view_init (MuMsgHeaderView *obj)
{
	obj->_priv = MU_MSG_HEADER_VIEW_GET_PRIVATE(obj);
	obj->_priv->_table = NULL;
}

static void
mu_msg_header_view_finalize (GObject *obj)
{
/* 	free/unref instance resources here */
	G_OBJECT_CLASS(parent_class)->finalize (obj);
}

GtkWidget*
mu_msg_header_view_new (void)
{
	return GTK_WIDGET(g_object_new(MU_TYPE_MSG_HEADER_VIEW, NULL));
}


static GtkWidget*
get_label (const gchar *txt, gboolean istitle)
{
	GtkWidget *label;

	label = gtk_label_new (NULL);
	if (istitle) {
		char* markup;
		markup = g_strdup_printf ("<b>%s</b>: ", txt);
		gtk_label_set_markup (GTK_LABEL(label), markup);
		gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);
		g_free (markup);		
	} else {
		gtk_label_set_selectable (GTK_LABEL (label), TRUE);
		gtk_label_set_text (GTK_LABEL(label), txt ? txt : "");
		gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	}

	return label;
}

static gboolean
add_row (GtkWidget *table, guint row, const char* fieldname, const char *value,
	 gboolean showempty)
{
	GtkWidget *label, *al;

	if (!value && !showempty)
		return FALSE;

	label = get_label (fieldname, TRUE);
	al = gtk_alignment_new (0.0, 0.0, 0.0, 0.0);
	gtk_container_add (GTK_CONTAINER (al), label);
	
	gtk_table_attach (
		GTK_TABLE(table), al,
		0, 1, row, row + 1, GTK_FILL, 0, 0, 0);

	al = gtk_alignment_new (0.0, 1.0, 0.0, 0.0);

	label = get_label (value, FALSE);
	gtk_container_add (GTK_CONTAINER (al), label);

	gtk_table_attach (
		GTK_TABLE(table), al, 1, 2, row, row + 1, GTK_FILL,
		0, 0, 0);
		
	return TRUE;
}

	
GtkWidget *
get_table (MuMsg *msg)
{
	GtkWidget *table;
	int row;
	
	row = 0;

	table = gtk_table_new (5, 2, FALSE);
	
	if (add_row (table, row, "From", mu_msg_get_from (msg), TRUE))
		++row;
	if (add_row (table, row, "To", mu_msg_get_to (msg), FALSE))
		++row;
	if (add_row (table, row, "Cc", mu_msg_get_cc (msg), FALSE))
		++row;
	if (add_row (table, row, "Subject", mu_msg_get_subject (msg), TRUE))
		++row;
	if (add_row (table, row, "Date", mu_str_date_s
			  ("%c", mu_msg_get_date (msg)),TRUE))
		++row;

	gtk_table_resize (GTK_TABLE(table), row, 2);

	return table;
}

void
mu_msg_header_view_set_message (MuMsgHeaderView *self, MuMsg *msg)
{
	g_return_if_fail (MU_IS_MSG_HEADER_VIEW(self));

	if (self->_priv->_table) {
		gtk_container_remove (GTK_CONTAINER(self), self->_priv->_table);
		self->_priv->_table = NULL;
	}

	if (msg) {
		self->_priv->_table = get_table (msg);
		gtk_box_pack_start (GTK_BOX(self), self->_priv->_table,
				    TRUE, TRUE, 0);
		gtk_widget_show_all (self->_priv->_table);
	}
}
