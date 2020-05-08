/*
 * Database record class for DPAP sharing
 *
 * Copyright (C) 2008 W. Michael Petullo <mike@flyn.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <libdmapsharing/dmap-utils.h>
#include <string.h>

#include "test-dmap-image-record.h"

struct TestDmapImageRecordPrivate {
	gint largefilesize;
	gint pixelheight;
	gint pixelwidth;
	gint rating;
	gint creationdate;
	char *location;
	char *aspectratio;
	char *filename;
	char *format;
	char *comments;
	GArray *thumbnail;
	GArray *hash;
};

enum {
        PROP_0,
        PROP_LARGE_FILESIZE,
        PROP_CREATION_DATE,
        PROP_RATING,
        PROP_LOCATION,
        PROP_FILENAME,
        PROP_ASPECT_RATIO,
        PROP_PIXEL_HEIGHT,
        PROP_PIXEL_WIDTH,
        PROP_FORMAT,
        PROP_THUMBNAIL,
	PROP_HASH,
        PROP_COMMENTS
};

static void
test_dmap_image_record_set_property (GObject *object,
                               guint prop_id,
                               const GValue *value,
                               GParamSpec *pspec)
{
        TestDmapImageRecord *record = TEST_DMAP_IMAGE_RECORD (object);

        switch (prop_id) {
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
			g_free (record->priv->location);
                        record->priv->location = g_value_dup_string (value);
                        break;
                case PROP_FILENAME:
			g_free (record->priv->filename);
                        record->priv->filename = g_value_dup_string (value);
                        break;
                case PROP_ASPECT_RATIO:
			g_free (record->priv->aspectratio);
                        record->priv->aspectratio = g_value_dup_string (value);
                        break;
                case PROP_PIXEL_HEIGHT:
                        record->priv->pixelheight = g_value_get_int (value);
                        break;
		case PROP_PIXEL_WIDTH:
                        record->priv->pixelwidth = g_value_get_int (value);
                        break;
                case PROP_FORMAT:
			g_free (record->priv->format);
                        record->priv->format = g_value_dup_string (value);
                        break;
                case PROP_THUMBNAIL:
			if (record->priv->thumbnail) {
				g_array_unref(record->priv->thumbnail);
			}
                        record->priv->thumbnail = g_array_ref(g_value_get_boxed (value));
                        break;
                case PROP_HASH:
			if (record->priv->hash) {
				g_array_unref(record->priv->hash);
			}
                        record->priv->hash = g_array_ref(g_value_get_boxed (value));
                        break;
                case PROP_COMMENTS:
			g_free (record->priv->comments);
                        record->priv->comments = g_value_dup_string (value);
                        break;
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                        break;

        }
}

static void
test_dmap_image_record_get_property (GObject *object,
                                guint prop_id,
                                GValue *value,
                                GParamSpec *pspec)
{
        TestDmapImageRecord *record = TEST_DMAP_IMAGE_RECORD (object);

        switch (prop_id) {
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
                case PROP_THUMBNAIL:
			g_value_set_boxed (value, g_array_ref(record->priv->thumbnail));
                        break;
                case PROP_HASH:
			g_value_set_boxed (value, g_array_ref(record->priv->hash));
                        break;
                case PROP_COMMENTS:
                        g_value_set_string (value, record->priv->comments);
                        break;
                default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                        break;

        }
}

GInputStream *test_dmap_image_record_read (DmapImageRecord *record, GError **error)
{
	GFile *file;
	GInputStream *stream;

	file = g_file_new_for_uri (TEST_DMAP_IMAGE_RECORD (record)->priv->location);
	stream = G_INPUT_STREAM (g_file_read (file, NULL, error));

	g_object_unref (file);

	return stream;
}

static void test_dmap_image_record_finalize (GObject *object);

static void
test_dmap_image_record_class_init (TestDmapImageRecordClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->set_property = test_dmap_image_record_set_property;
        gobject_class->get_property = test_dmap_image_record_get_property;
        gobject_class->finalize     = test_dmap_image_record_finalize;

        g_object_class_override_property (gobject_class, PROP_LARGE_FILESIZE, "large-filesize");
        g_object_class_override_property (gobject_class, PROP_CREATION_DATE, "creation-date");
        g_object_class_override_property (gobject_class, PROP_RATING, "rating");
        g_object_class_override_property (gobject_class, PROP_LOCATION, "location");
        g_object_class_override_property (gobject_class, PROP_FILENAME, "filename");
        g_object_class_override_property (gobject_class, PROP_ASPECT_RATIO, "aspect-ratio");
        g_object_class_override_property (gobject_class, PROP_PIXEL_HEIGHT, "pixel-height");
        g_object_class_override_property (gobject_class, PROP_PIXEL_WIDTH, "pixel-width");
        g_object_class_override_property (gobject_class, PROP_FORMAT, "format");
        g_object_class_override_property (gobject_class, PROP_THUMBNAIL, "thumbnail");
        g_object_class_override_property (gobject_class, PROP_HASH, "hash");
        g_object_class_override_property (gobject_class, PROP_COMMENTS, "comments");
}

static void
_dmap_image_record_iface_init (gpointer iface)
{
	DmapImageRecordInterface *dmap_image_record = iface;

	g_assert (G_TYPE_FROM_INTERFACE (dmap_image_record) == DMAP_TYPE_IMAGE_RECORD);

	dmap_image_record->read = test_dmap_image_record_read;
}

static void
_dmap_record_iface_init (gpointer iface)
{
        DmapRecordInterface *dmap_record = iface;

	g_assert (G_TYPE_FROM_INTERFACE (dmap_record) == DMAP_TYPE_RECORD);
}

G_DEFINE_TYPE_WITH_CODE (TestDmapImageRecord, test_dmap_image_record, G_TYPE_OBJECT, 
                         G_IMPLEMENT_INTERFACE (DMAP_TYPE_IMAGE_RECORD, _dmap_image_record_iface_init)
                         G_IMPLEMENT_INTERFACE (DMAP_TYPE_RECORD, _dmap_record_iface_init)
                         G_ADD_PRIVATE (TestDmapImageRecord))

static void
test_dmap_image_record_init (TestDmapImageRecord *record)
{
	record->priv = test_dmap_image_record_get_instance_private(record);
}

static void
test_dmap_image_record_finalize (GObject *object)
{
	TestDmapImageRecord *record = TEST_DMAP_IMAGE_RECORD (object);

	g_free (record->priv->location);
	g_free (record->priv->aspectratio);
	g_free (record->priv->filename);
	g_free (record->priv->format);
	g_free (record->priv->comments);

	if (record->priv->thumbnail) {
		g_array_unref (record->priv->thumbnail);
	}

	if (record->priv->hash) {
		g_array_unref (record->priv->hash);
	}

	G_OBJECT_CLASS (test_dmap_image_record_parent_class)->finalize (object);
}

TestDmapImageRecord *
test_dmap_image_record_new (void)
{
	unsigned char *thumbnail;
	GError *error;
	gchar *path;
	gsize size;
	TestDmapImageRecord *record;
	guchar hash[DMAP_HASH_SIZE];

	record = TEST_DMAP_IMAGE_RECORD (g_object_new (TYPE_TEST_DMAP_IMAGE_RECORD, NULL));

	/* Must be a URI. */
	record->priv->location = g_strdup_printf ("file://%s/media/test.jpeg",
						  g_get_current_dir ());

	/* Width / Height as a string. */
	record->priv->aspectratio = g_strdup ("1.333");

	record->priv->filename = g_path_get_basename (record->priv->location);

	record->priv->format = g_strdup ("JPEG");

	record->priv->comments = g_strdup ("Test comments about an image.");

	record->priv->largefilesize = 13953;

	record->priv->pixelheight = 480;

	record->priv->pixelwidth = 640;

	record->priv->rating = 5;

	record->priv->creationdate = 0;

	/* Normally, this data is scaled down to thumbnail size. I have not
	 * done this here because I don't want any external dependencies.
	 * In many cases the VIPS library is a good solution.
	 */
	path = g_strdup_printf ("%s/media/test.jpeg", g_get_current_dir ());
	g_file_get_contents (path, (gchar **) &thumbnail, &size, &error);
	g_free (path);
	record->priv->thumbnail = g_array_sized_new (FALSE, FALSE, 1, size);
	g_array_append_vals (record->priv->thumbnail, thumbnail, size);

	record->priv->hash = g_array_sized_new (FALSE, FALSE, 1, size);
	memset(hash, 0xaa, DMAP_HASH_SIZE);
	g_array_append_vals (record->priv->hash, hash, DMAP_HASH_SIZE);

	return record;
}
