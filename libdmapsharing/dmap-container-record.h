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
/**
 * DMAP_TYPE_CONTAINER_RECORD:
 *
 * The type for #DMAPContainerRecord.
 */
#define DMAP_TYPE_CONTAINER_RECORD	     (dmap_container_record_get_type ())
/**
 * DMAP_CONTAINER_RECORD:
 * @o: Object which is subject to casting.
 *
 * Casts a #DMAPContainerRecord or derived pointer into a (DMAPContainerRecord*) 
 * pointer. Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DMAP_CONTAINER_RECORD(o)		     (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				      DMAP_TYPE_CONTAINER_RECORD, DMAPContainerRecord))
/**
 * IS_DMAP_CONTAINER_RECORD:
 * @o: Instance to check for being a %DMAP_TYPE_CONTAINER_RECORD.
 *
 * Checks whether a valid #GTypeInstance pointer is of type
 * %DMAP_TYPE_CONTAINER_RECORD.
 */
#define IS_DMAP_CONTAINER_RECORD(o)	     (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				      DMAP_TYPE_CONTAINER_RECORD))
/**
 * DMAP_CONTAINER_RECORD_GET_INTERFACE:
 * @o: a #DMAPContainerRecord instance.
 *
 * Get the class structure associated to a #DMAPContainerRecord instance.
 *
 * Returns: pointer to object interface structure.
 */
#define DMAP_CONTAINER_RECORD_GET_INTERFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), \
				      DMAP_TYPE_CONTAINER_RECORD, DMAPContainerRecordIface))
typedef struct _DMAPContainerRecord DMAPContainerRecord;
typedef struct _DMAPContainerRecordIface DMAPContainerRecordIface;

struct _DMAPContainerRecordIface
{
	GTypeInterface parent;

	  guint (*get_id) (DMAPContainerRecord * record);

	void (*add_entry) (DMAPContainerRecord * container_record,
			   DMAPRecord * record, gint id);

	  guint64 (*get_entry_count) (DMAPContainerRecord * record);

	DMAPDb *(*get_entries) (DMAPContainerRecord * record);
};

GType dmap_container_record_get_type (void);

/**
 * dmap_container_record_get_id:
 * @record: A DMAPContainerRecord.
 *
 * Returns: the ID for the given record.
 */
guint dmap_container_record_get_id (DMAPContainerRecord * record);

/**
 * dmap_container_record_add_entry:
 * @container_record: A DMAPContainerRecord.
 * @record: A DMAPRecord.
 * @id: The record's ID.
 *
 * Add a record to the database. It is assumed that the record is placed
 * directly into the database (not copied) and not freed.
 */
void dmap_container_record_add_entry (DMAPContainerRecord * container_record,
				      DMAPRecord * record, gint id);

/**
 * dmap_container_record_get_entry_count:
 * @record: A DMAPContainerRecord.
 *
 * Returns: the number of records in the container record.
 */
guint64 dmap_container_record_get_entry_count (DMAPContainerRecord * record);

/**
 * dmap_container_record_get_entries:
 * @record: A DMAPContainerRecord.
 *
 * Returns: A pointer to a DMAPDb containing the entries contained in record.
 */
DMAPDb *dmap_container_record_get_entries (DMAPContainerRecord * record);

#endif /* __DMAP_CONTAINER_RECORD_H */

G_END_DECLS
