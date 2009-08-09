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

#define TYPE_DPAP_RECORD	     (dpap_record_get_type ())
#define DPAP_RECORD(o)		     (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				      TYPE_DPAP_RECORD, DPAPRecord))
#define IS_DPAP_RECORD(o)	     (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				      TYPE_DPAP_RECORD))
#define DPAP_RECORD_GET_INTERFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), \
				      TYPE_DPAP_RECORD, DPAPRecordInterface))

typedef struct _DPAPRecord DPAPRecord;
typedef struct _DPAPRecordInterface DPAPRecordInterface;

struct _DPAPRecordInterface {
	GTypeInterface parent;

	GInputStream *  (*read)              (DPAPRecord *record, gchar *transcode_mimetype, GError **err);
};

GType          dpap_record_get_type          (void);
GInputStream  *dpap_record_read              (DPAPRecord *record, gchar *transcode_mimetype, GError **err);

#endif /* __DPAP_RECORD_H */

G_END_DECLS
