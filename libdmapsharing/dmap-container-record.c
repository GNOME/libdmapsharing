/*
 *  Database record interface for DMAP containers
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

#include <libdmapsharing/dmap-container-record.h>

static void
dmap_container_record_default_init (DmapContainerRecordInterface * iface)
{
	static gboolean is_initialized = FALSE;

	if (!is_initialized) {
		g_object_interface_install_property (iface,
						     g_param_spec_string
						     ("name",
						      "Container name",
						      "Container name", NULL,
						      G_PARAM_READWRITE));

		is_initialized = TRUE;
	}
}

G_DEFINE_INTERFACE(DmapContainerRecord, dmap_container_record, G_TYPE_OBJECT)

guint
dmap_container_record_get_id (DmapContainerRecord * record)
{
	return DMAP_CONTAINER_RECORD_GET_INTERFACE (record)->get_id (record);
}

/* NOTE: record is not used in dmapd implementation, only ID. Should we get rid
 * of record in next API generation? Should we add a function to explicitly set
 * a pointer to the "whole" media database (in which the ID is valid)?
 */
void
dmap_container_record_add_entry (DmapContainerRecord * container_record,
				 DmapRecord * record, gint id, GError **error)
{
	DMAP_CONTAINER_RECORD_GET_INTERFACE (container_record)->
		add_entry (container_record, record, id, error);
}

guint64
dmap_container_record_get_entry_count (DmapContainerRecord * record)
{
	return DMAP_CONTAINER_RECORD_GET_INTERFACE (record)->
		get_entry_count (record);
}

DmapDb *
dmap_container_record_get_entries (DmapContainerRecord * record)
{
	return DMAP_CONTAINER_RECORD_GET_INTERFACE (record)->
		get_entries (record);
}
