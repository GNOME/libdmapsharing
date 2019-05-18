/*
 * Database class for DMAP sharing
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

#include "test-dmap-container-db.h"

/* This test implementation of the DmapContainerDb interface allows for one
 * record.  Normally, one would befine some data structure to contain
 * multiple records.
 */
static DmapContainerRecord *_record = NULL;

static DmapContainerRecord *
test_dmap_container_db_lookup_by_id (G_GNUC_UNUSED DmapContainerDb *db,
                                     G_GNUC_UNUSED guint id)
{
	/* In reality, lookup the proper record and return it. */
	return g_object_ref (_record);
}

static void
test_dmap_container_db_foreach (G_GNUC_UNUSED DmapContainerDb *db,
				DmapIdContainerRecordFunc func,
				gpointer data)
{
	/* In reality, pull each record from the db and execute func on it. */
        func (1, _record, data);
}

static gint64
test_dmap_container_db_count (G_GNUC_UNUSED DmapContainerDb *db)
{
	/* In reality, return the record count. */
	return 1;
}

static void
test_dmap_container_db_init (G_GNUC_UNUSED TestDmapContainerDb *db)
{
}

static void
test_dmap_container_db_class_init (G_GNUC_UNUSED TestDmapContainerDbClass *klass)
{
}

static void
_dmap_container_db_iface_init (gpointer iface)
{
	DmapContainerDbInterface *dmap_container_db = iface;

	g_assert (G_TYPE_FROM_INTERFACE (dmap_container_db) == DMAP_TYPE_CONTAINER_DB);

	dmap_container_db->lookup_by_id = test_dmap_container_db_lookup_by_id;
	dmap_container_db->foreach = test_dmap_container_db_foreach;
	dmap_container_db->count = test_dmap_container_db_count;
}

G_DEFINE_TYPE_WITH_CODE (TestDmapContainerDb, test_dmap_container_db, G_TYPE_OBJECT, 
			 G_IMPLEMENT_INTERFACE (DMAP_TYPE_CONTAINER_DB,
						_dmap_container_db_iface_init))

TestDmapContainerDb *
test_dmap_container_db_new (DmapContainerRecord *r)
{
	TestDmapContainerDb *db;

	db = TEST_DMAP_CONTAINER_DB (g_object_new (TYPE_TEST_DMAP_CONTAINER_DB, NULL));

	_record = r;

	return db;
}
