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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <math.h>
#ifdef HAVE_LIBZ
#include <zlib.h>
#endif

#include <libsoup/soup.h>

#include "dmap-md5.h"
#include "dmap-connection.h"
#include "dmap-structure.h"
#include "dmap-record-factory.h"
#include "dmap-marshal.h"

#define DMAP_USER_AGENT "iTunes/4.6 (Windows; N)"

#define ITUNES_7_SERVER "iTunes/7"

static void      dmap_connection_dispose      (GObject *obj);
static void      dmap_connection_set_property (GObject *object,
						  guint prop_id,
						  const GValue *value,
						  GParamSpec *pspec);
static void      dmap_connection_get_property (GObject *object,
						  guint prop_id,
						  GValue *value,
						  GParamSpec *pspec);

static gboolean dmap_connection_do_something  (DMAPConnection *connection);
static void     dmap_connection_state_done    (DMAPConnection *connection,
						  gboolean           result);

static gboolean emit_progress_idle (DMAPConnection *connection);

G_DEFINE_TYPE (DMAPConnection, dmap_connection, G_TYPE_OBJECT)

#define DMAP_CONNECTION_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_DMAP_CONNECTION, DMAPConnectionPrivate))

struct DMAPConnectionPrivate {
	char *name;
	gboolean password_protected;
	char *username;
	char *password;
	char *host;
	guint port;

	gboolean is_connected;
	gboolean is_connecting;

	SoupSession *session;
	SoupURI *base_uri;
	gchar *daap_base_uri;

	gdouble dmap_version;
	guint32 session_id;
	gint revision_number;

	gint request_id;
	gint database_id;

	guint reading_playlist;
	GSList *playlists;
	GHashTable *item_id_to_uri;

	DMAPDb *db;
	DMAPRecordFactory *record_factory;

	DMAPConnectionState state;
	DMAPResponseHandler response_handler;
	gboolean use_response_handler_thread;
	float progress;

	guint emit_progress_id;
	guint do_something_id;

	gboolean result;
	char *last_error_message;
};

enum {
	PROP_0,
	PROP_DB,
	PROP_FACTORY,
	PROP_NAME,
	PROP_ENTRY_TYPE,
	PROP_PASSWORD_PROTECTED,
	PROP_HOST,
	PROP_PORT,
	PROP_BASE_URI,
	PROP_DATABASE_ID,
	PROP_SESSION_ID,
	PROP_DMAP_VERSION,
	PROP_REVISION_NUMBER,
};

