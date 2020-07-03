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

static gint dmap_container_db_init_count = 0;

static void
dmap_container_db_init (G_GNUC_UNUSED DMAPContainerDbIface * iface)
{
	dmap_container_db_init_count++;
}

static void
dmap_container_db_finalize (G_GNUC_UNUSED DMAPContainerDbIface * iface)
{
	dmap_container_db_init_count--;
}

/* FIXME: No G_DEFINE_INTERFACE available in GObject headers: */
GType
dmap_container_db_get_type (void)
{
	static GType object_type = 0;

	if (!object_type) {
		static const GTypeInfo object_info = {
			class_size:     sizeof (DMAPContainerDbIface),
			base_init:     (GBaseInitFunc) dmap_container_db_init,
			base_finalize: (GBaseFinalizeFunc) dmap_container_db_finalize
		};
		object_type =
			g_type_register_static (G_TYPE_INTERFACE,
						"DMAPContainerDb",
						&object_info, 0);
		g_type_interface_add_prerequisite (object_type,
						   G_TYPE_OBJECT);
	}
	return object_type;
}

void
dmap_container_db_add (DMAPContainerDb * db, DMAPContainerRecord *record)
{
	return DMAP_CONTAINER_DB_GET_INTERFACE (db)->add (db, record);
}

DMAPContainerRecord *
dmap_container_db_lookup_by_id (DMAPContainerDb * db, guint id)
{
	return DMAP_CONTAINER_DB_GET_INTERFACE (db)->lookup_by_id (db, id);
}

void
dmap_container_db_foreach (DMAPContainerDb * db, GHFunc func, gpointer data)
{
	DMAP_CONTAINER_DB_GET_INTERFACE (db)->foreach (db, func, data);
}

gulong
dmap_container_db_count (DMAPContainerDb * db)
{
	return DMAP_CONTAINER_DB_GET_INTERFACE (db)->count (db);
}
