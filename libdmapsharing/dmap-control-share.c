/* Implmentation of DACP (e.g., iTunes Remote) sharing
 *
 * Copyright (C) 2010 Alexandre Rosenfeld <airmind@gmail.com>
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
 *
 */

#include "config.h"

#include <time.h>
#include <string.h>
#include <stdlib.h>

#include <glib.h>
#include <glib-object.h>

#ifdef HAVE_GDKPIXBUF
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif /* HAVE_GDKPIXBUF */

#include <libsoup/soup.h>

#include <libdmapsharing/dmap.h>
#include <libdmapsharing/dmap-structure.h>
#include <libdmapsharing/dmap-share-private.h>

#include <libdmapsharing/dmap-share.h>
#include <libdmapsharing/dmap-connection-private.h>
#include <libdmapsharing/dmap-control-share.h>
#include <libdmapsharing/dmap-control-connection.h>
#include <libdmapsharing/dmap-control-player.h>

void dmap_control_share_ctrl_int (DmapShare * share,
			  SoupServerMessage * message,
			  const char *path,
			  GHashTable * query);
void dmap_control_share_login (DmapShare * share,
		       SoupServerMessage * message,
		       const char *path,
		       GHashTable * query);

#define DACP_TYPE_OF_SERVICE "_touch-able._tcp"
#define DACP_PORT 3689

struct DmapControlSharePrivate
{
	DmapMdnsBrowser *mdns_browser;

	gchar *library_name;
	GHashTable *remotes;

	gint current_revision;

	GSList *update_queue;

	DmapControlPlayer *player;
};

/*
 * Internal representation of a DACP remote.
 */
typedef struct {
	gchar *host;
	guint port;
	gchar *pair_txt;
	DmapConnection *connection;
} DACPRemoteInfo;

enum {
	PROP_0,
	PROP_LIBRARY_NAME,
	PROP_PLAYER
};

enum {
	REMOTE_FOUND,
	REMOTE_LOST,
	REMOTE_PAIRED,

	LOOKUP_GUID,
	ADD_GUID,

	LAST_SIGNAL
};

static guint _signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE_WITH_PRIVATE (DmapControlShare,
                            dmap_control_share,
                            DMAP_TYPE_SHARE);

static gchar *
_get_dbid (void)
{
	static gchar *dbid;

	if (!dbid) {
		GString *name;

		// Creates a service name 14 characters long concatenating the hostname
		// hash hex value with itself.
		// Idea taken from stereo.
		name = g_string_new (NULL);
		g_string_printf (name, "%.8x",
				 g_str_hash (g_get_host_name ()));
		g_string_ascii_up (name);
		g_string_append_len (name, name->str, 4);

		dbid = g_string_free (name, FALSE);
	}
	return dbid;
}

static void
_update_txt_records (DmapControlShare * share)
{
	gchar *dbid_record;
	gchar *library_name_record;

	library_name_record =
		g_strdup_printf ("CtlN=%s", share->priv->library_name);
	dbid_record = g_strdup_printf ("DbId=%s", _get_dbid ());

	gchar *txt_records[] = { "Ver=131073",
		"DvSv=2049",
		dbid_record,
		"DvTy=iTunes",
		"OSsi=0x1F6",
		"txtvers=1",
		library_name_record,
		NULL
	};

	g_object_set (share, "txt-records", txt_records, NULL);

	g_free (dbid_record);
	g_free (library_name_record);
}

