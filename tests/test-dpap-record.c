/*
 * Database record class for DPAP sharing
 *
 * Copyright (C) 2008 W. Michael Petullo <mike@flyn.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "test-dpap-record.h"

struct TestDPAPRecordPrivate {
	gint filesize;
	gint largefilesize;
	gint pixelheight;
	gint pixelwidth;
	gint rating;
	gint creationdate;
	const char *location;
	const char *title;
	const char *aspectratio;
	const char *filename;
	const char *format;
	const char *comments;
	unsigned char *thumbnail;
};

enum {
        PROP_0,
        PROP_FILESIZE,
        PROP_LARGE_FILESIZE,
        PROP_CREATION_DATE,
        PROP_RATING,
        PROP_LOCATION,
        PROP_FILENAME,
        PROP_ASPECT_RATIO,
        PROP_PIXEL_HEIGHT,
        PROP_PIXEL_WIDTH,
        PROP_FORMAT,
        PROP_COMMENTS
};

static void
test_dpap_record_set_property (GObject *object,
                               guint prop_id,
                               const GValue *value,
                               GParamSpec *pspec)
{
        TestDPAPRecord *record = TEST_DPAP_RECORD (object);

        switch (prop_id) {
                case PROP_FILESIZE:
                        record->priv->filesize = g_value_get_int (value);
                        break;
                case PROP_LARGE_FILESIZE:
                        record->priv->largefilesize = g_value_get_int (value);
                        break;
                case PROP_CREATION_DATE:
                        record->priv->creationdate = g_value_get_int (value);
                        break;
                case PROP_RATING:
                        record->priv->rating = g_value_get_int (value);
                        break;
                case PROP_LOCATION:
                        record->priv->location = g_value_dup_string (value);
                        break;
                case PROP_FILENAME:
                        record->priv->filename = g_value_dup_string (value);
                        break;
                case PROP_ASPECT_RATIO:
                        record->priv->aspectratio = g_value_dup_string (value);
                        break;
                case PROP_PIXEL_HEIGHT:
                        record->priv->pixelheight = g_value_get_int (value);
                        break;                case PROP_PIXEL_WIDTH:
                        record->priv->pixelwidth = g_value_get_int (value);
                        break;
                case PROP_FORMAT:
                        record->priv->format = g_value_dup_string (value);
                        break;
                case PROP_COMMENTS:
                        record->priv->comments = g_value_dup_string (value);
                        break;
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                        break;

        }
}

static void
test_dpap_record_get_property (GObject *object,
                                guint prop_id,
                                GValue *value,
                                GParamSpec *pspec)
{
        TestDPAPRecord *record = TEST_DPAP_RECORD (object);

        switch (prop_id) {
                case PROP_FILESIZE:
                        g_value_set_int (value, record->priv->filesize);
                        break;
                case PROP_LARGE_FILESIZE:
                        g_value_set_int (value, record->priv->largefilesize);
                        break;
                case PROP_CREATION_DATE:
                        g_value_set_int (value, record->priv->creationdate);
                        break;
                case PROP_RATING:
                        g_value_set_int (value, record->priv->rating);
                        break;
                case PROP_LOCATION:
                        g_value_set_string (value, record->priv->location);
                        break;
                case PROP_FILENAME:
                        g_value_set_string (value, record->priv->filename);
                        break;
                case PROP_ASPECT_RATIO:
                        g_value_set_string (value, record->priv->aspectratio);
                        break;
                case PROP_PIXEL_HEIGHT:
                        g_value_set_int (value, record->priv->pixelheight);
                        break;
                case PROP_PIXEL_WIDTH:
                        g_value_set_int (value, record->priv->pixelwidth);
                        break;
                case PROP_FORMAT:
                        g_value_set_string (value, record->priv->format);
                        break;
                case PROP_COMMENTS:
                        g_value_set_string (value, record->priv->comments);
                        break;
                default:                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                        break;

        }
}

GInputStream *test_dpap_record_read (DPAPRecord *record, gchar *transcode_mimetype, GError **error)
{
	GFile *file;
	GInputStream *stream;

	file = g_file_new_for_uri (TEST_DPAP_RECORD (record)->priv->location);
	stream = G_INPUT_STREAM (g_file_read (file, NULL, error));

	g_object_unref (file);

	return stream;
}



static void
test_dpap_record_init (TestDPAPRecord *record)
{
	record->priv = TEST_DPAP_RECORD_GET_PRIVATE (record);
}

static void
test_dpap_record_class_init (TestDPAPRecordClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (TestDPAPRecordPrivate));

	gobject_class->set_property = test_dpap_record_set_property;
        gobject_class->get_property = test_dpap_record_get_property;

        g_object_class_override_property (gobject_class, PROP_FILESIZE, "filesize");
        g_object_class_override_property (gobject_class, PROP_LARGE_FILESIZE, "large-filesize");
        g_object_class_override_property (gobject_class, PROP_CREATION_DATE, "creation-date");
        g_object_class_override_property (gobject_class, PROP_RATING, "rating");
        g_object_class_override_property (gobject_class, PROP_LOCATION, "location");
        g_object_class_override_property (gobject_class, PROP_FILENAME, "filename");
        g_object_class_override_property (gobject_class, PROP_FILESIZE, "filesize");
        g_object_class_override_property (gobject_class, PROP_ASPECT_RATIO, "aspect-ratio");
        g_object_class_override_property (gobject_class, PROP_PIXEL_HEIGHT, "pixel-height");
        g_object_class_override_property (gobject_class, PROP_PIXEL_WIDTH, "pixel-width");
        g_object_class_override_property (gobject_class, PROP_FORMAT, "format");
        g_object_class_override_property (gobject_class, PROP_COMMENTS, "comments");
}

static void
test_dpap_record_dpap_iface_init (gpointer iface, gpointer data)
{
	DPAPRecordInterface *dpap_record = iface;

	g_assert (G_TYPE_FROM_INTERFACE (dpap_record) == TYPE_DPAP_RECORD);

	dpap_record->read = test_dpap_record_read;
}

static void
test_dpap_record_dmap_iface_init (gpointer iface, gpointer data)
{
        DMAPRecordInterface *dmap_record = iface;

	g_assert (G_TYPE_FROM_INTERFACE (dmap_record) == TYPE_DMAP_RECORD);
}

G_DEFINE_TYPE_WITH_CODE (TestDPAPRecord, test_dpap_record, G_TYPE_OBJECT, 
			 G_IMPLEMENT_INTERFACE (TYPE_DPAP_RECORD, test_dpap_record_dpap_iface_init)
			 G_IMPLEMENT_INTERFACE (TYPE_DMAP_RECORD, test_dpap_record_dmap_iface_init))

TestDPAPRecord *
test_dpap_record_new (void)
{
	unsigned char *thumbnail;
	GError *error;
	gchar *path;
	gsize size;
	TestDPAPRecord *record;

	record = TEST_DPAP_RECORD (g_object_new (TYPE_TEST_DPAP_RECORD, NULL));

	/* Must be a URI. */
	record->priv->location = g_strdup_printf ("file://%s/media/test.jpeg",
						  g_get_current_dir ());

	record->priv->title = "Title of Photograph";

	/* Width / Height as a string. */
	record->priv->aspectratio = "1.333";

	record->priv->filename = g_basename (record->priv->location);

	record->priv->format = "JPEG";

	record->priv->comments = "Test comments about an image.";

	record->priv->largefilesize = 13953;

	record->priv->pixelheight = 480;

	record->priv->pixelwidth = 640;

	record->priv->rating = 5;

	record->priv->creationdate = 0;

	/* Normally, this data is scaled down to thumbnail size. I have not
	 * done this here because I don't want any external dependencies.
	 * In many cases the ImageMagick library is a good solution.
	 */
	path = g_strdup_printf ("%s/media/test.jpeg", g_get_current_dir ());
	g_file_get_contents (path, (gchar **) &thumbnail, &size, &error);
	g_free (path);
	record->priv->filesize = size;
	record->priv->thumbnail = thumbnail;

	return record;
}
