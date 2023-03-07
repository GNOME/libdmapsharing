/*
 *  Database record interface for DMAP sharing
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

#ifndef _DMAP_RECORD_H
#define _DMAP_RECORD_H

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * SECTION: dmap-record
 * @short_description: An abstract parent to the various record classes.
 *
 * #DmapRecord provides an abstract parent to the #DmapAvRecord and #DmapImageRecord classes.
 */

/**
 * DMAP_TYPE_RECORD:
 *
 * The type for #DmapRecord.
 */
#define DMAP_TYPE_RECORD	     (dmap_record_get_type ())
/**
 * DMAP_RECORD:
 * @o: Object which is subject to casting.
 *
 * Casts a #DmapRecord or derived pointer into a (DmapRecord *) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DMAP_RECORD(o)		     (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				      DMAP_TYPE_RECORD, DmapRecord))
/**
 * DMAP_IS_RECORD:
 * @o: Instance to check for being a %DMAP_TYPE_RECORD.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %DMAP_TYPE_RECORD.
 */
#define DMAP_IS_RECORD(o)	     (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				      DMAP_TYPE_RECORD))
/**
 * DMAP_RECORD_GET_INTERFACE:
 * @o: a #DmapAvRecord instance.
 *
 * Get the class structure associated to a #DmapAvRecord instance.
 *
 * Returns: pointer to object class structure.
 */
#define DMAP_RECORD_GET_INTERFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), \
				      DMAP_TYPE_RECORD, DmapRecordInterface))
typedef struct _DmapRecord DmapRecord;

typedef struct {
	GTypeInterface parent;

	GArray *(*to_blob) (DmapRecord * record);
	gboolean   (*set_from_blob) (DmapRecord * record, GArray * blob);
} DmapRecordInterface;

typedef enum {
	DMAP_MEDIA_KIND_MUSIC = 1,
	DMAP_MEDIA_KIND_MOVIE = 2,
	DMAP_MEDIA_KIND_PODCAST = 32,
	DMAP_MEDIA_KIND_TV_SHOW = 64 
} DmapMediaKind;

GType dmap_record_get_type (void);

/**
 * dmap_record_to_blob:
 * @record: A DmapRecord.
 *
 * Returns: (element-type guint8) (transfer container): A byte array representation of the record.
 */
GArray *dmap_record_to_blob (DmapRecord * record);

/**
 * dmap_record_set_from_blob:
 * @record: The record to set.
 * @blob: (element-type guint8): A byte array representation of a record.
 *
 * Returns: True on success, else false.
 */
gboolean dmap_record_set_from_blob (DmapRecord * record,
                                    GArray * blob);

#endif /* _DMAP_RECORD_H */

G_END_DECLS
