/*
 * document-loader.c
 * This file is part of gedit
 *
 * Copyright (C) 2010 - Jesse van den Kieboom
 *
 * gedit is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * gedit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gedit; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gio/gio.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <string.h>
#include "gedit-dirs.h"
#include "gedit-document-loader.h"
#ifndef ENABLE_GVFS_METADATA
#include "gedit-metadata-manager.h"
#endif

static gboolean test_completed;

typedef struct
{
	const gchar   *in_buffer;
	gint           newline_type;
} LoaderTestData;

static GFile *
create_document (const gchar *filename,
                 const gchar *contents)
{
	GError *error = NULL;

	if (!g_file_set_contents (filename, contents, -1, &error))
	{
		g_assert_no_error (error);
	}

	return g_file_new_for_path (filename);
}

static void
delete_document (GFile *location)
{
	if (g_file_query_exists (location, NULL))
	{
		GError *err = NULL;

		g_file_delete (location, NULL, &err);
		g_assert_no_error (err);
	}
}

static void
on_document_loaded (GeditDocument  *document,
                    GError         *error,
                    LoaderTestData *data)
{
	GtkTextIter start, end;

	g_assert_no_error (error);

	if (data->in_buffer != NULL)
	{
		gchar *text;

		gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (document), &start, &end);
		text = gtk_text_iter_get_slice (&start, &end);

		g_assert_cmpstr (text, ==, data->in_buffer);

		g_free (text);
	}

	if (data->newline_type != -1)
	{
		g_assert_cmpint (gedit_document_get_newline_type (document),
		                 ==,
		                 data->newline_type);
	}

	test_completed = TRUE;
}

static void
test_loader (const gchar *filename,
             const gchar *contents,
             const gchar *in_buffer,
             gint         newline_type)
{
	GFile *file;
	GeditDocument *document;

	file = create_document (filename, contents);

	document = gedit_document_new ();

	LoaderTestData *data = g_slice_new (LoaderTestData);
	data->in_buffer = in_buffer;
	data->newline_type = newline_type;

	test_completed = FALSE;

	g_signal_connect (document,
	                  "loaded",
	                  G_CALLBACK (on_document_loaded),
	                  data);

	gedit_document_load (document, file, gedit_encoding_get_utf8 (), 0, 0, FALSE);

	while (!test_completed)
	{
		g_main_context_iteration (NULL, TRUE);
	}

	g_slice_free (LoaderTestData, data);
	g_object_unref (document);

	delete_document (file);
	g_object_unref (file);
}

static void
test_end_line_stripping ()
{
	test_loader ("document-loader.txt",
	             "hello world\n",
	             "hello world",
	             -1);

	test_loader ("document-loader.txt",
	             "hello world",
	             "hello world",
	             -1);

	test_loader ("document-loader.txt",
	             "\nhello world",
	             "\nhello world",
	             -1);

	test_loader ("document-loader.txt",
	             "\nhello world\n",
	             "\nhello world",
	             -1);

	test_loader ("document-loader.txt",
	             "hello world\n\n",
	             "hello world\n",
	             -1);

	test_loader ("document-loader.txt",
	             "hello world\r\n",
	             "hello world",
	             -1);

	test_loader ("document-loader.txt",
	             "hello world\r\n\r\n",
	             "hello world\r\n",
	             -1);

	test_loader ("document-loader.txt",
	             "\n",
	             "",
	             -1);

	test_loader ("document-loader.txt",
	             "\r\n",
	             "",
	             -1);

	test_loader ("document-loader.txt",
	             "\n\n",
	             "\n",
	             -1);

	test_loader ("document-loader.txt",
	             "\r\n\r\n",
	             "\r\n",
	             -1);
}

static void
test_end_new_line_detection ()
{
	test_loader ("document-loader.txt",
	             "hello world\n",
	             NULL,
	             GEDIT_DOCUMENT_NEWLINE_TYPE_LF);

	test_loader ("document-loader.txt",
	             "hello world\r\n",
	             NULL,
	             GEDIT_DOCUMENT_NEWLINE_TYPE_CR_LF);

	test_loader ("document-loader.txt",
	             "hello world\r",
	             NULL,
	             GEDIT_DOCUMENT_NEWLINE_TYPE_CR);
}

static void
test_begin_new_line_detection ()
{
	test_loader ("document-loader.txt",
	             "\nhello world",
	             NULL,
	             GEDIT_DOCUMENT_NEWLINE_TYPE_LF);

	test_loader ("document-loader.txt",
	             "\r\nhello world",
	             NULL,
	             GEDIT_DOCUMENT_NEWLINE_TYPE_CR_LF);

	test_loader ("document-loader.txt",
	             "\rhello world",
	             NULL,
	             GEDIT_DOCUMENT_NEWLINE_TYPE_CR);
}

int main (int   argc,
          char *argv[])
{
	int ret;

	g_test_init (&argc, &argv, NULL);

	gedit_dirs_init ();

#ifndef ENABLE_GVFS_METADATA
	gedit_metadata_manager_init ();
#endif

	g_test_add_func ("/document-loader/end-line-stripping", test_end_line_stripping);
	g_test_add_func ("/document-loader/end-new-line-detection", test_end_new_line_detection);
	g_test_add_func ("/document-loader/begin-new-line-detection", test_begin_new_line_detection);

	ret = g_test_run ();

#ifndef ENABLE_GVFS_METADATA
	gedit_metadata_manager_shutdown ();
#endif

	gedit_dirs_shutdown ();

	return ret;
}

/* ex:set ts=8 noet: */