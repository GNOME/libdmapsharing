/*
 * Database record class for DMAP sharing
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

#include "test-dmap-container-record.h"

enum {
        PROP_0,
	PROP_NAME
};

static DmapDb *_entries = NULL;

static void
test_dmap_container_record_set_property (GObject *object,
                                         guint prop_id,
                                         G_GNUC_UNUSED const GValue *value,
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


static guint
_get_id (G_GNUC_UNUSED DmapContainerRecord *record)
{
	return 2;
}

static void
_add_entry (G_GNUC_UNUSED DmapContainerRecord *container_record,
            G_GNUC_UNUSED DmapRecord *record,
            G_GNUC_UNUSED gint id,
            G_GNUC_UNUSED GError **error)
{
}

static guint64
_get_entry_count (G_GNUC_UNUSED DmapContainerRecord *record)
{
        return 1;
}

static DmapDb *
_get_entries (G_GNUC_UNUSED DmapContainerRecord *record)
{
	return g_object_ref (_entries);
}

static void
test_dmap_container_record_init (G_GNUC_UNUSED TestDmapContainerRecord *record)
{
	_entries = DMAP_DB (test_dmap_db_new ());
}

static void
test_dmap_container_record_class_init (TestDmapContainerRecordClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->set_property = test_dmap_container_record_set_property;
        gobject_class->get_property = test_dmap_container_record_get_property;

        g_object_class_override_property (gobject_class, PROP_NAME, "name");
}

static void
_dmap_container_record_iface_init (gpointer iface)
{
	DmapContainerRecordInterface *dmap_container_record = iface;

	g_assert (G_TYPE_FROM_INTERFACE (dmap_container_record) == DMAP_TYPE_CONTAINER_RECORD);

	dmap_container_record->get_id = _get_id;
	dmap_container_record->add_entry = _add_entry;
	dmap_container_record->get_entry_count = _get_entry_count;
	dmap_container_record->get_entries = _get_entries;
}

G_DEFINE_TYPE_WITH_CODE (TestDmapContainerRecord, test_dmap_container_record, G_TYPE_OBJECT, 
			G_IMPLEMENT_INTERFACE (DMAP_TYPE_CONTAINER_RECORD,
					       _dmap_container_record_iface_init))

TestDmapContainerRecord *test_dmap_container_record_new (void)
{
	TestDmapContainerRecord *record;

	record = TEST_DMAP_CONTAINER_RECORD (g_object_new (TYPE_TEST_DMAP_CONTAINER_RECORD, NULL));

	return record;
}
