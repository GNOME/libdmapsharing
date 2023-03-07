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

#ifndef _DMAP_CONNECTION_H
#define _DMAP_CONNECTION_H

#include <glib.h>
#include <glib-object.h>
#include <libsoup/soup.h>

#include <libdmapsharing/dmap-cc.h>
#include <libdmapsharing/dmap-db.h>
#include <libdmapsharing/dmap-record-factory.h>

G_BEGIN_DECLS

/**
 * SECTION: dmap-connection
 * @short_description: An abstract parent to the various connection classes.
 *
 * #DmapConnection provides an abstract parent to the #DmapAvConnection, #DmapControlConnection, and #DmapImageConnection classes.
 */

typedef struct {
	char *name;
	int id;
	GList *uris;
} DmapPlaylist;

/**
 * DMAP_TYPE_CONNECTION:
 *
 * The type for #DmapConnection.
 */
#define DMAP_TYPE_CONNECTION		(dmap_connection_get_type ())
/**
 * DMAP_CONNECTION:
 * @o: Object which is subject to casting.
 *
 * Casts a #DmapConnection or derived pointer into a (DmapConnection *) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DMAP_CONNECTION(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), DMAP_TYPE_CONNECTION, DmapConnection))
/**
 * DMAP_CONNECTION_CLASS:
 * @k: a valid #DmapConnectionClass
 *
 * Casts a derived #DmapConnectionClass structure into a #DmapConnectionClass
 * structure.
 */
#define DMAP_CONNECTION_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), DMAP_TYPE_CONNECTION, DmapConnectionClass))
/**
 * DMAP_IS_CONNECTION:
 * @o: Instance to check for being a %DMAP_TYPE_CONNECTION.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %DMAP_TYPE_CONNECTION.
 */
#define DMAP_IS_CONNECTION(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), DMAP_TYPE_CONNECTION))
/**
 * DMAP_IS_CONNECTION_CLASS:
 * @k: a #DmapConnectionClass
 *
 * Checks whether @k "is a" valid #DmapConnectionClass structure of type
 * %DMAP_CONNECTION or derived.
 */
#define DMAP_IS_CONNECTION_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), DMAP_TYPE_CONNECTION))
/**
 * DMAP_CONNECTION_GET_CLASS:
 * @o: a #DmapConnection instance.
 *
 * Get the class structure associated to a #DmapConnection instance.
 *
 * Returns: pointer to object class structure.
 */
#define DMAP_CONNECTION_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), DMAP_TYPE_CONNECTION, DmapConnectionClass))

typedef struct DmapConnectionPrivate DmapConnectionPrivate;

/**
 * DmapConnectionState:
 * @DMAP_GET_INFO: getting DMAP server information
 * @DMAP_LOGIN: logging in to DMAP server
 * @DMAP_GET_REVISION_NUMBER: getting server's database revision number
 * @DMAP_GET_DB_INFO: getting DMAP database information
 * @DMAP_GET_MEDIA: getting DMAP media listing
 * @DMAP_GET_PLAYLISTS: getting DMAP playlists
 * @DMAP_GET_PLAYLIST_ENTRIES: getting entries in playlist
 * @DMAP_LOGOUT: logging out of DMAP server
 * @DMAP_DONE: done
 *
 * Enumeration of connection states.
 */
typedef enum {
	DMAP_GET_INFO = 0,
	DMAP_LOGIN,
	DMAP_GET_REVISION_NUMBER,
	DMAP_GET_DB_INFO,
	DMAP_GET_MEDIA,
	DMAP_GET_PLAYLISTS,
	DMAP_GET_PLAYLIST_ENTRIES,
	DMAP_LOGOUT,
	DMAP_DONE
} DmapConnectionState;

typedef struct {
	GObject parent;
	DmapConnectionPrivate *priv;
} DmapConnection;

typedef struct {
	GObjectClass parent;

	/* Pure virtual methods: */
	  DmapContentCode (*get_protocol_version_cc) (DmapConnection *
						      connection);
	gchar *(*get_query_metadata) (DmapConnection * connection);
	DmapRecord *(*handle_mlcl) (DmapConnection * connection,
				    DmapRecordFactory * factory, GNode * mlcl,
				    gint * item_id);

	SoupMessage *(*build_message) (DmapConnection * connection,
	                               const gchar * path,
	                               gboolean need_hash,
	                               gdouble version,
	                               gint req_id,
	                               gboolean send_close);
	void (*connected) (DmapConnection * connection);
	void (*disconnected) (DmapConnection * connection);

	char *(*authenticate) (DmapConnection * connection, const char *name);
	void (*connecting) (DmapConnection * connection,
			    DmapConnectionState state, float progress);

	void (*operation_done) (DmapConnection * connection);

} DmapConnectionClass;

/* hmm, maybe should give more error information? */
typedef void (*DmapConnectionFunc) (DmapConnection * connection,
                                    gboolean result,
                                    const char *reason,
                                    gpointer user_data);

GType dmap_connection_get_type (void);

/**
 * dmap_connection_is_connected:
 * @connection: The connection.
 *
 * Returns TRUE if the connection is presently connected.
 */
gboolean dmap_connection_is_connected (DmapConnection * connection);

/**
 * dmap_connection_start:
 * @connection: The connection.
 * @callback: (scope async): The function to call once the connection is complete.
 * @user_data: The data to pass to the callback.
 *
 * Connect to the remote DMAP share.
 */
void dmap_connection_start (DmapConnection * connection,
			    DmapConnectionFunc callback,
			    gpointer user_data);

/**
 * dmap_connection_stop:
 * @connection: The connection.
 * @callback: (scope async): The function to call once the connection is complete.
 * @user_data: The data to pass to the callback.
 *
 * Disconnect from the remote DMAP share.
 */
void dmap_connection_stop(DmapConnection * connection,
                          DmapConnectionFunc callback,
                          gpointer user_data);

SoupMessageHeaders *dmap_connection_get_headers (DmapConnection * connection,
						 const char *uri);

/**
 * dmap_connection_get_playlists:
 * @connection: A #DmapConnection
 *
 * Get the playlists associated with a #DmapConnection instance.
 *
 * Returns: (element-type DmapPlaylist) (transfer none): pointer to a list of playlists.
 */
GSList *dmap_connection_get_playlists (DmapConnection * connection);

/**
 * dmap_connection_authenticate_message:
 * @connection: A #DmapConnection
 * @session: A #SoupSession
 * @message: A #SoupMessage
 * @auth: A #SoupAuth
 * @password: A password
 *
 * Attach an authentication credential to a request. This
 * method should be called by a function that is connected to the
 * #DmapConnection::authenticate signal. The signal will provide the
 * connection, session, message and auth to that function. That function
 * should obtain a password and provide it to this method.
 */
void dmap_connection_authenticate_message (DmapConnection *connection,
                                           SoupSession *session,
                                           SoupMessage *message,
					   SoupAuth *auth,
					   const char *password);

/**
 * dmap_connection_emit_error:
 * @connection: a #DmapConnection instance.
 * @code: error code.
 * @format: printf()-style format for error message
 * @...: parameters for message format
 */
void dmap_connection_emit_error(DmapConnection *connection, gint code,
                                const gchar *format, ...);

G_END_DECLS
#endif /* _DMAP_CONNECTION_H */
