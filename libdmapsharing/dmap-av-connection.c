/*
 * Copyright (C) 2004,2005 Charles Schmidt <cschmidt2@emich.edu>
 * Copyright (C) 2006 INDT
 *  Andre Moreira Magalhaes <andre.magalhaes@indt.org.br>
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

#include <libdmapsharing/dmap-av-connection.h>
#include <libdmapsharing/dmap-structure.h>
#include <libdmapsharing/test-dmap-db.h>

static DmapContentCode
_get_protocol_version_cc (G_GNUC_UNUSED DmapConnection * connection)
{
	return DMAP_CC_APRO;
}

static gchar *
_get_query_metadata (G_GNUC_UNUSED DmapConnection * connection)
{
	return g_strdup ("dmap.itemid,dmap.itemname,daap.songalbum,"
			 "daap.songartist,daap.songgenre,daap.songsize,"
			 "daap.songtime,daap.songtrackcount,daap.songtracknumber,"
			 "daap.songyear,daap.songformat,"
			 "daap.songbitrate,daap.songdiscnumber,daap.songdataurl,"
			 "daap.sortartist,daap.sortalbum,com.apple.itunes.has-video");
}

static DmapRecord *
_handle_mlcl (DmapConnection * connection, DmapRecordFactory * factory,
	      GNode * n, int *item_id)
{
	GNode *n2;
	GError *error = NULL;
	DmapRecord *record = NULL;
	const gchar *title = NULL;
	const gchar *album = NULL;
	const gchar *artist = NULL;
	const gchar *format = NULL;
	const gchar *genre = NULL;
	const gchar *sort_artist = NULL;
	const gchar *sort_album = NULL;
	gint length = 0;
	gint track_number = 0;
	gint disc_number = 0;
	gint year = 0;
	gboolean has_video = FALSE;
	gint size = 0;
	gint bitrate = 0;

	for (n2 = n->children; n2; n2 = n2->next) {
		DmapStructureItem *meta_item;

		meta_item = n2->data;

		switch (meta_item->content_code) {
		case DMAP_CC_MIID:
			*item_id = g_value_get_int (&(meta_item->content));
			break;
		case DMAP_CC_MINM:
			title = g_value_get_string (&(meta_item->content));
			break;
		case DMAP_CC_ASAL:
			album = g_value_get_string (&(meta_item->content));
			break;
		case DMAP_CC_ASAR:
			artist = g_value_get_string (&(meta_item->content));
			break;
		case DMAP_CC_ASFM:
			format = g_value_get_string (&(meta_item->content));
			break;
		case DMAP_CC_ASGN:
			genre = g_value_get_string (&(meta_item->content));
			break;
		case DMAP_CC_ASSA:
			sort_artist =
				g_value_get_string (&(meta_item->content));
			break;
		case DMAP_CC_ASSU:
			sort_album =
				g_value_get_string (&(meta_item->content));
			break;
		case DMAP_CC_AEHV:
			has_video = g_value_get_schar (&(meta_item->content));
			break;
		case DMAP_CC_ASTM:
			length = g_value_get_int (&(meta_item->content));
			break;
		case DMAP_CC_ASTN:
			track_number =
				g_value_get_int (&(meta_item->content));
			break;
		case DMAP_CC_ASDN:
			disc_number = g_value_get_int (&(meta_item->content));
			break;
		case DMAP_CC_ASYR:
			year = g_value_get_int (&(meta_item->content));
			break;
		case DMAP_CC_ASSZ:
			size = g_value_get_int (&(meta_item->content));
			break;
		case DMAP_CC_ASBR:
			bitrate = g_value_get_int (&(meta_item->content));
			break;
		default:
			break;
		}
	}

	record = dmap_record_factory_create (factory, NULL, &error);
	if (NULL != error) {
		g_signal_emit_by_name (connection, "error", error);
		goto done;
	}
	g_assert(NULL != record);

	/*
	 * We do not free the dynamically-allocated properties
	 * here. dmap-connection.c's actual_http_response_handler calls
	 * dmap_structure_destroy to free the structure containing the
	 * elements processed here.
	 *
	 * TODO: This could probably be made more clear.
	 */
	g_object_set (record,
		      "year", year,
		      "has-video", has_video,
		      "track", track_number,
		      "disc", disc_number,
		      "bitrate", bitrate,
		      "duration", length / 1000,
		      "filesize", (guint64) size,
		      "format", format,
		      "title", title,
		      "songalbum", album,
		      "songartist", artist,
		      "songgenre", genre,
		      "sort-artist", sort_artist,
		      "sort-album", sort_album, NULL);

