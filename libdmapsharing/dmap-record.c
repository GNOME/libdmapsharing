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

static void
dmap_record_default_init (G_GNUC_UNUSED DmapRecordInterface * iface)
{
}

G_DEFINE_INTERFACE(DmapRecord, dmap_record, G_TYPE_OBJECT)

GArray *
dmap_record_to_blob (DmapRecord * record)
{
	return DMAP_RECORD_GET_INTERFACE (record)->to_blob (record);
}

gboolean
dmap_record_set_from_blob (DmapRecord * record, GArray * blob)
{
	return DMAP_RECORD_GET_INTERFACE (record)->set_from_blob (record,
								  blob);
}
