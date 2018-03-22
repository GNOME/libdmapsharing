/*
 * Database class for DMAP sharing
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

#ifndef _TEST_DMAP_CONTAINER_DB_H
#define _TEST_DMAP_CONTAINER_DB_H

#include <libdmapsharing/dmap.h>

G_BEGIN_DECLS

#define TYPE_TEST_DMAP_CONTAINER_DB         (test_dmap_container_db_get_type ())
#define TEST_DMAP_CONTAINER_DB(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				   TYPE_TEST_DMAP_CONTAINER_DB, TestDmapContainerDb))
#define TEST_DMAP_CONTAINER_DB_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), \
				   TYPE_TEST_DMAP_CONTAINER_DB, TestDmapContainerDbClass))
#define IS_TEST_DMAP_CONTAINER_DB(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				   TYPE_TEST_DMAP_CONTAINER_DB))
#define IS_TEST_DMAP_CONTAINER_DB_CLASS (k) (G_TYPE_CHECK_CLASS_TYPE ((k), \
				   TYPE_TEST_DMAP_CONTAINER_DB_CLASS))
#define TEST_DMAP_CONTAINER_DB_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), \
				   TYPE_TEST_DMAP_CONTAINER_DB, TestDmapContainerDbClass))

typedef struct TestDmapContainerDbPrivate TestDmapContainerDbPrivate;

typedef struct {
	GObject parent;
} TestDmapContainerDb;

typedef struct {
	GObjectClass parent;
} TestDmapContainerDbClass;

TestDmapContainerDb *test_dmap_container_db_new             (DmapContainerRecord *record);

GType       test_dmap_container_db_get_type        (void);

G_END_DECLS

#endif
