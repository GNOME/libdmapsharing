/*
 *  Database interface for DMAP containers
 *
 *  Copyright (C) 2008 W. Michael Petullo <mike@flyn.org>
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

#ifndef __DMAP_CONTAINER_DB_H
#define __DMAP_CONTAINER_DB_H

#include <glib-object.h>

#include <libdmapsharing/dmap-container-record.h>

G_BEGIN_DECLS
/**
 * DMAP_TYPE_CONTAINER_DB:
 *
 * The type for #DMAPContainerDb.
 */
#define DMAP_TYPE_CONTAINER_DB		 (dmap_container_db_get_type ())
/**
 * DMAP_CONTAINER_DB:
 * @o: Object which is subject to casting.
 *
 * Casts a #DMAPContainerDb or derived pointer into a (DMAPContainerDb*) 
 * pointer. Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DMAP_CONTAINER_DB(o)		 (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				          DMAP_TYPE_CONTAINER_DB, DMAPContainerDb))
/**
 * IS_DMAP_CONTAINER_DB:
 * @o: Instance to check for being a %DMAP_TYPE_CONTAINER_DB.
 *
 * Checks whether a valid #GTypeInstance pointer is of type
 * %DMAP_TYPE_CONTAINER_DB.
 */
#define IS_DMAP_CONTAINER_DB(o)		 (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				          DMAP_TYPE_CONTAINER_DB))
/**
 * DMAP_CONTAINER_DB_GET_INTERFACE:
 * @o: a #DMAPContainerDb instance.
 *
 * Get the interface structure associated to a #DMAPContainerDb instance.
 *
 * Returns: pointer to object interface structure.
 */
#define DMAP_CONTAINER_DB_GET_INTERFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), \
				            DMAP_TYPE_CONTAINER_DB, DMAPContainerDbIface))
typedef struct _DMAPContainerDb DMAPContainerDb;
typedef struct _DMAPContainerDbIface DMAPContainerDbIface;

struct _DMAPContainerDbIface
{
	GTypeInterface parent;

	void (*add) (DMAPContainerDb * db, DMAPContainerRecord * record);

	DMAPContainerRecord *(*lookup_by_id) (DMAPContainerDb * db, guint id);

	void (*foreach) (DMAPContainerDb * db, GHFunc func, gpointer data);

	  gint64 (*count) (DMAPContainerDb * db);
};

GType dmap_container_db_get_type (void);

/**
 * dmap_container_db_add:
 * @db: A container database.
 * @record: A record.
 *
 * Add a record to the database.
 */
void dmap_container_db_add (DMAPContainerDb * db,
                            DMAPContainerRecord * record);

/**
 * dmap_container_db_lookup_by_id:
 * @db: A container database.
 * @id: A record ID.
 *
 * Returns: the database record corresponding to @id. This record should
 * be unrefed when no longer required.
 */
DMAPContainerRecord *dmap_container_db_lookup_by_id (DMAPContainerDb * db,
						     guint id);

/**
 * dmap_container_db_foreach:
 * @db: A container database.
 * @func: The function to apply to each record in the database.
 * @data: User data to pass to the function.
 *
 * Apply a function to each record in a container database.
 */
void dmap_container_db_foreach (DMAPContainerDb * db,
				GHFunc func, gpointer data);

/**
 * dmap_container_db_count:
 * @db: A container database.
 *
 * Returns: the number of records in the database.
 */
gulong dmap_container_db_count (DMAPContainerDb * db);

#endif /* __DMAP_CONTAINER_DB_H */

G_END_DECLS
