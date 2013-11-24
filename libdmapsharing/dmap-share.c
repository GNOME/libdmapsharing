/* Implmentation of DMAP (e.g., iTunes Music or iPhoto Picture) sharing
 *
 * Copyright (C) 2005 Charles Schmidt <cschmidt2@emich.edu>
 *
 * Modifications Copyright (C) 2008 W. Michael Petullo <mike@flyn.org>
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

#include <glib/gi18n.h>

#include <libdmapsharing/dmap.h>
#include <libdmapsharing/dmap-structure.h>

#define TYPE_OF_SERVICE "_daap._tcp"
#define STANDARD_DAAP_PORT 3689
#define STANDARD_DPAP_PORT 8770
#define DMAP_VERSION 2.0
#define DAAP_VERSION 3.0
#define DMAP_TIMEOUT 1800
#define DMAP_STATUS_OK 200

typedef enum
{
	DMAP_SHARE_AUTH_METHOD_NONE = 0,
	DMAP_SHARE_AUTH_METHOD_NAME_AND_PASSWORD = 1,
	DMAP_SHARE_AUTH_METHOD_PASSWORD = 2
} DMAPShareAuthMethod;

enum
{
	PROP_0,
	/* I'd be nice to unify these, once libsoup supports it. See:
	 * http://mail.gnome.org/archives/libsoup-list/2011-January/msg00000.html
	 */
	PROP_SERVER_IPV4,
	PROP_SERVER_IPV6,
	PROP_NAME,
	PROP_PASSWORD,
	PROP_REVISION_NUMBER,
	PROP_AUTH_METHOD,
	PROP_DB,
	PROP_CONTAINER_DB,
	PROP_TRANSCODE_MIMETYPE,
	PROP_TXT_RECORDS
};

struct DMAPSharePrivate
{
	gchar *name;
	guint port;
	char *password;

	/* FIXME: eventually, this should be determined dynamically, based
	 * on what client has connected and its supported mimetypes.
	 */
	char *transcode_mimetype;

	DMAPShareAuthMethod auth_method;

	/* mDNS/DNS-SD publishing things */
	gboolean server_active;
	gboolean published;
	DMAPMdnsPublisher *publisher;

	/* HTTP server things */
	SoupServer *server_ipv4;
	SoupServer *server_ipv6;
	guint revision_number;

	/* The media database */
	DMAPDb *db;
	DMAPContainerDb *container_db;

	/* TXT-RECORDS published by mDNS */
	gchar **txt_records;

	GHashTable *session_ids;
};

typedef void (*ShareBitwiseDestroyFunc) (void *);
typedef DMAPRecord *(*ShareBitwiseLookupByIdFunc) (void *db, guint id);

/* FIXME: name this something else, as it is more than just share/bitwise now */
struct share_bitwise_t
{
	SoupServer *server;	/* Also in share, but we need to know whether server_ipv6 or _ipv4. */
	struct MLCL_Bits mb;
	GSList *id_list;
	guint32 size;

	/* FIXME: ick, void * is DMAPDDb * or GHashTable * 
	 * in next two fields:*/
	void *db;
	DMAPRecord *(*lookup_by_id) (void *db, guint id);

	void (*destroy) (void *);
};

static void dmap_share_init (DMAPShare * share);
static void dmap_share_class_init (DMAPShareClass * klass);

G_DEFINE_ABSTRACT_TYPE (DMAPShare, dmap_share, G_TYPE_OBJECT)
#define DMAP_SHARE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
				   DMAP_TYPE_SHARE, DMAPSharePrivate));

     static gboolean
	     _dmap_share_soup_auth_callback (SoupAuthDomain * auth_domain,
					     SoupMessage * msg,
					     const char *username,
					     gpointer password,
					     DMAPShare * share)
{
	gboolean allowed;
	const char *path;

	path = soup_message_get_uri (msg)->path;
	g_debug ("Auth request for %s, user %s", path, username);

	allowed = !strcmp (password, share->priv->password);
	g_debug ("Auth request: %s", allowed ? "ALLOWED" : "DENIED");

	return allowed;
}

static void
server_info_adapter (SoupServer * server,
		     SoupMessage * message,
		     const char *path,
		     GHashTable * query,
		     SoupClientContext * context, DMAPShare * share)
{
	DMAP_SHARE_GET_CLASS (share)->server_info (share,
						   server,
						   message,
						   path, query, context);
}

static void
content_codes_adapter (SoupServer * server,
		       SoupMessage * message,
		       const char *path,
		       GHashTable * query,
		       SoupClientContext * context, DMAPShare * share)
{
	DMAP_SHARE_GET_CLASS (share)->content_codes (share,
						     server,
						     message,
						     path, query, context);
}

static void
login_adapter (SoupServer * server,
	       SoupMessage * message,
	       const char *path,
	       GHashTable * query,
	       SoupClientContext * context, DMAPShare * share)
{
	DMAP_SHARE_GET_CLASS (share)->login (share,
					     server,
					     message, path, query, context);
}

static void
logout_adapter (SoupServer * server,
		SoupMessage * message,
		const char *path,
		GHashTable * query,
		SoupClientContext * context, DMAPShare * share)
{
	DMAP_SHARE_GET_CLASS (share)->logout (share,
					      server,
					      message, path, query, context);
}

static void
update_adapter (SoupServer * server,
		SoupMessage * message,
		const char *path,
		GHashTable * query,
		SoupClientContext * context, DMAPShare * share)
{
	DMAP_SHARE_GET_CLASS (share)->update (share,
					      server,
					      message, path, query, context);
}

static void
databases_adapter (SoupServer * server,
		   SoupMessage * message,
		   const char *path,
		   GHashTable * query,
		   SoupClientContext * context, DMAPShare * share)
{
	DMAP_SHARE_GET_CLASS (share)->databases (share,
						 server,
						 message,
						 path, query, context);
}

static void
ctrl_int_adapter (SoupServer * server,
		  SoupMessage * message,
		  const char *path,
		  GHashTable * query,
		  SoupClientContext * context, DMAPShare * share)
{
	DMAP_SHARE_GET_CLASS (share)->ctrl_int (share,
						server,
						message,
						path, query, context);
}

static void
_dmap_share_server_setup_handlers (DMAPShare * share, SoupServer * server)
{
	gboolean password_required =
		(share->priv->auth_method != DMAP_SHARE_AUTH_METHOD_NONE);

	if (password_required) {
		SoupAuthDomain *auth_domain;

		auth_domain =
			soup_auth_domain_basic_new (SOUP_AUTH_DOMAIN_REALM,
						    "Music Sharing",
						    SOUP_AUTH_DOMAIN_ADD_PATH,
						    "/login",
						    SOUP_AUTH_DOMAIN_ADD_PATH,
						    "/update",
						    SOUP_AUTH_DOMAIN_ADD_PATH,
						    "/database",
						    SOUP_AUTH_DOMAIN_FILTER,
						    _dmap_share_soup_auth_filter,
						    NULL);
		soup_auth_domain_basic_set_auth_callback (auth_domain,
							  (SoupAuthDomainBasicAuthCallback)
							  _dmap_share_soup_auth_callback,
							  g_object_ref
							  (share),
							  g_object_unref);
		soup_server_add_auth_domain (server, auth_domain);
	}

	soup_server_add_handler (server, "/server-info",
				 (SoupServerCallback) server_info_adapter,
				 share, NULL);
	soup_server_add_handler (server, "/content-codes",
				 (SoupServerCallback) content_codes_adapter,
				 share, NULL);
	soup_server_add_handler (server, "/login",
				 (SoupServerCallback) login_adapter,
				 share, NULL);
	soup_server_add_handler (server, "/logout",
				 (SoupServerCallback) logout_adapter,
				 share, NULL);
	soup_server_add_handler (server, "/update",
				 (SoupServerCallback) update_adapter,
				 share, NULL);
	soup_server_add_handler (server, "/databases",
				 (SoupServerCallback) databases_adapter,
				 share, NULL);
	soup_server_add_handler (server, "/ctrl-int",
				 (SoupServerCallback) ctrl_int_adapter,
				 share, NULL);
	soup_server_run_async (server);

}

