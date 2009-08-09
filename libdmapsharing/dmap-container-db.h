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

#define TYPE_DMAP_CONTAINER_DB		 (dmap_container_db_get_type ())
#define DMAP_CONTAINER_DB(o)		 (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				          TYPE_DMAP_CONTAINER_DB, DMAPContainerDb))
#define IS_DMAP_CONTAINER_DB(o)		 (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				          TYPE_DMAP_CONTAINER_DB))
#define DMAP_CONTAINER_DB_GET_INTERFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), \
				            TYPE_DMAP_CONTAINER_DB, DMAPContainerDbInterface))

typedef struct _DMAPContainerDb DMAPContainerDb;
typedef struct _DMAPContainerDbInterface DMAPContainerDbInterface;

struct _DMAPContainerDbInterface {
	GTypeInterface parent;

	DMAPContainerRecord *(*lookup_by_id)    (DMAPContainerDb *db, gint id);

	void        (*foreach) (DMAPContainerDb *db,
					void (*fn) (DMAPContainerRecord *record,
						    gpointer data),
					gpointer data);

	gint64 (*count)        (DMAPContainerDb *db);
};

GType	    dmap_container_db_get_type        (void);

gint        dmap_container_db_get_id          (DMAPContainerDb *db);

DMAPContainerRecord *dmap_container_db_lookup_by_id    (DMAPContainerDb *db, gint id);

void	    dmap_container_db_foreach         (DMAPContainerDb *db,
				     	       void (*fn) (DMAPContainerRecord *record,
						 	   gpointer data),
				     	       gpointer data);

gulong      dmap_container_db_count           (DMAPContainerDb *db);

#endif /* __DMAP_CONTAINER_DB_H */

G_END_DECLS
