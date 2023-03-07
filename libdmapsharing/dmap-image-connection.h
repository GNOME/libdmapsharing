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

#ifndef _DMAP_IMAGE_CONNECTION_H
#define _DMAP_IMAGE_CONNECTION_H

#include <glib-object.h>

#include <libdmapsharing/dmap-connection.h>
#include <libdmapsharing/dmap-db.h>

G_BEGIN_DECLS
/**
 * SECTION: dmap-image-connection
 * @short_description: A DPAP connection.
 *
 * #DmapImageConnection objects encapsulate a DPAP connection.
 */

/**
 * DMAP_TYPE_IMAGE_CONNECTION:
 *
 * The type for #DmapImageConnection.
 */
#define DMAP_TYPE_IMAGE_CONNECTION		(dmap_image_connection_get_type ())
/**
 * DMAP_IMAGE_CONNECTION:
 * @o: Object which is subject to casting.
 *
 * Casts a #DmapImageConnection or derived pointer into a (DmapImageConnection *) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DMAP_IMAGE_CONNECTION(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), DMAP_TYPE_IMAGE_CONNECTION, DmapImageConnection))
/**
 * DMAP_IMAGE_CONNECTION_CLASS:
 * @k: a valid #DmapImageConnectionClass
 *
 * Casts a derived #DmapImageConnectionClass structure into a #DmapImageConnectionClass
 * structure.
 */
#define DMAP_IMAGE_CONNECTION_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), DMAP_TYPE_IMAGE_CONNECTION, DmapImageConnectionClass))
/**
 * DMAP_IS_IMAGE_CONNECTION:
 * @o: Instance to check for being a %DMAP_TYPE_IMAGE_CONNECTION.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %DMAP_TYPE_IMAGE_CONNECTION.
 */
#define DMAP_IS_IMAGE_CONNECTION(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), DMAP_TYPE_IMAGE_CONNECTION))
/**
 * DMAP_IS_IMAGE_CONNECTION_CLASS:
 * @k: a #DmapImageConnectionClass
 *
 * Checks whether @k "is a" valid #DmapImageConnectionClass structure of type
 * %DMAP_IMAGE_CONNECTION or derived.
 */
#define DMAP_IS_IMAGE_CONNECTION_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), DMAP_TYPE_IMAGE_CONNECTION))
/**
 * DMAP_IMAGE_CONNECTION_GET_CLASS:
 * @o: a #DmapImageConnection instance.
 *
 * Get the class structure associated to a #DmapImageConnection instance.
 *
 * Returns: pointer to object class structure.
 */
#define DMAP_IMAGE_CONNECTION_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), DMAP_TYPE_IMAGE_CONNECTION, DmapImageConnectionClass))
typedef struct DmapImageConnectionPrivate DmapImageConnectionPrivate;

typedef struct {
	DmapConnectionClass dmap_connection_class;
} DmapImageConnectionClass;

typedef struct {
	DmapConnection dmap_connection_instance;
	DmapImageConnectionPrivate *priv;
} DmapImageConnection;

GType dmap_image_connection_get_type (void);

DmapImageConnection *dmap_image_connection_new (const char *name,
				     const char *host,
				     guint port,
				     DmapDb * db,
				     DmapRecordFactory * factory);
G_END_DECLS
#endif /* _DMAP_IMAGE_CONNECTION_H */