gboolean
_dmap_share_server_start (DMAPShare * share)
{
	SoupAddress *addr;
	guint port = DMAP_SHARE_GET_CLASS (share)->get_desired_port (share);

	addr = soup_address_new_any (SOUP_ADDRESS_FAMILY_IPV6, port);
	share->priv->server_ipv6 =
		soup_server_new (SOUP_SERVER_INTERFACE, addr, NULL);
	g_object_unref (addr);

	/* NOTE: On Linux, opening a socket may give a IPv6-wrapped IPv4 address.
	 * in this case, the server_ipv6 object will service requests from both
	 * IPv6 and IPv4 clients.
	 */
	if (share->priv->server_ipv6 == NULL) {
		g_debug
			("Unable to start music sharing server on port %d, trying any open port",
			 port);
		addr = soup_address_new_any (SOUP_ADDRESS_FAMILY_IPV6,
					     SOUP_ADDRESS_ANY_PORT);
		share->priv->server_ipv6 =
			soup_server_new (SOUP_SERVER_INTERFACE, addr, NULL);
		g_object_unref (addr);
	}

	if (share->priv->server_ipv6 != NULL) {
		/* Use same port for IPv4 as IPv6. */
		port = soup_server_get_port (share->priv->server_ipv6);
	} else {
		g_debug ("Unable to start music sharing server (IPv6)");
	}

	/* NOTE: In the case mentioned above, this will fail as the server_ipv6 is already
	 * servicing IPv4 requests. In this case server_ipv6 handles both IPv6 and IPv4
	 * and server_ipv4 is NULL.
	 */
	addr = soup_address_new_any (SOUP_ADDRESS_FAMILY_IPV4, port);
	share->priv->server_ipv4 =
		soup_server_new (SOUP_SERVER_INTERFACE, addr, NULL);
	g_object_unref (addr);

	/* Don't try any port on IPv4 unless IPv6 failed. We don't want to listen to one
	 * port on IPv6 and another on IPv4 */
	if (share->priv->server_ipv6 == NULL
	    && share->priv->server_ipv4 == NULL) {
		g_debug
			("Unable to start music sharing server on port %d, trying IPv4 only, any open port",
			 port);
		addr = soup_address_new_any (SOUP_ADDRESS_FAMILY_IPV4,
					     SOUP_ADDRESS_ANY_PORT);
		share->priv->server_ipv4 =
			soup_server_new (SOUP_SERVER_INTERFACE, addr, NULL);
		g_object_unref (addr);
	}

	if (share->priv->server_ipv4 == NULL) {
		g_debug ("Unable to start music sharing server (IPv4)");
		if (share->priv->server_ipv6 == NULL) {
			g_warning ("Unable to start music sharing server (both IPv4 and IPv6 failed)");
			return FALSE;
		}
	}

	if (share->priv->server_ipv6)
		share->priv->port =
			(guint) soup_server_get_port (share->priv->
						      server_ipv6);
	else
		share->priv->port =
			(guint) soup_server_get_port (share->priv->
						      server_ipv4);

	g_debug ("Started DMAP server on port %u (IPv6: %s, explicit IPv4: %s)",
	          share->priv->port,
		  share->priv->server_ipv6 ? "yes" : "no",
		  share->priv->server_ipv4 ? "yes" : "no");

	if (share->priv->server_ipv6)
		_dmap_share_server_setup_handlers (share,
						   share->priv->server_ipv6);

	if (share->priv->server_ipv4)
		_dmap_share_server_setup_handlers (share,
						   share->priv->server_ipv4);

	/* using direct since there is no g_uint_hash or g_uint_equal */
	share->priv->session_ids =
		g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL,
				       g_free);

	share->priv->server_active = TRUE;

	return TRUE;
}

static gboolean
_dmap_share_server_stop (DMAPShare * share)
{
	g_debug ("Stopping music sharing server on port %d",
		 share->priv->port);

	if (share->priv->server_ipv4) {
		soup_server_quit (share->priv->server_ipv4);
		g_object_unref (share->priv->server_ipv4);
		share->priv->server_ipv4 = NULL;
	}

	if (share->priv->server_ipv6) {
		soup_server_quit (share->priv->server_ipv6);
		g_object_unref (share->priv->server_ipv6);
		share->priv->server_ipv6 = NULL;
	}

	if (share->priv->session_ids) {
		g_hash_table_destroy (share->priv->session_ids);
		share->priv->session_ids = NULL;
	}

	share->priv->server_active = FALSE;

	return TRUE;
}

gboolean
_dmap_share_publish_start (DMAPShare * share)
{
	GError *error;
	gboolean res;
	gboolean password_required;

	password_required =
		(share->priv->auth_method != DMAP_SHARE_AUTH_METHOD_NONE);

	error = NULL;
	res = dmap_mdns_publisher_publish (share->priv->publisher,
					   share->priv->name,
					   share->priv->port,
					   DMAP_SHARE_GET_CLASS (share)->
					   get_type_of_service (share),
					   password_required,
					   share->priv->txt_records, &error);

	if (res == FALSE) {
		if (error != NULL) {
			g_warning
				("Unable to notify network of media sharing: %s",
				 error->message);
			g_error_free (error);
		} else {
			g_warning
				("Unable to notify network of media sharing");
		}
		return FALSE;
	} else {
		g_debug ("Published DMAP server information to mdns");
	}

	return TRUE;
}

static gboolean
_dmap_share_publish_stop (DMAPShare * share)
{
	if (share->priv->publisher) {
		gboolean res;
		GError *error;

		error = NULL;
		res = dmap_mdns_publisher_withdraw (share->priv->publisher,
						    share->priv->port,
						    &error);
		if (error != NULL) {
			g_warning
				("Unable to withdraw music sharing service: %s",
				 error->message);
			g_error_free (error);
		}
		return res;
	}

	share->priv->published = FALSE;
	return TRUE;
}

static void
_dmap_share_restart (DMAPShare * share)
{
	gboolean res;

	_dmap_share_server_stop (share);
	res = _dmap_share_server_start (share);
	if (res) {
		/* To update information just publish again */
		_dmap_share_publish_start (share);
	} else {
		_dmap_share_publish_stop (share);
	}
}

static void
_dmap_share_maybe_restart (DMAPShare * share)
{
	if (share->priv->published) {
		_dmap_share_restart (share);
	}
}

static void
_dmap_share_set_name (DMAPShare * share, const char *name)
{
	GError *error;

	g_return_if_fail (share != NULL);

	g_free (share->priv->name);
	share->priv->name = g_strdup (name);

	if (share->priv->published) {
		error = NULL;
		dmap_mdns_publisher_rename_at_port (share->priv->
						    publisher,
						    share->priv->port,
						    name,
						    &error);
		if (error != NULL) {
			g_warning ("Unable to change MDNS service name: %s",
				   error->message);
			g_error_free (error);
		}
	}
}

static void
_dmap_share_set_password (DMAPShare * share, const char *password)
{
	g_return_if_fail (share != NULL);

	if (share->priv->password && password &&
	    strcmp (password, share->priv->password) == 0) {
		return;
	}

	g_free (share->priv->password);
	share->priv->password = g_strdup (password);
	if (password != NULL) {
		share->priv->auth_method = DMAP_SHARE_AUTH_METHOD_PASSWORD;
	} else {
		share->priv->auth_method = DMAP_SHARE_AUTH_METHOD_NONE;
	}

	_dmap_share_maybe_restart (share);
}

