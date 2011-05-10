/*
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

#ifndef __DACP_CONNECTION_H
#define __DACP_CONNECTION_H

#include <glib-object.h>

#include <libdmapsharing/dmap-connection.h>
#include <libdmapsharing/dmap-db.h>

G_BEGIN_DECLS
/**
 * DACP_TYPE_CONNECTION:
 *
 * The type for #DACPConnection.
 */
#define DACP_TYPE_CONNECTION		(dacp_connection_get_type ())
/**
 * DACP_CONNECTION:
 * @o: Object which is subject to casting.
 *
 * Casts a #DACPConnection or derived pointer into a (DACPConnection *) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DACP_CONNECTION(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), DACP_TYPE_CONNECTION, DACPConnection))
/**
 * DACP_CONNECTION_CLASS:
 * @k: a valid #DACPConnectionClass
 *
 * Casts a derived #DACPConnectionClass structure into a #DACPConnectionClass
 * structure.
 */
#define DACP_CONNECTION_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), DACP_TYPE_CONNECTION, DACPConnectionClass))
/**
 * IS_DACP_CONNECTION:
 * @o: Instance to check for being a %DACP_TYPE_CONNECTION.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %DACP_TYPE_CONNECTION.
 */
#define IS_DACP_CONNECTION(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), DACP_TYPE_CONNECTION))
/**
 * IS_DACP_CONNECTION_CLASS:
 * @k: a #DACPConnectionClass
 *
 * Checks whether @k "is a" valid #DACPConnectionClass structure of type
 * %DACP_CONNECTION or derived.
 */
#define IS_DACP_CONNECTION_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), DACP_TYPE_CONNECTION))
/**
 * DACP_CONNECTION_GET_CLASS:
 * @o: a #DACPConnection instance.
 *
 * Get the class structure associated to a #DACPConnection instance.
 *
 * Returns: pointer to object class structure.
 */
#define DACP_CONNECTION_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), DACP_TYPE_CONNECTION, DACPConnectionClass))
typedef struct DACPConnectionPrivate DACPConnectionPrivate;

typedef struct
{
	DMAPConnectionClass dmap_connection_class;
} DACPConnectionClass;

typedef struct
{
	DMAPConnection dmap_connection_instance;
	DACPConnectionPrivate *priv;
} DACPConnection;

GType dacp_connection_get_type (void);

DACPConnection *dacp_connection_new (const char *name,
				     const char *host,
				     guint port,
				     DMAPDb * db,
				     DMAPRecordFactory * factory);
G_END_DECLS
#endif /* __DACP_CONNECTION_H */
