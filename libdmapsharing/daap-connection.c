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

/* FIXME:
struct DAAPConnectionPrivate {
};
*/

static DMAPContentCode
get_protocol_version_cc (DMAPConnection *connection)
{
	return DMAP_CC_APRO;
}

static gchar *
get_query_metadata (void)
{
	return g_strdup ("dmap.itemid,dmap.itemname,daap.songalbum,"
			 "daap.songartist,daap.songgenre,daap.songsize,"
			 "daap.songtime,daap.songtrackcount,daap.songtracknumber,"
			 "daap.songyear,daap.songformat,"
			 "daap.songbitrate,daap.songdiscnumber,daap.songdataurl,"
			 "daap.sortartist,daap.sortalbum");
}

static DMAPRecord *
handle_mlcl (DMAPConnection *connection, DMAPRecordFactory *factory, GNode *n, int *item_id)
{
	GNode *n2;
	DMAPRecord *record = NULL;
	const gchar *title = NULL;
	const gchar *album = NULL;
	const gchar *artist = NULL;
	const gchar *format = NULL;
	const gchar *genre = NULL;
	const gchar *streamURI = NULL;
	const gchar *sort_artist = NULL;
	const gchar *sort_album = NULL;
	gint length = 0;
	gint track_number = 0;
	gint disc_number = 0;
	gint year = 0;
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
			case DMAP_CC_ASTM:
				length = g_value_get_int (&(meta_item->content));
				break;
			case DMAP_CC_ASTN:
				track_number = g_value_get_int (&(meta_item->content));
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
			case DMAP_CC_ASUL:
				streamURI = g_value_get_string (&(meta_item->content));
				break;
			case DMAP_CC_ASSA:
				sort_artist = g_value_get_string (&(meta_item->content));
				break;
			case DMAP_CC_ASSU:
				sort_album = g_value_get_string (&(meta_item->content));
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
		     "sort-album", sort_album,
		      NULL);

_return:
	return record;
}

static void
daap_connection_class_init (DAAPConnectionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	DMAPConnectionClass *parent_class = DMAP_CONNECTION_CLASS (object_class);

	parent_class->get_protocol_version_cc = get_protocol_version_cc;
	parent_class->get_query_metadata = get_query_metadata;
	parent_class->handle_mlcl = handle_mlcl;

	/* FIXME:
	g_type_class_add_private (klass, sizeof (DAAPConnectionPrivate));
	*/
}

DAAPConnection *
daap_connection_new (const char        *name,
		     const char        *host,
		     guint              port,
		     gboolean           password_protected,
		     DMAPDb            *db,
		     DMAPRecordFactory *factory)
{
	DAAPConnection *connection;
	
	connection = g_object_new (DAAP_TYPE_CONNECTION,
			          "name", name,
			          "password-protected", password_protected,
			          "db", db,
			          "host", host,
			          "port", port,
				  "factory", factory,
			           NULL);

	return connection;
}

static void
daap_connection_init (DAAPConnection *connection)
{
	/* FIXME: 
	connection->priv = DAAP_CONNECTION_GET_PRIVATE (connection);
	*/
}

G_DEFINE_TYPE (DAAPConnection, daap_connection, DMAP_TYPE_CONNECTION)
