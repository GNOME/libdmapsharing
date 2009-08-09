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

#define TYPE_DMAP_DB		 (dmap_db_get_type ())
#define DMAP_DB(o)		 (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				  TYPE_DMAP_DB, DMAPDb))
#define IS_DMAP_DB(o)		 (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				  TYPE_DMAP_DB))
#define DMAP_DB_GET_INTERFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), \
				  TYPE_DMAP_DB, DMAPDbInterface))

typedef struct _DMAPDb DMAPDb;
typedef struct _DMAPDbInterface DMAPDbInterface;

struct _DMAPDbInterface {
	GTypeInterface parent;

	gint (*add)			(DMAPDb *db, DMAPRecord *record);
	DMAPRecord *(*lookup_by_id)	(DMAPDb *db, guint id);
	void (*foreach)			(const DMAPDb *db,
					 void (*fn) (gpointer id,
					 	     DMAPRecord *record,
					 	     gpointer data),
					 gpointer data);
	gint64 (*count) 		(const DMAPDb *db);
};

typedef const char *(*RecordGetValueFunc) (DMAPRecord *record);

typedef struct FilterDefinition {
	gchar *value;
	const char *(*record_get_value) (DMAPRecord *record);
} FilterDefinition;

GType dmap_db_get_type		    (void);

gint        dmap_db_add		    (DMAPDb *db, DMAPRecord *record);

DMAPRecord *dmap_db_lookup_by_id    (DMAPDb *db, guint id);

void        dmap_db_foreach	    (const DMAPDb *db,
				     void (*fn) (gpointer id,
				     		 DMAPRecord *record,
						 gpointer data),
				     gpointer data);

gulong      dmap_db_count	    (const DMAPDb *db);

gchar     **dmap_db_strsplit_using_quotes (const gchar *str);

GHashTable *dmap_db_apply_filter    (DMAPDb *db, GSList *filter_def);

#endif /* __DMAP_DB_H */

G_END_DECLS
