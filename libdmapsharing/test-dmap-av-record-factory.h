/*
 * TestDmapAvRecord factory class
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

#ifndef _TEST_DMAP_AV_RECORD_FACTORY_H
#define _TEST_DMAP_AV_RECORD_FACTORY_H

#include <libdmapsharing/dmap.h>

G_BEGIN_DECLS

#define TYPE_TEST_DMAP_AV_RECORD_FACTORY         (test_dmap_av_record_factory_get_type ())
#define TEST_DMAP_AV_RECORD_FACTORY(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				               TYPE_TEST_DMAP_AV_RECORD_FACTORY, TestDmapAvRecordFactory))
#define TEST_DMAP_AV_RECORD_FACTORY_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), \
				               TYPE_TEST_DMAP_AV_RECORD_FACTORY, TestDmapAvRecordFactoryClass))
#define IS_TEST_DMAP_AV_RECORD_FACTORY(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				               TYPE_TEST_DMAP_AV_RECORD_FACTORY))
#define IS_TEST_DMAP_AV_RECORD_FACTORY_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), \
				               TYPE_TEST_DMAP_AV_RECORD_FACTORY_CLASS))
#define TEST_DMAP_AV_RECORD_FACTORY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), \
				               TYPE_TEST_DMAP_AV_RECORD_FACTORY, TestDmapAvRecordFactoryClass))

typedef struct TestDmapAvRecordFactoryPrivate TestDmapAvRecordFactoryPrivate;

typedef struct {
	GObject parent;
} TestDmapAvRecordFactory;

typedef struct {
	GObjectClass parent;
} TestDmapAvRecordFactoryClass;

GType                  test_dmap_av_record_factory_get_type(void);

TestDmapAvRecordFactory *test_dmap_av_record_factory_new(void);

DmapRecord            *test_dmap_av_record_factory_create(DmapRecordFactory *factory,
                                                          gpointer user_data,
                                                          GError **error);

G_END_DECLS

#endif
