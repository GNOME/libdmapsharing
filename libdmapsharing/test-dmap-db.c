/*
 * Database record class for DMAP sharing
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

#include <glib.h>

#include "test-dmap-db.h"

struct TestDmapDbPrivate {
	GHashTable *db;
	guint nextid;
};

static DmapRecord *
test_dmap_db_lookup_by_id (const DmapDb *db, guint id)
{
	DmapRecord *record;
	record = g_hash_table_lookup (TEST_DMAP_DB (db)->priv->db, GUINT_TO_POINTER (id));
	g_object_ref (record);
	return record;
}

static void
test_dmap_db_foreach	     (const DmapDb *db,
			      DmapIdRecordFunc func,
			      gpointer data)
{
	g_hash_table_foreach (TEST_DMAP_DB (db)->priv->db, (GHFunc) func, data);
}

static gint64
test_dmap_db_count (const DmapDb *db)
{
	return g_hash_table_size (TEST_DMAP_DB (db)->priv->db);
}

guint
test_dmap_db_add (DmapDb *db, DmapRecord *record, GError **error)
{
        guint id;
	id = TEST_DMAP_DB (db)->priv->nextid--;
	g_object_ref (record);
	g_hash_table_insert (TEST_DMAP_DB (db)->priv->db, GUINT_TO_POINTER (id), record);
	return id;
}

static void
test_dmap_db_init (TestDmapDb *db)
{
	db->priv = TEST_DMAP_DB_GET_PRIVATE (db);
	db->priv->db = g_hash_table_new (g_direct_hash, g_direct_equal);

	/* Media ID's start at max and go down.
	 * Container ID's start at 1 and go up.
	 */
	db->priv->nextid = G_MAXINT;
}

static void
test_dmap_db_class_init (TestDmapDbClass *klass)
{
}

static void
_dmap_db_iface_init (gpointer iface, gpointer data)
{
	DmapDbInterface *dmap_db = iface;

	g_assert (G_TYPE_FROM_INTERFACE (dmap_db) == DMAP_TYPE_DB);

	dmap_db->add = test_dmap_db_add;
	dmap_db->lookup_by_id = test_dmap_db_lookup_by_id;
	dmap_db->foreach = test_dmap_db_foreach;
	dmap_db->count = test_dmap_db_count;
}

G_DEFINE_TYPE_WITH_CODE (TestDmapDb, test_dmap_db, G_TYPE_OBJECT, 
                         G_IMPLEMENT_INTERFACE (DMAP_TYPE_DB,
                                                _dmap_db_iface_init)
                         G_ADD_PRIVATE (TestDmapDb))

TestDmapDb *
test_dmap_db_new (void)
{
	TestDmapDb *db;

	db = TEST_DMAP_DB (g_object_new (TYPE_TEST_DMAP_DB, NULL));

	return db;
}
