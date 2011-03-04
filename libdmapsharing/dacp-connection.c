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

#include <libdmapsharing/dacp-connection.h>
#include <libdmapsharing/dmap-structure.h>

#define DACP_CONNECTION_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), DACP_TYPE_CONNECTION, DACPConnectionPrivate))

/* FIXME:
struct DACPConnectionPrivate {
};
*/

static DMAPContentCode
get_protocol_version_cc (DMAPConnection * connection)
{
	/* FIXME: */
	g_error ("Not implemented");
}

static gchar *
get_query_metadata (void)
{
	/* FIXME: */
	g_error ("Not implemented");
}

static DMAPRecord *
handle_mlcl (DMAPConnection * connection, DMAPRecordFactory * factory,
	     GNode * n, int *item_id)
{
	/* FIXME: */
	g_error ("Not implemented");
}

static void
dacp_connection_class_init (DACPConnectionClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	DMAPConnectionClass *parent_class =
		DMAP_CONNECTION_CLASS (object_class);

	parent_class->get_protocol_version_cc = get_protocol_version_cc;
	parent_class->get_query_metadata = get_query_metadata;
	parent_class->handle_mlcl = handle_mlcl;

	/* FIXME:
	 * g_type_class_add_private (klass, sizeof (DACPConnectionPrivate));
	 */
}

DACPConnection *
dacp_connection_new (const char *name,
		     const char *host,
		     guint port,
		     gboolean password_protected,
		     DMAPDb * db, DMAPRecordFactory * factory)
{
	DACPConnection *connection;

	connection = g_object_new (DACP_TYPE_CONNECTION,
				   "name", name,
				   "password-protected", password_protected,
				   "db", db,
				   "host", host,
				   "port", port, "factory", factory, NULL);

	return connection;
}

static void
dacp_connection_init (DACPConnection * connection)
{
	/* FIXME: 
	 * connection->priv = DACP_CONNECTION_GET_PRIVATE (connection);
	 */
}

G_DEFINE_TYPE (DACPConnection, dacp_connection, DMAP_TYPE_CONNECTION)
