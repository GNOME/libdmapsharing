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

G_BEGIN_DECLS

#define TYPE_DAAP_RECORD	     (daap_record_get_type ())
#define DAAP_RECORD(o)		     (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				      TYPE_DAAP_RECORD, DAAPRecord))
#define IS_DAAP_RECORD(o)	     (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				      TYPE_DAAP_RECORD))
#define DAAP_RECORD_GET_INTERFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), \
				      TYPE_DAAP_RECORD, DAAPRecordInterface))

typedef struct _DAAPRecord DAAPRecord;
typedef struct _DAAPRecordInterface DAAPRecordInterface;

struct _DAAPRecordInterface {
	GTypeInterface parent;

	gboolean	(*itunes_compat) (DAAPRecord *record);
	GInputStream *	(*read)	         (DAAPRecord *record, GError **err);
};

GType         daap_record_get_type      (void);
gboolean      daap_record_itunes_compat (DAAPRecord *record);
GInputStream *daap_record_read          (DAAPRecord *record, GError **err);

#endif /* __DAAP_RECORD_H */

G_END_DECLS
