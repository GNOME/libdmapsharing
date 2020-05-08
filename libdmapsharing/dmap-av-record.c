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

#include "config.h"

#include <libdmapsharing/dmap-av-record.h>
#include <libdmapsharing/dmap-enums.h>

static void
dmap_av_record_default_init (DmapAvRecordInterface * iface)
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

G_DEFINE_INTERFACE(DmapAvRecord, dmap_av_record, G_TYPE_OBJECT)

gboolean
dmap_av_record_itunes_compat (DmapAvRecord * record)
{
	return DMAP_AV_RECORD_GET_INTERFACE (record)->itunes_compat (record);
}

GInputStream *
dmap_av_record_read (DmapAvRecord * record, GError ** err)
{
	return DMAP_AV_RECORD_GET_INTERFACE (record)->read (record, err);
}

gint
dmap_av_record_cmp_by_album (gpointer a, gpointer b, DmapDb * db)
{
	DmapAvRecord *record_a, *record_b;
	gchar *album_a, *album_b;
	gchar *sort_album_a, *sort_album_b;
	gint track_a, track_b;
	gint ret;

	record_a =
		DMAP_AV_RECORD (dmap_db_lookup_by_id (db, GPOINTER_TO_UINT (a)));
	record_b =
		DMAP_AV_RECORD (dmap_db_lookup_by_id (db, GPOINTER_TO_UINT (b)));

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

#ifdef HAVE_CHECK

#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <libdmapsharing/test-dmap-av-record.h>
#include <libdmapsharing/test-dmap-db.h>

#define TMP "/tmp/libdmapsharing-test-XXXXXX"

START_TEST(_read_test)
{
	DmapAvRecord *record;
	GInputStream *stream;
	GError *error = NULL;
	gssize count1, count2;
	char buf[PATH_MAX];
	char template[sizeof TMP];
	char uri[PATH_MAX];
	int tmp;

	strcpy(template, TMP);

	tmp = mkstemp(template);
	if (-1 == tmp) {
		ck_abort();
	}

	/* Use randomization of template name for test data. */
	count1 = write(tmp, template, strlen(template));
	if (-1 == count1) {
		ck_abort();
	}

	ck_assert_int_eq(count1, strlen(template));

	sprintf(uri, "file://%s", template);

	record = DMAP_AV_RECORD(test_dmap_av_record_new());
	g_object_set(record, "location", uri, NULL);

	stream = dmap_av_record_read(record, &error);

	ck_assert(NULL == error);

	count2 = g_input_stream_read(stream,
	                            buf,
	                            BUFSIZ,
	                            NULL,
	                           &error);
	ck_assert(NULL == error);
	ck_assert_int_eq(count1, count2);
	ck_assert_str_eq(buf, template);

	g_input_stream_close(stream, NULL, NULL);
	g_object_unref(record);
	close(tmp);
	unlink(template);
}
END_TEST

START_TEST(_read_bad_path_test)
{
	DmapAvRecord *record;
	GError *error = NULL;
	const char *uri = "/xxx";

	record = DMAP_AV_RECORD(test_dmap_av_record_new());
	g_object_set(record, "location", uri, NULL);

	dmap_av_record_read(record, &error);

	ck_assert(NULL != error);

	g_object_unref(record);
}
END_TEST

START_TEST(_itunes_compat_test)
{
	DmapAvRecord *record;
	gboolean ok;

	record = DMAP_AV_RECORD(test_dmap_av_record_new());
	g_object_set(record, "format", "mp3", NULL);

	ok = dmap_av_record_itunes_compat(record);
	ck_assert(TRUE == ok);

	g_object_unref(record);
}
END_TEST

START_TEST(_itunes_compat_no_test)
{
	DmapAvRecord *record;
	gboolean ok;

	record = DMAP_AV_RECORD(test_dmap_av_record_new());
	g_object_set(record, "format", "ogg", NULL);

	ok = dmap_av_record_itunes_compat(record);
	ck_assert(FALSE == ok);

	g_object_unref(record);
}
END_TEST

START_TEST(_cmp_by_album_test)
{
	gint id;
	gchar *album;
	TestDmapAvRecord *record;
	GList *records = NULL;
	DmapDb *db = DMAP_DB(test_dmap_db_new());

	/* Create records, add to database, add identifiers to list. */
	record = test_dmap_av_record_new();
	g_object_set(record, "songalbum", "a", NULL);
	g_object_set(record, "sort-album", "a", NULL);
	id = dmap_db_add(db, DMAP_RECORD(record), NULL);
	g_object_unref(record);
	records = g_list_append (records, GINT_TO_POINTER(id));

	record = test_dmap_av_record_new();
	g_object_set(record, "songalbum",  "c", NULL);
	g_object_set(record, "sort-album", "c", NULL);
	id = dmap_db_add(db, DMAP_RECORD(record), NULL);
	g_object_unref(record);
	records = g_list_append (records, GINT_TO_POINTER(id));

	record = test_dmap_av_record_new();
	g_object_set(record, "songalbum",  "b", NULL);
	g_object_set(record, "sort-album", "b", NULL);
	id = dmap_db_add(db, DMAP_RECORD(record), NULL);
	g_object_unref(record);
	records = g_list_append (records, GINT_TO_POINTER(id));

	/* Check list of record identifiers is not yet sorted (a, c, b). */
	id = GPOINTER_TO_INT(g_list_nth_data(records, 0));
	record = TEST_DMAP_AV_RECORD(dmap_db_lookup_by_id(db, id));
	g_object_get(record, "songalbum", &album, NULL);
	ck_assert_str_eq("a", album);
	g_object_unref(record);
	g_free(album);

	id = GPOINTER_TO_INT(g_list_nth_data(records, 1));
	record = TEST_DMAP_AV_RECORD(dmap_db_lookup_by_id(db, id));
	g_object_get(record, "songalbum", &album, NULL);
	ck_assert_str_eq("c", album);
	g_object_unref(record);
	g_free(album);

	id = GPOINTER_TO_INT(g_list_nth_data(records, 2));
	record = TEST_DMAP_AV_RECORD(dmap_db_lookup_by_id(db, id));
	g_object_get(record, "songalbum", &album, NULL);
	ck_assert_str_eq("b", album);
	g_object_unref(record);
	g_free(album);

	records = g_list_sort_with_data(records,
	                               (GCompareDataFunc) dmap_av_record_cmp_by_album,
	                                db);

	/* Check list of record identifiers is now sorted (a, b, c). */
	id = GPOINTER_TO_INT(g_list_nth_data(records, 0));
	record = TEST_DMAP_AV_RECORD(dmap_db_lookup_by_id(db, id));
	g_object_get(record, "songalbum", &album, NULL);
	ck_assert_str_eq("a", album);
	g_object_unref(record);
	g_free(album);

	id = GPOINTER_TO_INT(g_list_nth_data(records, 1));
	record = TEST_DMAP_AV_RECORD(dmap_db_lookup_by_id(db, id));
	g_object_get(record, "songalbum", &album, NULL);
	ck_assert_str_eq("b", album);
	g_object_unref(record);
	g_free(album);

	id = GPOINTER_TO_INT(g_list_nth_data(records, 2));
	record = TEST_DMAP_AV_RECORD(dmap_db_lookup_by_id(db, id));
	g_object_get(record, "songalbum", &album, NULL);
	ck_assert_str_eq("c", album);
	g_object_unref(record);
	g_free(album);

	g_list_free(records);
	g_object_unref(db);
}
END_TEST

#include "dmap-av-record-suite.c"

#endif
