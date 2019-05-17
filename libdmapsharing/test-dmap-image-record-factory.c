/*
 * DmapImageRecord factory class
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

#include "test-dmap-image-record-factory.h"
#include "test-dmap-image-record.h"

DmapRecord *
test_dmap_image_record_factory_create(G_GNUC_UNUSED DmapRecordFactory *factory,
                                      G_GNUC_UNUSED gpointer user_data,
                                      G_GNUC_UNUSED GError **error)
{
	return DMAP_RECORD (test_dmap_image_record_new ());
}

static void
test_dmap_image_record_factory_init (G_GNUC_UNUSED TestDmapImageRecordFactory *factory)
{
}

static void
test_dmap_image_record_factory_class_init (G_GNUC_UNUSED TestDmapImageRecordFactoryClass *klass)
{
}

static void
_dmap_record_factory_iface_init (gpointer iface)
{
	DmapRecordFactoryInterface *factory = iface;

	g_assert (G_TYPE_FROM_INTERFACE (factory) == DMAP_TYPE_RECORD_FACTORY);

	factory->create = test_dmap_image_record_factory_create;
}

G_DEFINE_TYPE_WITH_CODE (TestDmapImageRecordFactory, test_dmap_image_record_factory, G_TYPE_OBJECT, 
			 G_IMPLEMENT_INTERFACE (DMAP_TYPE_RECORD_FACTORY,
					        _dmap_record_factory_iface_init))

TestDmapImageRecordFactory *
test_dmap_image_record_factory_new (void)
{
	TestDmapImageRecordFactory *factory;

	factory = TEST_DMAP_IMAGE_RECORD_FACTORY (g_object_new (TYPE_TEST_DMAP_IMAGE_RECORD_FACTORY, NULL));

	return factory;
}
