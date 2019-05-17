/*
 *  Database interface for a DmapRecord Factory
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

static void
dmap_record_factory_default_init (G_GNUC_UNUSED DmapRecordFactoryInterface * iface)
{
}

G_DEFINE_INTERFACE(DmapRecordFactory, dmap_record_factory, G_TYPE_OBJECT)

DmapRecord *
dmap_record_factory_create (DmapRecordFactory *factory,
                            gpointer user_data,
                            GError **error)
{
	DmapRecord *record = DMAP_RECORD_FACTORY_GET_INTERFACE
		(factory)->create (factory,
		                   user_data,
		                   error);

	g_assert((NULL == record && (NULL == error || NULL != *error))
	      || (NULL != record && (NULL == error || NULL == *error)));

	return record;
}
