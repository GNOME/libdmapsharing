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

#include "test-dmap-db.h"
#include "test-daap-record-factory.h"
#include "test-dpap-record-factory.h"

enum {
    DAAP,
    DPAP
};

static GMainLoop *loop;
static guint conn_type = DAAP;

static void
print_record (gpointer id, DMAPRecord *record, gpointer user_data)
{
	gboolean has_video;
	gchar   *artist, *title;

	g_object_get (record,
	             "has-video", &has_video,
	             "songartist", &artist,
	             "title",  &title,
	              NULL);

	g_print ("%d: %s %s (has video: %s)\n", GPOINTER_TO_UINT (id), artist, title, has_video ? "Y" : "N");

	g_free (artist);
	g_free (title);
}

static void
connected_cb (DMAPConnection *connection,
			 gboolean        result,
			 const char     *reason,
			 DMAPDb         *db)
{
	g_print ("Connection cb., DB has %lu entries\n", dmap_db_count (db));

	dmap_db_foreach (db, (GHFunc) print_record, NULL);
}

static void
authenticate_cb (DMAPConnection *connection,
		 const char *name,
		 SoupSession *session,
		 SoupMessage *msg,
		 SoupAuth *auth,
		 gboolean retrying,
		 gpointer user_data)
{
	char *username, password[BUFSIZ + 1];
	g_object_get (connection, "username", &username, NULL);
	g_print ("Password required (username is %s): ", username);
	fgets (password, BUFSIZ, stdin);
	password[strlen(password) - 1] = 0x00; // Remove newline.
	g_object_set (connection, "password", password, NULL);
	soup_auth_authenticate (auth, username, password);
	soup_session_unpause_message (session, msg);
}

static void
service_added_cb (DMAPMdnsBrowser *browser,
                  DMAPMdnsService *service,
                  gpointer user_data)
{
    gchar *service_name, *name, *host;
    guint port;
    DMAPRecordFactory *factory;
    DMAPConnection *conn;
    DMAPDb *db;

    g_object_get (service, 
                 "service-name", &service_name,
                 "name", &name,
                 "host", &host,
                 "port", &port,
                  NULL);

    g_debug ("service added %s:%s:%s:%d", service_name, name, host, port);

    db = DMAP_DB (test_dmap_db_new ());
    if (db == NULL) {
    	g_error ("Error creating DB");
    }

    if (conn_type == DAAP) {
        factory = DMAP_RECORD_FACTORY (test_daap_record_factory_new ());
        if (factory == NULL) {
   	    g_error ("Error creating record factory");
        }
        conn = DMAP_CONNECTION (daap_connection_new (name, host, port, db, factory));
    } else {
        factory = DMAP_RECORD_FACTORY (test_dpap_record_factory_new ());
        if (factory == NULL) {
   	    g_error ("Error creating record factory");
        }
        conn = DMAP_CONNECTION (dpap_connection_new (name, host, port, db, factory));
    }
    g_signal_connect (DMAP_CONNECTION (conn), "authenticate", G_CALLBACK(authenticate_cb), NULL);
    dmap_connection_start (DMAP_CONNECTION (conn), (DMAPConnectionCallback) connected_cb, db);
}

int main(int argc, char **argv)
{
    DMAPMdnsBrowser *browser;
    GError *error = NULL;

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
