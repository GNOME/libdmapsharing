/*
 * Database class for DMAP sharing
 *
 * Copyright (C) 2008 W. Michael Petullo <mike@flyn.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "test-dmap-container-db.h"

/* This test implementation of the DMAPContainerDb interface allows for one
 * record.  Normally, one would befine some data structure to contain
 * multiple records.
 */
static DMAPContainerRecord *record = NULL;

static DMAPContainerRecord *
test_dmap_container_db_lookup_by_id (DMAPContainerDb *db, guint id)
{
	/* In reality, lookup the proper record and return it. */
	return record;
}

static void
test_dmap_container_db_foreach (DMAPContainerDb *db,
				GHFunc func,
				gpointer data)
{
	/* In reality, pull each record from the db and execute func on it. */
        func (record, data, NULL);
}

static gint64
test_dmap_container_db_count (DMAPContainerDb *db)
{
	/* In reality, return the record count. */
	return 1;
}

static void
test_dmap_container_db_init (TestDMAPContainerDb *db)
{
}

static void
test_dmap_container_db_class_init (TestDMAPContainerDbClass *klass)
{
}

static void
test_dmap_container_db_interface_init (gpointer iface, gpointer data)
{
	DMAPContainerDbInterface *dmap_container_db = iface;

	g_assert (G_TYPE_FROM_INTERFACE (dmap_container_db) == TYPE_DMAP_CONTAINER_DB);

	dmap_container_db->lookup_by_id = test_dmap_container_db_lookup_by_id;
	dmap_container_db->foreach = test_dmap_container_db_foreach;
	dmap_container_db->count = test_dmap_container_db_count;
}

G_DEFINE_TYPE_WITH_CODE (TestDMAPContainerDb, test_dmap_container_db, G_TYPE_OBJECT, 
			 G_IMPLEMENT_INTERFACE (TYPE_DMAP_CONTAINER_DB,
						test_dmap_container_db_interface_init))

TestDMAPContainerDb *
test_dmap_container_db_new (DMAPContainerRecord *r)
{
	TestDMAPContainerDb *db;

	db = TEST_DMAP_CONTAINER_DB (g_object_new (TYPE_TEST_DMAP_CONTAINER_DB, NULL));

	record = r;

	return db;
}