done:
	return record;
}

static void
dmap_av_connection_class_init (DmapAvConnectionClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	DmapConnectionClass *parent_class =
		DMAP_CONNECTION_CLASS (object_class);

	parent_class->get_protocol_version_cc = _get_protocol_version_cc;
	parent_class->get_query_metadata = _get_query_metadata;
	parent_class->handle_mlcl = _handle_mlcl;
}

DmapAvConnection *
dmap_av_connection_new (const char *name,
		     const char *host,
		     guint port,
		     DmapDb * db,
		     DmapRecordFactory * factory)
{
	DmapAvConnection *connection;

	connection = g_object_new (DMAP_TYPE_AV_CONNECTION,
				  "name", name,
				  "host", host,
				  "port", port,
				  "db", db,
				  "factory", factory,
				   NULL);

	return connection;
}

static void
dmap_av_connection_init (G_GNUC_UNUSED DmapAvConnection * connection)
{
}

G_DEFINE_TYPE (DmapAvConnection, dmap_av_connection, DMAP_TYPE_CONNECTION);

#ifdef HAVE_CHECK

#include <check.h>
#include <libdmapsharing/test-dmap-av-record.h>
#include <libdmapsharing/test-dmap-av-record-factory.h>

START_TEST(_get_protocol_version_cc_test)
{
	DmapConnection *conn = g_object_new (DMAP_TYPE_AV_CONNECTION, NULL);
	DmapContentCode cc = _get_protocol_version_cc (conn);
	fail_unless (cc == DMAP_CC_APRO);
	g_object_unref (conn);
}
END_TEST

START_TEST(_get_query_metadata_test)
{
	char *str = _get_query_metadata(NULL);

	ck_assert_str_eq(str, "dmap.itemid,dmap.itemname,daap.songalbum,"
	                      "daap.songartist,daap.songgenre,daap.songsize,"
	                      "daap.songtime,daap.songtrackcount,daap.songtracknumber,"
	                      "daap.songyear,daap.songformat,"
	                      "daap.songbitrate,daap.songdiscnumber,daap.songdataurl,"
	                      "daap.sortartist,daap.sortalbum,com.apple.itunes.has-video");

	g_free(str);
}
END_TEST

START_TEST(_new_test)
{
	char *str;
	int   port;
	DmapDb *db1, *db2;
	DmapRecordFactory *factory1, *factory2;

	db1 = DMAP_DB(test_dmap_db_new());
	factory1 = DMAP_RECORD_FACTORY(test_dmap_av_record_factory_new());

	DmapAvConnection *connection = dmap_av_connection_new("foo",
	                                                 "foo.example.com",
	                                                  3689,
	                                                  db1,
	                                                  factory1);

	g_object_get(connection, "name", &str, NULL);
	ck_assert_str_eq("foo", str);
	g_free(str);

	g_object_get(connection, "host", &str, NULL);
	ck_assert_str_eq("foo.example.com", str);
	g_free(str);

	g_object_get(connection, "port", &port, NULL);
	ck_assert_int_eq(3689, port);

	g_object_get(connection, "db", &db2, NULL);
	ck_assert(db1 == db2);
	g_object_unref(db2);

	g_object_get(connection, "factory", &factory2, NULL);
	ck_assert(factory1 == factory2);
	g_object_unref(factory2);

	g_object_unref(db1);
	g_object_unref(factory1);
	g_object_unref(connection);
}
END_TEST

