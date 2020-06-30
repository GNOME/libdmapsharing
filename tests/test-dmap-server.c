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
#include <libdmapsharing/test-dmap-av-record.h>
#include <libdmapsharing/test-dmap-image-record.h>
#include <libdmapsharing/test-dmap-av-record-factory.h>
#include <libdmapsharing/test-dmap-image-record-factory.h>
#include <libdmapsharing/test-dmap-db.h>
#include <libdmapsharing/test-dmap-container-record.h>
#include <libdmapsharing/test-dmap-container-db.h>

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
error_cb(G_GNUC_UNUSED DmapShare *share, GError *error, G_GNUC_UNUSED gpointer user_data)
{
	g_error("%s", error->message);
}

static gboolean
_quit(gpointer user_data)
{
	g_main_loop_quit(user_data);
	return FALSE;
}

static DmapShare *
create_share (guint conn_type, GMainLoop *loop)
{
	char *name = dmap_sharing_default_share_name ();
	DmapContainerRecord *dmap_container_record = \
		DMAP_CONTAINER_RECORD (test_dmap_container_record_new ());
	DmapContainerDb *container_db = \
		DMAP_CONTAINER_DB (test_dmap_container_db_new
					(dmap_container_record));
	DmapRecordFactory *factory;
	DmapRecord *record;
	DmapShare *share = NULL;
	GError *error = NULL;
	gboolean ok;
	DmapDb *db;

	switch (conn_type) {
	default:
		g_idle_add(_quit, loop);
		factory = DMAP_RECORD_FACTORY (test_dmap_av_record_factory_new ());
		break;
	case DAAP:
		factory = DMAP_RECORD_FACTORY (test_dmap_av_record_factory_new ());
		break;
	case DPAP:
		factory = DMAP_RECORD_FACTORY (test_dmap_image_record_factory_new ());
		break;
	}

	record = DMAP_RECORD (dmap_record_factory_create (factory, NULL, NULL));
	db = DMAP_DB (test_dmap_db_new ());
	dmap_db_add (db, record, NULL);
	g_object_unref (record);

	g_warning ("initialize DAAP sharing");

	switch (conn_type) {
	default:
	case DAAP:
		share = DMAP_SHARE (dmap_av_share_new (name,
                                                       NULL,
                                                       db,
                                                       container_db,
		                                       NULL));
		break;
	case DPAP:
		share = DMAP_SHARE (dmap_image_share_new (name,
                                                          NULL,
                                                          db,
                                                          container_db,
                                                          NULL));
		break;
	}

	g_assert (NULL != share);

	g_signal_connect(share, "error", G_CALLBACK(error_cb), NULL);

	ok = dmap_share_serve(share, &error);
	if (!ok) {
		g_error("Error starting server: %s", error->message);
	}

	ok = dmap_share_publish(share, &error);
	if (!ok) {
		g_error("Error publishing server: %s", error->message);
	}

	g_object_unref (factory);
	g_object_unref (container_db);
	g_object_unref (db);
	g_free (name);

	return share;
}

int
main (int argc, char *argv[])
{
	DmapShare *share = NULL;
	guint conn_type = DAAP;
	static GMainLoop *loop;

	if (argc == 2)
		conn_type = atoi (argv[1]);

	loop = g_main_loop_new (NULL, FALSE);

	share = create_share (conn_type, loop);

	g_main_loop_run (loop);

	g_object_unref(share);

	exit(EXIT_SUCCESS);
}
