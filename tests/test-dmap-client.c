/*
 * Copyright (C) 2006 INDT
 *  Andre Moreira Magalhaes <andre.magalhaes@indt.org.br>
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

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libdmapsharing/dmap.h>
#include <libdmapsharing/test-dmap-av-record-factory.h>
#include <libdmapsharing/test-dmap-image-record-factory.h>
#include <libdmapsharing/test-dmap-db.h>

enum {
    DAAP,
    DPAP
};

static GMainLoop *loop;
static guint conn_type = DAAP;

static void
print_record (gpointer id, DmapRecord *record, G_GNUC_UNUSED gpointer user_data)
{
	if (DMAP_IS_AV_RECORD(record)) {
		gboolean has_video;
		gchar   *artist, *title;

		g_object_get (record,
			     "has-video", &has_video,
			     "songartist", &artist,
			     "title",  &title,
			      NULL);

		g_print ("%d: %s %s (has video: %s)\n", GPOINTER_TO_UINT(id), artist, title, has_video ? "Y" : "N");

		g_free (artist);
		g_free (title);
	} else if (DMAP_IS_IMAGE_RECORD(record)) {
		gchar   *format, *location;

		g_object_get (record,
			     "format", &format,
			     "location", &location,
			      NULL);

		g_print ("%d: %s %s\n", GPOINTER_TO_UINT(id), format, location);

		g_free (format);
		g_free (location);
	} else {
		g_assert_not_reached();
	}
}

static void error_cb(G_GNUC_UNUSED DmapConnection *connection,
                     GError *error,
                     G_GNUC_UNUSED gpointer user_data)
{
	g_error("%s", error->message);
}

static void
connected_cb (G_GNUC_UNUSED DmapConnection *connection,
              G_GNUC_UNUSED gboolean        result,
              G_GNUC_UNUSED const char     *reason,
              DmapDb         *db)
{
	g_print ("Connection cb., DB has %lu entries\n", dmap_db_count (db));

	dmap_db_foreach (db, (DmapIdRecordFunc) print_record, NULL);
}

static void
authenticate_cb (DmapConnection *connection,
		 G_GNUC_UNUSED const char *name,
		 SoupSession *session,
		 SoupMessage *msg,
		 SoupAuth *auth,
		 G_GNUC_UNUSED gboolean retrying,
		 G_GNUC_UNUSED gpointer user_data)
{
	char *username, password[BUFSIZ + 1], *rc;
	g_object_get (connection, "username", &username, NULL);
	g_print ("Password required (username is %s): ", username);

	rc = fgets (password, BUFSIZ, stdin);
	if (rc != password) {
		g_error ("failed to read password");
	}

	password[strlen(password) - 1] = 0x00; // Remove newline.

	dmap_connection_authenticate_message(connection, session, msg, auth, password);

	g_free(username);
}

static void
service_added_cb (G_GNUC_UNUSED DmapMdnsBrowser *browser,
                  DmapMdnsService *service,
                  G_GNUC_UNUSED gpointer user_data)
{
    DmapRecordFactory *factory;
    DmapConnection *conn;
    DmapDb *db;
    guint port;
    gchar *service_name, *name, *host;

    g_object_get(service, "service-name", &service_name,
                          "name", &name,
                          "host", &host,
                          "port", &port, NULL);

    g_debug ("service added %s:%s:%s:%d",
             service_name,
             name,
             host,
             port);

    db = DMAP_DB (test_dmap_db_new ());
    if (db == NULL) {
    	g_error ("Error creating DB");
    }

    if (conn_type == DAAP) {
        factory = DMAP_RECORD_FACTORY (test_dmap_av_record_factory_new ());
        if (factory == NULL) {
   	    g_error ("Error creating record factory");
        }
        conn = DMAP_CONNECTION (dmap_av_connection_new (name, host, port, db, factory));
    } else {
        factory = DMAP_RECORD_FACTORY (test_dmap_image_record_factory_new ());
        if (factory == NULL) {
   	    g_error ("Error creating record factory");
        }
        conn = DMAP_CONNECTION (dmap_image_connection_new (name, host, port, db, factory));
    }
    g_signal_connect (DMAP_CONNECTION (conn), "authenticate", G_CALLBACK(authenticate_cb), NULL);
    g_signal_connect (DMAP_CONNECTION (conn), "error", G_CALLBACK(error_cb), NULL);

    dmap_connection_start (DMAP_CONNECTION (conn), (DmapConnectionFunc) connected_cb, db);

    g_free(service_name);
    g_free(name);
    g_free(host);
}

static void
_log_printf(const char *log_domain,
            G_GNUC_UNUSED GLogLevelFlags level,
            const gchar *message,
            G_GNUC_UNUSED gpointer user_data)
{
    g_printerr("%s: %s\n", log_domain, message);
}

int main(int argc, char **argv)
{
    DmapMdnsBrowser *browser;
    GError *error = NULL;

    g_log_set_handler ("libdmapsharing",
                        G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,
                        _log_printf,
                        NULL);

    g_log_set_handler ("dmapd",
                        G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,
                        _log_printf,
                        NULL);

    if (argc == 2)
        conn_type = atoi (argv[1]);

    loop = g_main_loop_new (NULL, FALSE);

    if (conn_type == DAAP)
        browser = dmap_mdns_browser_new (DMAP_MDNS_SERVICE_TYPE_DAAP);
    else
        browser = dmap_mdns_browser_new (DMAP_MDNS_SERVICE_TYPE_DPAP);
    g_signal_connect (G_OBJECT (browser),
                      "service-added",
                      G_CALLBACK (service_added_cb),
                      NULL);
    g_debug ("starting mdns browser");
    dmap_mdns_browser_start (browser, &error);
    if (error) {
        g_warning ("error starting browser. code: %d message: %s",
                error->code,
                error->message);
        return 1;
    }

    g_main_loop_run (loop);

    return 0;
}