START_TEST(_handle_mlcl_test)
{
	TestDmapAvRecordFactory *factory;
	GNode *parent;
	DmapRecord *record;
	char *expected_title        = "title", *title             = NULL;
	char *expected_album        = "album", *album             = NULL;
	char *expected_artist       = "artist", *artist           = NULL;
	char *expected_format       = "format", *format           = NULL;
	char *expected_genre        = "genre", *genre             = NULL;
	char *expected_sort_artist  = "sort-artist", *sort_artist = NULL;
	char *expected_sort_album   = "sort-album", *sort_album   = NULL;
	gboolean expected_has_video =  TRUE, has_video            = FALSE;
	gint  expected_length       =  10000, length              = 0;
	gint  expected_track        =  20, track                  = 0;
	gint  expected_disc         =  30, disc                   = 0;
	gint  expected_year         =  40, year                   = 0;
	gint  expected_size         =  50, size                   = 0;
	gint  expected_bitrate      =  60, bitrate                = 0;
	gint  expected_item_id      =  70, item_id                = 0;

	parent = dmap_structure_add(NULL, DMAP_CC_MLCL);

	dmap_structure_add(parent, DMAP_CC_MIID, expected_item_id);
	dmap_structure_add(parent, DMAP_CC_MINM, expected_title);
	dmap_structure_add(parent, DMAP_CC_ASAL, expected_album);
	dmap_structure_add(parent, DMAP_CC_ASAR, expected_artist);
	dmap_structure_add(parent, DMAP_CC_ASFM, expected_format);
	dmap_structure_add(parent, DMAP_CC_ASGN, expected_genre);
	dmap_structure_add(parent, DMAP_CC_ASSA, expected_sort_artist);
	dmap_structure_add(parent, DMAP_CC_ASSU, expected_sort_album);
	dmap_structure_add(parent, DMAP_CC_AEHV, expected_has_video);
	dmap_structure_add(parent, DMAP_CC_ASTM, expected_length);
	dmap_structure_add(parent, DMAP_CC_ASTN, expected_track);
	dmap_structure_add(parent, DMAP_CC_ASDN, expected_disc);
	dmap_structure_add(parent, DMAP_CC_ASYR, expected_year);
	dmap_structure_add(parent, DMAP_CC_ASSZ, expected_size);
	dmap_structure_add(parent, DMAP_CC_ASBR, expected_bitrate);

	factory = test_dmap_av_record_factory_new();
	record  = _handle_mlcl(NULL, DMAP_RECORD_FACTORY(factory), parent, &item_id);

	ck_assert_int_eq(expected_item_id, item_id);

	g_object_get(record, "title", &title, NULL);
	ck_assert_str_eq(expected_title, title);
	g_free(title);

	g_object_get(record, "songalbum", &album, NULL);
	ck_assert_str_eq(expected_album, album);
	g_free(album);

	g_object_get(record, "songartist", &artist, NULL);
	ck_assert_str_eq(expected_artist, artist);
	g_free(artist);

	g_object_get(record, "format", &format, NULL);
	ck_assert_str_eq(expected_format, format);
	g_free(format);

	g_object_get(record, "songgenre", &genre, NULL);
	ck_assert_str_eq(expected_genre, genre);
	g_free(genre);

	g_object_get(record, "sort-artist", &sort_artist, NULL);
	ck_assert_str_eq(expected_sort_artist, sort_artist);
	g_free(sort_artist);

	g_object_get(record, "sort-album", &sort_album, NULL);
	ck_assert_str_eq(expected_sort_album, sort_album);
	g_free(sort_album);

	g_object_get(record, "has-video", &has_video, NULL);
	ck_assert_int_eq(expected_has_video, has_video);

	g_object_get(record, "duration", &length, NULL);
	ck_assert_int_eq(expected_length, length * 1000);

	g_object_get(record, "track", &track, NULL);
	ck_assert_int_eq(expected_track, track);

	g_object_get(record, "disc", &disc, NULL);
	ck_assert_int_eq(expected_disc, disc);

	g_object_get(record, "year", &year, NULL);
	ck_assert_int_eq(expected_year, year);

	g_object_get(record, "filesize", &size, NULL);
	ck_assert_int_eq(expected_size, size);

	g_object_get(record, "bitrate", &bitrate, NULL);
	ck_assert_int_eq(expected_bitrate, bitrate);

	g_object_unref(record);

	dmap_structure_destroy(parent);
}
END_TEST

/* Do not crash on bad field code (~0). */
START_TEST(_handle_mlcl_bad_code_test)
{
	DmapStructureItem *item;
	TestDmapAvRecordFactory *factory;
	GNode *parent, *child;
	DmapRecord *record;
	int item_id;
	char *set_value      = "value";
	char *expected_title = "title", *title = NULL;

	parent = dmap_structure_add(NULL, DMAP_CC_MLCL);

	/* A node with a bad content code. */
	item = g_new0(DmapStructureItem, 1);
	item->content_code = ~0;
	item->size = strlen(set_value);
	g_value_init(&(item->content), G_TYPE_STRING);
	g_value_set_string (&(item->content), set_value);
	child = g_node_new(item);
	g_node_append(parent, child);

	/* A well-formed node. */
	dmap_structure_add(parent, DMAP_CC_MINM, expected_title);

	factory = test_dmap_av_record_factory_new();
	record  = _handle_mlcl(NULL, DMAP_RECORD_FACTORY(factory), parent, &item_id);

	g_object_get(record, "title", &title, NULL);
	ck_assert_str_eq(expected_title, title);
	g_free(title);

	dmap_structure_destroy(parent);
}
END_TEST

#include "dmap-av-connection-suite.c"

#endif
