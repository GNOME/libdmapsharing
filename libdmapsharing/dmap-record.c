/*
 *  Database record interface for DMAP sharing
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

#include <libdmapsharing/dmap-record.h>

static gint dmap_record_init_count = 0;

static void
dmap_record_init (DMAPRecordInterface *iface)
{
        dmap_record_init_count++;
}

static void
dmap_record_finalize (DMAPRecordInterface *iface)
{
	dmap_record_init_count--;
}

/* FIXME: No G_DEFINE_INTERFACE available in GObject headers: */
GType
dmap_record_get_type (void)
{
	static GType object_type = 0;
	if (!object_type) {
		static const GTypeInfo object_info = {
			sizeof(DMAPRecordInterface),
			(GBaseInitFunc) dmap_record_init,
			(GBaseFinalizeFunc) dmap_record_finalize
		};
		object_type =
		    g_type_register_static(G_TYPE_INTERFACE,
					   "DMAPRecord",
					   &object_info, 0);
	}
	return object_type;
}

GByteArray *
dmap_record_to_blob (DMAPRecord *record)
{
        return DMAP_RECORD_GET_INTERFACE (record)->to_blob (record);
}

DMAPRecord *
dmap_record_set_from_blob (DMAPRecord *record, GByteArray *blob)
{
	return DMAP_RECORD_GET_INTERFACE (record)->set_from_blob (record, blob);
}
