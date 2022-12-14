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
#include "dmap-connection-private.h"
#include "dmap-error.h"
#include "dmap-record-factory.h"
#include "dmap-structure.h"

#define DMAP_USER_AGENT "iTunes/4.6 (Windows; N)"

#define ITUNES_7_SERVER "iTunes/7"

static gboolean _do_something (DmapConnection * connection);

struct DmapConnectionPrivate
{
	char *name;
	char *username;
	char *password;
	char *host;
	guint port;

	gboolean is_connected;
	gboolean is_connecting;

	SoupSession *session;
	GUri *base_uri;
	gchar *daap_base_uri;

	gdouble dmap_version;
	guint32 session_id;
	gint revision_number;

	gint request_id;
	gint database_id;

	guint reading_playlist;
	GSList *playlists;
	GHashTable *item_id_to_uri;

	DmapDb *db;
	DmapRecordFactory *record_factory;

	DmapConnectionState state;
	gboolean use_response_handler_thread;
	float progress;

	guint emit_progress_id;
	guint do_something_id;

	gboolean result;
	char *last_error_message;
};

G_DEFINE_TYPE_WITH_PRIVATE (DmapConnection,
                            dmap_connection,
                            G_TYPE_OBJECT);

static void
dmap_connection_init (DmapConnection * connection)
{
	connection->priv = dmap_connection_get_instance_private(connection);
}

enum
{
	PROP_0,
	PROP_DB,
	PROP_FACTORY,
	PROP_NAME,
	PROP_ENTRY_TYPE,
	PROP_HOST,
	PROP_PORT,
	PROP_BASE_URI,
	PROP_DATABASE_ID,
	PROP_SESSION_ID,
	PROP_DMAP_VERSION,
	PROP_REVISION_NUMBER,
	PROP_USERNAME,
	PROP_PASSWORD,
};

enum
{
	AUTHENTICATE,
	CONNECTING,
	CONNECTED,
	DISCONNECTED,
	OPERATION_DONE,
	ERROR,
	LAST_SIGNAL
};

static guint _signals[LAST_SIGNAL] = { 0, };

