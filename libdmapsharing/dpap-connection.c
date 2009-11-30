/*
 * Copyright (C) 2004,2005 Charles Schmidt <cschmidt2@emich.edu>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <string.h>
#include <libsoup/soup.h>

#include "dpap-connection.h"
#include "dmap-md5.h"
#include "dmap-structure.h"

#define DPAP_USER_AGENT "iPhoto/4.01 (Macintosh; PPC)"

static SoupMessage *
dpap_connection_build_message (DMAPConnection *connection,
	       const char       *path,
	       gboolean          need_hash,
	       gdouble           version,
	       gint              req_id,
	       gboolean          send_close);

G_DEFINE_TYPE (DPAPConnection, dpap_connection, TYPE_DMAP_CONNECTION)

static void
dpap_connection_class_init (DPAPConnectionClass *klass)
{
    DMAPConnectionClass *dmap_connection_class = (DMAPConnectionClass *) klass;

    dmap_connection_class->build_message = dpap_connection_build_message;
}

static void
dpap_connection_init (DPAPConnection *self)
{
}

DPAPConnection *
dpap_connection_new (const gchar *name,
                     const gchar *host,
                     gint         port,
                     gboolean     password_protected,
		     DMAPDb      *db,
		     DMAPRecordFactory *factory)
{
    return DPAP_CONNECTION (g_object_new (TYPE_DPAP_CONNECTION,
            "name", name,
            "host", host,
            "port", port,
	    "password-protected", password_protected,
            "db",   db,
	    "factory", factory,
            "need-revision-number", FALSE,
            NULL));
}

static SoupMessage *
dpap_connection_build_message (DMAPConnection *connection,
	       const char       *path,
	       gboolean          need_hash,
	       gdouble           version,
	       gint              req_id,
	       gboolean          send_close)
{
	SoupMessage *message = NULL;
	SoupURI *base_uri = NULL;
	SoupURI *uri = NULL;

	g_object_get (connection, "base-uri", &base_uri, NULL);
	if (base_uri == NULL) {
		return NULL;
	}

	uri = soup_uri_new_with_base (base_uri, path);
	if (uri == NULL) {
		soup_uri_free (base_uri);
		return NULL;
	}

	message = soup_message_new_from_uri (SOUP_METHOD_GET, uri);

	soup_message_headers_append (message->request_headers, "Client-DAAP-Version", 		"3.0");
	soup_message_headers_append (message->request_headers, "Accept-Language", 		"en-us, en;q=5.0");
#ifdef HAVE_LIBZ
	soup_message_headers_append (message->request_headers, "Accept-Encoding",		"gzip");
#endif
	soup_message_headers_append (message->request_headers, "Client-DAAP-Access-Index", 	"2");

	/* FIXME:
	if (priv->password_protected) {
		char *h;
		char *user_pass;
		char *token;

		user_pass = g_strdup_printf ("%s:%s", priv->username, priv->password);
		token = g_base64_encode ((guchar *)user_pass, strlen (user_pass));
		h = g_strdup_printf ("Basic %s", token);

		g_free (token);
		g_free (user_pass);

		soup_message_headers_append (message->request_headers, "Authorization", h);
		g_free (h);
	}
	*/

	if (need_hash) {
		gchar hash[33] = {0};
		gchar *no_daap_path = (gchar *)path;

		if (g_ascii_strncasecmp (path, "daap://", 7) == 0) {
			no_daap_path = strstr (path, "/data");
		}

		dmap_hash_generate ((short)floor (version), (const guchar*)no_daap_path, 2, (guchar*)hash, req_id);

		soup_message_headers_append (message->request_headers, "Client-DAAP-Validation", hash);
	}
	if (send_close) {
		soup_message_headers_append (message->request_headers, "Connection", "close");
	}

	soup_uri_free (base_uri);
	soup_uri_free (uri);

	return message;
}