enum {
	AUTHENTICATE,
	CONNECTING,
	CONNECTED,
	DISCONNECTED,
	OPERATION_DONE,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = { 0, };

static void
dmap_connection_finalize (GObject *object)
{
	DMAPConnection *connection;

	g_return_if_fail (object != NULL);
	g_return_if_fail (IS_DMAP_CONNECTION (object));

	connection = DMAP_CONNECTION (object);

	g_return_if_fail (connection->priv != NULL);

	g_debug ("Finalize");

	G_OBJECT_CLASS (dmap_connection_parent_class)->finalize (object);
}

static void
dmap_connection_class_init (DMAPConnectionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize	   = dmap_connection_finalize;
	object_class->dispose       = dmap_connection_dispose;
	object_class->set_property = dmap_connection_set_property;
	object_class->get_property = dmap_connection_get_property;

	g_type_class_add_private (klass, sizeof (DMAPConnectionPrivate));

	g_object_class_install_property (object_class,
					 PROP_DB,
					 g_param_spec_pointer ("db",
							       "DMAPDb",
							       "DMAPDb object",
							       G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
					 PROP_FACTORY,
					 g_param_spec_pointer ("factory",
							       "record factory",
							       "record factory",
							       G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
					 PROP_PASSWORD_PROTECTED,
					 g_param_spec_boolean ("password-protected",
							       "password protected",
							       "connection is password protected",
							       FALSE,
							       G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (object_class,
					 PROP_NAME,
					 g_param_spec_string ("name",
						 	      "connection name",
							      "connection name",
							      NULL,
							       G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (object_class,
					 PROP_HOST,
					 g_param_spec_string ("host",
						 	      "host",
							      "host",
							      NULL,
							       G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (object_class,
					 PROP_PORT,
					 g_param_spec_uint ("port",
							    "port",
							    "port",
							    0, G_MAXINT, 0,
							    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));


	g_object_class_install_property (object_class,
					 PROP_BASE_URI,
					 g_param_spec_pointer ("base-uri",
							       "base URI",
							       "base URI",
							       G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
					 PROP_DATABASE_ID,
					 g_param_spec_int ("database-id",
							   "database ID",
							   "database ID",
							   0, G_MAXINT, 0,
							   G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
					 PROP_SESSION_ID,
					 g_param_spec_int ("session-id",
							   "session ID",
							   "session ID",
							   0, G_MAXINT, 0,
							   G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
					 PROP_DMAP_VERSION,
					 g_param_spec_double ("dmap-version",
							   "DMAP version",
							   "DMAP version",
							   0, G_MAXDOUBLE, 0,
							   G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
					 PROP_REVISION_NUMBER,
					 g_param_spec_int ("revision-number",
							   "revision number",
							   "revision number",
							   0, G_MAXINT, 0,
							   G_PARAM_READWRITE));

	signals [AUTHENTICATE] = g_signal_new ("authenticate",
					       G_TYPE_FROM_CLASS (object_class),
					       G_SIGNAL_RUN_LAST,
					       G_STRUCT_OFFSET (DMAPConnectionClass, authenticate),
					       NULL,
					       NULL,
					       dmap_marshal_STRING__STRING,
					       G_TYPE_STRING,
					       1, G_TYPE_STRING);
	signals [CONNECTING] = g_signal_new ("connecting",
					     G_TYPE_FROM_CLASS (object_class),
					     G_SIGNAL_RUN_LAST,
					     G_STRUCT_OFFSET (DMAPConnectionClass, connecting),
					     NULL,
					     NULL,
					     dmap_marshal_VOID__ULONG_FLOAT,
					     G_TYPE_NONE,
					     2, G_TYPE_ULONG, G_TYPE_FLOAT);
	signals [CONNECTED] = g_signal_new ("connected",
					    G_TYPE_FROM_CLASS (object_class),
					    G_SIGNAL_RUN_LAST,
					    G_STRUCT_OFFSET (DMAPConnectionClass, connected),
					    NULL,
					    NULL,
					    g_cclosure_marshal_VOID__VOID,
					       G_TYPE_NONE,
					    0);
	signals [DISCONNECTED] = g_signal_new ("disconnected",
					       G_TYPE_FROM_CLASS (object_class),
					       G_SIGNAL_RUN_LAST,
					       G_STRUCT_OFFSET (DMAPConnectionClass, disconnected),
					       NULL,
					       NULL,
					       g_cclosure_marshal_VOID__VOID,
					       G_TYPE_NONE,
					       0);
	signals [OPERATION_DONE] = g_signal_new ("operation-done",
						 G_TYPE_FROM_CLASS (object_class),
						 G_SIGNAL_RUN_FIRST,
						 G_STRUCT_OFFSET (DMAPConnectionClass, operation_done),
						 NULL,
						 NULL,
						 g_cclosure_marshal_VOID__VOID,
						 G_TYPE_NONE,
						 0);
}

static void
dmap_connection_init (DMAPConnection *connection)
{
	connection->priv = DMAP_CONNECTION_GET_PRIVATE (connection);

	connection->priv->username = g_strdup_printf ("libdmapsharing_%s", VERSION);
}

static char *
connection_get_password (DMAPConnection *connection)
{
	char *password = NULL;;

	// FIXME: GDK_THREADS_ENTER ();
	g_signal_emit (connection,
		       signals [AUTHENTICATE],
		       0,
		       connection->priv->name,
		       &password);
	// FIXME: GDK_THREADS_LEAVE ();

	return password;
}

static void
connection_connected (DMAPConnection *connection)
{
	g_debug ("Emitting connected");

	connection->priv->is_connected = TRUE;

	// FIXME: GDK_THREADS_ENTER ();
	g_signal_emit (connection,
		       signals [CONNECTED],
		       0);
	// FIXME: GDK_THREADS_LEAVE ();
}

static void
connection_disconnected (DMAPConnection *connection)
{
	g_debug ("Emitting disconnected");

	connection->priv->is_connected = FALSE;

	// FIXME: GDK_THREADS_ENTER ();
	g_signal_emit (connection,
		       signals [DISCONNECTED],
		       0);
	// FIXME: GDK_THREADS_LEAVE ();
}

static void
connection_operation_done (DMAPConnection *connection)
{
	g_debug ("Emitting operation done");

	// FIXME: GDK_THREADS_ENTER ();
	g_signal_emit (connection,
		       signals [OPERATION_DONE],
		       0);
	// FIXME: GDK_THREADS_LEAVE ();
}

SoupMessage *
dmap_connection_build_message (DMAPConnection *connection,
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
		return NULL;
	}

	message = soup_message_new_from_uri (SOUP_METHOD_GET, uri);

	soup_message_headers_append (message->request_headers, "User-Agent", DMAP_USER_AGENT);
	soup_message_headers_append (message->request_headers, "Client-DAAP-Version", 		"3.0");
	soup_message_headers_append (message->request_headers, "Accept-Language", 		"en-us, en;q=5.0");
#ifdef HAVE_LIBZ
	soup_message_headers_append (message->request_headers, "Accept-Encoding",		"gzip");
#endif
	soup_message_headers_append (message->request_headers, "Client-DAAP-Access-Index", 	"2");

	if (connection->priv->password_protected == TRUE
	    && (connection->priv->username == NULL
	        || connection->priv->password == NULL)) {
		g_debug ("No username or no password provided");
	} else {

		char *h;
		char *user_pass;
		char *token;

		user_pass = g_strdup_printf ("%s:%s", connection->priv->username, connection->priv->password);
		token = g_base64_encode ((guchar *)user_pass, strlen (user_pass));
		h = g_strdup_printf ("Basic %s", token);

		g_free (token);
		g_free (user_pass);

		soup_message_headers_append (message->request_headers, "Authorization", h);
		g_free (h);
	}

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

	soup_uri_free (uri);

	return message;
}

#ifdef HAVE_LIBZ
static void
*g_zalloc_wrapper (voidpf opaque, uInt items, uInt size)
{
	if ((items != 0) && (size >= G_MAXUINT/items)) {
		return Z_NULL;
	}
	if ((size != 0) && (items >= G_MAXUINT/size)) {
		return Z_NULL;
	}
	return g_malloc0 (items * size);
}

static void
g_zfree_wrapper (voidpf opaque, voidpf address)
{
	g_free (address);
}
#endif

static void
connection_set_error_message (DMAPConnection *connection,
			      const char       *message)
{
	/* FIXME: obtain a lock */
	g_free (connection->priv->last_error_message);
	if (message != NULL) {
		connection->priv->last_error_message = g_strdup (message);
	} else {
		connection->priv->last_error_message = NULL;
	}
}

typedef struct {
	SoupMessage *message;
	int status;
	DMAPConnection *connection;
} DAAPResponseData;

static void
actual_http_response_handler (DAAPResponseData *data)
{
	DMAPConnectionPrivate *priv;
	GNode *structure;
	char *new_response = NULL;
	const char *response;
	const char *encoding_header;
	char *message_path;
	int response_length;
	gboolean compatible_server = TRUE;

	priv = data->connection->priv;
	structure = NULL;
	encoding_header = NULL;
	response = data->message->response_body->data;
	response_length = data->message->response_body->length;

	message_path = soup_uri_to_string (soup_message_get_uri (data->message), FALSE);

	g_debug ("Received response from %s: %d, %s\n",
		  message_path,
		  data->message->status_code,
		  data->message->reason_phrase);

	if (data->message->response_headers) {
		const char *server;

		encoding_header = soup_message_headers_get (data->message->response_headers, "Content-Encoding");

		server = soup_message_headers_get (data->message->response_headers, "DAAP-Server");
		if (server != NULL && strstr (server, ITUNES_7_SERVER) != NULL) {
			g_debug ("giving up.  we can't talk to %s", server);
			compatible_server = FALSE;
		}
	}

	if (SOUP_STATUS_IS_SUCCESSFUL (data->status) && encoding_header && strcmp (encoding_header, "gzip") == 0) {
#ifdef HAVE_LIBZ
		z_stream stream;
		unsigned int factor = 4;
		unsigned int unc_size = response_length * factor;

		stream.next_in = (unsigned char *)response;
		stream.avail_in = response_length;
		stream.total_in = 0;

		new_response = g_malloc (unc_size + 1);
		stream.next_out = (unsigned char *)new_response;
		stream.avail_out = unc_size;
		stream.total_out = 0;
		stream.zalloc = g_zalloc_wrapper;
		stream.zfree = g_zfree_wrapper;
		stream.opaque = NULL;

		if (inflateInit2 (&stream, 32 /* auto-detect */ + 15 /* max */ ) != Z_OK) {
			inflateEnd (&stream);
			g_free (new_response);
			g_debug ("Unable to decompress response from %s",
				  message_path);
			data->status = SOUP_STATUS_MALFORMED;
		} else {
			do {
				int z_res;

				z_res = inflate (&stream, Z_FINISH);
				if (z_res == Z_STREAM_END) {
					break;
				}
				if ((z_res != Z_OK && z_res != Z_BUF_ERROR) || stream.avail_out != 0 || unc_size > 40*1000*1000) {
					inflateEnd (&stream);
					g_free (new_response);
					new_response = NULL;
					break;
				}

				factor *= 4;
				unc_size = (response_length * factor);
				/* unc_size can't grow bigger than 40MB, so
				 * unc_size can't overflow, and this realloc
				 * call is safe
				 */
				new_response = g_realloc (new_response, unc_size + 1);
				stream.next_out = (unsigned char *)(new_response + stream.total_out);
				stream.avail_out = unc_size - stream.total_out;
			} while (1);
		}

		if (new_response) {
			response = new_response;
			response_length = stream.total_out;
		}
#else
		g_debug ("Received compressed response from %s but can't handle it",
			  message_path);
		data->status = SOUP_STATUS_MALFORMED;
#endif
	}

	if (compatible_server == FALSE) {
		/* leaving structure == NULL here causes the connection process
		 * to fail at the first step.
		 */
		connection_set_error_message (data->connection,
					      ("libdmapsharing is not able to connect to iTunes 7 shares"));
	} else if (SOUP_STATUS_IS_SUCCESSFUL (data->status)) {
		DMAPStructureItem *item;

		if (/* FIXME: !rb_is_main_thread () */ TRUE) {
			priv->progress = -1.0f;
			if (priv->emit_progress_id != 0) {
				g_source_remove (priv->emit_progress_id);
			}
			priv->emit_progress_id = g_idle_add ((GSourceFunc) emit_progress_idle, data->connection);
		}
		structure = dmap_structure_parse (response, response_length);
		if (structure == NULL) {
			g_debug ("No daap structure returned from %s",
				  message_path);

			data->status = SOUP_STATUS_MALFORMED;
		} else {
			int dmap_status = 0;
			item = dmap_structure_find_item (structure, DMAP_CC_MSTT);
			if (item)
				dmap_status = g_value_get_int (&(item->content));

			if (dmap_status != 200) {
				g_debug ("Error, dmap.status is not 200 in response from %s",
					  message_path);

				data->status = SOUP_STATUS_MALFORMED;
			}
		}
		if (/* FIXME: ! rb_is_main_thread () */ TRUE) {
			priv->progress = 1.0f;
			if (priv->emit_progress_id != 0) {
				g_source_remove (priv->emit_progress_id);
			}
			priv->emit_progress_id = g_idle_add ((GSourceFunc) emit_progress_idle, data->connection);
		}
	} else {
		g_debug ("Error getting %s: %d, %s\n",
			  message_path,
			  data->message->status_code,
			  data->message->reason_phrase);
		connection_set_error_message (data->connection, data->message->reason_phrase);
	}

	if (priv->response_handler) {
		DMAPResponseHandler h = priv->response_handler;
		priv->response_handler = NULL;
		(*h) (data->connection, data->status, structure);
	}

	if (structure) {
		dmap_structure_destroy (structure);
	}

	g_free (new_response);
	g_free (message_path);
	g_object_unref (G_OBJECT (data->connection));
	g_object_unref (G_OBJECT (data->message));
	g_free (data);
}

static void
http_response_handler (SoupSession      *session,
		       SoupMessage      *message,
		       DMAPConnection *connection)
{
	DAAPResponseData *data;
	int response_length;

	if (message->status_code == SOUP_STATUS_CANCELLED) {
		g_debug ("Message cancelled");
		return;
	}

	data = g_new0 (DAAPResponseData, 1);
	data->status = message->status_code;
	response_length = message->response_body->length;

	g_object_ref (G_OBJECT (connection));
	data->connection = connection;

	g_object_ref (G_OBJECT (message));
	data->message = message;

	if (response_length >= G_MAXUINT/4 - 1) {
		/* If response_length is too big,
		 * the g_malloc (unc_size + 1) below would overflow
		 */
		data->status = SOUP_STATUS_MALFORMED;
	}

	/* to avoid blocking the UI, handle big responses in a separate thread */
	if (SOUP_STATUS_IS_SUCCESSFUL (data->status) && connection->priv->use_response_handler_thread) {
		GError *error = NULL;
		g_debug ("creating thread to handle daap response");
		g_thread_create ((GThreadFunc) actual_http_response_handler,
				 data,
				 FALSE,
				 &error);
		if (error) {
			g_warning ("fuck");
		}
	} else {
		actual_http_response_handler (data);
	}
}

static gboolean
http_get (DMAPConnection     *connection,
	  const char           *path,
	  gboolean              need_hash,
	  gdouble               version,
	  gint                  req_id,
	  gboolean              send_close,
	  DMAPResponseHandler handler,
	  gboolean              use_thread)
{
	DMAPConnectionPrivate *priv = connection->priv;
	SoupMessage *message;

	message = dmap_connection_build_message (connection, path, need_hash, version, req_id, send_close);
	if (message == NULL) {
		g_debug ("Error building message for http://%s:%d/%s",
			  priv->base_uri->host,
			  priv->base_uri->port,
			  path);
		return FALSE;
	}

	priv->use_response_handler_thread = use_thread;
	priv->response_handler = handler;
	soup_session_queue_message (priv->session, message,
				    (SoupSessionCallback) http_response_handler,
				    connection);
	g_debug ("Queued message for http://%s:%d/%s",
		  priv->base_uri->host,
		  priv->base_uri->port,
		  path);
	return TRUE;
}

gboolean
dmap_connection_get (DMAPConnection *self,
                     const gchar *path,
                     gboolean need_hash,
                     DMAPResponseHandler handler,
                     gpointer user_data)
{
    /* FIXME: used to pass user_data as 8th argument, but this is not right. */
    return http_get (self, path, need_hash,
            self->priv->dmap_version, 0, FALSE,
            (DMAPResponseHandler) handler, FALSE);
}

static gboolean
emit_progress_idle (DMAPConnection *connection)
{
	g_debug ("Emitting progress");

	// FIXME: GDK_THREADS_ENTER ();
	g_signal_emit (G_OBJECT (connection), signals[CONNECTING], 0,
		       connection->priv->state,
		       connection->priv->progress);
	connection->priv->emit_progress_id = 0;
	// FIXME: GDK_THREADS_LEAVE ();
	return FALSE;
}

static void
handle_server_info (DMAPConnection *connection,
		    guint             status,
		    GNode            *structure)
{
	DMAPConnectionPrivate *priv = connection->priv;
	DMAPStructureItem *item = NULL;

	if (!SOUP_STATUS_IS_SUCCESSFUL (status) || structure == NULL) {
		dmap_connection_state_done (connection, FALSE);
		return;
	}

	/* get the daap version number */
	item = dmap_structure_find_item (structure, DMAP_CC_APRO);
	if (item == NULL) {
		dmap_connection_state_done (connection, FALSE);
		return;
	}

	priv->dmap_version = g_value_get_double (&(item->content));
	dmap_connection_state_done (connection, TRUE);
}

static void
handle_login (DMAPConnection *connection,
	      guint             status,
	      GNode            *structure)
{
	DMAPConnectionPrivate *priv = connection->priv;
	DMAPStructureItem *item = NULL;

	if (status == SOUP_STATUS_UNAUTHORIZED || status == SOUP_STATUS_FORBIDDEN) {
		g_debug ("Incorrect password");
		priv->state = DMAP_GET_PASSWORD;
		if (priv->do_something_id != 0) {
			g_source_remove (priv->do_something_id);
		}
		priv->do_something_id = g_idle_add ((GSourceFunc) dmap_connection_do_something, connection);
		return;
	}

	if (structure == NULL || SOUP_STATUS_IS_SUCCESSFUL (status) == FALSE) {
		dmap_connection_state_done (connection, FALSE);
		return;
	}

	item = dmap_structure_find_item (structure, DMAP_CC_MLID);
	if (item == NULL) {
		g_debug ("Could not find daap.sessionid item in /login");
		dmap_connection_state_done (connection, FALSE);
		return;
	}

	priv->session_id = (guint32) g_value_get_int (&(item->content));

	connection_connected (connection);

	dmap_connection_state_done (connection, TRUE);
}

static void
handle_update (DMAPConnection *connection,
	       guint             status,
	       GNode            *structure)
{
	DMAPConnectionPrivate *priv = connection->priv;
	DMAPStructureItem *item;

	if (structure == NULL || SOUP_STATUS_IS_SUCCESSFUL (status) == FALSE) {
		dmap_connection_state_done (connection, FALSE);
		return;
	}

	/* get a revision number */
	item = dmap_structure_find_item (structure, DMAP_CC_MUSR);
	if (item == NULL) {
		g_debug ("Could not find daap.serverrevision item in /update");
		dmap_connection_state_done (connection, FALSE);
		return;
	}

	priv->revision_number = g_value_get_int (&(item->content));
	dmap_connection_state_done (connection, TRUE);
}

static void
handle_database_info (DMAPConnection *connection,
		      guint             status,
		      GNode            *structure)
{
	DMAPConnectionPrivate *priv = connection->priv;
	DMAPStructureItem *item = NULL;
	GNode *listing_node;
	gint n_databases = 0;

	/* get a list of databases, there should be only 1 */

	if (structure == NULL || SOUP_STATUS_IS_SUCCESSFUL (status) == FALSE) {
		dmap_connection_state_done (connection, FALSE);
		return;
	}

	item = dmap_structure_find_item (structure, DMAP_CC_MRCO);
	if (item == NULL) {
		g_debug ("Could not find dmap.returnedcount item in /databases");
		dmap_connection_state_done (connection, FALSE);
		return;
	}

	n_databases = g_value_get_int (&(item->content));
	if (n_databases != 1) {
		g_debug ("Host seems to have more than 1 database, how strange\n");
	}

	listing_node = dmap_structure_find_node (structure, DMAP_CC_MLCL);
	if (listing_node == NULL) {
		g_debug ("Could not find dmap.listing item in /databases");
		dmap_connection_state_done (connection, FALSE);
		return;
	}

	item = dmap_structure_find_item (listing_node->children, DMAP_CC_MIID);
	if (item == NULL) {
		g_debug ("Could not find dmap.itemid item in /databases");
		dmap_connection_state_done (connection, FALSE);
		return;
	}

	priv->database_id = g_value_get_int (&(item->content));
	dmap_connection_state_done (connection, TRUE);
}

static void
handle_song_listing (DMAPConnection *connection,
		     guint             status,
		     GNode            *structure)
{
	DMAPConnectionPrivate *priv = connection->priv;
	DMAPStructureItem *item = NULL;
	GNode *listing_node;
	gint returned_count;
	gint i;
	GNode *n;
	gint specified_total_count;
	gboolean update_type;
	gint commit_batch;

	/* get the songs */

	if (structure == NULL || SOUP_STATUS_IS_SUCCESSFUL (status) == FALSE) {
		dmap_connection_state_done (connection, FALSE);
		return;
	}

	item = dmap_structure_find_item (structure, DMAP_CC_MRCO);
	if (item == NULL) {
		g_debug ("Could not find dmap.returnedcount item in /databases/%d/items",
			  priv->database_id);
		dmap_connection_state_done (connection, FALSE);
		return;
	}
	returned_count = g_value_get_int (&(item->content));
	if (returned_count > 20) {
		commit_batch = returned_count / 20;
	} else {
		commit_batch = 1;
	}

	item = dmap_structure_find_item (structure, DMAP_CC_MTCO);
	if (item == NULL) {
		g_debug ("Could not find dmap.specifiedtotalcount item in /databases/%d/items",
			  priv->database_id);
		dmap_connection_state_done (connection, FALSE);
		return;
	}
	specified_total_count = g_value_get_int (&(item->content));

	item = dmap_structure_find_item (structure, DMAP_CC_MUTY);
	if (item == NULL) {
		g_debug ("Could not find dmap.updatetype item in /databases/%d/items",
			  priv->database_id);
		dmap_connection_state_done (connection, FALSE);
		return;
	}
	update_type = g_value_get_char (&(item->content));

	listing_node = dmap_structure_find_node (structure, DMAP_CC_MLCL);
	if (listing_node == NULL) {
		g_debug ("Could not find dmap.listing item in /databases/%d/items",
			  priv->database_id);
		dmap_connection_state_done (connection, FALSE);
		return;
	}

	/* FIXME: refstring: */
	priv->item_id_to_uri = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, (GDestroyNotify) g_free);

	priv->progress = 0.0f;
	if (priv->emit_progress_id != 0) {
		g_source_remove (priv->emit_progress_id);
	}
	connection->priv->emit_progress_id = g_idle_add ((GSourceFunc) emit_progress_idle, connection);

	for (i = 0, n = listing_node->children; n; i++, n = n->next) {
		GNode *n2;
		DMAPRecord *record = NULL;
		gchar *uri = NULL;
		gint item_id = 0;
		const gchar *title = NULL;
		const gchar *album = NULL;
		const gchar *artist = NULL;
		const gchar *format = NULL;
		const gchar *genre = NULL;
		const gchar *streamURI = NULL;
		gint length = 0;
		gint track_number = 0;
		gint disc_number = 0;
		gint year = 0;
		gint size = 0;
		gint bitrate = 0;

		for (n2 = n->children; n2; n2 = n2->next) {
			DMAPStructureItem *meta_item;

			meta_item = n2->data;

			switch (meta_item->content_code) {
				case DMAP_CC_MIID:
					item_id = g_value_get_int (&(meta_item->content));
					break;
				case DMAP_CC_MINM:
					title = g_value_get_string (&(meta_item->content));
					break;
				case DMAP_CC_ASAL:
					album = g_value_get_string (&(meta_item->content));
					break;
				case DMAP_CC_ASAR:
					artist = g_value_get_string (&(meta_item->content));
					break;
				case DMAP_CC_ASFM:
					format = g_value_get_string (&(meta_item->content));
					break;
				case DMAP_CC_ASGN:
					genre = g_value_get_string (&(meta_item->content));
					break;
				case DMAP_CC_ASTM:
					length = g_value_get_int (&(meta_item->content));
					break;
				case DMAP_CC_ASTN:
					track_number = g_value_get_int (&(meta_item->content));
					break;
				case DMAP_CC_ASDN:
					disc_number = g_value_get_int (&(meta_item->content));
					break;
				case DMAP_CC_ASYR:
					year = g_value_get_int (&(meta_item->content));
					break;
				case DMAP_CC_ASSZ:
					size = g_value_get_int (&(meta_item->content));
					break;
				case DMAP_CC_ASBR:
					bitrate = g_value_get_int (&(meta_item->content));
					break;
				case DMAP_CC_ASUL:
					streamURI = g_value_get_string (&(meta_item->content));
					break;
				default:
					break;
			}
		}

		/*if (connection->dmap_version == 3.0) {*/
			uri = g_strdup_printf ("%s/databases/%d/items/%d.%s?session-id=%u",
					       priv->daap_base_uri,
					       priv->database_id,
					       item_id, format,
					       priv->session_id);
		/*} else {*/
		/* uri should be
		 * "/databases/%d/items/%d.%s?session-id=%u&revision-id=%d";
		 * but its not going to work cause the other parts of the code
		 * depend on the uri to have the ip address so that the
		 * DAAPSource can be found to ++request_id
		 * maybe just /dont/ support older itunes.  doesn't seem
		 * unreasonable to me, honestly
		 */
		/*}*/
		record = dmap_record_factory_create (priv->record_factory, NULL);
		if (record == NULL) {
			g_debug ("cannot create record for daap track %s", uri);
			continue;
		}
		/* FIXME: This is DAAP-specific! */
		g_object_set (record,
			     "location", uri,
			     "year", year,
			     "track", track_number,
			     "disc", disc_number,
			     "bitrate", bitrate,
			     "duration", length / 1000,
			     "filesize", (guint64) size,
			     "format", format,
			     "title", title,
			     "album", album,
			     "artist", artist,
			     "genre", genre,
			      NULL);
		g_hash_table_insert (priv->item_id_to_uri, GINT_TO_POINTER (item_id), g_strdup (uri));
		g_free (uri);

		dmap_db_add (priv->db, record);
		g_debug ("Got song: %s", title);
		g_object_unref (record);

		if (i % commit_batch == 0) {
			connection->priv->progress = ((float) i / (float) returned_count);
			if (priv->emit_progress_id != 0) {
				g_source_remove (connection->priv->emit_progress_id);
			}
			connection->priv->emit_progress_id = g_idle_add ((GSourceFunc) emit_progress_idle, connection);
		}
	}

	dmap_connection_state_done (connection, TRUE);
}

static int
compare_playlists_by_name(gconstpointer a, gconstpointer b)
{
	const DMAPPlaylist *playlist1 = a;
	const DMAPPlaylist *playlist2 = b;

	return strcmp(playlist1->name, playlist2->name);
}

/* FIXME
 * what we really should do is only get a list of playlists and their ids
 * then when they are clicked on ('activate'd) by the user, get a list of
 * the files that are actually in them.  This will speed up initial daap
 * connection times and reduce memory consumption.
 */

static void
handle_playlists (DMAPConnection *connection,
		  guint             status,
		  GNode            *structure)
{
	DMAPConnectionPrivate *priv = connection->priv;
	GNode *listing_node;
	gint i;
	GNode *n;

	if (structure == NULL || SOUP_STATUS_IS_SUCCESSFUL (status) == FALSE) {
		dmap_connection_state_done (connection, FALSE);
		return;
	}

	listing_node = dmap_structure_find_node (structure, DMAP_CC_MLCL);
	if (listing_node == NULL) {
		g_debug ("Could not find dmap.listing item in /databases/%d/containers",
			  priv->database_id);
		dmap_connection_state_done (connection, FALSE);
		return;
	}

	for (i = 0, n = listing_node->children; n; n = n->next, i++) {
		DMAPStructureItem *item;
		gint id;
		gchar *name;
		DMAPPlaylist *playlist;

		item = dmap_structure_find_item (n, DMAP_CC_ABPL);
		if (item != NULL) {
			continue;
		}

		item = dmap_structure_find_item (n, DMAP_CC_MIID);
		if (item == NULL) {
			g_debug ("Could not find dmap.itemid item in /databases/%d/containers",
				  priv->database_id);
			continue;
		}
		id = g_value_get_int (&(item->content));

		item = dmap_structure_find_item (n, DMAP_CC_MINM);
		if (item == NULL) {
			g_debug ("Could not find dmap.itemname item in /databases/%d/containers",
				  priv->database_id);
			continue;
		}
		name = g_value_dup_string (&(item->content));

		playlist = g_new0 (DMAPPlaylist, 1);
		playlist->id = id;
		playlist->name = name;
		g_debug ("Got playlist %p: name %s, id %d", playlist, playlist->name, playlist->id);

		priv->playlists = g_slist_prepend (priv->playlists, playlist);
	}

	/* Sort the playlists into lexical order. Established DAAP clients already
	 * do this leading to an absence of sorting functionality in DAAP servers. */
	priv->playlists = g_slist_sort (priv->playlists, compare_playlists_by_name);

	dmap_connection_state_done (connection, TRUE);
}

static void
handle_playlist_entries (DMAPConnection *connection,
			 guint             status,
			 GNode            *structure)
{
	DMAPConnectionPrivate *priv = connection->priv;
	DMAPPlaylist *playlist;
	GNode *listing_node;
	GNode *node;
	gint i;
	GList *playlist_uris = NULL;

	if (structure == NULL || SOUP_STATUS_IS_SUCCESSFUL (status) == FALSE) {
		dmap_connection_state_done (connection, FALSE);
		return;
	}

	playlist = (DMAPPlaylist *)g_slist_nth_data (priv->playlists, priv->reading_playlist);
	g_assert (playlist);

	listing_node = dmap_structure_find_node (structure, DMAP_CC_MLCL);
	if (listing_node == NULL) {
		g_debug ("Could not find dmap.listing item in /databases/%d/containers/%d/items",
			  priv->database_id, playlist->id);
		dmap_connection_state_done (connection, FALSE);
		return;
	}

	for (i = 0, node = listing_node->children; node; node = node->next, i++) {
		gchar *item_uri;
		gint playlist_item_id;
		DMAPStructureItem *item;

		item = dmap_structure_find_item (node, DMAP_CC_MIID);
		if (item == NULL) {
			g_debug ("Could not find dmap.itemid item in /databases/%d/containers/%d/items",
				  priv->database_id, playlist->id);
			continue;
		}
		playlist_item_id = g_value_get_int (&(item->content));

		item_uri = g_hash_table_lookup (priv->item_id_to_uri, GINT_TO_POINTER (playlist_item_id));
		if (item_uri == NULL) {
			g_debug ("Entry %d in playlist %s doesn't exist in the database\n",
				  playlist_item_id, playlist->name);
			continue;
		}

		/* FIXME: Rhythmbox used "refstrings," so that the same memory would exist in both
		 * this list and the hash table. See FIXME: refstring thoughout. */
		playlist_uris = g_list_prepend (playlist_uris, g_strdup (item_uri));
	}

	playlist->uris = g_list_reverse (playlist_uris);
	dmap_connection_state_done (connection, TRUE);
}

static void
handle_logout (DMAPConnection *connection,
	       guint             status,
	       GNode            *structure)
{
	connection_disconnected (connection);

	/* is there any point handling errors here? */
	dmap_connection_state_done (connection, TRUE);
}

DMAPConnection *
dmap_connection_new (const char        *name,
		     const char        *host,
		     int                port,
		     gboolean           password_protected,
		     DMAPDb            *db,
		     DMAPRecordFactory *factory)
{
	DMAPConnection *connection;
	
	connection = g_object_new (TYPE_DMAP_CONNECTION,
			          "name", name,
			          "password-protected", password_protected,
			          "db", db,
			          "host", host,
			          "port", port,
				  "factory", factory,
			           NULL);

	return connection;
}

gboolean
dmap_connection_is_connected (DMAPConnection *connection)
{
	g_return_val_if_fail (IS_DMAP_CONNECTION (connection), FALSE);

	return connection->priv->is_connected;
}

typedef struct {
	DMAPConnection        *connection;
	DMAPConnectionCallback callback;
	gpointer                 data;
	GDestroyNotify           destroy;
} ConnectionResponseData;

static void
connection_response_data_free (gpointer data)
{
	ConnectionResponseData *rdata = data;

	g_object_unref (rdata->connection);
	g_free (rdata);
}

static void
connected_cb (DMAPConnection       *connection,
	      ConnectionResponseData *rdata)
{
	gboolean result;

	g_debug ("Connected callback");

	connection->priv->is_connecting = FALSE;

	g_signal_handlers_disconnect_by_func (connection,
					      G_CALLBACK (connected_cb),
					      rdata);

	/* if connected then we succeeded */
	result = dmap_connection_is_connected (connection);

	if (rdata->callback) {
		rdata->callback (rdata->connection,
				 result,
				 rdata->connection->priv->last_error_message,
				 rdata->data);
	}

	if (rdata->destroy) {
		rdata->destroy (rdata);
	}
}

void
dmap_connection_connect (DMAPConnection        *connection,
			    DMAPConnectionCallback callback,
			    gpointer                 user_data)
{
	ConnectionResponseData *rdata;

	g_return_if_fail (IS_DMAP_CONNECTION (connection));
	g_return_if_fail (connection->priv->state == DMAP_GET_INFO);

	g_debug ("Creating new DAAP connection to %s:%d", connection->priv->host, connection->priv->port);

	connection->priv->session = soup_session_async_new ();

	connection->priv->base_uri = soup_usr_new (NULL);
	soup_uri_set_scheme (connection->priv->base_uri, SOUP_URI_SCHEME_HTTP);
	soup_uri_set_host (connection->priv->base_uri, connection->priv->host);
	soup_uri_set_port (connection->priv->base_uri, connection->priv->port);

	if (connection->priv->base_uri == NULL) {
		g_debug ("Error parsing http://%s:%d", connection->priv->host, connection->priv->port);
		/* FIXME: do callback */
		return;
	}

	connection->priv->daap_base_uri = g_strdup_printf ("daap://%s:%d", connection->priv->host, connection->priv->port);

	rdata = g_new (ConnectionResponseData, 1);
	rdata->connection = g_object_ref (connection);
	rdata->callback = callback;
	rdata->data = user_data;
	rdata->destroy = connection_response_data_free;
	g_signal_connect (connection, "operation-done", G_CALLBACK (connected_cb), rdata);

	if (connection->priv->do_something_id != 0) {
		g_source_remove (connection->priv->do_something_id);
	}

	connection->priv->is_connecting = TRUE;
	connection->priv->do_something_id = g_idle_add ((GSourceFunc) dmap_connection_do_something, connection);
}

static void
disconnected_cb (DMAPConnection       *connection,
		 ConnectionResponseData *rdata)
{
	gboolean result;

	g_debug ("Disconnected callback");

	g_signal_handlers_disconnect_by_func (connection,
					      G_CALLBACK (disconnected_cb),
					      rdata);

	/* if not connected then we succeeded */
	result = ! dmap_connection_is_connected (connection);

	if (rdata->callback) {
		rdata->callback (rdata->connection,
				 result,
				 rdata->connection->priv->last_error_message,
				 rdata->data);
	}

	if (rdata->destroy) {
		rdata->destroy (rdata);
	}
}

static void
dmap_connection_finish (DMAPConnection *connection)
{
	g_return_if_fail (IS_DMAP_CONNECTION (connection));

	g_debug ("DAAP finish");
	connection->priv->state = DMAP_DONE;
	connection->priv->progress = 1.0f;

	connection_operation_done (connection);
}

void
dmap_connection_disconnect (DMAPConnection        *connection,
			       DMAPConnectionCallback callback,
			       gpointer                 user_data)
{
	DMAPConnectionPrivate *priv = connection->priv;
	ConnectionResponseData  *rdata;

	g_return_if_fail (IS_DMAP_CONNECTION (connection));

	g_debug ("Disconnecting");

	if (connection->priv->is_connecting) {
		/* this is a special case where the async connection
		   hasn't returned yet so we need to force the connection
		   to finish */
		priv->state = DMAP_DONE;
		// FIXME: GDK_THREADS_LEAVE ();
		dmap_connection_finish (connection);
		// FIXME: GDK_THREADS_ENTER ();
	}

	rdata = g_new (ConnectionResponseData, 1);
	rdata->connection = g_object_ref (connection);
	rdata->callback = callback;
	rdata->data = user_data;
	rdata->destroy = connection_response_data_free;

	g_signal_connect (connection, "operation-done", G_CALLBACK (disconnected_cb), rdata);

	if (priv->do_something_id != 0) {
		g_source_remove (priv->do_something_id);
	}

	if (! connection->priv->is_connected) {
		priv->state = DMAP_DONE;
		// FIXME: GDK_THREADS_LEAVE ();
		dmap_connection_finish (connection);
		// FIXME: GDK_THREADS_ENTER ();
	} else {
		priv->state = DMAP_LOGOUT;

		priv->do_something_id = g_idle_add ((GSourceFunc) dmap_connection_do_something, connection);
	}
}

static void
dmap_connection_state_done (DMAPConnection *connection,
			       gboolean          result)
{
	DMAPConnectionPrivate *priv = connection->priv;

	g_debug ("Transitioning to next state from %d", priv->state);

	if (result == FALSE) {
		priv->state = DMAP_DONE;
		priv->result = FALSE;
	} else {
		switch (priv->state) {
		case DMAP_GET_PLAYLISTS:
			if (priv->playlists == NULL)
				priv->state = DMAP_DONE;
			else
				priv->state = DMAP_GET_PLAYLIST_ENTRIES;
			break;
		case DMAP_GET_PLAYLIST_ENTRIES:
			/* keep reading playlists until we've got them all */
			if (++priv->reading_playlist >= g_slist_length (priv->playlists))
				priv->state = DMAP_DONE;
			break;

		case DMAP_LOGOUT:
			priv->state = DMAP_DONE;
			break;

		case DMAP_DONE:
			/* uhh.. */
			g_debug ("This should never happen.");
			break;

		default:
			/* in most states, we just move on to the next */
			if (priv->state > DMAP_DONE) {
				g_debug ("This should REALLY never happen.");
				return;
			}
			priv->state++;
			break;
		}

		priv->progress = 1.0f;
		if (connection->priv->emit_progress_id != 0) {
			g_source_remove (connection->priv->emit_progress_id);
		}
		connection->priv->emit_progress_id = g_idle_add ((GSourceFunc) emit_progress_idle, connection);
	}

	if (priv->do_something_id != 0) {
		g_source_remove (priv->do_something_id);
	}
	priv->do_something_id = g_idle_add ((GSourceFunc) dmap_connection_do_something, connection);
}

static gboolean
dmap_connection_do_something (DMAPConnection *connection)
{
	DMAPConnectionPrivate *priv = connection->priv;
	char *path;

	g_debug ("Doing something for state: %d", priv->state);

	priv->do_something_id = 0;

	switch (priv->state) {
	case DMAP_GET_INFO:
		g_debug ("Getting DAAP server info");
		if (! http_get (connection, "/server-info", FALSE, 0.0, 0, FALSE,
				(DMAPResponseHandler) handle_server_info, FALSE)) {
			g_debug ("Could not get DAAP connection info");
			dmap_connection_state_done (connection, FALSE);
		}
		break;

	case DMAP_GET_PASSWORD:
		if (priv->password_protected) {
			/* FIXME this bit is still synchronous */
			g_debug ("Need a password for %s", priv->name);
			g_free (priv->password);
			priv->password = connection_get_password (connection);

			if (priv->password == NULL || priv->password[0] == '\0') {
				g_debug ("Password entry cancelled");
				priv->result = FALSE;
				priv->state = DMAP_DONE;
				dmap_connection_do_something (connection);
				return FALSE;
			}

			/* If the share went away while we were asking for the password,
			 * don't bother trying to log in.
			 */
			if (priv->state != DMAP_GET_PASSWORD) {
				return FALSE;
			}
		}

		/* otherwise, fall through */
		priv->state = DMAP_LOGIN;

	case DMAP_LOGIN:
		g_debug ("Logging into DAAP server");
		if (! http_get (connection, "/login", FALSE, 0.0, 0, FALSE,
			       (DMAPResponseHandler) handle_login, FALSE)) {
			g_debug ("Could not login to DAAP server");
			/* FIXME: set state back to GET_PASSWORD to try again */
			dmap_connection_state_done (connection, FALSE);
		}

		break;

	case DMAP_GET_REVISION_NUMBER:
		g_debug ("Getting DAAP server database revision number");
		path = g_strdup_printf ("/update?session-id=%u&revision-number=1", priv->session_id);
		if (! http_get (connection, path, TRUE, priv->dmap_version, 0, FALSE,
			       (DMAPResponseHandler) handle_update, FALSE)) {
			g_debug ("Could not get server database revision number");
			dmap_connection_state_done (connection, FALSE);
		}
		g_free (path);
		break;

	case DMAP_GET_DB_INFO:
		g_debug ("Getting DAAP database info");
		path = g_strdup_printf ("/databases?session-id=%u&revision-number=%d",
					priv->session_id, priv->revision_number);
		if (! http_get (connection, path, TRUE, priv->dmap_version, 0, FALSE,
			       (DMAPResponseHandler) handle_database_info, FALSE)) {
			g_debug ("Could not get DAAP database info");
			dmap_connection_state_done (connection, FALSE);
		}
		g_free (path);
		break;

	case DMAP_GET_SONGS:
		g_debug ("Getting DAAP song listing");
		path = g_strdup_printf ("/databases/%i/items?session-id=%u&revision-number=%i"
				        "&meta=dmap.itemid,dmap.itemname,daap.songalbum,"
					"daap.songartist,daap.daap.songgenre,daap.songsize,"
					"daap.songtime,daap.songtrackcount,daap.songtracknumber,"
					"daap.songyear,daap.songformat,daap.songgenre,"
					"daap.songbitrate,daap.songdiscnumber,daap.songdataurl",
					priv->database_id,
					priv->session_id,
					priv->revision_number);
		if (! http_get (connection, path, TRUE, priv->dmap_version, 0, FALSE,
			       (DMAPResponseHandler) handle_song_listing, TRUE)) {
			g_debug ("Could not get DAAP song listing");
			dmap_connection_state_done (connection, FALSE);
		}
		g_free (path);
		break;

	case DMAP_GET_PLAYLISTS:
		g_debug ("Getting DAAP playlists");
		path = g_strdup_printf ("/databases/%d/containers?session-id=%u&revision-number=%d",
					priv->database_id,
					priv->session_id,
					priv->revision_number);
		if (! http_get (connection, path, TRUE, priv->dmap_version, 0, FALSE,
			       (DMAPResponseHandler) handle_playlists, TRUE)) {
			g_debug ("Could not get DAAP playlists");
			dmap_connection_state_done (connection, FALSE);
		}
		g_free (path);
		break;

	case DMAP_GET_PLAYLIST_ENTRIES:
		{
			DMAPPlaylist *playlist =
				(DMAPPlaylist *) g_slist_nth_data (priv->playlists,
								     priv->reading_playlist);
			g_assert (playlist);
			g_debug ("Reading DAAP playlist %d entries", priv->reading_playlist);
			path = g_strdup_printf ("/databases/%d/containers/%d/items?session-id=%u&revision-number=%d&meta=dmap.itemid",
						priv->database_id,
						playlist->id,
						priv->session_id, priv->revision_number);
			if (! http_get (connection, path, TRUE, priv->dmap_version, 0, FALSE,
				       (DMAPResponseHandler) handle_playlist_entries, TRUE)) {
				g_debug ("Could not get entries for DAAP playlist %d",
					  priv->reading_playlist);
				dmap_connection_state_done (connection, FALSE);
			}
			g_free (path);
		}
		break;

	case DMAP_LOGOUT:
		g_debug ("Logging out of DAAP server");
		path = g_strdup_printf ("/logout?session-id=%u", priv->session_id);
		if (! http_get (connection, path, TRUE, priv->dmap_version, 0, FALSE,
			       (DMAPResponseHandler) handle_logout, FALSE)) {
			g_debug ("Could not log out of DAAP server");
			dmap_connection_state_done (connection, FALSE);
		}

		g_free (path);
		break;

	case DMAP_DONE:
		g_debug ("DAAP done");

		dmap_connection_finish (connection);

		break;
	}

	return FALSE;
}

SoupMessageHeaders *
dmap_connection_get_headers (DMAPConnection *connection,
				const gchar *uri)
{
	DMAPConnectionPrivate *priv = connection->priv;
	SoupMessageHeaders *headers = NULL;
	char hash[33] = {0};
	char *norb_daap_uri = (char *)uri;
	char *request_id;

	priv->request_id++;

	if (g_ascii_strncasecmp (uri, "daap://", 7) == 0) {
		norb_daap_uri = strstr (uri, "/data");
	}

	dmap_hash_generate ((short)floorf (priv->dmap_version),
			       (const guchar*)norb_daap_uri, 2,
			       (guchar*)hash,
			       priv->request_id);

	headers = soup_message_headers_new (SOUP_MESSAGE_HEADERS_REQUEST);

	soup_message_headers_append (headers, "Accept", "*/*");
	soup_message_headers_append (headers, "Cache-Control", "no-cache");
	soup_message_headers_append (headers, "User-Agent", DMAP_USER_AGENT);
	soup_message_headers_append (headers, "Accept-Language",
				    "en-us, en;q=5.0");
	soup_message_headers_append (headers, "Client-DAAP-Access-Index",
				    "2");
	soup_message_headers_append (headers, "Client-DAAP-Version", "3.0");
	soup_message_headers_append (headers, "Client-DAAP-Validation", hash);

	request_id = g_strdup_printf ("%d", priv->request_id);
	soup_message_headers_append (headers, "Client-DAAP-Request-ID",
				     request_id);
	g_free (request_id);

	soup_message_headers_append (headers, "Connection", "close");

	if (priv->password_protected) {
		char *h;
		char *user_pass;
		char *token;

		user_pass = g_strdup_printf ("%s:%s", priv->username, priv->password);
		token = g_base64_encode ((guchar *)user_pass, strlen (user_pass));
		h = g_strdup_printf ("Basic %s", token);

		g_free (token);
		g_free (user_pass);

		soup_message_headers_append (headers, "Authentication", h);
		g_free (h);
	}

	return headers;
}

GSList *
dmap_connection_get_playlists (DMAPConnection *connection)
{
	return connection->priv->playlists;
}

static void
dmap_connection_dispose (GObject *object)
{
	DMAPConnectionPrivate *priv = DMAP_CONNECTION (object)->priv;
	GSList *l;

	g_debug ("DAAP connection dispose");

	if (priv->emit_progress_id != 0) {
		g_source_remove (priv->emit_progress_id);
		priv->emit_progress_id = 0;
	}

	if (priv->do_something_id != 0) {
		g_source_remove (priv->do_something_id);
		priv->do_something_id = 0;
	}

	if (priv->name) {
		g_free (priv->name);
		priv->name = NULL;
	}

	if (priv->username) {
		g_free (priv->username);
		priv->username = NULL;
	}

	if (priv->password) {
		g_free (priv->password);
		priv->password = NULL;
	}

	if (priv->host) {
		g_free (priv->host);
		priv->host = NULL;
	}

	if (priv->playlists) {
		for (l = priv->playlists; l; l = l->next) {
			DMAPPlaylist *playlist = l->data;

			/* FIXME: refstring: */
			g_list_foreach (playlist->uris, (GFunc) g_free, NULL);
			g_list_free (playlist->uris);
			g_free (playlist->name);
			g_free (playlist);
			l->data = NULL;
		}
		g_slist_free (priv->playlists);
		priv->playlists = NULL;
	}

	if (priv->item_id_to_uri) {
		g_hash_table_destroy (priv->item_id_to_uri);
		priv->item_id_to_uri = NULL;
	}

	if (priv->session) {
		g_debug ("Aborting all pending requests");
		soup_session_abort (priv->session);
		g_object_unref (G_OBJECT (priv->session));
		priv->session = NULL;
	}

	if (priv->base_uri) {
		soup_uri_free (priv->base_uri);
		priv->base_uri = NULL;
	}

	if (priv->daap_base_uri) {
		g_free (priv->daap_base_uri);
		priv->daap_base_uri = NULL;
	}

	if (priv->db) {
		g_object_unref (G_OBJECT (priv->db));
		priv->db = NULL;
	}

	if (priv->last_error_message != NULL) {
		g_free (priv->last_error_message);
		priv->last_error_message = NULL;
	}

	G_OBJECT_CLASS (dmap_connection_parent_class)->dispose (object);
}

static void
dmap_connection_set_property (GObject *object,
				 guint prop_id,
				 const GValue *value,
				 GParamSpec *pspec)
{
	DMAPConnectionPrivate *priv = DMAP_CONNECTION (object)->priv;

	switch (prop_id) {
	case PROP_NAME:
		g_free (priv->name);
		priv->name = g_value_dup_string (value);
		break;
	case PROP_DB:
		priv->db = DMAP_DB (g_value_get_pointer (value));
		break;
	case PROP_FACTORY:
		priv->record_factory = DMAP_RECORD_FACTORY (g_value_get_pointer (value));
		break;
	case PROP_PASSWORD_PROTECTED:
		priv->password_protected = g_value_get_boolean (value);
		break;
	case PROP_HOST:
		g_free (priv->host);
		priv->host = g_value_dup_string (value);
		break;
	case PROP_PORT:
		priv->port = g_value_get_uint (value);
		break;
	case PROP_BASE_URI:
		priv->base_uri = g_value_get_pointer (value);
		break;
	case PROP_DATABASE_ID:
		priv->database_id = g_value_get_int (value);
		break;
	case PROP_SESSION_ID:
		priv->session_id = g_value_get_int (value);
		break;
	case PROP_DMAP_VERSION:
		priv->dmap_version = g_value_get_double (value);
		break;
	case PROP_REVISION_NUMBER:
		priv->revision_number = g_value_get_int (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
dmap_connection_get_property (GObject *object,
				 guint prop_id,
				 GValue *value,
				 GParamSpec *pspec)
{
	DMAPConnectionPrivate *priv = DMAP_CONNECTION (object)->priv;

	switch (prop_id) {
	case PROP_DB:
		g_value_set_pointer (value, priv->db);
		break;
	case PROP_FACTORY:
		g_value_set_pointer (value, priv->record_factory);
		break;
	case PROP_NAME:
		g_value_set_string (value, priv->name);
		break;
	case PROP_PASSWORD_PROTECTED:
		g_value_set_boolean (value, priv->password_protected);
		break;
	case PROP_HOST:
		g_value_set_string (value, priv->host);
		break;
	case PROP_PORT:
		g_value_set_uint (value, priv->port);
		break;
	case PROP_BASE_URI:
		g_value_set_pointer (value, priv->base_uri);
		break;
	case PROP_DATABASE_ID:
		g_value_set_int (value, priv->database_id);
		break;
	case PROP_SESSION_ID:
		g_value_set_int (value, priv->session_id);
		break;
	case PROP_DMAP_VERSION:
		g_value_set_double (value, priv->dmap_version);
		break;
	case PROP_REVISION_NUMBER:
		g_value_set_int (value, priv->revision_number);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}
