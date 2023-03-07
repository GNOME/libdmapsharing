/*
 *  Database record interface for DPAP sharing
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

#ifndef _DMAP_IMAGE_RECORD_H
#define _DMAP_IMAGE_RECORD_H

#include <glib.h>
#include <gio/gio.h>

#include <libdmapsharing/dmap-record.h>

G_BEGIN_DECLS
/**
 * SECTION: dmap-image-record
 * @short_description: The description of an item shared using DPAP.
 *
 * #DmapImageRecord objects encapsulate the description of an item shared using DPAP.
 */

/**
 * DMAP_TYPE_IMAGE_RECORD:
 *
 * The type for #DmapImageRecord.
 */
#define DMAP_TYPE_IMAGE_RECORD	     (dmap_image_record_get_type ())
/**
 * DMAP_IMAGE_RECORD:
 * @o: Object which is subject to casting.
 *
 * Casts a #DmapImageRecord or derived pointer into a (DmapImageRecord *) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DMAP_IMAGE_RECORD(o)		     (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				      DMAP_TYPE_IMAGE_RECORD, DmapImageRecord))
/**
 * DMAP_IS_IMAGE_RECORD:
 * @o: Instance to check for being a %DMAP_TYPE_IMAGE_RECORD.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %DMAP_TYPE_IMAGE_RECORD.
 */
#define DMAP_IS_IMAGE_RECORD(o)	     (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				      DMAP_TYPE_IMAGE_RECORD))
/**
 * DMAP_IMAGE_RECORD_GET_INTERFACE:
 * @o: a #DmapImageRecord instance.
 *
 * Get the class structure associated to a #DmapImageRecord instance.
 *
 * Returns: pointer to object interface structure.
 */
#define DMAP_IMAGE_RECORD_GET_INTERFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), \
				      DMAP_TYPE_IMAGE_RECORD, DmapImageRecordInterface))
typedef struct _DmapImageRecord DmapImageRecord;
typedef struct _DmapImageRecordInterface DmapImageRecordInterface;

struct _DmapImageRecordInterface
{
	GTypeInterface parent;

	GInputStream *(*read) (DmapImageRecord * record, GError ** err);
};

GType dmap_image_record_get_type (void);

/**
 * dmap_image_record_read:
 * @record: a DmapImageRecord.
 * @err: a GError.
 *
 * Returns: (transfer full): a GInputStream that provides read-only access to the data stream
 * associated with record.
 */
GInputStream *dmap_image_record_read (DmapImageRecord * record, GError ** err);

#endif /* _DMAP_IMAGE_RECORD_H */

G_END_DECLS
