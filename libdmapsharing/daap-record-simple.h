/*
 * Database record class for DAAP sharing
 *
 * Copyright (C) 2008 W. Michael Petullo <mike@flyn.org>
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

#ifndef __DAAP_RECORD_SIMPLE
#define __DAAP_RECORD_SIMPLE

#include <gio/gio.h>
#include <libdmapsharing/daap-record.h>

G_BEGIN_DECLS

#define DAAP_TYPE_RECORD_SIMPLE         (daap_record_simple_get_type ())
#define DAAP_RECORD_SIMPLE(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				       DAAP_TYPE_RECORD_SIMPLE, DAAPRecordSimple))
#define DAAP_RECORD_SIMPLE_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), \
			               DAAP_TYPE_RECORD_SIMPLE, \
				       DAAPRecordSimpleClass))
#define IS_DAAP_RECORD_SIMPLE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				       DAAP_TYPE_RECORD_SIMPLE))
#define IS_DAAP_RECORD_SIMPLE_CLASS (k) (G_TYPE_CHECK_CLASS_TYPE ((k), \
				       DAAP_TYPE_RECORD_SIMPLE_CLASS))
#define DAAP_RECORD_SIMPLE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), \
				       DAAP_TYPE_RECORD_SIMPLE, \
				       DAAPRecordSimpleClass))
#define DAAP_RECORD_SIMPLE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
					 DAAP_TYPE_RECORD_SIMPLE, \
					 DAAPRecordSimplePrivate))

typedef struct DAAPRecordSimplePrivate DAAPRecordSimplePrivate;

typedef struct {
	GObject parent;
	DAAPRecordSimplePrivate *priv;
} DAAPRecordSimple;

typedef struct {
	GObjectClass parent;
} DAAPRecordSimpleClass;

GType daap_record_simple_get_type (void);

DAAPRecordSimple *daap_record_simple_new           (void);

GInputStream   *daap_record_simple_read          (DAAPRecord *record, GError **err);

#endif /* __DAAP_RECORD_SIMPLE */

G_END_DECLS
