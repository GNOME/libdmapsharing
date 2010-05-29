/*
 * Database record class for DMAP sharing
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

#include "test-dmap-container-record.h"

enum {
        PROP_0,
	PROP_NAME
};

static DMAPDb *entries = NULL;

static void
test_dmap_container_record_set_property (GObject *object,
                                         guint prop_id,
                                         const GValue *value,
                                         GParamSpec *pspec)
{
        switch (prop_id) {
                case PROP_NAME:
			/* NOTE: do nothing; test name is always the same. */
                        break;
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                        break;
        }
}

static void
test_dmap_container_record_get_property (GObject *object,
                                         guint prop_id,
                                         GValue *value,
                                         GParamSpec *pspec)
{
        switch (prop_id) {
                case PROP_NAME:
                        g_value_set_string (value, "Test");
                        break;
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                        break;
        }
}


guint
test_dmap_container_record_get_id (DMAPContainerRecord *record)
{
	return 2;
}

void
test_dmap_container_record_add_entry (DMAPContainerRecord *container_record,
				      DMAPRecord *record,
				      gint id)
{
}

guint64 
test_dmap_container_record_get_entry_count (DMAPContainerRecord *record)
{
        return 1;
}

const DMAPDb *
test_dmap_container_record_get_entries (DMAPContainerRecord *record)
{
	return entries;
}

static void
test_dmap_container_record_init (TestDMAPContainerRecord *record)
{
	entries = DMAP_DB (test_dmap_db_new ());
}

static void
test_dmap_container_record_class_init (TestDMAPContainerRecordClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->set_property = test_dmap_container_record_set_property;
        gobject_class->get_property = test_dmap_container_record_get_property;

        g_object_class_override_property (gobject_class, PROP_NAME, "name");
}

static void
test_dmap_container_record_interface_init (gpointer iface, gpointer data)
{
	DMAPContainerRecordInterface *dmap_container_record = iface;

	g_assert (G_TYPE_FROM_INTERFACE (dmap_container_record) == TYPE_DMAP_CONTAINER_RECORD);

	dmap_container_record->get_id = test_dmap_container_record_get_id;
	dmap_container_record->add_entry = test_dmap_container_record_add_entry;
	dmap_container_record->get_entry_count = test_dmap_container_record_get_entry_count;
	dmap_container_record->get_entries = test_dmap_container_record_get_entries;
}

G_DEFINE_TYPE_WITH_CODE (TestDMAPContainerRecord, test_dmap_container_record, G_TYPE_OBJECT, 
			G_IMPLEMENT_INTERFACE (TYPE_DMAP_CONTAINER_RECORD,
					       test_dmap_container_record_interface_init))

TestDMAPContainerRecord *test_dmap_container_record_new (void)
{
	TestDMAPContainerRecord *record;

	record = TEST_DMAP_CONTAINER_RECORD (g_object_new (TYPE_TEST_DMAP_CONTAINER_RECORD, NULL));

	return record;
}
