/*
 *  Database interface for DMAP containers
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

#include <libdmapsharing/dmap-container-db.h>

static void
dmap_container_db_default_init (G_GNUC_UNUSED DmapContainerDbInterface * iface)
{
}

G_DEFINE_INTERFACE(DmapContainerDb, dmap_container_db, G_TYPE_OBJECT)

void
dmap_container_db_add (DmapContainerDb *db, DmapContainerRecord *record, GError **error)
{
	return DMAP_CONTAINER_DB_GET_INTERFACE (db)->add (db, record, error);
}

DmapContainerRecord *
dmap_container_db_lookup_by_id (DmapContainerDb * db, guint id)
{
	return DMAP_CONTAINER_DB_GET_INTERFACE (db)->lookup_by_id (db, id);
}

void
dmap_container_db_foreach (DmapContainerDb * db, DmapIdContainerRecordFunc func, gpointer data)
{
	DMAP_CONTAINER_DB_GET_INTERFACE (db)->foreach (db, func, data);
}

gulong
dmap_container_db_count (DmapContainerDb * db)
{
	return DMAP_CONTAINER_DB_GET_INTERFACE (db)->count (db);
}
