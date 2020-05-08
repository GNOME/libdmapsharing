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

#ifndef _TEST_DMAP_AV_RECORD_H
#define _TEST_DMAP_AV_RECORD_H

#include <libdmapsharing/dmap.h>

G_BEGIN_DECLS

#define TYPE_TEST_DMAP_AV_RECORD         (test_dmap_av_record_get_type ())
#define TEST_DMAP_AV_RECORD(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				       TYPE_TEST_DMAP_AV_RECORD, TestDmapAvRecord))
#define TEST_DMAP_AV_RECORD_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), \
			               TYPE_TEST_DMAP_AV_RECORD, \
				       TestDmapAvRecordClass))
#define IS_TEST_DMAP_AV_RECORD(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				       TYPE_TEST_DMAP_AV_RECORD))
#define IS_TEST_DMAP_AV_RECORD_CLASS (k) (G_TYPE_CHECK_CLASS_TYPE ((k), \
				       TYPE_TEST_DMAP_AV_RECORD_CLASS))
#define TEST_DMAP_AV_RECORD_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), \
				       TYPE_TEST_DMAP_AV_RECORD, \
				       TestDmapAvRecordClass))

typedef struct TestDmapAvRecordPrivate TestDmapAvRecordPrivate;

typedef struct {
	GObject parent;
	TestDmapAvRecordPrivate *priv;
} TestDmapAvRecord;

typedef struct {
	GObjectClass parent;
} TestDmapAvRecordClass;

GType test_dmap_av_record_get_type (void);

TestDmapAvRecord *test_dmap_av_record_new           (void);

GInputStream   *test_dmap_av_record_read          (DmapAvRecord *record, GError **err);

G_END_DECLS

#endif
