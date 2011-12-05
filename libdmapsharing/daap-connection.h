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

#ifndef __DAAP_CONNECTION_H
#define __DAAP_CONNECTION_H

#include <glib-object.h>

#include <libdmapsharing/dmap-connection.h>
#include <libdmapsharing/dmap-db.h>

G_BEGIN_DECLS
/**
 * DAAP_TYPE_CONNECTION:
 *
 * The type for #DAAPConnection.
 */
#define DAAP_TYPE_CONNECTION		(daap_connection_get_type ())
/**
 * DAAP_CONNECTION:
 * @o: Object which is subject to casting.
 *
 * Casts a #DAAPConnection or derived pointer into a (DAAPConnection *) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DAAP_CONNECTION(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), DAAP_TYPE_CONNECTION, DAAPConnection))
/**
 * DAAP_CONNECTION_CLASS:
 * @k: a valid #DAAPConnectionClass
 *
 * Casts a derived #DAAPConnectionClass structure into a #DAAPConnectionClass
 * structure.
 */
#define DAAP_CONNECTION_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), DAAP_TYPE_CONNECTION, DAAPConnectionClass))
/**
 * IS_DAAP_CONNECTION:
 * @o: Instance to check for being a %DAAP_TYPE_CONNECTION.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %DAAP_TYPE_CONNECTION.
 */
#define IS_DAAP_CONNECTION(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), DAAP_TYPE_CONNECTION))
/**
 * IS_DAAP_CONNECTION_CLASS:
 * @k: a #DAAPConnectionClass
 *
 * Checks whether @k "is a" valid #DAAPConnectionClass structure of type
 * %DAAP_CONNECTION or derived.
 */
#define IS_DAAP_CONNECTION_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), DAAP_TYPE_CONNECTION))
/**
 * DAAP_CONNECTION_GET_CLASS:
 * @o: a #DAAPConnection instance.
 *
 * Get the class structure associated to a #DAAPConnection instance.
 *
 * Returns: pointer to object class structure.
 */
#define DAAP_CONNECTION_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), DAAP_TYPE_CONNECTION, DAAPConnectionClass))
typedef struct DAAPConnectionPrivate DAAPConnectionPrivate;

typedef struct
{
	DMAPConnectionClass dmap_connection_class;
} DAAPConnectionClass;

typedef struct
{
	DMAPConnection dmap_connection_instance;
	DAAPConnectionPrivate *priv;
} DAAPConnection;

GType daap_connection_get_type (void);

DAAPConnection *daap_connection_new (const char *name,
				     const char *host,
				     guint port,
				     DMAPDb * db,
				     DMAPRecordFactory * factory);

#ifdef HAVE_CHECK
#include <check.h>

Suite *dmap_test_daap_connection_suite (void);
#endif

G_END_DECLS
#endif /* __DAAP_CONNECTION_H */
