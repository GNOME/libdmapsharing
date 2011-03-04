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

static gint dmap_container_record_init_count = 0;

static void
dmap_container_record_init (DMAPContainerRecordIface * iface)
{
	static gboolean is_initialized = FALSE;

	dmap_container_record_init_count++;

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

static void
dmap_container_record_finalize (DMAPContainerRecordIface * iface)
{
	dmap_container_record_init_count--;
}

/* FIXME: No G_DEFINE_INTERFACE available in GObject headers: */
GType
dmap_container_record_get_type (void)
{
	static GType object_type = 0;

	if (!object_type) {
		static const GTypeInfo object_info = {
			sizeof (DMAPContainerRecordIface),
			(GBaseInitFunc) dmap_container_record_init,
			(GBaseFinalizeFunc) dmap_container_record_finalize
		};
		object_type =
			g_type_register_static (G_TYPE_INTERFACE,
						"DMAPContainerRecord",
						&object_info, 0);
	}
	return object_type;
}

guint
dmap_container_record_get_id (DMAPContainerRecord * record)
{
	return DMAP_CONTAINER_RECORD_GET_INTERFACE (record)->get_id (record);
}

void
dmap_container_record_add_entry (DMAPContainerRecord * container_record,
				 DMAPRecord * record, gint id)
{
	DMAP_CONTAINER_RECORD_GET_INTERFACE (container_record)->
		add_entry (container_record, record, id);
}

guint64
dmap_container_record_get_entry_count (DMAPContainerRecord * record)
{
	return DMAP_CONTAINER_RECORD_GET_INTERFACE (record)->
		get_entry_count (record);
}

DMAPDb *
dmap_container_record_get_entries (DMAPContainerRecord * record)
{
	return DMAP_CONTAINER_RECORD_GET_INTERFACE (record)->
		get_entries (record);
}
