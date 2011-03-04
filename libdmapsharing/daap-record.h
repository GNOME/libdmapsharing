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

#ifndef __DAAP_RECORD_H
#define __DAAP_RECORD_H

#include <glib.h>
#include <gio/gio.h>

#include <libdmapsharing/dmap-record.h>
#include <libdmapsharing/dmap-db.h>

G_BEGIN_DECLS
/**
 * DAAP_TYPE_RECORD:
 *
 * The type for #DAAPRecord.
 */
#define DAAP_TYPE_RECORD	     (daap_record_get_type ())
/**
 * DAAP_RECORD:
 * @o: Object which is subject to casting.
 *
 * Casts a #DAAPRecord or derived pointer into a (DAAPRecord *) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DAAP_RECORD(o)		     (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				      DAAP_TYPE_RECORD, DAAPRecord))
/**
 * IS_DAAP_RECORD:
 * @o: Instance to check for being a %DAAP_TYPE_RECORD.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %DAAP_TYPE_RECORD.
 */
#define IS_DAAP_RECORD(o)	     (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				      DAAP_TYPE_RECORD))
/**
 * DAAP_RECORD_GET_INTERFACE:
 * @o: a #DAAPRecord instance.
 *
 * Get the class structure associated to a #DAAPRecord instance.
 *
 * Returns: pointer to object interface structure.
 */
#define DAAP_RECORD_GET_INTERFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), \
				      DAAP_TYPE_RECORD, DAAPRecordIface))
typedef struct _DAAPRecord DAAPRecord;
typedef struct _DAAPRecordIface DAAPRecordIface;

struct _DAAPRecordIface
{
	GTypeInterface parent;

	  gboolean (*itunes_compat) (DAAPRecord * record);
	GInputStream *(*read) (DAAPRecord * record, GError ** err);
};

GType daap_record_get_type (void);

/**
 * daap_record_itunes_compat:
 * @record: A DAAPRecord.
 *
 * Returns: TRUE if record is compatible with iTunes, else FALSE.
 */
gboolean daap_record_itunes_compat (DAAPRecord * record);

/**
 * daap_record_read:
 * @record: a DAAPRecord.
 * @err: a GError.
 *
 * Returns: A GInputStream that provides read-only access to the data stream
 * associated with record.
 */
GInputStream *daap_record_read (DAAPRecord * record, GError ** err);

/**
 * daap_record_cmp_by_album:
 * @a: first ID.
 * @b: second ID.
 * @db: A DMAPDb for which a and b are valid ID's.
 *
 * Compares the two records associated with the provided keys according
 * to album. Suitable to sort lists of albums.
 */
gint daap_record_cmp_by_album (gpointer a, gpointer b, DMAPDb * db);

#endif /* __DAAP_RECORD_H */

G_END_DECLS
