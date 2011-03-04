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

#ifndef __DPAP_RECORD_H
#define __DPAP_RECORD_H

#include <glib.h>
#include <gio/gio.h>

#include <libdmapsharing/dmap-record.h>

G_BEGIN_DECLS
/**
 * DPAP_TYPE_RECORD:
 *
 * The type for #DPAPRecord.
 */
#define DPAP_TYPE_RECORD	     (dpap_record_get_type ())
/**
 * DPAP_RECORD:
 * @o: Object which is subject to casting.
 *
 * Casts a #DPAPRecord or derived pointer into a (DPAPRecord *) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DPAP_RECORD(o)		     (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				      DPAP_TYPE_RECORD, DPAPRecord))
/**
 * IS_DPAP_RECORD:
 * @o: Instance to check for being a %DPAP_TYPE_RECORD.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %DPAP_TYPE_RECORD.
 */
#define IS_DPAP_RECORD(o)	     (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				      DPAP_TYPE_RECORD))
/**
 * DPAP_RECORD_GET_INTERFACE:
 * @o: a #DPAPRecord instance.
 *
 * Get the class structure associated to a #DPAPRecord instance.
 *
 * Returns: pointer to object interface structure.
 */
#define DPAP_RECORD_GET_INTERFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), \
				      DPAP_TYPE_RECORD, DPAPRecordIface))
typedef struct _DPAPRecord DPAPRecord;
typedef struct _DPAPRecordIface DPAPRecordIface;

struct _DPAPRecordIface
{
	GTypeInterface parent;

	GInputStream *(*read) (DPAPRecord * record, GError ** err);
};

GType dpap_record_get_type (void);

/**
 * dpap_record_read:
 * @record: a DPAPRecord.
 * @err: a GError.
 *
 * Returns: A GInputStream that provides read-only access to the data stream
 * associated with record.
 */
GInputStream *dpap_record_read (DPAPRecord * record, GError ** err);

#endif /* __DPAP_RECORD_H */

G_END_DECLS
