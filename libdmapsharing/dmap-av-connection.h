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

#ifndef _DMAP_AV_CONNECTION_H
#define _DMAP_AV_CONNECTION_H

#include <glib-object.h>

#include <libdmapsharing/dmap-connection.h>
#include <libdmapsharing/dmap-db.h>

G_BEGIN_DECLS
/**
 * SECTION: dmap-av-connection
 * @short_description: A DAAP connection.
 *
 * #DmapAvConnection objects encapsulate a DAAP connection.
 */

/**
 * DMAP_TYPE_AV_CONNECTION:
 *
 * The type for #DmapAvConnection.
 */
#define DMAP_TYPE_AV_CONNECTION		(dmap_av_connection_get_type ())
/**
 * DMAP_AV_CONNECTION:
 * @o: Object which is subject to casting.
 *
 * Casts a #DmapAvConnection or derived pointer into a (DmapAvConnection *) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DMAP_AV_CONNECTION(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), DMAP_TYPE_AV_CONNECTION, DmapAvConnection))
/**
 * DMAP_AV_CONNECTION_CLASS:
 * @k: a valid #DmapAvConnectionClass
 *
 * Casts a derived #DmapAvConnectionClass structure into a #DmapAvConnectionClass
 * structure.
 */
#define DMAP_AV_CONNECTION_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), DMAP_TYPE_AV_CONNECTION, DmapAvConnectionClass))
/**
 * DMAP_IS_AV_CONNECTION:
 * @o: Instance to check for being a %DMAP_TYPE_AV_CONNECTION.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %DMAP_TYPE_AV_CONNECTION.
 */
#define DMAP_IS_AV_CONNECTION(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), DMAP_TYPE_AV_CONNECTION))
/**
 * DMAP_IS_AV_CONNECTION_CLASS:
 * @k: a #DmapAvConnectionClass
 *
 * Checks whether @k "is a" valid #DmapAvConnectionClass structure of type
 * %DMAP_AV_CONNECTION or derived.
 */
#define DMAP_IS_AV_CONNECTION_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), DMAP_TYPE_AV_CONNECTION))
/**
 * DMAP_AV_CONNECTION_GET_CLASS:
 * @o: a #DmapAvConnection instance.
 *
 * Get the class structure associated to a #DmapAvConnection instance.
 *
 * Returns: pointer to object class structure.
 */
#define DMAP_AV_CONNECTION_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), DMAP_TYPE_AV_CONNECTION, DmapAvConnectionClass))
typedef struct DmapAvConnectionPrivate DmapAvConnectionPrivate;

typedef struct {
	DmapConnectionClass dmap_connection_class;
} DmapAvConnectionClass;

typedef struct {
	DmapConnection dmap_connection_instance;
	DmapAvConnectionPrivate *priv;
} DmapAvConnection;

GType dmap_av_connection_get_type (void);

/**
 * dmap_av_connection_new:
 * @name: The name of the share to connect to.
 * @host: The host of the share to connect to.
 * @port: The port of the share to connect to.
 * @db: (transfer full): The db that will receive the records found in the share.
 * @factory: (transfer full): A factory to create records.
 *
 * Create a new DAAP connection.
 *
 * Returns: a pointer to a DmapAvConnection.
 */
DmapAvConnection *dmap_av_connection_new (const char *name,
				     const char *host,
				     guint port,
				     DmapDb * db,
				     DmapRecordFactory * factory);

G_END_DECLS
#endif /* _DMAP_AV_CONNECTION_H */
