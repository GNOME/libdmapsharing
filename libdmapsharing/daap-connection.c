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

#include <libdmapsharing/daap-connection.h>
#include <libdmapsharing/dmap-structure.h>

#define DAAP_CONNECTION_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), DAAP_TYPE_CONNECTION, DAAPConnectionPrivate))

static DMAPContentCode
_get_protocol_version_cc (DMAPConnection * connection)
{
	return DMAP_CC_APRO;
}

static gchar *
_get_query_metadata (DMAPConnection * connection)
{
	return g_strdup ("dmap.itemid,dmap.itemname,daap.songalbum,"
			 "daap.songartist,daap.songgenre,daap.songsize,"
			 "daap.songtime,daap.songtrackcount,daap.songtracknumber,"
			 "daap.songyear,daap.songformat,"
			 "daap.songbitrate,daap.songdiscnumber,daap.songdataurl,"
			 "daap.sortartist,daap.sortalbum,com.apple.itunes.has-video");
}

static DMAPRecord *
_handle_mlcl (DMAPConnection * connection, DMAPRecordFactory * factory,
	      GNode * n, int *item_id)
{
	GNode *n2;
	DMAPRecord *record = NULL;
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
		DMAPStructureItem *meta_item;

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

	record = dmap_record_factory_create (factory, NULL);
	if (record == NULL) {
		goto _return;
	}

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

      _return:
	return record;
}

static void
daap_connection_class_init (DAAPConnectionClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	DMAPConnectionClass *parent_class =
		DMAP_CONNECTION_CLASS (object_class);

	parent_class->get_protocol_version_cc = _get_protocol_version_cc;
	parent_class->get_query_metadata = _get_query_metadata;
	parent_class->handle_mlcl = _handle_mlcl;
}

DAAPConnection *
daap_connection_new (const char *name,
		     const char *host,
		     guint port,
		     DMAPDb * db,
		     DMAPRecordFactory * factory)
{
	DAAPConnection *connection;

	connection = g_object_new (DAAP_TYPE_CONNECTION,
				  "name", name,
				  "host", host,
				  "port", port,
				  "db", db,
				  "factory", factory,
				   NULL);

	return connection;
}

static void
daap_connection_init (DAAPConnection * connection)
{
}

G_DEFINE_TYPE (DAAPConnection, daap_connection, DMAP_TYPE_CONNECTION);

#ifdef HAVE_CHECK

#include <check.h>
#include <libdmapsharing/test-daap-record.h>
#include <libdmapsharing/test-daap-record-factory.h>

