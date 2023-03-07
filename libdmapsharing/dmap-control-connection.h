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

#ifndef _DMAP_CONTROL_CONNECTION_H
#define _DMAP_CONTROL_CONNECTION_H

#include <glib-object.h>

#include <libdmapsharing/dmap-connection.h>
#include <libdmapsharing/dmap-db.h>

G_BEGIN_DECLS
/**
 * SECTION: dmap-control-connection
 * @short_description: A DACP connection.
 *
 * #DmapControlConnection objects encapsulate a DACP connection.
 */

/**
 * DACP_TYPE_CONNECTION:
 *
 * The type for #DmapControlConnection.
 */
#define DACP_TYPE_CONNECTION		(dmap_control_connection_get_type ())
/**
 * DMAP_CONTROL_CONNECTION:
 * @o: Object which is subject to casting.
 *
 * Casts a #DmapControlConnection or derived pointer into a (DmapControlConnection *) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DMAP_CONTROL_CONNECTION(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), DACP_TYPE_CONNECTION, DmapControlConnection))
/**
 * DMAP_CONTROL_CONNECTION_CLASS:
 * @k: a valid #DmapControlConnectionClass
 *
 * Casts a derived #DmapControlConnectionClass structure into a #DmapControlConnectionClass
 * structure.
 */
#define DMAP_CONTROL_CONNECTION_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), DACP_TYPE_CONNECTION, DmapControlConnectionClass))
/**
 * DMAP_IS_CONTROL_CONNECTION:
 * @o: Instance to check for being a %DACP_TYPE_CONNECTION.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %DACP_TYPE_CONNECTION.
 */
#define DMAP_IS_CONTROL_CONNECTION(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), DACP_TYPE_CONNECTION))
/**
 * DMAP_IS_CONTROL_CONNECTION_CLASS:
 * @k: a #DmapControlConnectionClass
 *
 * Checks whether @k "is a" valid #DmapControlConnectionClass structure of type
 * %DMAP_CONTROL_CONNECTION or derived.
 */
#define DMAP_IS_CONTROL_CONNECTION_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), DACP_TYPE_CONNECTION))
/**
 * DMAP_CONTROL_CONNECTION_GET_CLASS:
 * @o: a #DmapControlConnection instance.
 *
 * Get the class structure associated to a #DmapControlConnection instance.
 *
 * Returns: pointer to object class structure.
 */
#define DMAP_CONTROL_CONNECTION_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), DACP_TYPE_CONNECTION, DmapControlConnectionClass))
typedef struct DmapControlConnectionPrivate DmapControlConnectionPrivate;

typedef struct {
	DmapConnectionClass dmap_connection_class;
} DmapControlConnectionClass;

typedef struct {
	DmapConnection dmap_connection_instance;
	DmapControlConnectionPrivate *priv;
} DmapControlConnection;

GType dmap_control_connection_get_type (void);

DmapControlConnection *dmap_control_connection_new (const char *name,
				     const char *host,
				     guint port,
				     DmapDb * db,
				     DmapRecordFactory * factory);
G_END_DECLS
#endif /* _DMAP_CONTROL_CONNECTION_H */
