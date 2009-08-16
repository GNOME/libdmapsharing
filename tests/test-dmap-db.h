/*
 * Database class for DMAP sharing
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

#ifndef __TEST_DMAP_DB
#define __TEST_DMAP_DB

#include <libdmapsharing/dmap-db.h>

G_BEGIN_DECLS

#define TYPE_TEST_DMAP_DB           (test_dmap_db_get_type ())
#define TEST_DMAP_DB(o)             (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				     TYPE_TEST_DMAP_DB, TestDMAPDb))
#define TEST_DMAP_DB_CLASS(k)       (G_TYPE_CHECK_CLASS_CAST((k), \
				     TYPE_TEST_DMAP_DB, TestDMAPDbClass))
#define IS_TEST_DMAP_DB(o)          (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				     TYPE_TEST_DMAP_DB))
#define IS_TEST_DMAP_DB_CLASS (k)   (G_TYPE_CHECK_CLASS_TYPE ((k), \
				     TYPE_TEST_DMAP_DB_CLASS))
#define TEST_DMAP_DB_GET_CLASS(o)   (G_TYPE_INSTANCE_GET_CLASS ((o), \
				     TYPE_TEST_DMAP_DB, TestDMAPDbClass))
#define TEST_DMAP_DB_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
				     TYPE_TEST_DMAP_DB, \
				     TestDMAPDbPrivate))

typedef struct TestDMAPDbPrivate TestDMAPDbPrivate;

typedef struct {
	GObject parent;
	TestDMAPDbPrivate *priv;
} TestDMAPDb;

typedef struct {
	GObjectClass parent;
} TestDMAPDbClass;

TestDMAPDb *test_dmap_db_new (void);
GType test_dmap_db_get_type (void);

#endif /* __TEST_DMAP_DB */

G_END_DECLS
