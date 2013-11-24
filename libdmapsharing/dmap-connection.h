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

#ifndef __DMAP_CONNECTION_H
#define __DMAP_CONNECTION_H

#include <glib.h>
#include <glib-object.h>
#include <libsoup/soup.h>

#include <libdmapsharing/dmap-structure.h>
#include <libdmapsharing/dmap-db.h>
#include <libdmapsharing/dmap-record-factory.h>

G_BEGIN_DECLS typedef struct
{
	char *name;
	int id;
	GList *uris;
} DMAPPlaylist;

/**
 * DMAP_TYPE_CONNECTION:
 *
 * The type for #DMAPConnection.
 */
#define DMAP_TYPE_CONNECTION		(dmap_connection_get_type ())
/**
 * DMAP_CONNECTION:
 * @o: Object which is subject to casting.
 *
 * Casts a #DMAPConnection or derived pointer into a (DMAPConnection *) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DMAP_CONNECTION(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), DMAP_TYPE_CONNECTION, DMAPConnection))
/**
 * DMAP_CONNECTION_CLASS:
 * @k: a valid #DMAPConnectionClass
 *
 * Casts a derived #DMAPConnectionClass structure into a #DMAPConnectionClass
 * structure.
 */
#define DMAP_CONNECTION_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), DMAP_TYPE_CONNECTION, DMAPConnectionClass))
/**
 * IS_DMAP_CONNECTION:
 * @o: Instance to check for being a %DMAP_TYPE_CONNECTION.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %DMAP_TYPE_CONNECTION.
 */
#define IS_DMAP_CONNECTION(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), DMAP_TYPE_CONNECTION))
/**
 * IS_DMAP_CONNECTION_CLASS:
 * @k: a #DMAPConnectionClass
 *
 * Checks whether @k "is a" valid #DMAPConnectionClass structure of type
 * %DMAP_CONNECTION or derived.
 */
#define IS_DMAP_CONNECTION_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), DMAP_TYPE_CONNECTION))
/**
 * DMAP_CONNECTION_GET_CLASS:
 * @o: a #DMAPConnection instance.
 *
 * Get the class structure associated to a #DMAPConnection instance.
 *
 * Returns: pointer to object class structure.
 */
#define DMAP_CONNECTION_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), DMAP_TYPE_CONNECTION, DMAPConnectionClass))

typedef struct DMAPConnectionPrivate DMAPConnectionPrivate;

typedef enum
{
	DMAP_GET_INFO = 0,
	DMAP_LOGIN,
	DMAP_GET_REVISION_NUMBER,
	DMAP_GET_DB_INFO,
	DMAP_GET_SONGS,
	DMAP_GET_PLAYLISTS,
	DMAP_GET_PLAYLIST_ENTRIES,
	DMAP_LOGOUT,
	DMAP_DONE
} DMAPConnectionState;

typedef struct
{
	GObject parent;
	DMAPConnectionPrivate *priv;
} DMAPConnection;

typedef struct
{
	GObjectClass parent;

	/* Pure virtual methods: */
	  DMAPContentCode (*get_protocol_version_cc) (DMAPConnection *
						      connection);
	gchar *(*get_query_metadata) (DMAPConnection * connection);
	DMAPRecord *(*handle_mlcl) (DMAPConnection * connection,
				    DMAPRecordFactory * factory, GNode * mlcl,
				    gint * item_id);

	SoupMessage *(*build_message)
	 
		(DMAPConnection * connection,
	   const gchar * path,
	   gboolean need_hash,
	   gdouble version, gint req_id, gboolean send_close);
	void (*connected) (DMAPConnection * connection);
	void (*disconnected) (DMAPConnection * connection);

	char *(*authenticate) (DMAPConnection * connection, const char *name);
	void (*connecting) (DMAPConnection * connection,
			    DMAPConnectionState state, float progress);

	void (*operation_done) (DMAPConnection * connection);

} DMAPConnectionClass;

/* hmm, maybe should give more error information? */
typedef gboolean (*DMAPConnectionCallback) (DMAPConnection * connection,
					    gboolean result,
					    const char *reason,
					    gpointer user_data);

typedef void (*DMAPResponseHandler) (DMAPConnection * connection,
				     guint status,
				     GNode * structure, gpointer user_data);

GType dmap_connection_get_type (void);

gboolean dmap_connection_is_connected (DMAPConnection * connection);
void dmap_connection_setup (DMAPConnection * connection);
void dmap_connection_connect (DMAPConnection * connection,
			      DMAPConnectionCallback callback,
			      gpointer user_data);
void dmap_connection_disconnect (DMAPConnection * connection,
				 DMAPConnectionCallback callback,
				 gpointer user_data);

SoupMessageHeaders *dmap_connection_get_headers (DMAPConnection * connection,
						 const char *uri);

GSList *dmap_connection_get_playlists (DMAPConnection * connection);

SoupMessage *dmap_connection_build_message (DMAPConnection * connection,
					    const gchar * path,
					    gboolean need_hash,
					    gdouble version,
					    gint req_id, gboolean send_close);

/**
 * dmap_connection_authenticate_message:
 * @connection: A #DMAPConnection
 * @session: A #SoupSession
 * @message: A #SoupMessage
 * @auth: A #SoupAuth
 * @password: A password
 *     
 * Attach an authentication credential to a request. This
 * method should be called by a function that is connected to the
 * #DMAPConnection::authenticate signal. The signal will provide the
 * connection, session, message and auth to that function. That function
 * should obtain a password and provide it to this method.
 */
void dmap_connection_authenticate_message (DMAPConnection *connection,
                                           SoupSession *session,
                                           SoupMessage *message,
					   SoupAuth *auth,
					   const char *password);


gboolean dmap_connection_get (DMAPConnection * self,
			      const gchar * path,
			      gboolean need_hash,
			      DMAPResponseHandler handler,
			      gpointer user_data);

G_END_DECLS
#endif /* __DMAP_CONNECTION_H */
