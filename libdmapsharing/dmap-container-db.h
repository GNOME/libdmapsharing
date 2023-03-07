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

#ifndef _DMAP_CONTAINER_DB_H
#define _DMAP_CONTAINER_DB_H

#include <glib-object.h>

#include <libdmapsharing/dmap-container-record.h>

G_BEGIN_DECLS
/**
 * SECTION: dmap-container-db
 * @short_description: An interface for DMAP container databases.
 *
 * #DmapContainerDb provides an interface for DMAP container databases.
 */

/**
 * DMAP_TYPE_CONTAINER_DB:
 *
 * The type for #DmapContainerDb.
 */
#define DMAP_TYPE_CONTAINER_DB		 (dmap_container_db_get_type ())
/**
 * DMAP_CONTAINER_DB:
 * @o: Object which is subject to casting.
 *
 * Casts a #DmapContainerDb or derived pointer into a (DmapContainerDb*) 
 * pointer. Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DMAP_CONTAINER_DB(o)		 (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				          DMAP_TYPE_CONTAINER_DB, DmapContainerDb))
/**
 * DMAP_IS_CONTAINER_DB:
 * @o: Instance to check for being a %DMAP_TYPE_CONTAINER_DB.
 *
 * Checks whether a valid #GTypeInstance pointer is of type
 * %DMAP_TYPE_CONTAINER_DB.
 */
#define DMAP_IS_CONTAINER_DB(o)		 (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				          DMAP_TYPE_CONTAINER_DB))
/**
 * DMAP_CONTAINER_DB_GET_INTERFACE:
 * @o: a #DmapContainerDb instance.
 *
 * Get the interface structure associated to a #DmapContainerDb instance.
 *
 * Returns: pointer to object interface structure.
 */
#define DMAP_CONTAINER_DB_GET_INTERFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), \
				            DMAP_TYPE_CONTAINER_DB, DmapContainerDbInterface))
typedef struct _DmapContainerDb DmapContainerDb;
typedef struct _DmapContainerDbInterface DmapContainerDbInterface;

/**
 * DmapIdContainerRecordFunc:
 * @id: a DMAP container record ID
 * @record: a #DmapContainerRecord
 * @user_data: (closure): user data
 *
 * The type of function passed to dmap_container_db_foreach().
 */
typedef void (*DmapIdContainerRecordFunc) (guint id,
                                           DmapContainerRecord *record,
                                           gpointer user_data);

struct _DmapContainerDbInterface
{
	GTypeInterface parent;

	void (*add) (DmapContainerDb *db, DmapContainerRecord *record, GError **error);

	DmapContainerRecord *(*lookup_by_id) (DmapContainerDb * db, guint id);

	void (*foreach) (DmapContainerDb * db, DmapIdContainerRecordFunc func, gpointer data);

	  gint64 (*count) (DmapContainerDb * db);
};

GType dmap_container_db_get_type (void);

/**
 * dmap_container_db_add:
 * @db: A container database.
 * @record: A record.
 * @error: return location for a GError, or NULL.
 *
 * Add a record to the database.
 */
void dmap_container_db_add (DmapContainerDb * db,
                            DmapContainerRecord * record,
                            GError **error);

/**
 * dmap_container_db_lookup_by_id:
 * @db: A container database.
 * @id: A record ID.
 *
 * Returns: (transfer full): the database record corresponding to @id. This record should
 * be unrefed when no longer required.
 */
DmapContainerRecord *dmap_container_db_lookup_by_id (DmapContainerDb * db,
						     guint id);

/**
 * dmap_container_db_foreach:
 * @db: A container database.
 * @func: (scope call): The function to apply to each record in the database.
 * @data: User data to pass to the function.
 *
 * Apply a function to each record in a container database.
 */
void dmap_container_db_foreach (DmapContainerDb * db,
				DmapIdContainerRecordFunc func, gpointer data);

/**
 * dmap_container_db_count:
 * @db: A container database.
 *
 * Returns: the number of records in the database.
 */
gulong dmap_container_db_count (DmapContainerDb * db);

#endif /* _DMAP_CONTAINER_DB_H */

G_END_DECLS
