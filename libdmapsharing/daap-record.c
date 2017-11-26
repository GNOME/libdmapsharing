/*
 *  Database record interface for DAAP sharing
 *
 *  Copyright (C) 2008 W. Michael Petullo <mike@flyn.org>
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

#include <libdmapsharing/daap-record.h>
#include <libdmapsharing/dmap-enums.h>

static void
daap_record_default_init (DAAPRecordInterface * iface)
{
	static gboolean is_initialized = FALSE;

	if (!is_initialized) {
		g_object_interface_install_property (iface,
						     g_param_spec_string
						     ("location",
						      "URI pointing to song data",
						      "URI pointing to song data",
						      NULL,
						      G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
						     g_param_spec_boxed
						     ("hash",
						      "Hash of media file contents",
						      "Hash of media file contents",
		                                      G_TYPE_ARRAY,
						      G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
						     g_param_spec_string
						     ("title", "Song title",
						      "Song title", "Unknown",
						      G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
						     g_param_spec_enum
						     ("mediakind",
						      "Media kind",
						      "Media kind",
						      DMAP_TYPE_DMAP_MEDIA_KIND,
						      DMAP_MEDIA_KIND_MUSIC,
						      G_PARAM_READWRITE));

		/* NOTE: the name must match the part after the last dot of the
		 * DAAP name, so daap.songalbum becomes songalbum and so on. */
		g_object_interface_install_property (iface,
						     g_param_spec_string
						     ("songalbum",
						      "Album name",
						      "Album name", "Unknown",
						      G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
						     g_param_spec_int64
						     ("songalbumid",
						      "Album id", "Album id",
						      G_MININT64, G_MAXINT64,
						      0, G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
						     g_param_spec_string
						     ("sort-album",
						      "Album sort name",
						      "Album sort name",
						      "Unknown",
						      G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
						     g_param_spec_string
						     ("songartist",
						      "Song artist",
						      "Song artist",
						      "Unknown",
						      G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
						     g_param_spec_string
						     ("sort-artist",
						      "Song artist sort name",
						      "Song artist sort name",
						      "Unknown",
						      G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
						     g_param_spec_string
						     ("songgenre",
						      "Song genre",
						      "Song genre", "Unknown",
						      G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
						     g_param_spec_string
						     ("format",
						      "Song data format",
						      "Song data format",
						      "Unknown",
						      G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
						     g_param_spec_int
						     ("rating", "Song rating",
						      "Song rating", 0, 5, 0,
						      G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
						     g_param_spec_uint64
						     ("filesize",
						      "Song data size in bytes",
						      "Song data size in bytes",
						      0, G_MAXINT, 0,
						      G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
						     g_param_spec_int
						     ("duration",
						      "Song duration in seconds",
						      "Song duration in seconds",
						      0, G_MAXINT, 0,
						      G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
						     g_param_spec_int
						     ("track",
						      "Song track number",
						      "Song track number", 0,
						      G_MAXINT, 0,
						      G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
						     g_param_spec_int ("year",
								       "Song publication year",
								       "Song publication year",
								       0,
								       G_MAXINT,
								       0,
								       G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
						     g_param_spec_int
						     ("firstseen", "FIXME",
						      "FIXME", 0, G_MAXINT, 0,
						      G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
						     g_param_spec_int
						     ("mtime",
						      "Song modification time",
						      "Song modification time",
						      0, G_MAXINT, 0,
						      G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
						     g_param_spec_int ("disc",
								       "Song disc number",
								       "Song disc number",
								       0,
								       G_MAXINT,
								       0,
								       G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
						     g_param_spec_int
						     ("bitrate",
						      "Song data bitrate in Kb/s",
						      "Song data bitrate in Kb/s",
						      0, G_MAXINT, 0,
						      G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
						     g_param_spec_boolean
						     ("has-video",
						      "Song has video component",
						      "Song has video component",
						      FALSE,
						      G_PARAM_READWRITE));

		is_initialized = TRUE;
	}
}

G_DEFINE_INTERFACE(DAAPRecord, daap_record, G_TYPE_OBJECT)

gboolean
daap_record_itunes_compat (DAAPRecord * record)
{
	return DAAP_RECORD_GET_INTERFACE (record)->itunes_compat (record);
}

GInputStream *
daap_record_read (DAAPRecord * record, GError ** err)
{
	return DAAP_RECORD_GET_INTERFACE (record)->read (record, err);
}

gint
daap_record_cmp_by_album (gpointer a, gpointer b, DMAPDb * db)
{
	DAAPRecord *record_a, *record_b;
	gchar *album_a, *album_b;
	gchar *sort_album_a, *sort_album_b;
	gint track_a, track_b;
	gint ret;

	record_a =
		DAAP_RECORD (dmap_db_lookup_by_id (db, GPOINTER_TO_UINT (a)));
	record_b =
		DAAP_RECORD (dmap_db_lookup_by_id (db, GPOINTER_TO_UINT (b)));

	g_assert (record_a);
	g_assert (record_b);

	g_object_get (record_a, "songalbum", &album_a, "sort-album",
		      &sort_album_a, "track", &track_a, NULL);
	g_object_get (record_b, "songalbum", &album_b, "sort-album",
		      &sort_album_b, "track", &track_b, NULL);
	if (sort_album_a && sort_album_b) {
		ret = g_strcmp0 (sort_album_a, sort_album_b);
	} else {
		ret = g_strcmp0 (album_a, album_b);
	}
	if (ret == 0) {
		if (track_a < track_b) {
			ret = -1;
		} else {
			ret = (track_a == track_b) ? 0 : 1;
		}
	}
	g_object_unref (record_a);
	g_object_unref (record_b);
	g_free (album_a);
	g_free (album_b);
	g_free (sort_album_a);
	g_free (sort_album_b);
	return ret;
}
