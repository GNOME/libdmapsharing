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

#ifndef __DMAP_DB_H
#define __DMAP_DB_H

#include <glib-object.h>

#include <libdmapsharing/dmap-record.h>

G_BEGIN_DECLS

/**
 * TYPE_DMAP_DB:
 *
 * The type for #DMAPDb.
 */
#define TYPE_DMAP_DB		 (dmap_db_get_type ())
/**
 * DMAP_DB:
 * @o: Object which is subject to casting.
 *
 * Casts a #DMAPDb or derived pointer into a (DMAPDb *) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DMAP_DB(o)		 (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				  TYPE_DMAP_DB, DMAPDb))
/**
 * IS_DMAP_DB:
 * @o: Instance to check for being a %TYPE_DMAP_DB.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %TYPE_DMAP_DB.
 */
#define IS_DMAP_DB(o)		 (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				  TYPE_DMAP_DB))
/**
 * DMAP_DB_GET_INTERFACE:
 * @o: a #DMAPDb instance.
 *
 * Get the insterface structure associated to a #DMAPDb instance.
 *
 * Returns: pointer to object interface structure.
 */
#define DMAP_DB_GET_INTERFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), \
				  TYPE_DMAP_DB, DMAPDbInterface))

typedef struct _DMAPDb DMAPDb;
typedef struct _DMAPDbInterface DMAPDbInterface;

struct _DMAPDbInterface {
	GTypeInterface parent;

	guint (*add)			(DMAPDb *db, DMAPRecord *record);
	DMAPRecord *(*lookup_by_id)	(DMAPDb *db, guint id);
	void (*foreach)			(const DMAPDb *db,
					 GHFunc func,
					 gpointer data);
	gint64 (*count) 		(const DMAPDb *db);
};

typedef const char *(*RecordGetValueFunc) (DMAPRecord *record);

/* FIXME: This is in transition: how to keep this safe when value might 
 * be compared to a string, int, etc.? */
typedef struct FilterDefinition {
	gchar *key;
	gchar *value;
	gboolean is_string;
} FilterDefinition;

GType dmap_db_get_type		    (void);

/**
 * dmap_db_add:
 * @db: A media database.
 * @record: A database record.
 *
 * Add a record to the database. 
 *
 * Returns: The ID for the newly added record. A reference to the record
 * will be retained by the database (if required; an adapter-type 
 * implementation may not want to retain a reference as the record data may
 * be placed elsewhere). In all cases, the record should be unrefed by the 
 * calling code.
 */
guint        dmap_db_add	    (DMAPDb *db, DMAPRecord *record);

/**
 * dmap_db_lookup_by_id:
 * @db: A media database. 
 * @id: A record ID.
 *
 * Returns: the database record corresponding to @id. This record should
 * be unrefed by the calling code when no longer required.
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
DMAPRecord *dmap_db_lookup_by_id    (DMAPDb *db, guint id);

/**
 * dmap_db_foreach:
 * @db: A media database.
 * @fn: The function to apply to each record in the database.
 * @data: User data to pass to the function.
 *
 * Apply a function to each record in a media database.
 */
void        dmap_db_foreach	    (const DMAPDb *db,
				     GHFunc func,
				     gpointer data);

/**
 * dmap_db_count:
 * @db: A media database.
 *
 * Returns: the number of records in the database.
 */
gulong      dmap_db_count	    (const DMAPDb *db);

gchar     **_dmap_db_strsplit_using_quotes (const gchar *str);

GHashTable *dmap_db_apply_filter    (DMAPDb *db, GSList *filter_def);

#endif /* __DMAP_DB_H */

G_END_DECLS
