/*
 *  Database interface for DMAP sharing
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

#ifndef _DMAP_DB_H
#define _DMAP_DB_H

#include <glib-object.h>

#include <libdmapsharing/dmap-record.h>

G_BEGIN_DECLS
/**
 * SECTION: dmap-db
 * @short_description: An interface for DMAP databases.
 *
 * #DmapDb provides an interface for DMAP databases.
 */

/**
 * DMAP_TYPE_DB:
 *
 * The type for #DmapDb.
 */
#define DMAP_TYPE_DB		 (dmap_db_get_type ())
/**
 * DMAP_DB:
 * @o: Object which is subject to casting.
 *
 * Casts a #DmapDb or derived pointer into a (DmapDb *) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DMAP_DB(o)		 (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				  DMAP_TYPE_DB, DmapDb))
/**
 * DMAP_IS_DB:
 * @o: Instance to check for being a %DMAP_TYPE_DB.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %DMAP_TYPE_DB.
 */
#define DMAP_IS_DB(o)		 (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				  DMAP_TYPE_DB))
/**
 * DMAP_DB_GET_INTERFACE:
 * @o: a #DmapDb instance.
 *
 * Get the insterface structure associated to a #DmapDb instance.
 *
 * Returns: pointer to object interface structure.
 */
#define DMAP_DB_GET_INTERFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), \
				  DMAP_TYPE_DB, DmapDbInterface))

/**
 * DmapDbId:
 * @DMAP_DB_ID_BAD: the value which represents a bad DmapDb ID.
 *
 * Special DmapDb ID values.
 */
typedef enum
{
	DMAP_DB_ID_BAD = 0,
} DmapDbId;

typedef struct _DmapDb DmapDb;
typedef struct _DmapDbInterface DmapDbInterface;

/**
 * DmapIdRecordFunc:
 * @id: a DMAP record ID
 * @record: a #DmapRecord
 * @user_data: (closure): user data
 *
 * The type of function passed to dmap_db_foreach().
 */
typedef void (*DmapIdRecordFunc) (guint id, DmapRecord *record, gpointer user_data);

struct _DmapDbInterface
{
	GTypeInterface parent;

	guint (*add) (DmapDb *db, DmapRecord *record, GError **error);
	guint (*add_with_id) (DmapDb * db, DmapRecord * record, guint id, GError **error);
	guint (*add_path) (DmapDb * db, const gchar * path, GError **error);
	DmapRecord *(*lookup_by_id) (const DmapDb * db, guint id);
	guint (*lookup_id_by_location) (const DmapDb * db,
				  const gchar * location);
	void (*foreach) (const DmapDb * db, DmapIdRecordFunc func, gpointer data);
	gint64 (*count) (const DmapDb * db);
};

typedef struct DmapDbFilterDefinition
{
	gchar *key;
	gchar *value;
	gboolean negate;
} DmapDbFilterDefinition;

GType dmap_db_get_type (void);

/**
 * dmap_db_add:
 * @db: A media database.
 * @record: A database record.
 * @error: return location for a GError, or NULL.
 *
 * Add a record to the database. 
 *
 * Returns: The ID for the newly added record or @DMAP_DB_ID_BAD on failure. A
 * reference to the record will be retained by the database (if required; an
 * adapter-type implementation might not want to retain a reference as the
 * record data may be placed elsewhere). In all cases, a returned record should
 * be unrefed by the calling code.
 */
guint dmap_db_add (DmapDb *db, DmapRecord *record, GError **error);

/**
 * dmap_db_add_with_id:
 * @db: A media database.
 * @record: A database record.
 * @id: A database record ID.
 * @error: return location for a GError, or NULL.
 *
 * Add a record to the database and assign it the given ID. @id cannot be
 * @DMAP_DB_ID_BAD.
 *
 * Returns: The ID for the newly added record or DMAP_DB_ID_BAD on failure.
 *
 * See also the notes for dmap_db_add regarding reference counting.
 */
guint dmap_db_add_with_id (DmapDb *db, DmapRecord *record, guint id, GError **error);

/**
 * dmap_db_add_path:
 * @db: A media database.
 * @path: A path to an appropriate media file.
 * @error: return location for a GError, or NULL.
 *
 * Create a record and add it to the database. 
 *
 * Returns: The ID for the newly added record or DMAP_DB_ID_BAD on failure.
 *
 * See also the notes for dmap_db_add regarding reference counting.
 */
guint dmap_db_add_path (DmapDb * db, const gchar * path, GError **error);

/**
 * dmap_db_lookup_by_id:
 * @db: A media database. 
 * @id: A record ID.
 *
 * Returns: (transfer full): the database record corresponding to @id. @id
 * cannot be DMAP_DB_ID_BAD. The returned record should be unrefed by the
 * calling code when no longer required.
 *
 * If you are implementing a full database using this API, then you
 * probably want to increment the reference count before returning a record
 * pointer.
 *
 * On the other hand, if you are implementing an adapter class
 * and the records are stored elsewhere, then you will probably return a
 * transient record. That is, once the user is done using it, the returned
 * record should be free'd because it is a adapter copy of the real record.
 * In this case, the reference count should not be incremented before
 * returning a record pointer.
 */
DmapRecord *dmap_db_lookup_by_id (const DmapDb * db, guint id);

/**
 * dmap_db_lookup_id_by_location:
 * @db: A media database. 
 * @location: A record location.
 *
 * Returns: the database id for the record corresponding to @path or
 * DMAP_DB_ID_BAD if such a record does not exist.
 */
guint dmap_db_lookup_id_by_location (const DmapDb * db,
				     const gchar * location);

/**
 * dmap_db_foreach:
 * @db: A media database.
 * @func: (scope call): The function to apply to each record in the database.
 * @data: User data to pass to the function.
 *
 * Apply a function to each record in a media database.
 */
void dmap_db_foreach (const DmapDb * db, DmapIdRecordFunc func, gpointer data);

/**
 * dmap_db_count:
 * @db: A media database.
 *
 * Returns: the number of records in the database.
 */
gulong dmap_db_count (const DmapDb * db);

/**
 * dmap_db_apply_filter:
 * @db: A media database.
 * @filter_def: (element-type DmapDbFilterDefinition): A series of filter definitions.
 *
 * Returns: (element-type guint DmapRecord) (transfer full): the records which satisfy a record in @filter_def.
 */
GHashTable *dmap_db_apply_filter (DmapDb * db, GSList * filter_def);

#endif /* _DMAP_DB_H */

G_END_DECLS