static void
_dispose (GObject * object)
{
	DmapConnectionPrivate *priv = DMAP_CONNECTION (object)->priv;
	GSList *l;

	g_debug ("DMAP connection dispose");

	if (priv->emit_progress_id != 0) {
		g_source_remove (priv->emit_progress_id);
		priv->emit_progress_id = 0;
	}

	if (priv->do_something_id != 0) {
		g_source_remove (priv->do_something_id);
		priv->do_something_id = 0;
	}

	if (priv->playlists) {
		for (l = priv->playlists; l; l = l->next) {
			DmapPlaylist *playlist = l->data;

			/* FIXME: refstring: */
			g_list_free_full (playlist->uris, g_free);
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
		g_uri_unref (priv->base_uri);
		priv->base_uri = NULL;
	}

	if (priv->daap_base_uri) {
		g_free (priv->daap_base_uri);
		priv->daap_base_uri = NULL;
	}

	g_clear_object(&priv->db);
	g_clear_object(&priv->record_factory);

	if (priv->last_error_message != NULL) {
		g_free (priv->last_error_message);
		priv->last_error_message = NULL;
	}

	G_OBJECT_CLASS (dmap_connection_parent_class)->dispose (object);
}

static void
_finalize (GObject * object)
{
	g_debug ("Finalize");

	g_assert(DMAP_IS_CONNECTION (object));

	DmapConnection *connection = DMAP_CONNECTION (object);
	if (NULL == connection->priv) {
		goto done;
	}

	g_free (connection->priv->name);
	g_free (connection->priv->username);
	g_free (connection->priv->password);
	g_free (connection->priv->host);

	G_OBJECT_CLASS (dmap_connection_parent_class)->finalize (object);

done:
	return;
}

static void
_set_property (GObject * object, guint prop_id,
               const GValue * value, GParamSpec * pspec)
{
	DmapConnectionPrivate *priv = DMAP_CONNECTION (object)->priv;

	switch (prop_id) {
	case PROP_NAME:
		g_free (priv->name);
		priv->name = g_value_dup_string (value);
		break;
	case PROP_DB:
		if (priv->db) {
			g_object_unref(priv->db);
		}
		priv->db = DMAP_DB (g_value_dup_object (value));
		break;
	case PROP_FACTORY:
		if (priv->record_factory) {
			g_object_unref(priv->record_factory);
		}
		priv->record_factory =
			DMAP_RECORD_FACTORY (g_value_dup_object (value));
		break;
	case PROP_HOST:
		g_free (priv->host);
		priv->host = g_value_dup_string (value);
		break;
	case PROP_PORT:
		priv->port = g_value_get_uint (value);
		break;
	case PROP_BASE_URI:
		if (priv->base_uri) {
			g_uri_unref (priv->base_uri);
		}
		priv->base_uri = g_value_get_boxed (value);
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
	case PROP_USERNAME:
		g_free(priv->username);
		priv->username = g_value_dup_string (value);
		break;
	case PROP_PASSWORD:
		g_free(priv->password);
		priv->password = g_value_dup_string (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
_get_property (GObject * object, guint prop_id,
               GValue * value, GParamSpec * pspec)
{
	DmapConnectionPrivate *priv = DMAP_CONNECTION (object)->priv;

	switch (prop_id) {
	case PROP_DB:
		g_value_set_object (value, priv->db);
		break;
	case PROP_FACTORY:
		g_value_set_object (value, priv->record_factory);
		break;
	case PROP_NAME:
		g_value_set_string (value, priv->name);
		break;
	case PROP_HOST:
		g_value_set_string (value, priv->host);
		break;
	case PROP_PORT:
		g_value_set_uint (value, priv->port);
		break;
	case PROP_BASE_URI:
		g_value_set_boxed (value, priv->base_uri);
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
	case PROP_USERNAME:
		g_value_set_string (value, priv->username);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
dmap_connection_class_init (DmapConnectionClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	klass->get_protocol_version_cc = NULL;
	klass->get_query_metadata = NULL;
	klass->handle_mlcl = NULL;

	object_class->dispose = _dispose;
	object_class->finalize = _finalize;
	object_class->set_property = _set_property;
	object_class->get_property = _get_property;

	g_object_class_install_property (object_class,
					 PROP_DB,
					 g_param_spec_object ("db",
							      "DmapDb",
							      "DmapDb object",
	                                                      DMAP_TYPE_DB,
							      G_PARAM_READWRITE
							      |
							      G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
					 PROP_FACTORY,
					 g_param_spec_object ("factory",
							      "record factory",
							      "record factory",
	                                                       DMAP_TYPE_RECORD_FACTORY,
							      G_PARAM_READWRITE
							      |
							      G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class, PROP_NAME,
					 g_param_spec_string ("name",
							      "connection name",
							      "connection name",
							      NULL,
							      G_PARAM_READWRITE
							      |
							      G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (object_class, PROP_HOST,
					 g_param_spec_string ("host", "host",
							      "host", NULL,
							      G_PARAM_READWRITE
							      |
							      G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (object_class, PROP_PORT,
					 g_param_spec_uint ("port", "port",
							    "port", 0,
							    G_MAXINT, 0,
							    G_PARAM_READWRITE
							    |
							    G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
					 PROP_BASE_URI,
					 g_param_spec_boxed ("base-uri",
	                                                     "base URI",
	                                                     "base URI",
	                                                      G_TYPE_URI,
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
							      0, G_MAXDOUBLE,
							      0,
							      G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
					 PROP_REVISION_NUMBER,
					 g_param_spec_int ("revision-number",
							   "revision number",
							   "revision number",
							   0, G_MAXINT, 0,
							   G_PARAM_READWRITE));

	g_object_class_install_property (object_class, PROP_USERNAME,
					 g_param_spec_string ("username",
							      "connection username",
							      "connection username",
							      "libdmapsharing",
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class, PROP_PASSWORD,
					 g_param_spec_string ("password",
							      "connection password",
							      "connection password",
							      NULL,
							      G_PARAM_WRITABLE));

	_signals[AUTHENTICATE] = g_signal_new ("authenticate",
					      G_TYPE_FROM_CLASS
					      (object_class),
					      G_SIGNAL_RUN_LAST,
					      G_STRUCT_OFFSET
					      (DmapConnectionClass,
					       authenticate), NULL, NULL,
					      NULL,
					      G_TYPE_NONE, 5,
					      G_TYPE_STRING,
					      SOUP_TYPE_SESSION,
					      SOUP_TYPE_MESSAGE,
					      SOUP_TYPE_AUTH,
					      G_TYPE_BOOLEAN);
	_signals[CONNECTING] =
		g_signal_new ("connecting", G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (DmapConnectionClass,
					       connecting), NULL, NULL,
			      NULL, G_TYPE_NONE, 2,
			      G_TYPE_ULONG, G_TYPE_FLOAT);
	_signals[CONNECTED] =
		g_signal_new ("connected", G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (DmapConnectionClass,
					       connected), NULL, NULL,
			      NULL, G_TYPE_NONE, 0);
	_signals[DISCONNECTED] =
		g_signal_new ("disconnected",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (DmapConnectionClass,
					       disconnected), NULL, NULL,
			      NULL, G_TYPE_NONE, 0);
	_signals[OPERATION_DONE] =
		g_signal_new ("operation-done",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (DmapConnectionClass,
					       operation_done), NULL, NULL,
			      NULL, G_TYPE_NONE, 0);
	_signals[ERROR] =
		g_signal_new ("error",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_FIRST,
		              0, NULL, NULL,
		              NULL, G_TYPE_NONE, 1,
		              G_TYPE_POINTER);
}

static void
_connection_connected (DmapConnection * connection)
{
	g_debug ("Emitting connected");

	connection->priv->is_connected = TRUE;

	// FIXME: GDK_THREADS_ENTER ();
	g_signal_emit (connection, _signals[CONNECTED], 0);
	// FIXME: GDK_THREADS_LEAVE ();
}

static void
_connection_disconnected (DmapConnection * connection)
{
	g_debug ("Emitting disconnected");

	connection->priv->is_connected = FALSE;

	// FIXME: GDK_THREADS_ENTER ();
	g_signal_emit (connection, _signals[DISCONNECTED], 0);
	// FIXME: GDK_THREADS_LEAVE ();
}

static void
_connection_operation_done (DmapConnection * connection)
{
	g_debug ("Emitting operation done");

	// FIXME: GDK_THREADS_ENTER ();
	g_signal_emit (connection, _signals[OPERATION_DONE], 0);
	// FIXME: GDK_THREADS_LEAVE ();
}

static void
_message_add_headers (SoupMessage *message, DmapConnection * connection, const gchar * uri)
{
	DmapConnectionPrivate *priv = connection->priv;
	SoupMessageHeaders *headers;
	char hash[33] = { 0 };
	char *norb_daap_uri = (char *) uri;
	char *request_id;

	headers = soup_message_get_request_headers(message);

	priv->request_id++;

	if (g_ascii_strncasecmp (uri, "daap://", 7) == 0) {
		norb_daap_uri = strstr (uri, "/data");
	}

	dmap_md5_generate ((short) floorf (priv->dmap_version),
			    (const guchar *) norb_daap_uri, 2,
			    (guchar *) hash, priv->request_id);

	soup_message_headers_append (headers, "Accept", "*/*");
	soup_message_headers_append (headers, "Cache-Control", "no-cache");
	soup_message_headers_append (headers, "Accept-Language",
				     "en-us, en;q=5.0");
	soup_message_headers_append (headers, "Client-DAAP-Access-Index",
				     "2");
	soup_message_headers_append (headers, "Client-DAAP-Version", "3.0");
	soup_message_headers_append (headers, "Client-DAAP-Validation", hash);

	request_id = g_strdup_printf ("%d", priv->request_id);
	soup_message_headers_append (headers, "Client-DAAP-Request-ID",
				     request_id);
	soup_message_headers_append(headers, "User-Agent", DMAP_USER_AGENT);
	soup_message_headers_append(headers, "Connection", "close");
	g_free (request_id);
}

static void
_authenticate_cb (SoupMessage *msg, SoupAuth *auth, gboolean retrying,
                  DmapConnection *connection)
{
	if (retrying || ! connection->priv->password) {
		g_debug ("Requesting password from application");
		// FIXME: GDK_THREADS_ENTER ();
		g_signal_emit (connection,
			       _signals[AUTHENTICATE],
			       0,
			       connection->priv->name,
			       connection->priv->session,
			       msg,
			       auth,
			       retrying);
		// FIXME: GDK_THREADS_LEAVE ();
	} else {
		g_debug ("Using cached credentials");
		soup_auth_authenticate (auth, connection->priv->username, connection->priv->password);
	}
}

/*
 * FIXME: Assuming pause/unpause is really not needed, then session and message
 * parameters are not required. Drop them and prompt API change?
 */
void
dmap_connection_authenticate_message (DmapConnection * connection,
                                      G_GNUC_UNUSED SoupSession *session,
                                      G_GNUC_UNUSED SoupMessage *message,
                                      SoupAuth *auth,
                                      const char *password)
{
	char *username = NULL;

	g_object_set (connection, "password", password, NULL);

	g_object_get (connection, "username", &username, NULL);
	g_assert (username);

	soup_auth_authenticate (auth, username, password);

	g_free(username);
}

static SoupMessage *
_build_message (DmapConnection * connection,
                const char *path)
{
	SoupMessage *message = NULL;
	GUri *base_uri = NULL;
	GUri *uri = NULL;
	char *uri_str = NULL;

	g_object_get (connection, "base-uri", &base_uri, NULL);
	if (base_uri == NULL) {
		goto done;
	}

	uri = g_uri_parse_relative(base_uri, path, G_URI_FLAGS_NONE, NULL);
	if (uri == NULL) {
		goto done;
	}

	message = soup_message_new_from_uri (SOUP_METHOD_GET, uri);

	g_signal_connect (message, "authenticate", G_CALLBACK(_authenticate_cb), connection);

	/* FIXME: only set Client-DAAP-Validation if need_hash? */
	/* FIXME: only set Connection if send_close? */
	uri_str = g_uri_to_string (uri);

	_message_add_headers(message, connection, uri_str);

done:
	g_uri_unref (base_uri);
	g_uri_unref (uri);
	g_free (uri_str);

	return message;
}

#ifdef HAVE_LIBZ
static void *
_zalloc_wrapper (G_GNUC_UNUSED voidpf opaque, uInt items, uInt size)
{
	void *fnval = Z_NULL;

	if ((items != 0) && (size >= G_MAXUINT / items)) {
		goto done;
	}
	if ((size != 0) && (items >= G_MAXUINT / size)) {
		goto done;
	}

	fnval = g_malloc0 (items * size);

done:
	return fnval;
}

static void
_zfree_wrapper (G_GNUC_UNUSED voidpf opaque, voidpf address)
{
	g_free (address);
}
#endif

static void
_connection_set_error_message (DmapConnection * connection,
                               const char *message)
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
	GBytes *body;
	int status;
	DmapConnection *connection;

	char *message_path;
	char *reason_phrase;
	SoupMessageHeaders *headers;

	DmapResponseHandler response_handler;
	gpointer user_data;
} DmapResponseData;

static void
_dmap_response_data_free(DmapResponseData *data)
{
	if (NULL == data) {
		return;
	}

	g_bytes_unref(data->body);
	g_object_unref (G_OBJECT (data->connection));
	g_free(data->message_path);
	g_free(data->reason_phrase);
	soup_message_headers_unref(data->headers);
	g_free (data);
}

static gboolean
_emit_progress_idle (DmapConnection * connection)
{
	g_debug ("Emitting progress");

	// FIXME: GDK_THREADS_ENTER ();
	g_signal_emit (G_OBJECT (connection), _signals[CONNECTING], 0,
		       connection->priv->state, connection->priv->progress);
	connection->priv->emit_progress_id = 0;
	// FIXME: GDK_THREADS_LEAVE ();
	return FALSE;
}

static void
_state_done (DmapConnection * connection, gboolean result)
{
	DmapConnectionPrivate *priv = connection->priv;

	g_debug ("Transitioning to next state from %d", priv->state);

	if (result == FALSE) {
		priv->state = DMAP_DONE;
		priv->result = FALSE;
	} else {
		switch (priv->state) {
		case DMAP_GET_PLAYLISTS:
			if (priv->playlists == NULL) {
				priv->state = DMAP_DONE;
			} else {
				priv->state = DMAP_GET_PLAYLIST_ENTRIES;
			}
			break;
		case DMAP_GET_PLAYLIST_ENTRIES:
			/* keep reading playlists until we've got them all */
			if (++priv->reading_playlist >=
			    g_slist_length (priv->playlists)) {
				priv->state = DMAP_DONE;
			}
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
		connection->priv->emit_progress_id =
			g_idle_add ((GSourceFunc) _emit_progress_idle,
				    connection);
	}

	if (priv->do_something_id != 0) {
		g_source_remove (priv->do_something_id);
	}
	priv->do_something_id =
		g_idle_add ((GSourceFunc) _do_something, connection);
}

static gpointer
_actual_http_response_handler (DmapResponseData * data)
{
	DmapConnectionPrivate *priv;
	GNode *structure;
	guint8 *new_response = NULL;
	const guint8 *response;
	const char *encoding_header;
	gsize response_length;
	gboolean ok = FALSE;

	priv = data->connection->priv;
	structure = NULL;
	encoding_header = NULL;

	response = g_bytes_get_data(data->body, &response_length);

	g_debug ("Received response from %s: %d, %s",
		 data->message_path,
	         data->status,
		 data->reason_phrase);

	if (data->headers) {
		const char *server;

		encoding_header = soup_message_headers_get_one (data->headers, "Content-Encoding");
		server = soup_message_headers_get_one (data->headers, "DAAP-Server");

		if (server != NULL && strstr (server, ITUNES_7_SERVER) != NULL) {
			g_debug ("giving up. We can't talk to %s", server);
			_connection_set_error_message (
				data->connection,
				"libdmapsharing is not able to connect to iTunes 7 shares"
			);
			goto done;
		}
	}

	if (SOUP_STATUS_IS_SUCCESSFUL (data->status) && encoding_header
	    && strcmp (encoding_header, "gzip") == 0) {
#ifdef HAVE_LIBZ
		z_stream stream;
		unsigned int factor = 4;
		unsigned int unc_size = response_length * factor;

		stream.next_in = (unsigned char *) response;
		stream.avail_in = response_length;
		stream.total_in = 0;

		new_response = g_malloc (unc_size + 1);
		stream.next_out = (unsigned char *) new_response;
		stream.avail_out = unc_size;
		stream.total_out = 0;
		stream.zalloc = _zalloc_wrapper;
		stream.zfree = _zfree_wrapper;
		stream.opaque = NULL;

		if (inflateInit2
		    (&stream,
		     32 /* auto-detect */  + 15 /* max */ ) != Z_OK) {
			inflateEnd (&stream);
			g_free (new_response);
			new_response = NULL;
			g_debug ("Unable to decompress response from %s",
				 data->message_path);
			_connection_set_error_message (
				data->connection,
				"unable to decompress response"
			);
			goto done;
		} else {
			do {
				int z_res;

				z_res = inflate (&stream, Z_FINISH);
				if (z_res == Z_STREAM_END) {
					break;
				}
				if ((z_res != Z_OK && z_res != Z_BUF_ERROR)
				    || stream.avail_out != 0
				    || unc_size > 40 * 1000 * 1000) {
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
				new_response =
					g_realloc (new_response,
						   unc_size + 1);
				stream.next_out =
					(unsigned char *) (new_response +
							   stream.total_out);
				stream.avail_out =
					unc_size - stream.total_out;
			} while (1);
		}

		if (new_response) {
			response = new_response;
			response_length = stream.total_out;
		}
#else
		g_debug ("Received compressed response from %s but can't handle it", data->message_path);
		_connection_set_error_message (
			data->connection,
			"cannot handle compressed response"
		);
		goto done;
#endif
	}

	if (SOUP_STATUS_IS_SUCCESSFUL (data->status)) {
		GError *error = NULL;
		DmapStructureItem *item;

		if ( /* FIXME: !rb_is_main_thread () */ TRUE) {
			priv->progress = -1.0f;
			if (priv->emit_progress_id != 0) {
				g_source_remove (priv->emit_progress_id);
			}
			priv->emit_progress_id =
				g_idle_add ((GSourceFunc) _emit_progress_idle,
					    data->connection);
		}
		structure = dmap_structure_parse (response, response_length, &error);
		if (error != NULL) {
			dmap_connection_emit_error(data->connection, error->code,
			                          "Error parsing %s response: %s\n", data->message_path,
			                           error->message);
			g_clear_error(&error);
			goto done;
		} else {
			int dmap_status = 0;

			item = dmap_structure_find_item (structure,
							 DMAP_CC_MSTT);
			if (item) {
				dmap_status =
					g_value_get_int (&(item->content));

				if (dmap_status != 200) {
					g_debug ("Error, dmap.status is not 200 in response from %s", data->message_path);
					_connection_set_error_message (
						data->connection,
						"Bad response"
					);
					goto done;
				}
			}
		}
		if ( /* FIXME: ! rb_is_main_thread () */ TRUE) {
			priv->progress = 1.0f;
			if (priv->emit_progress_id != 0) {
				g_source_remove (priv->emit_progress_id);
			}
			priv->emit_progress_id =
				g_idle_add ((GSourceFunc) _emit_progress_idle,
					    data->connection);
		}
	} else {
		g_debug ("Error getting %s: %d, %s",
			 data->message_path,
		         data->status,
			 data->reason_phrase);
		_connection_set_error_message (data->connection,
					       data->reason_phrase);
	}

	if (data->response_handler) {
		(*(data->response_handler)) (data->connection, data->status,
					     structure, data->user_data);
	}

	ok = TRUE;

done:
	if (!ok) {
		_state_done (data->connection, FALSE);
	}

	if (structure) {
		dmap_structure_destroy (structure);
	}

	g_free (new_response);

	_dmap_response_data_free(data);

	return NULL;
}

static void
_http_response_handler (G_GNUC_UNUSED GObject *source,
                        GAsyncResult *result,
                        gpointer user_data)
{
	gboolean ok = FALSE;
	SoupSession *session = SOUP_SESSION(source);
	DmapResponseData *data = user_data;
	SoupMessage *message = NULL;
	goffset response_length;
	GError *error = NULL;

	data->body = soup_session_send_and_read_finish (session, result, &error);
	if (NULL == data->body) {
		g_debug("Failed to finish read: %s", error->message);
		goto done;
	}

	message = soup_session_get_async_result_message (session, result);
	if (NULL == message) {
		g_debug ("Failed to get message result");
		goto done;
	}

	data->status = soup_message_get_status(message);
	data->reason_phrase = g_strdup(soup_message_get_reason_phrase(message));
	data->headers = soup_message_headers_ref(soup_message_get_response_headers(message));
	response_length = g_bytes_get_size(data->body);

	if (response_length >= G_MAXUINT / 4 - 1) {
		/* If response_length is too big,
		 * the g_malloc (unc_size + 1) below would overflow
		 */
		g_debug ("Response length exceeded limit");
		goto done;
	}

	/* to avoid blocking the UI, handle big responses in a separate thread */
	if (SOUP_STATUS_IS_SUCCESSFUL (data->status)
	    && data->connection->priv->use_response_handler_thread) {
		g_debug ("creating thread to handle daap response");
		g_thread_new (NULL, (GThreadFunc) _actual_http_response_handler, data);
	} else {
		_actual_http_response_handler (data);
	}

	/*
	 * Ownership of data passed to _actual_http_response_handler; set to
	 * NULL to avoid freeing.
	 */
	data = NULL;

	ok = TRUE;

done:
	g_object_unref(message);

	if (!ok) {
		_state_done (data->connection, FALSE);
	}

	_dmap_response_data_free(data); /* Ownership possibly passed; see above. */

	return;
}

static gboolean
_http_get (DmapConnection * connection,
           const char *path,
           DmapResponseHandler handler,
           gpointer user_data, gboolean use_thread)
{
	gboolean ok = FALSE;
	DmapConnectionPrivate *priv = connection->priv;
	DmapResponseData *data;
	SoupMessage *message;

	message = _build_message (connection, path);
	if (message == NULL) {
		g_debug ("Error building message for http://%s:%d/%s",
			 g_uri_get_host(priv->base_uri), g_uri_get_port(priv->base_uri), path);
		goto done;
	}

	priv->use_response_handler_thread = use_thread;

	data = g_new0 (DmapResponseData, 1);
	data->message_path = g_uri_to_string (soup_message_get_uri (message));
	data->response_handler = handler;
	data->user_data = user_data;

	g_object_ref (G_OBJECT (connection));
	data->connection = connection;

	soup_session_send_and_read_async(
		priv->session,
		message,
		G_PRIORITY_DEFAULT,
		NULL,
		_http_response_handler,
		data
	);

	g_debug ("Queued message for http://%s:%d/%s", g_uri_get_host(priv->base_uri),
		 g_uri_get_port(priv->base_uri), path);

	ok = TRUE;

done:
	return ok;
}

gboolean
dmap_connection_get (DmapConnection * self,
		     const gchar * path,
		     DmapResponseHandler handler, gpointer user_data)
{
	return _http_get (self, path, (DmapResponseHandler) handler, user_data,
	                  FALSE);
}

static void
_handle_server_info (DmapConnection * connection, guint status,
                     GNode * structure, G_GNUC_UNUSED gpointer user_data)
{
	gboolean ok = FALSE;
	DmapConnectionPrivate *priv = connection->priv;
	DmapStructureItem *item = NULL;

	if (!SOUP_STATUS_IS_SUCCESSFUL (status) || structure == NULL) {
		goto done;
	}

	/* get the daap version number */
	item = dmap_structure_find_item (structure,
					 DMAP_CONNECTION_GET_CLASS
					 (connection)->get_protocol_version_cc
					 (connection));
	if (item == NULL) {
		goto done;
	}

	priv->dmap_version = g_value_get_double (&(item->content));
	ok = TRUE;

done:
	_state_done (connection, ok);
	return;
}

static void
_handle_login (DmapConnection * connection, guint status, GNode * structure,
               G_GNUC_UNUSED gpointer user_data)
{
	gboolean ok = FALSE;
	DmapConnectionPrivate *priv = connection->priv;
	DmapStructureItem *item = NULL;

	if (status == SOUP_STATUS_UNAUTHORIZED
	    || status == SOUP_STATUS_FORBIDDEN) {
		g_debug ("Incorrect password");
		// Maintain present state; libsoup will signal for password.
		if (priv->do_something_id != 0) {
			g_source_remove (priv->do_something_id);
		}
		priv->do_something_id = g_idle_add ((GSourceFunc) _do_something,
						    connection);
		goto done;
	}

	if (structure == NULL || SOUP_STATUS_IS_SUCCESSFUL (status) == FALSE) {
		goto done;
	}

	item = dmap_structure_find_item (structure, DMAP_CC_MLID);
	if (item == NULL) {
		g_debug ("Could not find daap.sessionid item in /login");
		goto done;
	}

	priv->session_id = (guint32) g_value_get_int (&(item->content));

	_connection_connected (connection);

	ok = TRUE;

done:
	_state_done (connection, ok);
	return;
}

static void
_handle_update (DmapConnection * connection, guint status, GNode * structure,
                G_GNUC_UNUSED gpointer user_data)
{
	gboolean ok = FALSE;
	DmapConnectionPrivate *priv = connection->priv;
	DmapStructureItem *item;

	if (structure == NULL || SOUP_STATUS_IS_SUCCESSFUL (status) == FALSE) {
		goto done;
	}

	/* get a revision number */
	item = dmap_structure_find_item (structure, DMAP_CC_MUSR);
	if (item == NULL) {
		g_debug ("Could not find daap.serverrevision item in /update");
		goto done;
	}

	priv->revision_number = g_value_get_int (&(item->content));

	ok = TRUE;

done:
	_state_done (connection, ok);
	return;
}

static void
_handle_database_info (DmapConnection * connection, guint status,
                       GNode * structure, G_GNUC_UNUSED gpointer user_data)
{
	gboolean ok = FALSE;
	DmapConnectionPrivate *priv = connection->priv;
	DmapStructureItem *item = NULL;
	GNode *listing_node;
	gint n_databases = 0;

	/* get a list of databases, there should be only 1 */

	if (structure == NULL || SOUP_STATUS_IS_SUCCESSFUL (status) == FALSE) {
		goto done;
	}

	item = dmap_structure_find_item (structure, DMAP_CC_MRCO);
	if (item == NULL) {
		g_debug ("Could not find dmap.returnedcount item in /databases");
		goto done;
	}

	n_databases = g_value_get_int (&(item->content));
	if (n_databases != 1) {
		g_debug ("Host seems to have more than 1 database, how strange");
	}

	listing_node = dmap_structure_find_node (structure, DMAP_CC_MLCL);
	if (listing_node == NULL) {
		g_debug ("Could not find dmap.listing item in /databases");
		goto done;
	}

	item = dmap_structure_find_item (listing_node->children,
					 DMAP_CC_MIID);
	if (item == NULL) {
		g_debug ("Could not find dmap.itemid item in /databases");
		goto done;
	}

	priv->database_id = g_value_get_int (&(item->content));

	ok = TRUE;

done:
	_state_done (connection, ok);
	return;
}

static void
_handle_song_listing (DmapConnection * connection, guint status,
                      GNode * structure, G_GNUC_UNUSED gpointer user_data)
{
	gboolean ok = FALSE;
	DmapConnectionPrivate *priv = connection->priv;
	DmapStructureItem *item = NULL;
	GNode *listing_node;
	gint returned_count;
	gint i;
	GNode *n;
	gint commit_batch;

	/* get the songs */

	if (structure == NULL || SOUP_STATUS_IS_SUCCESSFUL (status) == FALSE) {
		goto done;
	}

	item = dmap_structure_find_item (structure, DMAP_CC_MRCO);
	if (item == NULL) {
		g_debug ("Could not find dmap.returnedcount item in /databases/%d/items", priv->database_id);
		goto done;
	}
	returned_count = g_value_get_int (&(item->content));
	if (returned_count > 20) {
		commit_batch = returned_count / 20;
	} else {
		commit_batch = 1;
	}

	item = dmap_structure_find_item (structure, DMAP_CC_MTCO);
	if (item == NULL) {
		g_debug ("Could not find dmap.specifiedtotalcount item in /databases/%d/items", priv->database_id);
		goto done;
	}

	item = dmap_structure_find_item (structure, DMAP_CC_MUTY);
	if (item == NULL) {
		g_debug ("Could not find dmap.updatetype item in /databases/%d/items", priv->database_id);
		goto done;
	}

	listing_node = dmap_structure_find_node (structure, DMAP_CC_MLCL);
	if (listing_node == NULL) {
		g_debug ("Could not find dmap.listing item in /databases/%d/items", priv->database_id);
		goto done;
	}

	/* FIXME: refstring: */
	priv->item_id_to_uri =
		g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL,
				       (GDestroyNotify) g_free);

	priv->progress = 0.0f;
	if (priv->emit_progress_id != 0) {
		g_source_remove (priv->emit_progress_id);
	}
	priv->emit_progress_id =
		g_idle_add ((GSourceFunc) _emit_progress_idle, connection);

	for (i = 0, n = listing_node->children; n; i++, n = n->next) {
		gint item_id = 0;
		DmapRecord *record =
			DMAP_CONNECTION_GET_CLASS (connection)->handle_mlcl
			(connection, priv->record_factory, n,
			 &item_id);

		if (record) {
			GError *error = NULL;
			gchar *uri = NULL;
			gchar *format = NULL;

			g_object_get (record, "format", &format, NULL);
			if (format == NULL) {
				format = g_strdup ("Unknown");
			}

			/*if (connection->dmap_version == 3.0) { */
			uri = g_strdup_printf
				("%s/databases/%d/items/%d.%s?session-id=%u",
				 connection->priv->daap_base_uri,
				 connection->priv->database_id, item_id,
				 format, connection->priv->session_id);
			/*} else { */
			/* uri should be
			 * "/databases/%d/items/%d.%s?session-id=%u&revision-id=%d";
			 * but its not going to work cause the other parts of the code
			 * depend on the uri to have the ip address so that the
			 * DAAPSource can be found to ++request_id
			 * maybe just /dont/ support older itunes.  doesn't seem
			 * unreasonable to me, honestly
			 */
			/*} */

			g_object_set (record, "location", uri, NULL);
			dmap_db_add (connection->priv->db, record, &error);
			if (NULL != error) {
				g_signal_emit (connection, _signals[ERROR], 0, error);
			}
			g_object_unref (record);
			g_hash_table_insert (connection->priv->item_id_to_uri,
					     GINT_TO_POINTER (item_id),
					     g_strdup (uri));
			g_free (uri);
			g_free (format);
		} else {
			g_debug ("cannot create record for daap track");
		}

		if (i % commit_batch == 0) {
			priv->progress = ((float) i / (float) returned_count);
			if (priv->emit_progress_id != 0) {
				g_source_remove (connection->
						 priv->emit_progress_id);
			}
			priv->emit_progress_id =
				g_idle_add ((GSourceFunc) _emit_progress_idle,
					    connection);
		}
	}

	ok = TRUE;

done:
	_state_done (connection, ok);
	return;
}

static int
_compare_playlists_by_name (gconstpointer a, gconstpointer b)
{
	const DmapPlaylist *playlist1 = a;
	const DmapPlaylist *playlist2 = b;

	return strcmp (playlist1->name, playlist2->name);
}

/* FIXME
 * what we really should do is only get a list of playlists and their ids
 * then when they are clicked on ('activate'd) by the user, get a list of
 * the files that are actually in them.  This will speed up initial daap
 * connection times and reduce memory consumption.
 */

static void
_handle_playlists (DmapConnection * connection, guint status,
                   GNode * structure, G_GNUC_UNUSED gpointer user_data)
{
	gboolean ok = FALSE;
	DmapConnectionPrivate *priv = connection->priv;
	GNode *listing_node;
	gint i;
	GNode *n;

	if (structure == NULL || SOUP_STATUS_IS_SUCCESSFUL (status) == FALSE) {
		goto done;
	}

	listing_node = dmap_structure_find_node (structure, DMAP_CC_MLCL);
	if (listing_node == NULL) {
		g_debug ("Could not find dmap.listing item in /databases/%d/containers", priv->database_id);
		goto done;
	}

	for (i = 0, n = listing_node->children; n; n = n->next, i++) {
		DmapStructureItem *item;
		gint id;
		gchar *name;
		DmapPlaylist *playlist;

		item = dmap_structure_find_item (n, DMAP_CC_ABPL);
		if (item != NULL) {
			continue;
		}

		item = dmap_structure_find_item (n, DMAP_CC_MIID);
		if (item == NULL) {
			g_debug ("Could not find dmap.itemid item in /databases/%d/containers", priv->database_id);
			continue;
		}
		id = g_value_get_int (&(item->content));

		item = dmap_structure_find_item (n, DMAP_CC_MINM);
		if (item == NULL) {
			g_debug ("Could not find dmap.itemname item in /databases/%d/containers", priv->database_id);
			continue;
		}
		name = g_value_dup_string (&(item->content));

		playlist = g_new0 (DmapPlaylist, 1);
		playlist->id = id;
		playlist->name = name;
		g_debug ("Got playlist %p: name %s, id %d", playlist,
			 playlist->name, playlist->id);

		priv->playlists = g_slist_prepend (priv->playlists, playlist);
	}

	/* Sort the playlists into lexical order. Established DAAP clients already
	 * do this leading to an absence of sorting functionality in DAAP servers. */
	priv->playlists =
		g_slist_sort (priv->playlists, _compare_playlists_by_name);

	ok = TRUE;

done:
	_state_done (connection, ok);
	return;
}

static void
_handle_playlist_entries (DmapConnection * connection, guint status,
                          GNode * structure, G_GNUC_UNUSED gpointer user_data)
{
	gboolean ok = FALSE;
	DmapConnectionPrivate *priv = connection->priv;
	DmapPlaylist *playlist;
	GNode *listing_node;
	GNode *node;
	gint i;
	GList *playlist_uris = NULL;

	if (structure == NULL || SOUP_STATUS_IS_SUCCESSFUL (status) == FALSE) {
		goto done;
	}

	playlist =
		(DmapPlaylist *) g_slist_nth_data (priv->playlists,
						   priv->reading_playlist);
	g_assert (playlist);

	listing_node = dmap_structure_find_node (structure, DMAP_CC_MLCL);
	if (listing_node == NULL) {
		g_debug ("Could not find dmap.listing item in /databases/%d/containers/%d/items", priv->database_id, playlist->id);
		goto done;
	}

	for (i = 0, node = listing_node->children; node;
	     node = node->next, i++) {
		gchar *item_uri;
		gint playlist_item_id;
		DmapStructureItem *item;

		item = dmap_structure_find_item (node, DMAP_CC_MIID);
		if (item == NULL) {
			g_debug ("Could not find dmap.itemid item in /databases/%d/containers/%d/items", priv->database_id, playlist->id);
			continue;
		}
		playlist_item_id = g_value_get_int (&(item->content));

		item_uri =
			g_hash_table_lookup (priv->item_id_to_uri,
					     GINT_TO_POINTER
					     (playlist_item_id));
		if (item_uri == NULL) {
			g_debug ("Entry %d in playlist %s doesn't exist in the database", playlist_item_id, playlist->name);
			continue;
		}

		/* FIXME: Rhythmbox used "refstrings," so that the same memory would exist in both
		 * this list and the hash table. See FIXME: refstring thoughout. */
		playlist_uris =
			g_list_prepend (playlist_uris, g_strdup (item_uri));
	}

	playlist->uris = g_list_reverse (playlist_uris);

	ok = TRUE;

done:
	_state_done (connection, ok);
	return;
}

static void
_handle_logout (DmapConnection * connection, G_GNUC_UNUSED guint status,
                G_GNUC_UNUSED GNode * structure, G_GNUC_UNUSED gpointer user_data)
{
	_connection_disconnected (connection);

	/* is there any point handling errors here? */
	_state_done (connection, TRUE);
}

static void
_finish (DmapConnection * connection)
{
	g_assert(DMAP_IS_CONNECTION (connection));

	g_debug ("DMAP finish");
	connection->priv->state = DMAP_DONE;
	connection->priv->progress = 1.0f;

	_connection_operation_done (connection);

	return;
}

static gboolean
_do_something (DmapConnection * connection)
{
	DmapConnectionPrivate *priv = connection->priv;
	char *meta;
	char *path;

	g_debug ("Doing something for state: %d", priv->state);

	priv->do_something_id = 0;

	switch (priv->state) {
	case DMAP_GET_INFO:
		g_debug ("Getting DMAP server info");
		if (!_http_get
		    (connection, "/server-info",
		     (DmapResponseHandler) _handle_server_info, NULL, FALSE)) {
			g_debug ("Could not get DMAP connection info");
			_state_done (connection, FALSE);
		}
		break;

	case DMAP_LOGIN:
		// NOTE: libsoup will signal if password required and not present.
		g_debug ("Logging into DMAP server");
		if (!_http_get (connection, "/login",
			       (DmapResponseHandler) _handle_login, NULL,
			       FALSE)) {
			g_debug ("Could not login to DMAP server");
			_state_done (connection, FALSE);
		}

		break;

	case DMAP_GET_REVISION_NUMBER:
		g_debug ("Getting DMAP server database revision number");
		path = g_strdup_printf
			("/update?session-id=%u&revision-number=1",
			 priv->session_id);
		if (!_http_get
		    (connection, path,
		     (DmapResponseHandler) _handle_update, NULL, FALSE)) {
			g_debug ("Could not get server database revision number");
			_state_done (connection, FALSE);
		}
		g_free (path);
		break;

	case DMAP_GET_DB_INFO:
		g_debug ("Getting DMAP database info");
		path = g_strdup_printf
			("/databases?session-id=%u&revision-number=%d",
			 priv->session_id, priv->revision_number);
		if (!_http_get
		    (connection, path,
		     (DmapResponseHandler) _handle_database_info, NULL,
		     FALSE)) {
			g_debug ("Could not get DMAP database info");
			_state_done (connection, FALSE);
		}
		g_free (path);
		break;

	case DMAP_GET_MEDIA:
		g_debug ("Getting DMAP song listing");
		meta = DMAP_CONNECTION_GET_CLASS
			(connection)->get_query_metadata (connection);
		path = g_strdup_printf
			("/databases/%i/items?session-id=%u&revision-number=%i"
			 "&meta=%s", priv->database_id, priv->session_id,
			 priv->revision_number, meta);
		if (!_http_get
		    (connection, path,
		     (DmapResponseHandler) _handle_song_listing, NULL, TRUE)) {
			g_debug ("Could not get DMAP song listing");
			_state_done (connection, FALSE);
		}
		g_free (path);
		g_free (meta);
		break;

	case DMAP_GET_PLAYLISTS:
		g_debug ("Getting DMAP playlists");
		path = g_strdup_printf
			("/databases/%d/containers?session-id=%u&revision-number=%d",
			 priv->database_id, priv->session_id,
			 priv->revision_number);
		if (!_http_get
		    (connection, path,
		     (DmapResponseHandler) _handle_playlists, NULL, TRUE)) {
			g_debug ("Could not get DMAP playlists");
			_state_done (connection, FALSE);
		}
		g_free (path);
		break;

	case DMAP_GET_PLAYLIST_ENTRIES:
		{
			DmapPlaylist *playlist =
				(DmapPlaylist *)
				g_slist_nth_data (priv->playlists,
						  priv->reading_playlist);

			g_assert (playlist);
			g_debug ("Reading DMAP playlist %d entries",
				 priv->reading_playlist);
			path = g_strdup_printf
				("/databases/%d/containers/%d/items?session-id=%u&revision-number=%d&meta=dmap.itemid",
				 priv->database_id, playlist->id,
				 priv->session_id, priv->revision_number);
			if (!_http_get
			    (connection, path,
			     (DmapResponseHandler) _handle_playlist_entries,
			     NULL, TRUE)) {
				g_debug ("Could not get entries for DMAP playlist %d", priv->reading_playlist);
				_state_done (connection,
							    FALSE);
			}
			g_free (path);
		}
		break;

	case DMAP_LOGOUT:
		g_debug ("Logging out of DMAP server");
		path = g_strdup_printf ("/logout?session-id=%u",
					priv->session_id);
		if (!_http_get
		    (connection, path,
		     (DmapResponseHandler) _handle_logout, NULL, FALSE)) {
			g_debug ("Could not log out of DMAP server");
			_state_done (connection, FALSE);
		}

		g_free (path);
		break;

	case DMAP_DONE:
		g_debug ("DMAP done");

		_finish (connection);

		break;
	}

	return FALSE;
}

gboolean
dmap_connection_is_connected (DmapConnection * connection)
{
	g_assert(DMAP_IS_CONNECTION (connection));

	return connection->priv->is_connected;
}

typedef struct {
	DmapConnection *connection;
	DmapConnectionFunc callback;
	gpointer user_data;
	GDestroyNotify destroy;
} ConnectionResponseData;

static void
_connection_response_data_free (gpointer data)
{
	ConnectionResponseData *rdata = data;

	g_object_unref (rdata->connection);
	g_free (rdata);
}

static void
_connected_cb (DmapConnection * connection, ConnectionResponseData * rdata)
{
	gboolean result;

	g_debug ("Connected callback");

	connection->priv->is_connecting = FALSE;

	g_signal_handlers_disconnect_by_func (connection,
					      G_CALLBACK (_connected_cb),
					      rdata);

	/* if connected then we succeeded */
	result = connection->priv->is_connected;

	if (rdata->callback) {
		rdata->callback (rdata->connection,
				 result,
				 rdata->connection->priv->last_error_message,
				 rdata->user_data);
	}

	if (rdata->destroy) {
		rdata->destroy (rdata);
	}
}

void
dmap_connection_setup (DmapConnection * connection)
{
	connection->priv->session = soup_session_new ();

	connection->priv->base_uri = g_uri_build(
		G_URI_FLAGS_NONE,
		"http",
		NULL,
		connection->priv->host,
		connection->priv->port,
		"",
		NULL,
		NULL);
}

// FIXME: it would be nice if this mirrored the use of DmapMdnsBrowser. That is, connect callback handler to a signal.
// This would allow Vala to associate a lambda function with the signal.
void
dmap_connection_start (DmapConnection * connection,
                       DmapConnectionFunc callback, gpointer user_data)
{
	ConnectionResponseData *rdata;

	g_assert(DMAP_IS_CONNECTION (connection));
	g_assert(connection->priv->state == DMAP_GET_INFO);

	g_debug ("Creating new DMAP connection to %s:%d",
		 connection->priv->host, connection->priv->port);

	dmap_connection_setup (connection);

	connection->priv->daap_base_uri =
		g_strdup_printf ("daap://%s:%d", connection->priv->host,
				 connection->priv->port);

	rdata = g_new0 (ConnectionResponseData, 1);
	rdata->connection = g_object_ref (connection);
	rdata->callback = callback;
	rdata->user_data = user_data;
	rdata->destroy = _connection_response_data_free;
	g_signal_connect (connection, "operation-done",
			  G_CALLBACK (_connected_cb), rdata);

	if (connection->priv->do_something_id != 0) {
		g_source_remove (connection->priv->do_something_id);
	}

	connection->priv->is_connecting = TRUE;
	connection->priv->do_something_id =
		g_idle_add ((GSourceFunc) _do_something, connection);
}

static void
_disconnected_cb (DmapConnection * connection, ConnectionResponseData * rdata)
{
	gboolean result;

	g_debug ("Disconnected callback");

	g_signal_handlers_disconnect_by_func (connection,
					      G_CALLBACK (_disconnected_cb),
					      rdata);

	/* if not connected then we succeeded */
	result = !connection->priv->is_connected;

	if (rdata->callback) {
		rdata->callback (rdata->connection,
				 result,
				 rdata->connection->priv->last_error_message,
				 (gpointer) rdata->user_data);
	}

	if (rdata->destroy) {
		rdata->destroy (rdata);
	}
}

void
dmap_connection_stop(DmapConnection * connection,
                     DmapConnectionFunc callback,
                     gpointer user_data)
{
	DmapConnectionPrivate *priv = connection->priv;
	ConnectionResponseData *rdata;

	g_assert(DMAP_IS_CONNECTION (connection));

	g_debug ("Disconnecting");

	if (connection->priv->is_connecting) {
		/* this is a special case where the async connection
		 * hasn't returned yet so we need to force the connection
		 * to finish */
		priv->state = DMAP_DONE;
		// FIXME: GDK_THREADS_LEAVE ();
		_finish (connection);
		// FIXME: GDK_THREADS_ENTER ();
	}

	rdata = g_new0 (ConnectionResponseData, 1);
	rdata->connection = g_object_ref (connection);
	rdata->callback = callback;
	rdata->user_data = user_data;
	rdata->destroy = _connection_response_data_free;

	g_signal_connect (connection, "operation-done",
			  G_CALLBACK (_disconnected_cb), rdata);

	if (priv->do_something_id != 0) {
		g_source_remove (priv->do_something_id);
	}

	if (!connection->priv->is_connected) {
		priv->state = DMAP_DONE;
		// FIXME: GDK_THREADS_LEAVE ();
		_finish (connection);
		// FIXME: GDK_THREADS_ENTER ();
	} else {
		priv->state = DMAP_LOGOUT;

		priv->do_something_id = g_idle_add ((GSourceFunc) _do_something,
						    connection);
	}
}

SoupMessageHeaders *
dmap_connection_get_headers (DmapConnection * connection, const gchar * uri)
{
	DmapConnectionPrivate *priv = connection->priv;
	SoupMessageHeaders *headers = NULL;
	char hash[33] = { 0 };
	char *norb_daap_uri = (char *) uri;
	char *request_id;

	priv->request_id++;

	if (g_ascii_strncasecmp (uri, "daap://", 7) == 0) {
		norb_daap_uri = strstr (uri, "/data");
	}

	dmap_md5_generate ((short) floorf (priv->dmap_version),
			    (const guchar *) norb_daap_uri, 2,
			    (guchar *) hash, priv->request_id);

	headers = soup_message_headers_new (SOUP_MESSAGE_HEADERS_REQUEST);

	soup_message_headers_append (headers, "Accept", "*/*");
	soup_message_headers_append (headers, "Cache-Control", "no-cache");
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

	return headers;
}

// FIXME: unify this with share API? Build Container DB?
GSList *
dmap_connection_get_playlists (DmapConnection * connection)
{
	return connection->priv->playlists;
}

void
dmap_connection_emit_error(DmapConnection *connection, gint code,
                           const gchar *format, ...)
{
	va_list ap;
	GError *error;

	va_start(ap, format);
	error = g_error_new_valist(DMAP_ERROR, code, format, ap);
	g_signal_emit_by_name(connection, "error", error);

	va_end(ap);
}

#ifdef HAVE_CHECK

#include <check.h>
#include <libdmapsharing/dmap-av-connection.h>

static int _status = DMAP_STATUS_OK;

static void
_error_cb(G_GNUC_UNUSED DmapConnection *connection, GError *error,
          G_GNUC_UNUSED gpointer user_data)
{
	_status = error->code;
}

#define _ACTUAL_HTTP_RESPONSE_HANDLER_TEST(bytes, size, __status) \
{ \
	DmapConnection *connection; \
	DmapResponseData  *data; \
	\
	_status = DMAP_STATUS_OK; \
	\
	connection = g_object_new(DMAP_TYPE_AV_CONNECTION, NULL); \
	g_signal_connect(connection, "error", G_CALLBACK(_error_cb), NULL); \
	\
	data = g_new0(DmapResponseData, 1); \
	data->body    = g_bytes_new(bytes, sizeof(bytes)); \
	data->status     = SOUP_STATUS_OK; \
	data->connection = connection; \
	data->message_path = g_strdup("/"); \
	data->headers = NULL; \
	\
	_actual_http_response_handler(data); \
	\
	ck_assert(_status == __status); \
} \

START_TEST(_actual_http_response_handler_test) \
_ACTUAL_HTTP_RESPONSE_HANDLER_TEST("minm\x00\x00\x00\x0eHello, world!",
                                    sizeof bytes, DMAP_STATUS_OK);
END_TEST

/* Length < 8 only allowed for DMAP_RAW. */
START_TEST(_actual_http_response_handler_too_short_test) \
_ACTUAL_HTTP_RESPONSE_HANDLER_TEST("xxxx", sizeof bytes,
                                    DMAP_STATUS_RESPONSE_TOO_SHORT);
END_TEST

START_TEST(_actual_http_response_handler_bad_cc_test) \
_ACTUAL_HTTP_RESPONSE_HANDLER_TEST("xxxx\x00\x00\x00\x00", sizeof bytes,
                                    DMAP_STATUS_INVALID_CONTENT_CODE);
END_TEST

/* Length of 99 is larger than sizeof containing array. */
START_TEST(_actual_http_response_handler_bad_len_test) \
_ACTUAL_HTTP_RESPONSE_HANDLER_TEST("minm\x00\x00\x00\x99Hello, world!",
                                    sizeof bytes,
                                    DMAP_STATUS_INVALID_CONTENT_CODE_SIZE);
END_TEST

#include "dmap-connection-suite.c"

#endif
