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

#include <libdmapsharing/dmap-image-connection.h>
#include <libdmapsharing/dmap-structure.h>

#define DMAP_IMAGE_CONNECTION_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), DMAP_TYPE_IMAGE_CONNECTION, DmapImageConnectionPrivate))

/* FIXME:
struct DmapImageConnectionPrivate {
};
*/

static DmapContentCode
get_protocol_version_cc (DmapConnection * connection)
{
	return DMAP_CC_PPRO;
}

static gchar *
get_query_metadata (DmapConnection * connection)
{
	return g_strdup ("all");
}

static DmapRecord *
handle_mlcl (DmapConnection * connection, DmapRecordFactory * factory,
	     GNode * n, int *item_id)
{
	GNode *n2;
	DmapRecord *record = NULL;
	const gchar *filename = NULL;
	const gchar *aspect_ratio = NULL;
	const gchar *format = NULL;
	const gchar *comments = NULL;
	const gchar *thumbnail = NULL;
	GArray *ptr = NULL;
	gint creation_date = 0;
	gint filesize = 0;
	gint large_filesize = 0;
	gint height = 0;
	gint width = 0;
	gint rating = 0;

	for (n2 = n->children; n2; n2 = n2->next) {
		DmapStructureItem *meta_item;

		meta_item = n2->data;

		switch (meta_item->content_code) {
		case DMAP_CC_MIID:
			*item_id = g_value_get_int (&(meta_item->content));
			break;
		case DMAP_CC_PIMF:
			filename = g_value_get_string (&(meta_item->content));
			break;
		case DMAP_CC_PASP:
			aspect_ratio =
				g_value_get_string (&(meta_item->content));
			break;
		case DMAP_CC_PICD:
			creation_date =
				g_value_get_int (&(meta_item->content));
			break;
		case DMAP_CC_PFMT:
			format = g_value_get_string (&(meta_item->content));
			break;
		case DMAP_CC_PIFS:
			filesize = g_value_get_int (&(meta_item->content));
			break;
		case DMAP_CC_PLSZ:
			large_filesize =
				g_value_get_int (&(meta_item->content));
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
			thumbnail =
				g_value_get_pointer (&(meta_item->content));
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
		ptr = g_array_sized_new (FALSE, FALSE, 1, filesize);
		g_array_append_vals (ptr, (guint8 *) thumbnail, filesize);
	} else {
		ptr = g_array_sized_new (FALSE, FALSE, 1, 0);
	}

	/*
	 * We do not free the dynamically-allocated properties
	 * here. dmap-connection.c's actual_http_response_handler calls
	 * dmap_structure_destroy to free the structure containing the
	 * elements processed here.
	 *
	 * TODO: This could probably be made more clear.
	 */
	g_object_set (record,
		      "filename", filename,
		      "aspect-ratio", aspect_ratio,
		      "creation-date", creation_date,
		      "format", format,
		      "large-filesize", large_filesize,
		      "pixel-height", height,
		      "pixel-width", width,
		      "rating", rating,
		      "comments", comments, "thumbnail", ptr, NULL);

	if (ptr) {
		g_array_unref (ptr);
	}

      _return:
	return record;
}

static void
dmap_image_connection_class_init (DmapImageConnectionClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	DmapConnectionClass *parent_class =
		DMAP_CONNECTION_CLASS (object_class);

	parent_class->get_protocol_version_cc = get_protocol_version_cc;
	parent_class->get_query_metadata = get_query_metadata;
	parent_class->handle_mlcl = handle_mlcl;

	/* FIXME:
	 * g_type_class_add_private (klass, sizeof (DmapImageConnectionPrivate));
	 */
}

DmapImageConnection *
dmap_image_connection_new (const char *name,
		     const char *host,
		     guint port,
		     DmapDb * db, DmapRecordFactory * factory)
{
	DmapImageConnection *connection;

	connection = g_object_new (DMAP_TYPE_IMAGE_CONNECTION,
				   "name", name,
				   "db", db,
				   "host", host,
				   "port", port, "factory", factory, NULL);

	return connection;
}

static void
dmap_image_connection_init (DmapImageConnection * connection)
{
	/* FIXME: 
	 * connection->priv = DMAP_IMAGE_CONNECTION_GET_PRIVATE (connection);
	 */
}

G_DEFINE_TYPE (DmapImageConnection, dmap_image_connection, DMAP_TYPE_CONNECTION);