static void
_set_property (GObject * object,
               guint prop_id,
               const GValue * value, GParamSpec * pspec)
{
	DmapControlShare *share = DMAP_CONTROL_SHARE (object);

	switch (prop_id) {
	case PROP_LIBRARY_NAME:
		g_free (share->priv->library_name);
		share->priv->library_name = g_value_dup_string (value);
		_update_txt_records (share);
		break;
	case PROP_PLAYER:
		if (share->priv->player) {
			g_object_unref (share->priv->player);
		}
		share->priv->player = DMAP_CONTROL_PLAYER (g_value_dup_object (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
_get_property (GObject * object,
               guint prop_id, GValue * value, GParamSpec * pspec)
{
	DmapControlShare *share = DMAP_CONTROL_SHARE (object);

	switch (prop_id) {
	case PROP_LIBRARY_NAME:
		g_value_set_string (value, share->priv->library_name);
		break;
	case PROP_PLAYER:
		g_value_set_object (value, G_OBJECT (share->priv->player));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
_dispose (GObject * object)
{
	DmapControlShare *share = DMAP_CONTROL_SHARE (object);

	g_clear_object(&share->priv->mdns_browser);
	g_clear_object(&share->priv->player);

	if (NULL != share->priv->update_queue) {
		g_slist_free_full (share->priv->update_queue, g_object_unref);
		share->priv->update_queue = NULL;
	}

	if (NULL != share->priv->remotes) {
		g_hash_table_destroy (share->priv->remotes);
		share->priv->remotes = NULL;
	}

	G_OBJECT_CLASS (dmap_control_share_parent_class)->dispose (object);
}

static void
_finalize (GObject * object)
{
	DmapControlShare *share = DMAP_CONTROL_SHARE (object);

	g_free (share->priv->library_name);

	G_OBJECT_CLASS (dmap_control_share_parent_class)->finalize (object);
}

static const char *
_get_type_of_service (G_GNUC_UNUSED DmapShare * share)
{
	return DACP_TYPE_OF_SERVICE;
}

static void
dmap_control_share_class_init (DmapControlShareClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	DmapShareClass *dmap_class = DMAP_SHARE_CLASS (object_class);

	object_class->get_property = _get_property;
	object_class->set_property = _set_property;
	object_class->dispose = _dispose;
	object_class->finalize = _finalize;

	dmap_class->get_type_of_service = _get_type_of_service;
	dmap_class->ctrl_int = dmap_control_share_ctrl_int;
	dmap_class->login = dmap_control_share_login;

	g_object_class_install_property (object_class,
					 PROP_LIBRARY_NAME,
					 g_param_spec_string ("library-name",
							      "Library Name",
							      "Library name as will be shown in the Remote",
							      NULL,
							      G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
					 PROP_PLAYER,
					 g_param_spec_object ("player",
							      "Player",
							      "Player",
							      G_TYPE_OBJECT,
							      G_PARAM_READWRITE
							      |
							      G_PARAM_CONSTRUCT_ONLY));

	/**
	 * DmapControlShare::remote-found
	 * @share: the #DmapControlShare that received the signal.
	 * @service_name: the remote identifier.
	 * @remote_name: the remote friendly name.
	 *
	 * Signal emited when a remote is found in the local network.
	 */
	_signals[REMOTE_FOUND] =
		g_signal_new ("remote-found",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (DmapControlShareClass, remote_found),
			      NULL,
			      NULL,
			      NULL,
			      G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);

	/**
	 * DmapControlShare::remote-lost
	 * @share: the #DmapControlShare that received the signal
	 * @service_name: the remote identifier.
	 *
	 * Signal emited when a remote is lost in the local network.
	 */
	_signals[REMOTE_LOST] =
		g_signal_new ("remote-lost",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (DmapControlShareClass, remote_lost),
			      NULL,
			      NULL,
			      NULL,
			      G_TYPE_NONE, 1, G_TYPE_STRING);

	/**
	 * DmapControlShare::remote-paired
	 * @share: the #DmapControlShare that received the signal
	 * @service_name: the remote identifier.
	 * @connected: indicates if the connection was succesfull or not.
	 *
	 * Signal emited when a remote is paired.
	 */
	_signals[REMOTE_PAIRED] =
		g_signal_new ("remote-paired",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (DmapControlShareClass, remote_paired),
			      NULL,
			      NULL,
			      NULL,
			      G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_BOOLEAN);

	/**
	 * DmapControlShare::lookup-guid
	 * @share: the #DmapControlShare that received the signal
	 * @guid: a string containing the guid to be validated.
	 *
	 * Signal emited when the remote has logged in before and wants to be
	 * validated. An implementation must implement this signal to lookup
	 * for guids saved by ::add-guid
	 */
	_signals[LOOKUP_GUID] =
		g_signal_new ("lookup-guid",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (DmapControlShareClass, lookup_guid),
			      NULL,
			      NULL,
			      NULL,
			      G_TYPE_BOOLEAN, 1, G_TYPE_STRING);

	/**
	 * DmapControlShare::add-guid
	 * @share: the #DmapControlShare that received the signal
	 * @guid: a string containing the guid to be saved.
	 *
	 * Signal emited when the remote wants to log in and save a special guid
	 * which will be used later when it wants to reconnect. With this guid,
	 * we know that this remote has connected before, thus this signal must
	 * save somewhere all guids that connected before, so that ::lookup-guid
	 * will find this remote. The user interface probably wants to include
	 * a button to forget previously connected remotes, so that the user may
	 * disconnect all previously connected remotes.
	 */
	_signals[ADD_GUID] =
		g_signal_new ("add-guid",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (DmapControlShareClass, add_guid),
			      NULL,
			      NULL,
			      NULL,
			      G_TYPE_NONE, 1, G_TYPE_STRING);
}

static void
dmap_control_share_init (DmapControlShare * share)
{
	share->priv = dmap_control_share_get_instance_private(share);

	share->priv->current_revision = 2;

	share->priv->remotes = g_hash_table_new_full ((GHashFunc) g_str_hash,
						      (GEqualFunc)
						      g_str_equal,
						      (GDestroyNotify) g_free,
						      (GDestroyNotify)
						      g_free);
}

static void
_mdns_remote_added (G_GNUC_UNUSED DmapMdnsBrowser * browser,
                    DmapMdnsService * service, DmapControlShare * share)
{
	gboolean new;
	guint port;
	DACPRemoteInfo *remote_info;
	gchar *service_name, *name, *host, *pair;

	g_object_get (service, "service-name", &service_name,
	                       "name", &name,
	                       "host", &host,
	                       "port", &port,
	                       "pair", &pair, NULL);

	remote_info = g_new (DACPRemoteInfo, 1);
	remote_info->host = g_strdup (host);
	remote_info->port = port;
	remote_info->connection = NULL;
	remote_info->pair_txt = g_strdup (pair);

	g_debug ("New Remote found: %s name=%s host=%s port=%u pair=%s",
		 service_name,
		 name,
		 remote_info->host, remote_info->port, remote_info->pair_txt);

	new = g_hash_table_insert (share->priv->remotes,
	                           service_name, remote_info);
	g_assert(new);

	g_signal_emit (share, _signals[REMOTE_FOUND], 0, service_name, name);

	g_free(name);
	g_free(host);
	g_free(pair);
}

static void
_mdns_remote_removed (G_GNUC_UNUSED DmapMdnsBrowser * browser,
                      const char *service_name, DmapControlShare * share)
{
	gboolean found;
	g_signal_emit (share, _signals[REMOTE_LOST], 0, service_name);

	found = g_hash_table_remove (share->priv->remotes, service_name);
	g_assert(found);
}

DmapControlShare *
dmap_control_share_new (const gchar *library_name,
                        DmapControlPlayer *player,
                        DmapDb *db,
                        DmapContainerDb *container_db)
{
	g_object_ref (db);
	g_object_ref (container_db);

	return DMAP_CONTROL_SHARE (g_object_new (DMAP_TYPE_CONTROL_SHARE,
	                                        "name", _get_dbid (),
	                                        "library-name", library_name,
	                                        "password", NULL,
	                                        "db", db,
	                                        "container-db", container_db,
	                                        "player", G_OBJECT (player),
	                                        "transcode-mimetype", NULL, NULL));
}

gboolean
dmap_control_share_start_lookup (DmapControlShare * share, GError **error)
{
	g_assert(NULL == share->priv->mdns_browser);

	share->priv->mdns_browser =
		dmap_mdns_browser_new (DMAP_MDNS_SERVICE_TYPE_DACP);

	g_signal_connect_object (share->priv->mdns_browser,
				 "service-added",
				 G_CALLBACK (_mdns_remote_added), share, 0);
	g_signal_connect_object (share->priv->mdns_browser,
				 "service-removed",
				 G_CALLBACK (_mdns_remote_removed), share, 0);

	return dmap_mdns_browser_start (share->priv->mdns_browser, error);
}

static gboolean
_remove_remotes_cb (gpointer service_name, G_GNUC_UNUSED gpointer remote_info,
                    gpointer share)
{
	g_signal_emit ((DmapControlShare *) share,
		       _signals[REMOTE_LOST], 0, (gchar *) service_name);
	return TRUE;
}

gboolean
dmap_control_share_stop_lookup (DmapControlShare * share, GError **error)
{
	gboolean ok = FALSE;

	g_assert(NULL != share->priv->mdns_browser);

	g_hash_table_foreach_remove (share->priv->remotes,	
                                     _remove_remotes_cb,
	                             share);

	ok = dmap_mdns_browser_stop (share->priv->mdns_browser, error);
	if (!ok) {
		goto done;
	}

	share->priv->mdns_browser = NULL;

	ok = TRUE;

done:
	return ok;
}

static void
_fill_playstatusupdate (DmapControlShare * share, SoupServerMessage * message)
{
	GNode *cmst;
	DmapAvRecord *record;
	DmapControlPlayState play_state;
	DmapControlRepeatState repeat_state;
	gboolean shuffle_state;
	guint playing_time;

	g_object_get (share->priv->player,
		      "play-state", &play_state,
		      "repeat-state", &repeat_state,
		      "shuffle-state", &shuffle_state,
		      "playing-time", &playing_time, NULL);

	record = dmap_control_player_now_playing_record (share->priv->player);

	cmst = dmap_structure_add (NULL, DMAP_CC_CMST);
	dmap_structure_add (cmst, DMAP_CC_MSTT, (gint32) SOUP_STATUS_OK);
	dmap_structure_add (cmst, DMAP_CC_CMSR,
			    share->priv->current_revision);
	dmap_structure_add (cmst, DMAP_CC_CAVC, 1);
	dmap_structure_add (cmst, DMAP_CC_CAPS, (gint32) play_state);
	dmap_structure_add (cmst, DMAP_CC_CASH, shuffle_state ? 1 : 0);
	dmap_structure_add (cmst, DMAP_CC_CARP, (gint32) repeat_state);
	if (record) {
		gchar *title;
		gchar *artist;
		gchar *album;
		gint duration;
		guint track_time;

		g_object_get (record,
			      "title", &title,
			      "songartist", &artist,
			      "songalbum", &album,
			      "duration", &duration, NULL);
		track_time = duration * 1000;
		dmap_structure_add (cmst, DMAP_CC_CAAS, 2);
		dmap_structure_add (cmst, DMAP_CC_CAAR, 6);
		dmap_structure_add (cmst, DMAP_CC_CANP, (gint64) 0); // FIXME: may be wrong.
		if (title) {
			dmap_structure_add (cmst, DMAP_CC_CANN, title);
		}
		if (artist) {
			dmap_structure_add (cmst, DMAP_CC_CANA, artist);
		}
		if (album) {
			dmap_structure_add (cmst, DMAP_CC_CANL, album);
		}
		dmap_structure_add (cmst, DMAP_CC_CANG, "");
		dmap_structure_add (cmst, DMAP_CC_ASAI, 0);
		//dmap_structure_add (cmst, DMAP_CC_AEMK, 1);
		g_debug ("Playing time: %u, Track time: %u", playing_time,
			 track_time);
		dmap_structure_add (cmst, DMAP_CC_CANT,
				    track_time - playing_time);
		dmap_structure_add (cmst, DMAP_CC_CAST, track_time);

		g_free (title);
		g_free (artist);
		g_free (album);

		g_object_unref (record);
	}

	dmap_share_message_set_from_dmap_structure (DMAP_SHARE (share),
						     message, cmst);
	dmap_structure_destroy (cmst);
}

static void
_send_playstatusupdate (DmapControlShare * share)
{
	GSList *list;
	SoupServer *server = NULL;

	g_object_get (share, "server", &server, NULL);
	if (server) {
		for (list = share->priv->update_queue; list;
		     list = list->next) {
			_fill_playstatusupdate (share,
			                       (SoupServerMessage*) list->data);
			soup_server_message_unpause ((SoupServerMessage*) list->data);
		}
		g_object_unref (server);
	}
	g_slist_free (share->priv->update_queue);
	share->priv->update_queue = NULL;
}

void
dmap_control_share_player_updated (DmapControlShare * share)
{
	share->priv->current_revision++;
	_send_playstatusupdate (share);
}

static void
_status_update_message_finished (SoupServerMessage * message, DmapControlShare * share)
{
	share->priv->update_queue =
		g_slist_remove (share->priv->update_queue, message);
	g_object_unref (message);
}

static void
_debug_param (gpointer key, gpointer val, G_GNUC_UNUSED gpointer user_data)
{
	g_debug ("%s %s", (char *) key, (char *) val);
}

void
dmap_control_share_login (DmapShare * share,
		  SoupServerMessage * message,
		  const char *path,
		  GHashTable * query)
{
	gchar *pairing_guid;

	g_debug ("Path is %s.", path);
	if (query) {
		g_hash_table_foreach (query, _debug_param, NULL);
	}

	pairing_guid = g_hash_table_lookup (query, "pairing-guid");

	if (pairing_guid != NULL) {
		gboolean allow_login;

		g_signal_emit (share, _signals[LOOKUP_GUID], 0, pairing_guid,
			       &allow_login);

		if (!allow_login) {
			g_warning("Unknown remote trying to connect");
			soup_server_message_set_status (message, SOUP_STATUS_FORBIDDEN, NULL);
			return;
		}
	}

	dmap_share_login (share, message, path, query);
}

void
dmap_control_share_ctrl_int (DmapShare * share,
                             SoupServerMessage * message,
                             const char *path,
                             GHashTable * query)
{
	const char *rest_of_path;

	DmapControlShare *dmap_control_share = DMAP_CONTROL_SHARE (share);

	g_debug ("Path is %s.", path);
	if (query) {
		g_hash_table_foreach (query, _debug_param, NULL);
	}

	rest_of_path = strchr (path + 1, '/');

	/* If calling /ctrl-int without args, the client doesnt need a 
	 * session-id, otherwise it does and it should be validated. */
	if ((rest_of_path != NULL)
	    &&
	    (!dmap_share_session_id_validate
	     (share, message, query, NULL))) {
		soup_server_message_set_status (message, SOUP_STATUS_FORBIDDEN, NULL);
		goto done;
	}

	if (rest_of_path == NULL) {
		/* CACI control-int
		 *      MSTT status
		 *      MUTY update type
		 *      MTCO specified total count
		 *      MRCO returned count
		 *      MLCL listing
		 *              MLIT listing item
		 *                      MIID item id
		 *                      CMIK Unknown (TRUE)
		 *                      CMSP Unknown (TRUE)
		 *                      CMSV Unknown (TRUE)
		 *                      CASS Unknown (TRUE)
		 *                      CASU Unknown (TRUE)
		 *                      CASG Unknown (TRUE)
		 */

		GNode *caci;
		GNode *mlcl;
		GNode *mlit;

		// dacp.controlint
		caci = dmap_structure_add (NULL, DMAP_CC_CACI);
		// dmap.status
		dmap_structure_add (caci, DMAP_CC_MSTT,
				    (gint32) SOUP_STATUS_OK);
		// dmap.updatetype
		dmap_structure_add (caci, DMAP_CC_MUTY, 0);
		// dmap.specifiedtotalcount
		dmap_structure_add (caci, DMAP_CC_MTCO, (gint32) 1);
		// dmap.returnedcount
		dmap_structure_add (caci, DMAP_CC_MRCO, (gint32) 1);
		// dmap.listing
		mlcl = dmap_structure_add (caci, DMAP_CC_MLCL);
		// dmap.listingitem
		mlit = dmap_structure_add (mlcl, DMAP_CC_MLIT);
		// dmap.itemid
		dmap_structure_add (mlit, DMAP_CC_MIID, (gint32) 1);
		// Unknown (TRUE)
		dmap_structure_add (mlit, DMAP_CC_CMIK, (gint32) 1);
		// Unknown (TRUE)
		dmap_structure_add (mlit, DMAP_CC_CMPR, (gint32) (2 << 16 | 1));
		dmap_structure_add (mlit, DMAP_CC_CAPR, (gint32) (2 << 16 | 2));
		dmap_structure_add (mlit, DMAP_CC_CMSP, (gint32) 1);
		// Unknown (TRUE)
		dmap_structure_add (mlit, DMAP_CC_AEFR, 0x64);
		dmap_structure_add (mlit, DMAP_CC_CMSV, (gint32) 1);
		// Unknown (TRUE)
		dmap_structure_add (mlit, DMAP_CC_CASS, (gint32) 1);
		// Unknown (TRUE)
		dmap_structure_add (mlit, DMAP_CC_CAOV, 1);
		dmap_structure_add (mlit, DMAP_CC_CASU, (gint32) 1);
		// Unknown (TRUE)
		dmap_structure_add (mlit, DMAP_CC_CASG, (gint32) 1);
		dmap_structure_add (mlit, DMAP_CC_CMRL, 1);

		dmap_share_message_set_from_dmap_structure (share, message,
							     caci);
		dmap_structure_destroy (caci);
	} else if (g_ascii_strcasecmp ("/1/getproperty", rest_of_path) == 0) {
		gchar *properties_query, **properties, **property;
		GNode *cmgt;

		properties_query = g_hash_table_lookup (query, "properties");

		if (!properties_query) {
			g_warning ("No property specified");
			goto done;
		}

		cmgt = dmap_structure_add (NULL, DMAP_CC_CMGT);
		dmap_structure_add (cmgt, DMAP_CC_MSTT, SOUP_STATUS_OK);

		properties = g_strsplit (properties_query, ",", -1);
		for (property = properties; *property; property++) {
			if (g_ascii_strcasecmp (*property, "dmcp.volume") ==
			    0) {
				gulong volume;

				g_object_get (dmap_control_share->priv->player,
					      "volume", &volume, NULL);
				dmap_structure_add (cmgt, DMAP_CC_CMVO,
						    volume);
			} else {
				g_warning ("Unhandled property %s",
					   *property);
			}
		}

		g_strfreev (properties);

		dmap_share_message_set_from_dmap_structure (share, message,
							     cmgt);
		dmap_structure_destroy (cmgt);
	} else if (g_ascii_strcasecmp ("/1/setproperty", rest_of_path) == 0) {
		if (g_hash_table_lookup (query, "dmcp.volume")) {
			gdouble volume =
				strtod (g_hash_table_lookup
					(query, "dmcp.volume"), NULL);
			g_object_set (dmap_control_share->priv->player, "volume",
				      (gulong) volume, NULL);
		}
		soup_server_message_set_status (message, SOUP_STATUS_NO_CONTENT, NULL);
	} else if (g_ascii_strcasecmp ("/1/getspeakers", rest_of_path) == 0) {
		GNode *casp;
		gulong volume;

		casp = dmap_structure_add (NULL, DMAP_CC_CASP);
		dmap_structure_add (casp, DMAP_CC_MSTT,
				    (gint32) SOUP_STATUS_OK);
		dmap_structure_add (casp, DMAP_CC_MDCL);

		dmap_structure_add (casp, DMAP_CC_CAIA, TRUE);
		dmap_structure_add (casp, DMAP_CC_CAHP, 1);
		dmap_structure_add (casp, DMAP_CC_CAIV, 1);
		dmap_structure_add (casp, DMAP_CC_MINM, "Computer");
		dmap_structure_add (casp, DMAP_CC_MSMA, (gint32) 0);

		g_object_get (dmap_control_share->priv->player, "volume", &volume, NULL);
		dmap_structure_add (casp, DMAP_CC_CMVO, volume);

		dmap_share_message_set_from_dmap_structure (share, message,
							     casp);
		dmap_structure_destroy (casp);
	} else if (g_ascii_strcasecmp ("/1/playstatusupdate", rest_of_path) ==
		   0) {
		gchar *revision =
			g_hash_table_lookup (query, "revision-number");
		gint revision_number = atoi (revision);

		if (revision_number >= dmap_control_share->priv->current_revision) {
			g_object_ref (message);
			dmap_control_share->priv->update_queue =
				g_slist_prepend (dmap_control_share->
						 priv->update_queue, message);
			g_signal_connect_object (message, "finished",
						 G_CALLBACK
						 (_status_update_message_finished),
						 dmap_control_share, 0);
			soup_server_message_pause (message);
		} else {
			_fill_playstatusupdate (dmap_control_share, message);
		}
	} else if (g_ascii_strcasecmp ("/1/playpause", rest_of_path) == 0) {
		dmap_control_player_play_pause (dmap_control_share->priv->player);
		soup_server_message_set_status (message, SOUP_STATUS_NO_CONTENT, NULL);
	} else if (g_ascii_strcasecmp ("/1/pause", rest_of_path) == 0) {
		dmap_control_player_pause (dmap_control_share->priv->player);
		soup_server_message_set_status (message, SOUP_STATUS_NO_CONTENT, NULL);
	} else if (g_ascii_strcasecmp ("/1/nextitem", rest_of_path) == 0) {
		dmap_control_player_next_item (dmap_control_share->priv->player);
		soup_server_message_set_status (message, SOUP_STATUS_NO_CONTENT, NULL);
	} else if (g_ascii_strcasecmp ("/1/previtem", rest_of_path) == 0) {
		dmap_control_player_prev_item (dmap_control_share->priv->player);
		soup_server_message_set_status (message, SOUP_STATUS_NO_CONTENT, NULL);
	} else if (g_ascii_strcasecmp ("/1/nowplayingartwork", rest_of_path)
		   == 0) {
		guint width = 320;
		guint height = 320;
		gchar *artwork_filename;
		gchar *buffer;
		gsize buffer_len;

		if (g_hash_table_lookup (query, "mw")) {
			width = atoi (g_hash_table_lookup (query, "mw"));
		}
		if (g_hash_table_lookup (query, "mh")) {
			height = atoi (g_hash_table_lookup (query, "mh"));
		}
		artwork_filename =
			dmap_control_player_now_playing_artwork (dmap_control_share->
			                                         priv->player, width,
		                                                 height);
		if (!artwork_filename) {
			g_debug ("No artwork for currently playing song");
			soup_server_message_set_status (message,
			                                SOUP_STATUS_NOT_FOUND,
			                                NULL);
			goto done;
		}
#ifdef HAVE_GDKPIXBUF
		GdkPixbuf *artwork =
			gdk_pixbuf_new_from_file_at_scale (artwork_filename,
							   width, height,
							   TRUE, NULL);

		if (!artwork) {
			g_debug ("Error loading image file");
			g_free (artwork_filename);
			soup_server_message_set_status (message,
			                                SOUP_STATUS_INTERNAL_SERVER_ERROR,
			                                NULL);
			goto done;
		}
		if (!gdk_pixbuf_save_to_buffer
		    (artwork, &buffer, &buffer_len, "png", NULL, NULL)) {
			g_debug ("Error saving artwork to PNG");
			g_object_unref (artwork);
			g_free (artwork_filename);
			soup_server_message_set_status (message,
			                                SOUP_STATUS_INTERNAL_SERVER_ERROR,
			                                NULL);
			goto done;
		}
		g_object_unref (artwork);
#else
		if (!g_file_get_contents
		    (artwork_filename, &buffer, &buffer_len, NULL)) {
			g_debug ("Error getting artwork data");
			g_free (artwork_filename);
			soup_server_message_set_status (message,
			                                SOUP_STATUS_INTERNAL_SERVER_ERROR,
			                                NULL);
			goto done;
		}
#endif
		g_free (artwork_filename);
		soup_server_message_set_status (message, SOUP_STATUS_OK, NULL);
		soup_server_message_set_response (message, "image/png",
		                                  SOUP_MEMORY_TAKE, buffer,
		                                  buffer_len);
	} else if (g_ascii_strcasecmp ("/1/cue", rest_of_path) == 0) {
		gchar *command;

		command = g_hash_table_lookup (query, "command");

		if (!command) {
			g_debug ("No CUE command specified");
			soup_server_message_set_status (message,
			                                SOUP_STATUS_NO_CONTENT,
			                                NULL);
			goto done;
		} else if (g_ascii_strcasecmp ("clear", command) == 0) {
			dmap_control_player_cue_clear (dmap_control_share->priv->player);
			soup_server_message_set_status (message,
			                                SOUP_STATUS_NO_CONTENT,
			                                NULL);
		} else if (g_ascii_strcasecmp ("play", command) == 0) {
			GNode *cacr;
			gchar *record_query;
			gchar *sort_by;
			GHashTable *records;
			GList *sorted_records;
			GSList *filter_def;
			DmapDb *db;
			gint index =
				atoi (g_hash_table_lookup (query, "index"));

			g_object_get (share, "db", &db, NULL);
			record_query = g_hash_table_lookup (query, "query");
			filter_def = dmap_share_build_filter (record_query);
			records = dmap_db_apply_filter (db, filter_def);
			sorted_records = g_hash_table_get_values (records);
			sort_by = g_hash_table_lookup (query, "sort");
			if (g_strcmp0 (sort_by, "album") == 0) {
				sorted_records =
					g_list_sort_with_data (sorted_records,
							       (GCompareDataFunc)
							       dmap_av_record_cmp_by_album,
							       db);
			} else if (sort_by != NULL) {
				g_warning ("Unknown sort column: %s",
					   sort_by);
			}

			dmap_control_player_cue_play (dmap_control_share->priv->player,
					      sorted_records, index);

			g_list_free (sorted_records);
			g_hash_table_unref (records);
			dmap_share_free_filter (filter_def);

			cacr = dmap_structure_add (NULL, DMAP_CC_CACR);
			dmap_structure_add (cacr, DMAP_CC_MSTT,
					    SOUP_STATUS_OK);
			dmap_structure_add (cacr, DMAP_CC_MIID, index);

			dmap_share_message_set_from_dmap_structure (share,
								     message,
								     cacr);
			g_object_unref(db);
			dmap_structure_destroy (cacr);
		} else {
			g_warning ("Unhandled cue command: %s", command);
			soup_server_message_set_status (message,
			                                SOUP_STATUS_NO_CONTENT,
			                                NULL);
			goto done;
		}
	} else {
		g_warning ("Unhandled ctrl-int command: %s", rest_of_path);
		soup_server_message_set_status (message,
		                                SOUP_STATUS_BAD_REQUEST,
		                                NULL);
	}

done:
	return;
}

#define PAIR_TXT_LENGTH 16
#define PASSCODE_LENGTH 4

static gchar *
_pairing_code (gchar * pair_txt, gchar passcode[4])
{
	int i;
	gsize ssize, dsize;
	GString *pairing_code;
	gchar *pairing_string;
	gchar *ret;

	/* The pairing code is the MD5 sum of the concatenation of pair_txt
	 * with the passcode, but the passcode takes 16-bits unicodes characters */
	pairing_string =
		g_strnfill (PAIR_TXT_LENGTH + PASSCODE_LENGTH * 2, '\0');

	dsize = PAIR_TXT_LENGTH + PASSCODE_LENGTH * 2;
	ssize = g_strlcpy (pairing_string, pair_txt, dsize);
	g_assert(dsize >= ssize);

	for (i = 0; i < 4; i++) {
		pairing_string[PAIR_TXT_LENGTH + i * 2] = passcode[i];
	}

	pairing_code =
		g_string_new (g_compute_checksum_for_data
			      (G_CHECKSUM_MD5, (guchar *) pairing_string,
			       PAIR_TXT_LENGTH + PASSCODE_LENGTH * 2));
	g_string_ascii_up (pairing_code);
	ret = g_string_free (pairing_code, FALSE);

	return ret;
}

static void
_connection_handler_cb (DmapConnection * connection, guint status,
		       GNode * structure, gpointer user_data)
{
	gboolean connected;
	GHashTableIter iter;
	gpointer key, value;
	DmapControlShare *share = user_data;
	DACPRemoteInfo *remote_info = NULL;
	gchar *service_name = NULL;
	DmapStructureItem *item = NULL;
	gchar *pairing_guid;

	g_debug ("Pairing returned with code %u", status);
	if (SOUP_STATUS_IS_SUCCESSFUL (status)) {
		connected = TRUE;
	} else {
		connected = FALSE;
	}

	/* Get the pairing-guid to identify this remote in the future. */
	if (structure) {
		item = dmap_structure_find_item (structure, DMAP_CC_CMPG);
	}
	if (item) {
		guint64 guid = g_value_get_int64 (&(item->content));

		pairing_guid =
			g_strdup_printf ("0x%.16" G_GINT64_MODIFIER "X",
					 guid);
		g_signal_emit (share, _signals[ADD_GUID], 0, pairing_guid);
		g_free (pairing_guid);
	}

	/* Find the remote that initiated this connection */
	g_hash_table_iter_init (&iter, share->priv->remotes);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		if (((DACPRemoteInfo *) value)->connection == connection) {
			service_name = (gchar *) key;
			remote_info = (DACPRemoteInfo *) value;
			break;
		}
	}

	if (remote_info == NULL) {
		g_warning ("Remote for connection not found");
		return;
	}

	/* Frees the connection */
	remote_info->connection = NULL;
	g_object_unref (connection);

	/* FIXME: Send more detailed error info, such as wrong pair code, etc */
	g_signal_emit (share, _signals[REMOTE_PAIRED], 0, service_name,
		       connected);
}

void
dmap_control_share_pair (DmapControlShare * share, gchar * service_name, gchar passcode[4])
{
	gboolean ok;
	gchar *pairing_code;
	gchar *name = NULL;
	gchar *path = NULL;
	DACPRemoteInfo *remote_info;

	remote_info = g_hash_table_lookup (share->priv->remotes, service_name);
	if (remote_info == NULL) {
		g_warning ("Remote %s not found.", service_name);
		goto done;
	}

	if (remote_info->connection != NULL) {
		g_warning ("Already pairing remote %s.", service_name);
		goto done;
	}

	g_object_get (share, "name", &name, NULL);

	remote_info->connection = DMAP_CONNECTION (dmap_control_connection_new (name,
						                        remote_info->host, remote_info->port,
									NULL,
									NULL));
	/* This is required since we don't call DmapConnection default handler */
	dmap_connection_setup (remote_info->connection);

	/* Get the remote path for pairing */
	pairing_code = _pairing_code (remote_info->pair_txt, passcode);
	path = g_strdup_printf ("/pair?pairingcode=%s&servicename=%s",
				pairing_code, name);
	g_free (pairing_code);

	g_debug ("Pairing remote in %s:%d/%s", remote_info->host,
		 remote_info->port, path);

	/* Let DmapConnection do the heavy lifting. */
	ok = dmap_connection_get (remote_info->connection, path,
	                          _connection_handler_cb, share);
	if (!ok) {
		g_debug("Error pairing remote");
	}

done:
	g_free (name);
	g_free (path);
}
