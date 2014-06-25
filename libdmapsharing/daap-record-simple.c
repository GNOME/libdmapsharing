/*
 * Database record class for DAAP sharing
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

#include <string.h>

#include "daap-record-simple.h"

struct DAAPRecordSimplePrivate {
	gint filesize;
	gint rating;
	char *location;
	char *title;
	char *format;
	char *real_format;
	char *album;
	char *artist;
	gulong bitrate;
	gint32 firstseen;
	gint32 mtime;
	gint32 disc;
	char *genre;
	gint32 duration;
	gint32 track;
	gint32 year;
	gint8 has_video;
};

enum {
        PROP_0,
        PROP_LOCATION,
        PROP_TITLE,
        PROP_RATING,
        PROP_FILESIZE,
        PROP_ALBUM,
        PROP_ARTIST,
        PROP_GENRE,
        PROP_FORMAT,
        PROP_DURATION,
        PROP_TRACK,
        PROP_YEAR,
        PROP_FIRSTSEEN,
        PROP_MTIME,
        PROP_DISC,
        PROP_BITRATE,
        PROP_HAS_VIDEO,
        PROP_REAL_FORMAT
};

static void
daap_record_simple_set_property (GObject *object,
                                guint prop_id,
                                const GValue *value,
                                GParamSpec *pspec)
{
        DAAPRecordSimple *record = DAAP_RECORD_SIMPLE (object);

        switch (prop_id) {
                case PROP_LOCATION:
			g_free (record->priv->location);
                        record->priv->location = g_value_dup_string (value);
                        break;
                case PROP_TITLE:
			g_free (record->priv->title);
                        record->priv->title = g_value_dup_string (value);
                        break;
                case PROP_ALBUM:
			g_free (record->priv->album);
                        record->priv->album = g_value_dup_string (value);
                        break;
                case PROP_ARTIST:
			g_free (record->priv->artist);
                        record->priv->artist = g_value_dup_string (value);
                        break;
                case PROP_GENRE:
			g_free (record->priv->genre);
                        record->priv->genre = g_value_dup_string (value);
                        break;
                case PROP_FORMAT:
			g_free (record->priv->format);
                        record->priv->format = g_value_dup_string (value);
                        break;
                case PROP_REAL_FORMAT:
			g_free (record->priv->real_format);
                        record->priv->real_format = g_value_dup_string (value);
                        break;
                case PROP_RATING:
                        record->priv->rating = g_value_get_int (value);
                        break;
                case PROP_FILESIZE:
                        record->priv->filesize = g_value_get_int (value);
                        break;
                case PROP_DURATION:
                        record->priv->duration = g_value_get_int (value);
                        break;
                case PROP_TRACK:
                        record->priv->track = g_value_get_int (value);
                        break;
                case PROP_YEAR:
                        record->priv->year = g_value_get_int (value);
                        break;
                case PROP_FIRSTSEEN:
                        record->priv->firstseen = g_value_get_int (value);
                        break;
                case PROP_MTIME:
                        record->priv->mtime = g_value_get_int (value);
                        break;
                case PROP_DISC:
                        record->priv->disc = g_value_get_int (value);
                        break;
                case PROP_BITRATE:
                        record->priv->bitrate = g_value_get_long (value);
                        break;
                case PROP_HAS_VIDEO:
                        record->priv->has_video = g_value_get_boolean (value);
                        break;
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                        break;
        }
}

static void
daap_record_simple_get_property (GObject *object,
                                guint prop_id,
                                GValue *value,
                                GParamSpec *pspec)
{
        DAAPRecordSimple *record = DAAP_RECORD_SIMPLE (object);

        switch (prop_id) {
                case PROP_LOCATION:
                        g_value_set_string (value, record->priv->location);
                        break;
                case PROP_TITLE:
                        g_value_set_string (value, record->priv->title);
                        break;
                case PROP_ALBUM:
                        g_value_set_string (value, record->priv->album);
                        break;
                case PROP_ARTIST:
                        g_value_set_string (value, record->priv->artist);
                        break;
                case PROP_GENRE:
                        g_value_set_string (value, record->priv->genre);
                        break;
                case PROP_FORMAT:
                        g_value_set_string (value, record->priv->format);
                        break;
                case PROP_RATING:
                        g_value_set_int (value, record->priv->rating);
                        break;
                case PROP_FILESIZE:
                        g_value_set_int (value, record->priv->filesize);
                        break;
                case PROP_DURATION:
                        g_value_set_int (value, record->priv->duration);
                        break;
                case PROP_TRACK:
                        g_value_set_int (value, record->priv->track);
                        break;
                case PROP_YEAR:
                        g_value_set_int (value, record->priv->year);
                        break;
                case PROP_FIRSTSEEN:
                        g_value_set_int (value, record->priv->firstseen);
                        break;
                case PROP_MTIME:
                        g_value_set_int (value, record->priv->mtime);
                        break;
                case PROP_DISC:
                        g_value_set_int (value, record->priv->disc);
                        break;
                case PROP_BITRATE:
                        g_value_set_long (value, record->priv->bitrate);
                        break;
                case PROP_HAS_VIDEO:
                        g_value_set_boolean (value, record->priv->has_video);
                        break;
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                        break;
        }
}

GInputStream *daap_record_simple_read (DAAPRecord *record, GError **error)
{
	GFile *file;
	GInputStream *stream;

	file = g_file_new_for_uri (DAAP_RECORD_SIMPLE (record)->priv->location);
	stream = G_INPUT_STREAM (g_file_read (file, NULL, error));

	g_object_unref (file);

	return stream;
}

static void
daap_record_simple_init (DAAPRecordSimple *record)
{
	record->priv = DAAP_RECORD_SIMPLE_GET_PRIVATE (record);
}

static void daap_record_simple_finalize (GObject *object);

static void
daap_record_simple_class_init (DAAPRecordSimpleClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (DAAPRecordSimplePrivate));

	gobject_class->set_property = daap_record_simple_set_property;
        gobject_class->get_property = daap_record_simple_get_property;
        gobject_class->finalize     = daap_record_simple_finalize;

        g_object_class_override_property (gobject_class, PROP_LOCATION, "location");
        g_object_class_override_property (gobject_class, PROP_TITLE, "title");
        g_object_class_override_property (gobject_class, PROP_ALBUM, "songalbum");
        g_object_class_override_property (gobject_class, PROP_ARTIST, "songartist");
        g_object_class_override_property (gobject_class, PROP_GENRE, "songgenre");
        g_object_class_override_property (gobject_class, PROP_FORMAT, "format");
        g_object_class_override_property (gobject_class, PROP_RATING, "rating");
        g_object_class_override_property (gobject_class, PROP_FILESIZE, "filesize");
        g_object_class_override_property (gobject_class, PROP_DURATION, "duration");
        g_object_class_override_property (gobject_class, PROP_TRACK, "track");
        g_object_class_override_property (gobject_class, PROP_YEAR, "year");
        g_object_class_override_property (gobject_class, PROP_FIRSTSEEN, "firstseen");
        g_object_class_override_property (gobject_class, PROP_MTIME, "mtime");
        g_object_class_override_property (gobject_class, PROP_DISC, "disc");
        g_object_class_override_property (gobject_class, PROP_BITRATE, "bitrate");
        g_object_class_override_property (gobject_class, PROP_HAS_VIDEO, "has-video");

        g_object_class_install_property (gobject_class, PROP_REAL_FORMAT,
                                g_param_spec_string ("real-format",
                             "Real format of song data",
                             "Real format of song data",
                               NULL,
                            G_PARAM_READWRITE));
}

static void
daap_record_simple_daap_iface_init (gpointer iface, gpointer data)
{
	DAAPRecordIface *daap_record = iface;

	g_assert (G_TYPE_FROM_INTERFACE (daap_record) == DAAP_TYPE_RECORD);

	daap_record->read = daap_record_simple_read;
}

static void
daap_record_simple_dmap_iface_init (gpointer iface, gpointer data)
{
	DMAPRecordIface *dmap_record = iface;

	g_assert (G_TYPE_FROM_INTERFACE (dmap_record) == DMAP_TYPE_RECORD);
}

G_DEFINE_TYPE_WITH_CODE (DAAPRecordSimple, daap_record_simple, G_TYPE_OBJECT, 
			G_IMPLEMENT_INTERFACE (DAAP_TYPE_RECORD, daap_record_simple_daap_iface_init)
			G_IMPLEMENT_INTERFACE (DMAP_TYPE_RECORD, daap_record_simple_dmap_iface_init))

static void
daap_record_simple_finalize (GObject *object)
{
	DAAPRecordSimple *record = DAAP_RECORD_SIMPLE (object);

	g_free (record->priv->location);
	g_free (record->priv->title);
	g_free (record->priv->format);
	g_free (record->priv->album);
	g_free (record->priv->artist);
	g_free (record->priv->genre);

	G_OBJECT_CLASS (daap_record_simple_parent_class)->finalize (object);
}


DAAPRecordSimple *daap_record_simple_new (void)
{
	DAAPRecordSimple *record;

	record = DAAP_RECORD_SIMPLE (g_object_new (DAAP_TYPE_RECORD_SIMPLE, NULL));

	record->priv->location = g_strdup ("Unknown");

	record->priv->title = g_strdup ("Unknown");

	record->priv->album = g_strdup ("Unknown");

	record->priv->artist = g_strdup ("Unknown");

	record->priv->bitrate = 128;

	record->priv->firstseen = 1;

	record->priv->mtime = 1;

	record->priv->disc = 1;

	record->priv->genre = g_strdup ("Unknown");

	record->priv->format = g_strdup ("Unknown");

	record->priv->filesize = 33729;

	record->priv->duration = 60;

	record->priv->track = 1;

	record->priv->year = 2008;

	record->priv->has_video = 0;

	return record;
}
