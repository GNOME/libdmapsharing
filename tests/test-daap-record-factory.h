/*
 * TestDAAPRecord factory class
 *
 * Copyright (C) 2008 W. Michael Petullo <mike@flyn.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __TEST_DAAP_RECORD_FACTORY
#define __TEST_DAAP_RECORD_FACTORY

#include <libdmapsharing/dmap.h>

G_BEGIN_DECLS

#define TYPE_TEST_DAAP_RECORD_FACTORY         (test_daap_record_factory_get_type ())
#define TEST_DAAP_RECORD_FACTORY(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				               TYPE_TEST_DAAP_RECORD_FACTORY, TestDAAPRecordFactory))
#define TEST_DAAP_RECORD_FACTORY_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), \
				               TYPE_TEST_DAAP_RECORD_FACTORY, TestDAAPRecordFactoryClass))
#define IS_TEST_DAAP_RECORD_FACTORY(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				               TYPE_TEST_DAAP_RECORD_FACTORY))
#define IS_TEST_DAAP_RECORD_FACTORY_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), \
				               TYPE_TEST_DAAP_RECORD_FACTORY_CLASS))
#define TEST_DAAP_RECORD_FACTORY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), \
				               TYPE_TEST_DAAP_RECORD_FACTORY, TestDAAPRecordFactoryClass))

typedef struct TestDAAPRecordFactoryPrivate TestDAAPRecordFactoryPrivate;

typedef struct {
	GObject parent;
} TestDAAPRecordFactory;

typedef struct {
	GObjectClass parent;
} TestDAAPRecordFactoryClass;

GType                  test_daap_record_factory_get_type (void);

TestDAAPRecordFactory *test_daap_record_factory_new      (void);

DMAPRecord            *test_daap_record_factory_create   (DMAPRecordFactory *factory, gpointer user_data);

#endif /* __TEST_DAAP_RECORD_FACTORY */

G_END_DECLS
