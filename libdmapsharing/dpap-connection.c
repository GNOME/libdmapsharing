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

#include <libdmapsharing/dpap-connection.h>
#include <libdmapsharing/dmap-structure.h>

#define DPAP_CONNECTION_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), DPAP_TYPE_CONNECTION, DPAPConnectionPrivate))

/* FIXME:
struct DPAPConnectionPrivate {
};
*/

static DMAPContentCode
get_protocol_version_cc (DMAPConnection *connection)
{
	return DMAP_CC_PPRO;
}

static gchar *
get_query_metadata (void)
{
	return g_strdup ("all");
}

static DMAPRecord *
handle_mlcl (DMAPConnection *connection, DMAPRecordFactory *factory, GNode *n, int *item_id)
{
	GNode *n2;
	DMAPRecord *record = NULL;
	const gchar *filename = NULL;
	const gchar *aspect_ratio = NULL;
	const gchar *format = NULL;
	const gchar *comments = NULL;
	const gchar *thumbnail = NULL;
	const gchar *ptr = NULL;
	gint creation_date = 0;
	gint filesize = 0;
	gint large_filesize = 0;
	gint height = 0;
	gint width = 0;
	gint rating = 0;

	for (n2 = n->children; n2; n2 = n2->next) {
		DMAPStructureItem *meta_item;

		meta_item = n2->data;

		switch (meta_item->content_code) {
			case DMAP_CC_MIID:
				*item_id = g_value_get_int (&(meta_item->content));
				break;
			case DMAP_CC_PIMF:
				filename = g_value_get_string (&(meta_item->content));
				break;
			case DMAP_CC_PASP:
				aspect_ratio = g_value_get_string (&(meta_item->content));
				break;
			case DMAP_CC_PICD:
				creation_date = g_value_get_int (&(meta_item->content));
				break;
			case DMAP_CC_PFMT:
				format = g_value_get_string (&(meta_item->content));
				break;
			case DMAP_CC_PIFS:
				filesize = g_value_get_int (&(meta_item->content));
				break;
			case DMAP_CC_PLSZ:
				large_filesize = g_value_get_int (&(meta_item->content));
				break;
			case DMAP_CC_PHGT:
				height = g_value_get_int (&(meta_item->content));
				break;
			case DMAP_CC_PWTH:
				width = g_value_get_int (&(meta_item->content));
				break;
			case DMAP_CC_PRAT:
				rating = g_value_get_int (&(meta_item->content));
				break;
			case DMAP_CC_PCMT:
				comments = g_value_get_string (&(meta_item->content));
				break;
			case DMAP_CC_PFDT:
				thumbnail = g_value_get_pointer (&(meta_item->content));
				break;
			default:
				break;
		}
	}

	record = dmap_record_factory_create (factory, NULL);
	if (record == NULL) {
		goto _return;
	}

	if (filesize) {
		ptr = g_new (gchar, filesize);
		memcpy (ptr, thumbnail, filesize);
	}

	g_object_set (record,
		     "filename", filename,
		     "aspect-ratio", aspect_ratio,
		     "creation-date", creation_date,
		     "format", format,
		     "filesize", filesize,
		     "large-filesize", large_filesize,
		     "pixel-height", height,
		     "pixel-width", width,
		     "rating", rating,
		     "comments", comments,
		     "thumbnail", ptr,
		      NULL);

_return:
	return record;
}

static void
dpap_connection_class_init (DPAPConnectionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	DMAPConnectionClass *parent_class = DMAP_CONNECTION_CLASS (object_class);

	parent_class->get_protocol_version_cc = get_protocol_version_cc;
	parent_class->get_query_metadata = get_query_metadata;
	parent_class->handle_mlcl = handle_mlcl;

	/* FIXME:
	g_type_class_add_private (klass, sizeof (DPAPConnectionPrivate));
	*/
}

DPAPConnection *
dpap_connection_new (const char        *name,
		     const char        *host,
		     guint              port,
		     gboolean           password_protected,
		     DMAPDb            *db,
		     DMAPRecordFactory *factory)
{
	DPAPConnection *connection;
	
	connection = g_object_new (DPAP_TYPE_CONNECTION,
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
dpap_connection_init (DPAPConnection *connection)
{
	/* FIXME: 
	connection->priv = DPAP_CONNECTION_GET_PRIVATE (connection);
	*/
}

G_DEFINE_TYPE (DPAPConnection, dpap_connection, DMAP_TYPE_CONNECTION)
