/*
 *  Database record interface for DMAP containers
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

#ifndef __DMAP_CONTAINER_RECORD_H
#define __DMAP_CONTAINER_RECORD_H

#include <glib-object.h>
#include <libdmapsharing/dmap-db.h>

G_BEGIN_DECLS

#define TYPE_DMAP_CONTAINER_RECORD	     (dmap_container_record_get_type ())
#define DMAP_CONTAINER_RECORD(o)		     (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				      TYPE_DMAP_CONTAINER_RECORD, DMAPContainerRecord))
#define IS_DMAP_CONTAINER_RECORD(o)	     (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				      TYPE_DMAP_CONTAINER_RECORD))
#define DMAP_CONTAINER_RECORD_GET_INTERFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), \
				      TYPE_DMAP_CONTAINER_RECORD, DMAPContainerRecordInterface))

typedef struct _DMAPContainerRecord DMAPContainerRecord;
typedef struct _DMAPContainerRecordInterface DMAPContainerRecordInterface;

struct _DMAPContainerRecordInterface {
	GTypeInterface parent;
	
	gint		(*get_id)	    (DMAPContainerRecord *record);

	void (*add_entry) (DMAPContainerRecord *container_record, DMAPRecord *record, gint id);

	guint64 (*get_entry_count) (DMAPContainerRecord *record);

	const DMAPDb *(*get_entries) (DMAPContainerRecord *record);
};

GType       dmap_container_record_get_type         (void);
gint        dmap_container_record_get_id           (DMAPContainerRecord *record);
void        dmap_container_record_add_entry        (DMAPContainerRecord *container_record,
						    DMAPRecord *record,
						    gint id);

guint64     dmap_container_record_get_entry_count  (DMAPContainerRecord *record);

const DMAPDb *dmap_container_record_get_entries      (DMAPContainerRecord *record);

#endif /* __DMAP_CONTAINER_RECORD_H */

G_END_DECLS
