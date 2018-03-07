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

#include <libdmapsharing/dmap-control-connection.h>
#include <libdmapsharing/dmap-structure.h>

#define DMAP_CONTROL_CONNECTION_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), DACP_TYPE_CONNECTION, DmapControlConnectionPrivate))

/* FIXME:
struct DmapControlConnectionPrivate {
};
*/

static DmapContentCode
_get_protocol_version_cc (DmapConnection * connection)
{
	/* FIXME: */
	g_error ("Not implemented");
	return 0;
}

static gchar *
_get_query_metadata (DmapConnection * connection)
{
	/* FIXME: */
	g_error ("Not implemented");
	return NULL;
}

static DmapRecord *
_handle_mlcl (DmapConnection * connection, DmapRecordFactory * factory,
	     GNode * n, int *item_id)
{
	/* FIXME: */
	g_error ("Not implemented");
	return NULL;
}

static void
dmap_control_connection_class_init (DmapControlConnectionClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	DmapConnectionClass *parent_class =
		DMAP_CONNECTION_CLASS (object_class);

	parent_class->get_protocol_version_cc = _get_protocol_version_cc;
	parent_class->get_query_metadata = _get_query_metadata;
	parent_class->handle_mlcl = _handle_mlcl;

	/* FIXME:
	 * g_type_class_add_private (klass, sizeof (DmapControlConnectionPrivate));
	 */
}

DmapControlConnection *
dmap_control_connection_new (const char *name,
		     const char *host,
		     guint port,
		     DmapDb * db, DmapRecordFactory * factory)
{
	DmapControlConnection *connection;

	connection = g_object_new (DACP_TYPE_CONNECTION,
				   "name", name,
				   "db", db,
				   "host", host,
				   "port", port, "factory", factory, NULL);

	return connection;
}

static void
dmap_control_connection_init (DmapControlConnection * connection)
{
	/* FIXME: 
	 * connection->priv = DMAP_CONTROL_CONNECTION_GET_PRIVATE (connection);
	 */
}

G_DEFINE_TYPE (DmapControlConnection, dmap_control_connection, DMAP_TYPE_CONNECTION);
