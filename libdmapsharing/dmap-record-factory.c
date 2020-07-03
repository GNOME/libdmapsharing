/*
 *  Database interface for a DMAPRecord Factory
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

#include <libdmapsharing/dmap-record-factory.h>

static gint dmap_record_factory_init_count = 0;

static void
dmap_record_factory_init (G_GNUC_UNUSED DMAPRecordFactoryIface * iface)
{
	dmap_record_factory_init_count++;
}

static void
dmap_record_factory_finalize (G_GNUC_UNUSED DMAPRecordFactoryIface * iface)
{
	dmap_record_factory_init_count--;
}

/* FIXME: No G_DEFINE_INTERFACE available in GObject headers: */
GType
dmap_record_factory_get_type (void)
{
	static GType object_type = 0;

	if (!object_type) {
		static const GTypeInfo object_info = {
			class_size:     sizeof (DMAPRecordFactoryIface),
			base_init:     (GBaseInitFunc) dmap_record_factory_init,
			base_finalize: (GBaseFinalizeFunc) dmap_record_factory_finalize
		};
		object_type =
			g_type_register_static (G_TYPE_INTERFACE,
						"DMAPRecordFactory",
						&object_info, 0);
	}
	return object_type;
}

DMAPRecord *
dmap_record_factory_create (DMAPRecordFactory * factory, gpointer user_data)
{
	return DMAP_RECORD_FACTORY_GET_INTERFACE (factory)->create (factory,
								    user_data);
}
