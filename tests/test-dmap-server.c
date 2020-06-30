/*
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

#include "config.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <glib.h>

#include <libdmapsharing/dmap.h>
#include <libdmapsharing/test-daap-record.h>
#include <libdmapsharing/test-daap-record-factory.h>
#include <libdmapsharing/test-dpap-record.h>
#include <libdmapsharing/test-dpap-record-factory.h>

#include "test-dmap-container-record.h"
#include "test-dmap-container-db.h"
#include "test-dmap-db.h"

/* For use when deciding whether to test DAAP or DPAP. */
enum {
	DAAP,
	DPAP
};

static char *
dmap_sharing_default_share_name ()
{
	const gchar *real_name;

	real_name = g_get_real_name ();
	if (strcmp (real_name, "Unknown") == 0) {
		real_name = g_get_user_name ();
	}

	return g_strdup_printf ("%s's Media (libdmapsharing test)", real_name);
}

static void
create_share (guint conn_type)
{
	char *name = dmap_sharing_default_share_name ();
	DMAPContainerRecord *dmap_container_record = \
		DMAP_CONTAINER_RECORD (test_dmap_container_record_new ());
	DMAPContainerDb *dmap_container_db = \
		DMAP_CONTAINER_DB (test_dmap_container_db_new
					(dmap_container_record));
	DMAPRecordFactory *factory;
	DMAPRecord *record;
	DMAPShare *share = NULL;
	DMAPDb *db;

	if (conn_type == DAAP) { 
		factory = DMAP_RECORD_FACTORY (test_daap_record_factory_new ());

	} else {
		factory = DMAP_RECORD_FACTORY (test_dpap_record_factory_new ());
	}

	record = DMAP_RECORD (dmap_record_factory_create (factory, NULL));
	db = DMAP_DB (test_dmap_db_new ());
	dmap_db_add (db, record);
	g_object_unref (record);

	g_warning ("initialize DAAP sharing");

	if (conn_type == DAAP) {
		share = DMAP_SHARE (daap_share_new (name,
						    NULL,
						    db,
						    dmap_container_db,
						    NULL));
	} else {
		share = DMAP_SHARE (dpap_share_new (name,
						    NULL,
						    db,
						    dmap_container_db,
						    NULL));
	}

	g_assert (NULL != share);

	g_free (name);
}

int
main (int argc, char *argv[])
{
	guint conn_type = DAAP;
	static GMainLoop *loop;

	if (argc == 2)
		conn_type = atoi (argv[1]);

	loop = g_main_loop_new (NULL, FALSE);

	create_share (conn_type);

	g_main_loop_run (loop);

	exit(EXIT_SUCCESS);
}
