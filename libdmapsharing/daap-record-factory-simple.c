/*
 * DAAPRecord factory class
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

#include "daap-record-factory-simple.h"
#include "daap-record-simple.h"

DMAPRecord *
daap_record_factory_simple_create  (DMAPRecordFactory *factory, gpointer user_data)
{
	return DMAP_RECORD (daap_record_simple_new ());
}

static void
daap_record_factory_simple_init (DAAPRecordFactorySimple *factory)
{
}

static void
daap_record_factory_simple_class_init (DAAPRecordFactorySimpleClass *klass)
{
}

static void
daap_record_factory_simple_interface_init (gpointer iface, gpointer data)
{
	DMAPRecordFactoryIface *factory = iface;

	g_assert (G_TYPE_FROM_INTERFACE (factory) == DMAP_TYPE_RECORD_FACTORY);

	factory->create = daap_record_factory_simple_create;
}

G_DEFINE_TYPE_WITH_CODE (DAAPRecordFactorySimple, daap_record_factory_simple, G_TYPE_OBJECT, 
			 G_IMPLEMENT_INTERFACE (DMAP_TYPE_RECORD_FACTORY,
					        daap_record_factory_simple_interface_init))

DAAPRecordFactorySimple *
daap_record_factory_simple_new (void)
{
	DAAPRecordFactorySimple *factory;

	factory = DAAP_RECORD_FACTORY_SIMPLE (g_object_new (DAAP_TYPE_RECORD_FACTORY_SIMPLE, NULL));

	return factory;
}