static void
_dmap_share_set_property (GObject * object,
			  guint prop_id,
			  const GValue * value, GParamSpec * pspec)
{
	DMAPShare *share = DMAP_SHARE (object);

	switch (prop_id) {
	case PROP_NAME:
		_dmap_share_set_name (share, g_value_get_string (value));
		break;
	case PROP_PASSWORD:
		_dmap_share_set_password (share, g_value_get_string (value));
		break;
	case PROP_DB:
		share->priv->db = (DMAPDb *) g_value_get_pointer (value);
		break;
	case PROP_CONTAINER_DB:
		share->priv->container_db =
			(DMAPContainerDb *) g_value_get_pointer (value);
		break;
	case PROP_TRANSCODE_MIMETYPE:
		/* FIXME: get or dup? */
		share->priv->transcode_mimetype = g_value_dup_string (value);
		break;
	case PROP_TXT_RECORDS:
		share->priv->txt_records = g_value_dup_boxed (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
_dmap_share_get_property (GObject * object,
			  guint prop_id, GValue * value, GParamSpec * pspec)
{
	DMAPShare *share = DMAP_SHARE (object);

	switch (prop_id) {
	case PROP_SERVER_IPV4:
		g_value_set_object (value, share->priv->server_ipv4);
		return;
	case PROP_SERVER_IPV6:
		g_value_set_object (value, share->priv->server_ipv6);
		return;
	case PROP_NAME:
		g_value_set_string (value, share->priv->name);
		break;
	case PROP_PASSWORD:
		g_value_set_string (value, share->priv->password);
		break;
	case PROP_REVISION_NUMBER:
		g_value_set_uint (value,
				  _dmap_share_get_revision_number
				  (DMAP_SHARE (object)));
		break;
	case PROP_AUTH_METHOD:
		g_value_set_uint (value,
				  _dmap_share_get_auth_method
				  (DMAP_SHARE (object)));
		break;
	case PROP_DB:
		g_value_set_pointer (value, share->priv->db);
		break;
	case PROP_CONTAINER_DB:
		g_value_set_pointer (value, share->priv->container_db);
		break;
	case PROP_TRANSCODE_MIMETYPE:
		g_value_set_string (value, share->priv->transcode_mimetype);
		break;
	case PROP_TXT_RECORDS:
		g_value_set_boxed (value, share->priv->txt_records);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
_dmap_share_finalize (GObject * object)
{
	DMAPShare *share = DMAP_SHARE (object);

	g_debug ("Finalizing DMAPShare");

	if (share->priv->published) {
		_dmap_share_publish_stop (share);
	}

	if (share->priv->server_active) {
		_dmap_share_server_stop (share);
	}

	g_free (share->priv->name);
	g_free (share->priv->password);

	g_object_unref (share->priv->db);
	g_object_unref (share->priv->container_db);

	g_strfreev (share->priv->txt_records);

	if (share->priv->publisher) {
		g_object_unref (share->priv->publisher);
	}

	G_OBJECT_CLASS (dmap_share_parent_class)->finalize (object);
}

static void
dmap_share_class_init (DMAPShareClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = _dmap_share_get_property;
	object_class->set_property = _dmap_share_set_property;
	object_class->finalize = _dmap_share_finalize;

	/* Pure virtual methods: */
	klass->get_desired_port = NULL;
	klass->get_type_of_service = NULL;
	klass->message_add_standard_headers = NULL;
	klass->get_meta_data_map = NULL;
	klass->add_entry_to_mlcl = NULL;
	klass->databases_browse_xxx = NULL;
	klass->databases_items_xxx = NULL;

	/* Virtual methods: */
	klass->content_codes = _dmap_share_content_codes;
	klass->login = _dmap_share_login;
	klass->logout = _dmap_share_logout;
	klass->update = _dmap_share_update;
	klass->published = _dmap_share_published;
	klass->name_collision = _dmap_share_name_collision;
	klass->databases = _dmap_share_databases;
	klass->ctrl_int = _dmap_share_ctrl_int;

	g_object_class_install_property (object_class,
					 PROP_SERVER_IPV4,
					 g_param_spec_object ("server-ipv4",
							      "Soup Server",
							      "Soup server",
							      SOUP_TYPE_SERVER,
							      G_PARAM_READABLE));

	g_object_class_install_property (object_class,
					 PROP_SERVER_IPV6,
					 g_param_spec_object ("server-ipv6",
							      "Soup Server",
							      "Soup server",
							      SOUP_TYPE_SERVER,
							      G_PARAM_READABLE));

	g_object_class_install_property (object_class,
					 PROP_NAME,
					 g_param_spec_string ("name",
							      "Name",
							      "Share Name",
							      NULL,
							      G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_PASSWORD,
					 g_param_spec_string ("password",
							      "Authentication password",
							      "Authentication password",
							      NULL,
							      G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
					 PROP_REVISION_NUMBER,
					 g_param_spec_uint ("revision_number",
							    "Revision number",
							    "Revision number",
							    0,
							    G_MAXINT,
							    0,
							    G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
					 PROP_AUTH_METHOD,
					 g_param_spec_uint ("auth_method",
							    "Authentication method",
							    "Authentication method",
							    DMAP_SHARE_AUTH_METHOD_NONE,
							    DMAP_SHARE_AUTH_METHOD_PASSWORD,
							    0,
							    G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_DB,
					 g_param_spec_pointer ("db",
							       "DB",
							       "DB object",
							       G_PARAM_READWRITE
							       |
							       G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
					 PROP_CONTAINER_DB,
					 g_param_spec_pointer ("container-db",
							       "Container DB",
							       "Container DB object",
							       G_PARAM_READWRITE
							       |
							       G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
					 PROP_TRANSCODE_MIMETYPE,
					 g_param_spec_string
					 ("transcode-mimetype",
					  "Transcode mimetype",
					  "Set mimetype of stream after transcoding",
					  NULL,
					  G_PARAM_READWRITE |
					  G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
					 PROP_TXT_RECORDS,
					 g_param_spec_boxed ("txt-records",
							     "TXT-Records",
							     "Set TXT-Records used for MDNS publishing",
							     G_TYPE_STRV,
							     G_PARAM_READWRITE));

	g_type_class_add_private (klass, sizeof (DMAPSharePrivate));
}

static void
published_adapter (DMAPMdnsPublisher * publisher,
		   const char *name, DMAPShare * share)
{
	DMAP_SHARE_GET_CLASS (share)->published (share, publisher, name);
}

static void
name_collision_adapter (DMAPMdnsPublisher * publisher,
			const char *name, DMAPShare * share)
{
	DMAP_SHARE_GET_CLASS (share)->name_collision (share, publisher, name);
}

static void
dmap_share_init (DMAPShare * share)
{
	share->priv = DMAP_SHARE_GET_PRIVATE (share);

	share->priv->revision_number = 5;
	share->priv->auth_method = DMAP_SHARE_AUTH_METHOD_NONE;
	share->priv->publisher = dmap_mdns_publisher_new ();

	g_signal_connect_object (share->priv->publisher,
				 "published",
				 G_CALLBACK (published_adapter), share, 0);
	g_signal_connect_object (share->priv->publisher,
				 "name-collision",
				 G_CALLBACK (name_collision_adapter),
				 share, 0);
}

guint
_dmap_share_get_auth_method (DMAPShare * share)
{
	return share->priv->auth_method;
}

guint
_dmap_share_get_revision_number (DMAPShare * share)
{
	return share->priv->revision_number;
}

static gboolean
get_session_id (GHashTable * query, guint32 * id)
{
	char *session_id_str;
	guint32 session_id;

	session_id_str = g_hash_table_lookup (query, "session-id");
	if (session_id_str == NULL) {
		g_warning ("Session id not found.");
		return FALSE;
	}

	session_id = (guint32) strtoul (session_id_str, NULL, 10);
	if (id != NULL) {
		*id = session_id;
	}
	return TRUE;
}

gboolean
_dmap_share_get_revision_number_from_query (GHashTable * query,
					    guint * number)
{
	char *revision_number_str;
	guint revision_number;

	revision_number_str = g_hash_table_lookup (query, "revision-number");
	if (revision_number_str == NULL) {
		g_warning
			("Client asked for an update without a rev. number");
		return FALSE;
	}

	revision_number = strtoul (revision_number_str, NULL, 10);
	if (number != NULL) {
		*number = revision_number;
	}
	return TRUE;
}

gboolean
_dmap_share_session_id_validate (DMAPShare * share,
				 SoupClientContext * context,
				 SoupMessage * message,
				 GHashTable * query, guint32 * id)
{
	guint32 session_id;
	gboolean res;
	const char *addr;
	const char *remote_address;

	if (id) {
		*id = 0;
	}

	res = get_session_id (query, &session_id);
	if (!res) {
		g_warning ("Validation failed: Unable to parse session id");
		return FALSE;
	}

	/* check hash for remote address */
	addr = g_hash_table_lookup (share->priv->session_ids,
				    GUINT_TO_POINTER (session_id));
	if (addr == NULL) {
		g_warning
			("Validation failed: Unable to lookup session id %u",
			 session_id);
		return FALSE;
	}

	remote_address = soup_client_context_get_host (context);
	g_debug ("Validating session id %u from %s matches %s",
		 session_id, remote_address, addr);
	if (remote_address == NULL || strcmp (addr, remote_address) != 0) {
		g_warning
			("Validation failed: Remote address does not match stored address");
		return FALSE;
	}

	if (id) {
		*id = session_id;
	}

	return TRUE;
}

static guint32
session_id_generate (DMAPShare * share, SoupClientContext * context)
{
	guint32 id;

	id = g_random_int ();

	return id;
}

guint32
_dmap_share_session_id_create (DMAPShare * share, SoupClientContext * context)
{
	guint32 id;
	const char *addr;
	char *remote_address;

	do {
		/* create a unique session id */
		id = session_id_generate (share, context);
		g_debug ("Generated session id %u", id);

		/* if already used, try again */
		addr = g_hash_table_lookup (share->priv->session_ids,
					    GUINT_TO_POINTER (id));
	} while (addr != NULL);

	/* store session id and remote address */
	/* FIXME, warning, this fails against libsoup-2.33.90-1.fc15.x86_64 with:
	 * (dmapd:12917): libsoup-CRITICAL **: soup_address_get_physical: assertion `SOUP_IS_ADDRESS (addr)' failed
	 * Is this a bug in libsoup or libdmapsharing?
	 */
	remote_address = g_strdup (soup_client_context_get_host (context));
	g_hash_table_insert (share->priv->session_ids, GUINT_TO_POINTER (id),
			     remote_address);

	return id;
}

void
_dmap_share_session_id_remove (DMAPShare * share,
			       SoupClientContext * context, guint32 id)
{
	g_hash_table_remove (share->priv->session_ids, GUINT_TO_POINTER (id));
}

void
_dmap_share_message_set_from_dmap_structure (DMAPShare * share,
					     SoupMessage * message,
					     GNode * structure)
{
	gchar *resp;
	guint length;

	resp = dmap_structure_serialize (structure, &length);

	if (resp == NULL) {
		g_warning ("Serialize gave us null?\n");
		return;
	}

	soup_message_set_response (message, "application/x-dmap-tagged",
				   SOUP_MEMORY_TAKE, resp, length);

	DMAP_SHARE_GET_CLASS (share)->message_add_standard_headers (share,
								    message);

	soup_message_set_status (message, SOUP_STATUS_OK);
}

gboolean
_dmap_share_client_requested (bitwise bits, gint field)
{
	return 0 != (bits & (((bitwise) 1) << field));
}

gboolean
_dmap_share_uri_is_local (const char *text_uri)
{
	return g_str_has_prefix (text_uri, "file://");
}

gboolean
_dmap_share_soup_auth_filter (SoupAuthDomain * auth_domain,
			      SoupMessage * msg, gpointer user_data)
{
	const char *path;

	path = soup_message_get_uri (msg)->path;
	if (g_str_has_prefix (path, "/databases/")) {
		/* Subdirectories of /databases don't actually require
		 * authentication
		 */
		return FALSE;
	} else {
		/* Everything else in auth_domain's paths, including
		 * /databases itself, does require auth.
		 */
		return TRUE;
	}
}

void
_dmap_share_published (DMAPShare * share,
		       DMAPMdnsPublisher * publisher, const char *name)
{
	if (share->priv->name == NULL || name == NULL) {
		return;
	}

	if (strcmp (share->priv->name, name) == 0) {
		g_debug ("mDNS publish successful");
		share->priv->published = TRUE;
	}
}

void
_dmap_share_name_collision (DMAPShare * share,
			    DMAPMdnsPublisher * publisher, const char *name)
{
	char *new_name = "FIXME";

	if (share->priv->name == NULL || name == NULL) {
		return;
	}

	if (strcmp (share->priv->name, name) == 0) {
		g_warning ("Duplicate share name on mDNS");

		_dmap_share_set_name (DMAP_SHARE (share), new_name);
		g_free (new_name);
	}

	return;
}

void
_dmap_share_content_codes (DMAPShare * share,
			   SoupServer * server,
			   SoupMessage * message,
			   const char *path,
			   GHashTable * query, SoupClientContext * context)
{
/* MCCR content codes response
 * 	MSTT status
 * 	MDCL dictionary
 * 		MCNM content codes number
 * 		MCNA content codes name
 * 		MCTY content codes type
 * 	MDCL dictionary
 * 	...
 */
	const DMAPContentCodeDefinition *defs;
	guint num_defs = 0;
	guint i;
	GNode *mccr;

	g_debug ("Path is %s.", path);

	defs = dmap_content_codes (&num_defs);

	mccr = dmap_structure_add (NULL, DMAP_CC_MCCR);
	dmap_structure_add (mccr, DMAP_CC_MSTT, (gint32) DMAP_STATUS_OK);

	for (i = 0; i < num_defs; i++) {
		GNode *mdcl;

		mdcl = dmap_structure_add (mccr, DMAP_CC_MDCL);
		dmap_structure_add (mdcl, DMAP_CC_MCNM,
				    dmap_content_code_string_as_int32 (defs
								       [i].
								       string));
		dmap_structure_add (mdcl, DMAP_CC_MCNA, defs[i].name);
		dmap_structure_add (mdcl, DMAP_CC_MCTY,
				    (gint32) defs[i].type);
	}

	_dmap_share_message_set_from_dmap_structure (share, message, mccr);
	dmap_structure_destroy (mccr);
}

void
_dmap_share_login (DMAPShare * share,
		   SoupServer * server,
		   SoupMessage * message,
		   const char *path,
		   GHashTable * query, SoupClientContext * context)
{
/* MLOG login response
 * 	MSTT status
 * 	MLID session id
 */
	GNode *mlog;
	guint32 session_id;

	g_debug ("Path is %s.", path);

	session_id = _dmap_share_session_id_create (share, context);

	mlog = dmap_structure_add (NULL, DMAP_CC_MLOG);
	dmap_structure_add (mlog, DMAP_CC_MSTT, (gint32) DMAP_STATUS_OK);
	dmap_structure_add (mlog, DMAP_CC_MLID, session_id);

	_dmap_share_message_set_from_dmap_structure (share, message, mlog);
	dmap_structure_destroy (mlog);
}

void
_dmap_share_logout (DMAPShare * share,
		    SoupServer * server,
		    SoupMessage * message,
		    const char *path,
		    GHashTable * query, SoupClientContext * context)
{
	int status;
	guint32 id;

	g_debug ("Path is %s.", path);

	if (_dmap_share_session_id_validate
	    (share, context, message, query, &id)) {
		_dmap_share_session_id_remove (share, context, id);

		status = SOUP_STATUS_NO_CONTENT;
	} else {
		status = SOUP_STATUS_FORBIDDEN;
	}

	soup_message_set_status (message, status);
}

void
_dmap_share_update (DMAPShare * share,
		    SoupServer * server,
		    SoupMessage * message,
		    const char *path,
		    GHashTable * query, SoupClientContext * context)
{
	guint revision_number;
	gboolean res;

	g_debug ("Path is %s.", path);

	res = _dmap_share_get_revision_number_from_query (query,
							  &revision_number);

	if (res && revision_number != _dmap_share_get_revision_number (share)) {
		/* MUPD update response
		 *      MSTT status
		 *      MUSR server revision
		 */
		GNode *mupd;

		mupd = dmap_structure_add (NULL, DMAP_CC_MUPD);
		dmap_structure_add (mupd, DMAP_CC_MSTT,
				    (gint32) DMAP_STATUS_OK);
		dmap_structure_add (mupd, DMAP_CC_MUSR,
				    (gint32)
				    _dmap_share_get_revision_number (share));

		_dmap_share_message_set_from_dmap_structure (share, message,
							     mupd);
		dmap_structure_destroy (mupd);
	} else {
		/* FIXME: This seems like a bug. It just leaks the
		 * message (and socket) without ever replying.
		 */
		g_object_ref (message);
		soup_server_pause_message (server, message);
	}
}

bitwise
_dmap_share_parse_meta_str (const char *attrs, struct DMAPMetaDataMap *mdm)
{
	guint i;
	bitwise bits = 0;

	/* iTunes 8 uses meta=all for /databases/1/items query: */
	if (strcmp (attrs, "all") == 0) {
		bits = ~0;
	} else {
		gchar **attrsv;

		attrsv = g_strsplit (attrs, ",", -1);

		for (i = 0; attrsv[i]; i++) {
			guint j;
			gboolean found = FALSE;

			for (j = 0; mdm[j].tag; j++) {
				if (strcmp (mdm[j].tag, attrsv[i]) == 0) {
					bits |= (((bitwise) 1) << mdm[j].md);
					found = TRUE;
				}
			}

			if (found == FALSE)
				g_debug ("Unknown meta request: %s",
					 attrsv[i]);
		}
		g_strfreev (attrsv);
	}

	return bits;
}

bitwise
_dmap_share_parse_meta (GHashTable * query, struct DMAPMetaDataMap * mdm)
{
	const gchar *attrs;

	attrs = g_hash_table_lookup (query, "meta");
	if (attrs == NULL) {
		return 0;
	}
	return _dmap_share_parse_meta_str (attrs, mdm);
}

void
_dmap_share_add_playlist_to_mlcl (gpointer id, DMAPContainerRecord * record,
				  gpointer _mb)
{
	/* MLIT listing item
	 * MIID item id
	 * MPER persistent item id
	 * MINM item name
	 * MIMC item count
	 */
	GNode *mlit;
	guint num_songs;
	gchar *name;
	struct MLCL_Bits *mb = (struct MLCL_Bits *) _mb;

	num_songs = dmap_container_record_get_entry_count (record);
	g_object_get (record, "name", &name, NULL);

	/* FIXME: ITEM_ID, etc. is defined in DAAPShare, so I can't use
	 * with _dmap_share_client_requested() here (see add_entry_to_mlcl())
	 */

	mlit = dmap_structure_add (mb->mlcl, DMAP_CC_MLIT);
	dmap_structure_add (mlit, DMAP_CC_MIID,
			    dmap_container_record_get_id (record));
	/* we don't have a persistant ID for playlists, unfortunately */
	dmap_structure_add (mlit, DMAP_CC_MPER,
			    (gint64) dmap_container_record_get_id (record));
	dmap_structure_add (mlit, DMAP_CC_MINM, name);
	dmap_structure_add (mlit, DMAP_CC_MIMC, (gint32) num_songs);

	/* FIXME: Is this getting music-specific? */
	dmap_structure_add (mlit, DMAP_CC_FQUESCH, 0);
	dmap_structure_add (mlit, DMAP_CC_MPCO, 0);
	dmap_structure_add (mlit, DMAP_CC_AESP, 0);
	dmap_structure_add (mlit, DMAP_CC_AEPP, 0);
	dmap_structure_add (mlit, DMAP_CC_AEPS, 0);
	dmap_structure_add (mlit, DMAP_CC_AESG, 0);

	g_free (name);

	return;
}

GSList *
_dmap_share_build_filter (gchar * filterstr)
{
	/* Produces a list of lists, each being a filter definition that may
	 * be one or more filter criteria.
	 */

	/* A filter string looks like (iTunes):
	 * 'daap.songgenre:Other'+'daap.songartist:Band'.
	 * or (Roku):
	 * 'daap.songgenre:Other' 'daap.songartist:Band'.
	 * or
	 * 'dmap.itemid:1000'
	 * or
	 * 'dmap.itemid:1000','dmap:itemid:1001'
	 * or
	 * 'daap.songgenre:Foo'+'daap.songartist:Bar'+'daap.songalbum:Baz'
	 * or (iPhoto '09)
	 * ('daap.idemid:1000','dmap:itemid:1001')
	 * or (DACP):
	 * ('com.apple.itunes.mediakind:1','com.apple.itunes.mediakind:32') 'daap.songartist!:'
	 */

	gchar *next_char;
	GString *value;

	//gboolean save_value;
	gboolean is_key;
	gboolean is_value;
	gboolean new_group;
	gboolean accept;
	gboolean negate;
	gint parentheses_count;
	gint quotes_count;
	DMAPDbFilterDefinition *def;

	GSList *list = NULL;
	GSList *filter = NULL;

	g_debug ("Filter string is %s.", filterstr);

	if (filterstr == NULL)
		return NULL;

	next_char = filterstr;

	parentheses_count = 0;
	quotes_count = 0;
	is_key = TRUE;
	is_value = FALSE;
	new_group = FALSE;
	negate = FALSE;
	value = NULL;
	def = NULL;

	// The query string is divided in groups of AND separated by a space,
	// each group containing queries of OR separated by comma enclosed in
	// parentheses. Queries are key, value pairs separated by ':' enclosed 
	// in quotes or not.
	// The result from this is a list of lists. Here is an example:
	// String is ('com.apple.itunes.mediakind:1','com.apple.itunes.mediakind:32') 'daap.songartist!:'
	// list:
	// |-> filter1: -> com.apple.itunes.mediakind = 1
	// |   |        -> com.apple.itunes.mediakind = 32
	// |-> filter1: -> daap.songartist! = (null)
	// This basically means that the query is (filter1 AND filter2), and
	// filter1 is (com.apple.itunes.mediakind = 1) OR (com.apple.itunes.mediakind = 32)
	while (TRUE) {
		// We check each character to see if it should be included in
		// the current value (the current value will become the query key
		// or the query value). Anything that is not a special character
		// handled below will be accepted (this means weird characters
		// might appear in names).
		// Handling of unicode characters is unknown, but as long it 
		// doesn't appear as a character handled below, it will be
		// included.
		// This parser will result in unknown behaviour on bad-formatted
		// queries (it does not check for syntax errors), but should not 
		// crash. In this way it may accept much more syntaxes then
		// other parsers.
		accept = FALSE;
		// A slash can escape characters such as ', so add the character
		// after the slash.
		if (*next_char == '\\') {
			accept = TRUE;
			next_char++;
		} else {
			switch (*next_char) {
			case '(':
				if (is_value) {
					accept = TRUE;
				} else {
					parentheses_count++;
				}
				break;
			case ')':
				if (is_value) {
					accept = TRUE;
				} else {
					parentheses_count--;
				}
				break;
			case '\'':
				if (quotes_count > 0) {
					quotes_count = 0;
				} else {
					quotes_count = 1;
				}
				break;
			case ' ':
				if (is_value) {
					accept = TRUE;
				} else {
					new_group = TRUE;
				}
				break;
			case ':':
				// Inside values, they will be included in the 
				// query string, otherwise it indicates there
				// will be a value next.
				if (is_value) {
					accept = TRUE;
				} else if (is_key) {
					is_value = TRUE;
				}
				break;
			case '!':
				if (is_value) {
					accept = TRUE;
				} else if (is_key && value) {
					negate = TRUE;
				}
				break;
			case ',':
			case '+':
				// Accept these characters only if inside quotes
				if (is_value) {
					accept = TRUE;
				}
				break;
			case '\0':
				// Never accept
				break;
			default:
				accept = TRUE;
				break;
			}
		}
		//g_debug ("Char: %c, Accept: %s", *next_char, accept?"TRUE":"FALSE");
		//Is the next character to be accepted?
		if (accept) {
			if (!value) {
				value = g_string_new ("");
			}
			g_string_append_c (value, *next_char);
		} else if (value != NULL && *next_char != '!') {
			// If we won't accept this character, we are ending a 
			// query key or value, so we should save them in a new
			// DMAPDbFilterDefinition. If is_value is TRUE, we will still
			// parse the query value, so we must keep our def around.
			// Otherwise, save it in the list of filters.
			if (!def) {
				def = g_new0 (DMAPDbFilterDefinition, 1);
			}
			if (is_key) {
				def->key = value->str;
				g_string_free (value, FALSE);
				def->negate = negate;
				negate = FALSE;
				is_key = FALSE;
			} else {
				def->value = value->str;
				g_string_free (value, FALSE);
				is_value = FALSE;
				is_key = TRUE;
			}
			value = NULL;
			if (!is_value) {
				filter = g_slist_append (filter, def);
				def = NULL;
			}
		}
		if (new_group && filter) {
			list = g_slist_append (list, filter);
			filter = NULL;
			new_group = FALSE;
		}
		// Only handle \0 here so we can handle remaining values above.
		if (*next_char == '\0')
			break;
		next_char++;
	};

	// Any remaining def or filter must still be handled here.
	if (def) {
		filter = g_slist_append (filter, def);
	}

	if (filter) {
		list = g_slist_append (list, filter);
	}

	GSList *ptr1, *ptr2;

	for (ptr1 = list; ptr1 != NULL; ptr1 = ptr1->next) {
		for (ptr2 = ptr1->data; ptr2 != NULL; ptr2 = ptr2->next) {
			g_debug ("%s = %s",
				 ((DMAPDbFilterDefinition *) ptr2->data)->key,
				 ((DMAPDbFilterDefinition *) ptr2->data)->value);
		}
	}

	return list;
}

void
dmap_share_free_filter (GSList * filter)
{
	GSList *ptr1, *ptr2;

	for (ptr1 = filter; ptr1 != NULL; ptr1 = ptr1->next) {
		for (ptr2 = ptr1->data; ptr2 != NULL; ptr2 = ptr2->next) {
			g_free (((DMAPDbFilterDefinition *) ptr2->data)->value);
			g_free (ptr2->data);
		}
	}
}

typedef struct
{
	gchar *name;
	gint64 group_id;
	gchar *artist;
	int count;
} GroupInfo;

static void
group_items (gpointer key, DMAPRecord * record, GHashTable * groups)
{
	gchar *album, *artist;
	GroupInfo *group_info;
	gint64 group_id;

	g_object_get (record, "songartist", &artist, "songalbum", &album,
		      "songalbumid", &group_id, NULL);
	if (!album) {
		g_free (artist);
		return;
	}
	group_info = g_hash_table_lookup (groups, album);
	if (!group_info) {
		group_info = g_new0 (GroupInfo, 1);
		g_hash_table_insert (groups, album, group_info);
		// They will be freed when the hash table is freed.
		group_info->name = album;
		group_info->artist = artist;
		group_info->group_id = group_id;
	} else {
		g_free (album);
		g_free (artist);
	}
	(group_info->count)++;
}

static gint
group_info_cmp (gconstpointer group1, gconstpointer group2)
{
	return g_ascii_strcasecmp (((GroupInfo *) group1)->name,
				   ((GroupInfo *) group2)->name);
}

static void
debug_param (gpointer key, gpointer val, gpointer user_data)
{
	g_debug ("%s %s", (char *) key, (char *) val);
}

void
_dmap_share_ctrl_int (DMAPShare * share,
		      SoupServer * server,
		      SoupMessage * message,
		      const char *path,
		      GHashTable * query, SoupClientContext * context)
{
	g_debug ("Path is %s.", path);
	if (query) {
		g_hash_table_foreach (query, debug_param, NULL);
	}

	g_debug ("ctrl-int not implemented");
}

static void
accumulate_mlcl_size_and_ids (gpointer id,
			      DMAPRecord * record,
			      struct share_bitwise_t *share_bitwise)
{
	share_bitwise->id_list = g_slist_append (share_bitwise->id_list, id);

	/* Make copy and set mlcl to NULL so real MLCL does not get changed */
	struct MLCL_Bits mb_copy = share_bitwise->mb;

	mb_copy.mlcl = dmap_structure_add (NULL, DMAP_CC_MLCL);;

	DMAP_SHARE_GET_CLASS (share_bitwise->mb.share)->add_entry_to_mlcl (id,
									record,
									&mb_copy);
	share_bitwise->size += dmap_structure_get_size (mb_copy.mlcl);

	/* Minus eight because we do not want to add size of MLCL CC field + size field n times,
	 * where n == number of records.
	 */
	share_bitwise->size -= 8;

	/* Destroy created structures as we go. */
	dmap_structure_destroy (mb_copy.mlcl);
}

static void
write_dmap_preamble (SoupMessage * message, GNode * node)
{
	guint length;
	gchar *data = dmap_structure_serialize (node, &length);

	soup_message_body_append (message->response_body,
				  SOUP_MEMORY_TAKE, data, length);
	dmap_structure_destroy (node);
}

static void
write_next_mlit (SoupMessage * message, struct share_bitwise_t *share_bitwise)
{
	if (share_bitwise->id_list == NULL) {
		g_debug ("No more ID's, sending message complete.");
		soup_message_body_complete (message->response_body);
	} else {
		gchar *data = NULL;
		guint length;
		DMAPRecord *record;
		struct MLCL_Bits mb = { NULL, 0, NULL };

		record = share_bitwise->lookup_by_id (share_bitwise->db,
						      GPOINTER_TO_UINT
						      (share_bitwise->
						       id_list->data));

		mb.bits = share_bitwise->mb.bits;
		mb.mlcl = dmap_structure_add (NULL, DMAP_CC_MLCL);
		mb.share = share_bitwise->mb.share;

		DMAP_SHARE_GET_CLASS (share_bitwise->mb.share)->
			add_entry_to_mlcl (share_bitwise->id_list->data,
					   record, &mb);
		data = dmap_structure_serialize (g_node_first_child (mb.mlcl),
						 &length);

		soup_message_body_append (message->response_body,
					  SOUP_MEMORY_TAKE, data, length);
		g_debug ("Sending ID %u.",
			 GPOINTER_TO_UINT (share_bitwise->id_list->data));
		dmap_structure_destroy (mb.mlcl);

		share_bitwise->id_list =
			g_slist_remove (share_bitwise->id_list,
					share_bitwise->id_list->data);

		g_object_unref (record);
	}

	soup_server_unpause_message (share_bitwise->server, message);
}

static void
chunked_message_finished (SoupMessage * message,
			  struct share_bitwise_t *share_bitwise)
{
	g_debug ("Finished sending chunked data.");
	if (share_bitwise->destroy)
		share_bitwise->destroy (share_bitwise->db);
	g_free (share_bitwise);
}

DMAPRecord *
g_hash_table_lookup_adapter (GHashTable * ht, guint id)
{
	/* NOTE: each time this is called by write_next_mlit(), the
	 * returned value will be unref'ed by write_next_mlit(). We
	 * also need to destroy the GHashTable, so bump up the reference
	 * count so that both can happen. */
	return g_object_ref (g_hash_table_lookup (ht, GUINT_TO_POINTER (id)));
}

void
_dmap_share_databases (DMAPShare * share,
		       SoupServer * server,
		       SoupMessage * message,
		       const char *path,
		       GHashTable * query, SoupClientContext * context)
{
	const char *rest_of_path;

	g_debug ("Path is %s.", path);
	g_hash_table_foreach (query, debug_param, NULL);

	if (!_dmap_share_session_id_validate
	    (share, context, message, query, NULL)) {
		soup_message_set_status (message, SOUP_STATUS_FORBIDDEN);
		return;
	}

	rest_of_path = strchr (path + 1, '/');

	if (rest_of_path == NULL) {
		/* AVDB server databases
		 *      MSTT status
		 *      MUTY update type
		 *      MTCO specified total count
		 *      MRCO returned count
		 *      MLCL listing
		 *              MLIT listing item
		 *                      MIID item id
		 *                      MPER persistent id
		 *                      MINM item name
		 *                      MIMC item count
		 *                      MCTC container count
		 */
		GNode *avdb;
		GNode *mlcl;
		GNode *mlit;

		avdb = dmap_structure_add (NULL, DMAP_CC_AVDB);
		dmap_structure_add (avdb, DMAP_CC_MSTT,
				    (gint32) DMAP_STATUS_OK);
		dmap_structure_add (avdb, DMAP_CC_MUTY, 0);
		dmap_structure_add (avdb, DMAP_CC_MTCO, (gint32) 1);
		dmap_structure_add (avdb, DMAP_CC_MRCO, (gint32) 1);
		mlcl = dmap_structure_add (avdb, DMAP_CC_MLCL);
		mlit = dmap_structure_add (mlcl, DMAP_CC_MLIT);
		dmap_structure_add (mlit, DMAP_CC_MIID, (gint32) 1);
		dmap_structure_add (mlit, DMAP_CC_MPER, (gint64) 1);
		dmap_structure_add (mlit, DMAP_CC_MINM, share->priv->name);
		dmap_structure_add (mlit, DMAP_CC_MIMC,
				    dmap_db_count (share->priv->db));
		dmap_structure_add (mlit, DMAP_CC_MCTC, (gint32) 1);

		_dmap_share_message_set_from_dmap_structure (share, message,
							     avdb);
		dmap_structure_destroy (avdb);
	} else if (g_ascii_strcasecmp ("/1/groups", rest_of_path) == 0) {
		/* ADBS database songs
		 *      MSTT status
		 *      MUTY update type
		 *      MTCO specified total count
		 *      MRCO returned count
		 *      MLCL listing
		 *              MLIT
		 *                      attrs
		 *              MLIT
		 *              ...
		 */

		GSList *filter_def;
		gchar *record_query;
		GHashTable *records = NULL;
		GHashTable *groups;
		GList *values;
		GList *value;
		gchar *sort_by;
		GroupInfo *group_info;
		GNode *agal;
		GNode *mlcl;
		GNode *mlit;
		gint num;

		if (g_strcmp0
		    (g_hash_table_lookup (query, "group-type"),
		     "albums") != 0) {
			g_warning ("Unsupported grouping");
			soup_message_set_status (message,
						 SOUP_STATUS_INTERNAL_SERVER_ERROR);
			return;
		}

		record_query = g_hash_table_lookup (query, "query");
		filter_def = _dmap_share_build_filter (record_query);
		records =
			dmap_db_apply_filter (DMAP_DB (share->priv->db),
					      filter_def);

		groups = g_hash_table_new_full (g_str_hash, g_str_equal,
						g_free, g_free);
		g_hash_table_foreach (records, (GHFunc) group_items, groups);

		agal = dmap_structure_add (NULL, DMAP_CC_AGAL);
		dmap_structure_add (agal, DMAP_CC_MSTT,
				    (gint32) DMAP_STATUS_OK);
		dmap_structure_add (agal, DMAP_CC_MUTY, 0);

		num = g_hash_table_size (groups);
		dmap_structure_add (agal, DMAP_CC_MTCO, (gint32) num);
		dmap_structure_add (agal, DMAP_CC_MRCO, (gint32) num);

		mlcl = dmap_structure_add (agal, DMAP_CC_MLCL);

		values = g_hash_table_get_values (groups);
		if (g_hash_table_lookup (query, "include-sort-headers")) {
			sort_by = g_hash_table_lookup (query, "sort");
			if (g_strcmp0 (sort_by, "album") == 0) {
				values = g_list_sort (values, group_info_cmp);
			} else {
				g_warning ("Unknown sort column: %s",
					   sort_by);
			}
		}

		for (value = values; value; value = g_list_next (value)) {
			group_info = (GroupInfo *) value->data;
			mlit = dmap_structure_add (mlcl, DMAP_CC_MLIT);
			dmap_structure_add (mlit, DMAP_CC_MIID,
					    (gint) group_info->group_id);
			dmap_structure_add (mlit, DMAP_CC_MPER,
					    group_info->group_id);
			dmap_structure_add (mlit, DMAP_CC_MINM,
					    group_info->name);
			dmap_structure_add (mlit, DMAP_CC_ASAA,
					    group_info->artist);
			dmap_structure_add (mlit, DMAP_CC_MIMC,
					    (gint32) group_info->count);

			// Free this now, since the hash free func won't.
			// Name will be freed when the hash table keys are freed.
			g_free (group_info->artist);
		}

		g_list_free (values);
		dmap_share_free_filter (filter_def);

		_dmap_share_message_set_from_dmap_structure (share, message,
							     agal);

		g_hash_table_destroy (records);
		g_hash_table_destroy (groups);
		dmap_structure_destroy (agal);
	} else if (g_ascii_strcasecmp ("/1/items", rest_of_path) == 0) {
		/* ADBS database songs
		 *      MSTT status
		 *      MUTY update type
		 *      MTCO specified total count
		 *      MRCO returned count
		 *      MLCL listing
		 *              MLIT
		 *                      attrs
		 *              MLIT
		 *              ...
		 */
		GNode *adbs;
		gchar *record_query;
		GHashTable *records = NULL;
		struct DMAPMetaDataMap *map;
		gint32 num_songs;
		struct MLCL_Bits mb = { NULL, 0, NULL };
		struct share_bitwise_t *share_bitwise;

		record_query = g_hash_table_lookup (query, "query");
		if (record_query) {
			GSList *filter_def;

			filter_def = _dmap_share_build_filter (record_query);
			records =
				dmap_db_apply_filter (DMAP_DB
						      (share->priv->db),
						      filter_def);
			num_songs = g_hash_table_size (records);
			g_debug ("Found %d records", num_songs);
			dmap_share_free_filter (filter_def);
		} else {
			num_songs = dmap_db_count (share->priv->db);
		}

		map = DMAP_SHARE_GET_CLASS (share)->get_meta_data_map (share);
		mb.bits = _dmap_share_parse_meta (query, map);
		mb.share = share;

		/* NOTE:
		 * We previously simply called foreach...add_entry_to_mlcl and later serialized the entire
		 * structure. This has the disadvantage that the entire response must be in memory before
		 * libsoup sends it to the client.
		 *
		 * Now, we go through the database in multiple passes (as an interim solution):
		 *
		 * 1. Accumulate the eventual size of the MLCL by creating and then free'ing each MLIT.
		 * 2. Generate the DAAP preamble ending with the MLCL (with size fudged for ADBS and MLCL).
		 * 3. Setup libsoup response headers, etc.
		 * 4. Setup callback to transmit DAAP preamble (write_dmap_preamble)
		 * 5. Setup callback to transmit MLIT's (write_next_mlit)
		 */

		/* 1: */
		share_bitwise = g_new (struct share_bitwise_t, 1);

		share_bitwise->server = server;
		share_bitwise->mb = mb;
		share_bitwise->id_list = NULL;
		share_bitwise->size = 0;
		if (record_query) {
			share_bitwise->db = records;
			share_bitwise->lookup_by_id = (ShareBitwiseLookupByIdFunc)
				g_hash_table_lookup_adapter;
			share_bitwise->destroy = (ShareBitwiseDestroyFunc) g_hash_table_destroy;
			g_hash_table_foreach (records,
					      (GHFunc)
					      accumulate_mlcl_size_and_ids,
					      share_bitwise);
		} else {
			share_bitwise->db = share->priv->db;
			share_bitwise->lookup_by_id = (ShareBitwiseLookupByIdFunc) dmap_db_lookup_by_id;
			share_bitwise->destroy = NULL;
			dmap_db_foreach (share->priv->db,
					 (GHFunc)
					 accumulate_mlcl_size_and_ids,
					 share_bitwise);
		}

		/* 2: */
		adbs = dmap_structure_add (NULL, DMAP_CC_ADBS);
		dmap_structure_add (adbs, DMAP_CC_MSTT,
				    (gint32) DMAP_STATUS_OK);
		dmap_structure_add (adbs, DMAP_CC_MUTY, 0);
		dmap_structure_add (adbs, DMAP_CC_MTCO, (gint32) num_songs);
		dmap_structure_add (adbs, DMAP_CC_MRCO, (gint32) num_songs);
		mb.mlcl = dmap_structure_add (adbs, DMAP_CC_MLCL);
		dmap_structure_increase_by_predicted_size (adbs,
							   share_bitwise->
							   size);
		dmap_structure_increase_by_predicted_size (mb.mlcl,
							   share_bitwise->
							   size);

		/* 3: */
		/* Free memory after each chunk sent out over network. */
		soup_message_body_set_accumulate (message->response_body,
						  FALSE);
		soup_message_headers_append (message->response_headers,
					     "Content-Type",
					     "application/x-dmap-tagged");
		DMAP_SHARE_GET_CLASS (share)->
			message_add_standard_headers (share, message);
		soup_message_headers_set_content_length (message->
							 response_headers,
							 dmap_structure_get_size
							 (adbs));
		soup_message_set_status (message, SOUP_STATUS_OK);

		/* 4: */
		g_signal_connect (message, "wrote_headers",
				  G_CALLBACK (write_dmap_preamble), adbs);

		/* 5: */
		g_signal_connect (message, "wrote_chunk",
				  G_CALLBACK (write_next_mlit),
				  share_bitwise);
		g_signal_connect (message, "finished",
				  G_CALLBACK (chunked_message_finished),
				  share_bitwise);

	} else if (g_ascii_strcasecmp ("/1/containers", rest_of_path) == 0) {
		/* APLY database playlists
		 *      MSTT status
		 *      MUTY update type
		 *      MTCO specified total count
		 *      MRCO returned count
		 *      MLCL listing
		 *              MLIT listing item
		 *                      MIID item id
		 *                      MPER persistent item id
		 *                      MINM item name
		 *                      MIMC item count
		 *                      ABPL baseplaylist (only for base)
		 *              MLIT
		 *              ...
		 */
		GNode *aply;
		GNode *mlit;
		struct DMAPMetaDataMap *map;
		struct MLCL_Bits mb = { NULL, 0, NULL };

		map = DMAP_SHARE_GET_CLASS (share)->get_meta_data_map (share);
		mb.bits = _dmap_share_parse_meta (query, map);
		mb.share = share;

		aply = dmap_structure_add (NULL, DMAP_CC_APLY);
		dmap_structure_add (aply, DMAP_CC_MSTT,
				    (gint32) DMAP_STATUS_OK);
		dmap_structure_add (aply, DMAP_CC_MUTY, 0);
		dmap_structure_add (aply, DMAP_CC_MTCO,
				    (gint32) dmap_container_db_count (share->
								      priv->
								      container_db)
				    + 1);
		dmap_structure_add (aply, DMAP_CC_MRCO,
				    (gint32) dmap_container_db_count (share->
								      priv->
								      container_db)
				    + 1);
		mb.mlcl = dmap_structure_add (aply, DMAP_CC_MLCL);

		/* Base playlist (playlist 1 contains all songs): */
		mlit = dmap_structure_add (mb.mlcl, DMAP_CC_MLIT);
		dmap_structure_add (mlit, DMAP_CC_MIID, (gint32) 1);
		dmap_structure_add (mlit, DMAP_CC_MPER, (gint64) 1);
		dmap_structure_add (mlit, DMAP_CC_MINM, share->priv->name);
		dmap_structure_add (mlit, DMAP_CC_MIMC,
				    dmap_db_count (share->priv->db));
		dmap_structure_add (mlit, DMAP_CC_FQUESCH, 0);
		dmap_structure_add (mlit, DMAP_CC_MPCO, 0);
		dmap_structure_add (mlit, DMAP_CC_AESP, 0);
		dmap_structure_add (mlit, DMAP_CC_AEPP, 0);
		dmap_structure_add (mlit, DMAP_CC_AEPS, 0);
		dmap_structure_add (mlit, DMAP_CC_AESG, 0);

		dmap_structure_add (mlit, DMAP_CC_ABPL, (gchar) 1);

		dmap_container_db_foreach (share->priv->container_db,
					   (GHFunc)
					   _dmap_share_add_playlist_to_mlcl,
					   &mb);

		_dmap_share_message_set_from_dmap_structure (share, message,
							     aply);
		dmap_structure_destroy (aply);
	} else if (g_ascii_strncasecmp ("/1/containers/", rest_of_path, 14) ==
		   0) {
		/* APSO playlist songs
		 *      MSTT status
		 *      MUTY update type
		 *      MTCO specified total count
		 *      MRCO returned count
		 *      MLCL listing
		 *              MLIT listing item
		 *                      MIKD item kind
		 *                      MIID item id
		 *                      MCTI container item id
		 *              MLIT
		 *              ...
		 */
		GNode *apso;
		struct DMAPMetaDataMap *map;
		struct MLCL_Bits mb = { NULL, 0, NULL };
		guint pl_id;
		gchar *record_query;
		GSList *filter_def;
		GHashTable *records;

		map = DMAP_SHARE_GET_CLASS (share)->get_meta_data_map (share);
		mb.bits = _dmap_share_parse_meta (query, map);
		mb.share = share;

		apso = dmap_structure_add (NULL, DMAP_CC_APSO);
		dmap_structure_add (apso, DMAP_CC_MSTT,
				    (gint32) DMAP_STATUS_OK);
		dmap_structure_add (apso, DMAP_CC_MUTY, 0);

		if (g_ascii_strcasecmp ("/1/items", rest_of_path + 13) == 0) {
			GList *id;
			gchar *sort_by;
			GList *keys;

			record_query = g_hash_table_lookup (query, "query");
			filter_def = _dmap_share_build_filter (record_query);
			records =
				dmap_db_apply_filter (DMAP_DB
						      (share->priv->db),
						      filter_def);
			gint32 num_songs = g_hash_table_size (records);

			g_debug ("Found %d records", num_songs);
			dmap_share_free_filter (filter_def);

			dmap_structure_add (apso, DMAP_CC_MTCO,
					    (gint32) num_songs);
			dmap_structure_add (apso, DMAP_CC_MRCO,
					    (gint32) num_songs);
			mb.mlcl = dmap_structure_add (apso, DMAP_CC_MLCL);

			sort_by = g_hash_table_lookup (query, "sort");
			keys = g_hash_table_get_keys (records);
			if (g_strcmp0 (sort_by, "album") == 0) {
				keys = g_list_sort_with_data (keys,
							      (GCompareDataFunc)
							      daap_record_cmp_by_album,
							      share->priv->
							      db);
			} else if (sort_by != NULL) {
				g_warning ("Unknown sort column: %s",
					   sort_by);
			}

			for (id = keys; id; id = id->next) {
				(*
				 (DMAP_SHARE_GET_CLASS (share)->
				  add_entry_to_mlcl)) (id->data,
						       g_hash_table_lookup
						       (records, id->data),
						       &mb);
			}

			g_list_free (keys);
			g_hash_table_destroy (records);
		} else {
			pl_id = strtoul (rest_of_path + 14, NULL, 10);
			if (pl_id == 1) {
				gint32 num_songs =
					dmap_db_count (share->priv->db);
				dmap_structure_add (apso, DMAP_CC_MTCO,
						    (gint32) num_songs);
				dmap_structure_add (apso, DMAP_CC_MRCO,
						    (gint32) num_songs);
				mb.mlcl =
					dmap_structure_add (apso,
							    DMAP_CC_MLCL);

				dmap_db_foreach (share->priv->db,
						 (GHFunc)
						 DMAP_SHARE_GET_CLASS
						 (share)->add_entry_to_mlcl,
						 &mb);
			} else {
				DMAPContainerRecord *record;
				DMAPDb *entries;
				guint num_songs;

				record = dmap_container_db_lookup_by_id
					(share->priv->container_db, pl_id);
				entries =
					dmap_container_record_get_entries
					(record);
				/* FIXME: what if entries is NULL (handled in dmapd but should be [also] handled here)? */
				num_songs = dmap_db_count (entries);

				dmap_structure_add (apso, DMAP_CC_MTCO,
						    (gint32) num_songs);
				dmap_structure_add (apso, DMAP_CC_MRCO,
						    (gint32) num_songs);
				mb.mlcl =
					dmap_structure_add (apso,
							    DMAP_CC_MLCL);

				dmap_db_foreach (entries,
						 (GHFunc)
						 DMAP_SHARE_GET_CLASS
						 (share)->add_entry_to_mlcl,
						 &mb);

				g_object_unref (entries);
				g_object_unref (record);
			}
		}

		_dmap_share_message_set_from_dmap_structure (share, message,
							     apso);
		dmap_structure_destroy (apso);
	} else if (g_ascii_strncasecmp ("/1/browse/", rest_of_path, 9) == 0) {
		DMAP_SHARE_GET_CLASS (share)->databases_browse_xxx (share,
								    server,
								    message,
								    path,
								    query,
								    context);
	} else if (g_ascii_strncasecmp ("/1/items/", rest_of_path, 9) == 0) {
		/* just the file :) */
		DMAP_SHARE_GET_CLASS (share)->databases_items_xxx (share,
								   server,
								   message,
								   path,
								   query,
								   context);
	} else if (g_str_has_prefix (rest_of_path, "/1/groups/") &&
		   g_str_has_suffix (rest_of_path, "/extra_data/artwork")) {
		/* We don't yet implement cover requests here, say no cover */
		g_debug ("Assuming no artwork for requested group/album");
		soup_message_set_status (message, SOUP_STATUS_NOT_FOUND);
	} else {
		g_warning ("Unhandled: %s\n", path);
	}
}
