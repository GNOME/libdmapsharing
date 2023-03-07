/*
 *  Database record interface for DAAP sharing
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

#ifndef _DMAP_AV_RECORD_H
#define _DMAP_AV_RECORD_H

#include <glib.h>
#include <gio/gio.h>

#include <libdmapsharing/dmap-record.h>
#include <libdmapsharing/dmap-db.h>

G_BEGIN_DECLS
/**
 * SECTION: dmap-av-record
 * @short_description: The description of an item shared using DAAP.
 *
 * #DmapAvRecord objects encapsulate the description of an item shared using DAAP.
 */

/**
 * DMAP_TYPE_AV_RECORD:
 *
 * The type for #DmapAvRecord.
 */
#define DMAP_TYPE_AV_RECORD	     (dmap_av_record_get_type ())
/**
 * DMAP_AV_RECORD:
 * @o: Object which is subject to casting.
 *
 * Casts a #DmapAvRecord or derived pointer into a (DmapAvRecord *) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DMAP_AV_RECORD(o)		     (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				      DMAP_TYPE_AV_RECORD, DmapAvRecord))
/**
 * DMAP_IS_AV_RECORD:
 * @o: Instance to check for being a %DMAP_TYPE_AV_RECORD.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %DMAP_TYPE_AV_RECORD.
 */
#define DMAP_IS_AV_RECORD(o)	     (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				      DMAP_TYPE_AV_RECORD))
/**
 * DMAP_AV_RECORD_GET_INTERFACE:
 * @o: a #DmapAvRecord instance.
 *
 * Get the class structure associated to a #DmapAvRecord instance.
 *
 * Returns: pointer to object interface structure.
 */
#define DMAP_AV_RECORD_GET_INTERFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), \
				      DMAP_TYPE_AV_RECORD, DmapAvRecordInterface))
typedef struct _DmapAvRecord DmapAvRecord;
typedef struct _DmapAvRecordInterface DmapAvRecordInterface;

struct _DmapAvRecordInterface
{
	GTypeInterface parent;

	  gboolean (*itunes_compat) (DmapAvRecord * record);
	GInputStream *(*read) (DmapAvRecord * record, GError ** err);
};

GType dmap_av_record_get_type (void);

/**
 * dmap_av_record_itunes_compat:
 * @record: A DmapAvRecord.
 *
 * Returns: TRUE if record is compatible with iTunes, else FALSE.
 */
gboolean dmap_av_record_itunes_compat (DmapAvRecord * record);

/**
 * dmap_av_record_read:
 * @record: a DmapAvRecord.
 * @err: a GError.
 *
 * Returns: (transfer full): A GInputStream that provides read-only access to the data stream
 * associated with record.
 */
GInputStream *dmap_av_record_read (DmapAvRecord * record, GError ** err);

/**
 * dmap_av_record_cmp_by_album:
 * @a: first ID.
 * @b: second ID.
 * @db: A DmapDb for which a and b are valid ID's.
 *
 * Compares the two records associated with the provided keys according
 * to album. Suitable to sort lists of albums.
 */
gint dmap_av_record_cmp_by_album (gpointer a, gpointer b, DmapDb * db);

#endif /* _DMAP_AV_RECORD_H */

G_END_DECLS