START_TEST(_get_protocol_version_cc_test)
{
	DMAPConnection *conn = g_object_new (DAAP_TYPE_CONNECTION, NULL);
	DMAPContentCode cc = _get_protocol_version_cc (conn);
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

START_TEST(daap_connection_new_test)
{
	char *str;
	int   port;
	DMAPDb *db;
	DMAPRecordFactory *factory;

	DAAPConnection *connection = daap_connection_new("foo",
	                                                 "foo.example.com",
	                                                  3689,
	                                                  NULL,
	                                                  NULL);

	g_object_get(connection, "name", &str, NULL);
	ck_assert_str_eq("foo", str);

	g_object_get(connection, "host", &str, NULL);
	ck_assert_str_eq("foo.example.com", str);

	g_object_get(connection, "port", &port, NULL);
	ck_assert_int_eq(3689, port);

	g_object_get(connection, "db", &db, NULL);
	ck_assert(NULL == db);

	g_object_get(connection, "factory", &factory, NULL);
	ck_assert(NULL == factory);

	g_object_unref(connection);
}
END_TEST

static void
_append_str_test(GNode *parent, int code, char *value)
{
	DMAPStructureItem *item;
	GNode *child;

	item = g_new0(DMAPStructureItem, 1);
	item->content_code = code;
	item->size = strlen(value);
	g_value_init(&(item->content), G_TYPE_STRING);
	g_value_take_string (&(item->content), value);
	child = g_node_new(item);
	g_node_append(parent, child);
}

static void
_append_boolean_test(GNode *parent, int code, const gboolean value)
{
	DMAPStructureItem *item;
	GNode *child;

	item = g_new0(DMAPStructureItem, 1);
	item->content_code = code;
	item->size = 1;
	g_value_init(&(item->content), G_TYPE_CHAR);
	g_value_set_schar(&(item->content), value);
	child = g_node_new(item);
	g_node_append(parent, child);
}

static void
_append_int_test(GNode *parent, int code, const int value)
{
	DMAPStructureItem *item;
	GNode *child;

	item = g_new0(DMAPStructureItem, 1);
	item->content_code = code;
	item->size = 4;
	g_value_init(&(item->content), G_TYPE_INT);
	g_value_set_int(&(item->content), value);
	child = g_node_new(item);
	g_node_append(parent, child);
}

START_TEST(_handle_mlcl_test)
{
	DMAPStructureItem *item;
	TestDAAPRecordFactory *factory;
	GNode *parent;
	DMAPRecord *record;
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

	item = g_new0(DMAPStructureItem, 1);
	item->content_code = 0;
	parent = g_node_new(item);

	_append_int_test(parent, DMAP_CC_MIID, expected_item_id);
	_append_str_test(parent, DMAP_CC_MINM, expected_title);
	_append_str_test(parent, DMAP_CC_ASAL, expected_album);
	_append_str_test(parent, DMAP_CC_ASAR, expected_artist);
	_append_str_test(parent, DMAP_CC_ASFM, expected_format);
	_append_str_test(parent, DMAP_CC_ASGN, expected_genre);
	_append_str_test(parent, DMAP_CC_ASSA, expected_sort_artist);
	_append_str_test(parent, DMAP_CC_ASSU, expected_sort_album);
	_append_boolean_test(parent, DMAP_CC_AEHV, expected_has_video);
	_append_int_test(parent, DMAP_CC_ASTM, expected_length);
	_append_int_test(parent, DMAP_CC_ASTN, expected_track);
	_append_int_test(parent, DMAP_CC_ASDN, expected_disc);
	_append_int_test(parent, DMAP_CC_ASYR, expected_year);
	_append_int_test(parent, DMAP_CC_ASSZ, expected_size);
	_append_int_test(parent, DMAP_CC_ASBR, expected_bitrate);

	factory = test_daap_record_factory_new();
	record  = _handle_mlcl(NULL, DMAP_RECORD_FACTORY(factory), parent, &item_id);

	ck_assert_int_eq(expected_item_id, item_id);

	g_object_get(record, "title", &title, NULL);
	ck_assert_str_eq(expected_title, title);

	g_object_get(record, "songalbum", &album, NULL);
	ck_assert_str_eq(expected_album, album);

	g_object_get(record, "songartist", &artist, NULL);
	ck_assert_str_eq(expected_artist, artist);

	g_object_get(record, "format", &format, NULL);
	ck_assert_str_eq(expected_format, format);

	g_object_get(record, "songgenre", &genre, NULL);
	ck_assert_str_eq(expected_genre, genre);

	g_object_get(record, "sort-artist", &sort_artist, NULL);
	ck_assert_str_eq(expected_sort_artist, sort_artist);

	g_object_get(record, "sort-album", &sort_album, NULL);
	ck_assert_str_eq(expected_sort_album, sort_album);

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
}
END_TEST

/* Do not crash on bad field code (~0). */
START_TEST(_handle_mlcl_bad_code_test)
{
	DMAPStructureItem *item;
	TestDAAPRecordFactory *factory;
	GNode *parent;
	DMAPRecord *record;
	int item_id;
	char *set_value      = "value", *value = NULL;
	char *expected_title = "title", *title = NULL;

	item = g_new0(DMAPStructureItem, 1);
	item->content_code = 0;
	parent = g_node_new(item);

	_append_str_test(parent, ~0, set_value);
	_append_str_test(parent, DMAP_CC_MINM, expected_title);

	factory = test_daap_record_factory_new();
	record  = _handle_mlcl(NULL, DMAP_RECORD_FACTORY(factory), parent, &item_id);

	g_object_get(record, "title", &title, NULL);
	ck_assert_str_eq(expected_title, title);
}
END_TEST

#include "daap-connection-suite.c"

#endif
