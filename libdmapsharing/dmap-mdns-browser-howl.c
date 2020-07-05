/*
 * Copyright (C) 2005 Charles Schmidt <cschmidt2@emich.edu>
 * Copyright (C) 2006 William Jon McCann <mccann@jhu.edu>
 * Copyright (C) 2006 INdT
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

#include <libdmapsharing/dmap.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <glib-object.h>

#undef PACKAGE
#undef PACKAGE_NAME
#undef PACKAGE_VERSION
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef VERSION
#include <howl.h>

#include <libgnomevfs/gnome-vfs-address.h>
#include <libgnomevfs/gnome-vfs-resolve.h>

struct DmapMdnsBrowserPrivate
{
	DmapMdnsBrowserServiceType service_type;
	sw_discovery *discovery;
	sw_discovery_oid *oid;

	GnomeVFSAddress *local_address;
	guint watch_id;
	GSList *services;
	GSList *resolvers;
};

enum
{
	SERVICE_ADDED,
	SERVICE_REMOVED,
	LAST_SIGNAL
};

/* stupid howl includes howl_config.h */
#undef PACKAGE_VERSION
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE
#undef VERSION
#include <howl.h>

static void dmap_mdns_browser_class_init (DmapMdnsBrowserClass * klass);
static void dmap_mdns_browser_init (DmapMdnsBrowser * browser);
static void dmap_mdns_browser_dispose (GObject * object);
static void dmap_mdns_browser_finalize (GObject * object);

static guint signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE_WITH_PRIVATE (DmapMdnsBrowser,
                            dmap_mdns_browser,
                            G_TYPE_OBJECT);

GQuark
dmap_mdns_browser_error_quark (void)
{
	static GQuark quark = 0;

	if (!quark)
		quark = g_quark_from_static_string
			("dmap_mdns_browser_error");

	return quark;
}

static gboolean
howl_in_cb (GIOChannel * io_channel,
	    GIOCondition condition, DmapMdnsBrowser * browser)
{
	sw_salt salt;

	if (sw_discovery_salt (*browser->priv->discovery, &salt) == SW_OKAY) {
		sw_salt_lock (salt);
		sw_discovery_read_socket (*browser->priv->discovery);
		sw_salt_unlock (salt);
	}

	return TRUE;
}

static void
howl_client_init (DmapMdnsBrowser * browser)
{
	sw_result result;
	int fd;
	GIOChannel *channel;

	browser->priv->discovery = g_new0 (sw_discovery, 1);
	result = sw_discovery_init (browser->priv->discovery);

	if (result != SW_OKAY) {
		g_free (browser->priv->discovery);
		browser->priv->discovery = NULL;
		return;
	}

	fd = sw_discovery_socket (*browser->priv->discovery);

	channel = g_io_channel_unix_new (fd);
	browser->priv->watch_id =
		g_io_add_watch (channel, G_IO_IN, (GIOFunc) howl_in_cb,
				browser);
	g_io_channel_unref (channel);
}

static gboolean
host_is_local (DmapMdnsBrowser * browser, const char *host)
{
	GnomeVFSAddress *remote;
	gboolean equal;
	guint32 l_ip;
	guint32 r_ip;

	if (browser->priv->local_address == NULL) {
		g_warning ("Unable to resolve address");
		return FALSE;
	}

	remote = gnome_vfs_address_new_from_string (host);
	if (remote == NULL) {
		g_warning ("Unable to resolve address for %s", host);
		return FALSE;
	}

	l_ip = gnome_vfs_address_get_ipv4 (browser->priv->local_address);
	r_ip = gnome_vfs_address_get_ipv4 (remote);
	equal = l_ip == r_ip;

	/* FIXME: Use this when we can depend on gnome-vfs 2.14 */
	/*equal = gnome_vfs_address_equal (browser->priv->local_address, remote); */

	gnome_vfs_address_free (remote);

	return equal;
}

static void
set_local_address (DmapMdnsBrowser * browser)
{
	char host_name[256];
	GnomeVFSResolveHandle *rh;
	GnomeVFSAddress *address;
	GnomeVFSResult res;

	if (gethostname (host_name, sizeof (host_name)) != 0) {
		g_warning ("gethostname failed: %s", g_strerror (errno));
		return;
	}

	res = gnome_vfs_resolve (host_name, &rh);

	if (res != GNOME_VFS_OK) {
		return;
	}

	address = NULL;
	while (gnome_vfs_resolve_next_address (rh, &address)) {
		if (browser->priv->local_address == NULL) {
			browser->priv->local_address =
				gnome_vfs_address_dup (address);
		}
		gnome_vfs_address_free (address);
	}

	gnome_vfs_resolve_free (rh);
}

static sw_result
resolve_cb (sw_discovery discovery,
	    sw_discovery_oid oid,
	    sw_uint32 interface_index,
	    sw_const_string service_name,
	    sw_const_string type,
	    sw_const_string domain,
	    sw_ipv4_address address,
	    sw_port port,
	    sw_octets text_record,
	    sw_ulong text_record_length, DmapMdnsBrowser * browser)
{
	char *host;
	char *name;
	char *pair;
	sw_text_record_iterator it;
	gboolean pp = FALSE;
	DmapMdnsBrowserService *service;

	host = g_malloc (16);
	name = NULL;

	sw_ipv4_address_name (address, host, 16);

	/* skip local services */
	if (host_is_local (browser, host)) {
		goto done;
	}

	if (sw_text_record_iterator_init
	    (&it, text_record, text_record_length) == SW_OKAY) {
		sw_char key[SW_TEXT_RECORD_MAX_LEN];
		sw_octet val[SW_TEXT_RECORD_MAX_LEN];
		sw_ulong val_len;

		while (sw_text_record_iterator_next
		       (it, (char *) key, val, &val_len) == SW_OKAY) {
			if (strcmp ((char *) key, "Password") == 0) {
				if (val_len >= 4
				    && strncmp ((char *) val, "true",
						4) == 0) {
					pp = TRUE;
				}
			}
			if (strcmp ((char *) key, "Machine Name") == 0) {
				if (name != NULL)
					g_free (name);
				name = g_strdup ((char *) val);
			} else if (strcmp ((char *) key, "DvNm") == 0) {
				if (name != NULL)
					g_free (name);
				/* Remote's name is presented as DvNm in DACP */
				name = g_strdup ((char *) val);
			} else if (strcmp ((char *) key, "Pair") == 0) {
				if (pair != NULL)
					g_free (pair);
				/* Pair is used when first connecting to a DACP remote */
				pair = g_strdup ((char *) val);
			}
		}

		sw_text_record_iterator_fina (it);
	}

	if (name == NULL) {
		name = g_strdup (service_name);
	}

	service = g_new0 (DmapMdnsBrowserService, 1);
	service->service_name = g_strdup (service_name);
	service->name = name;
	service->host = g_strdup (host);
	service->port = port;
	service->password_protected = pp;
	service->pair = pair;
	service->transport_protocol = DMAP_MDNS_BROWSER_TRANSPORT_PROTOCOL_TCP;

	browser->priv->services =
		g_slist_append (browser->priv->services, service);

	g_signal_emit (browser, signals[SERVICE_ADDED], 0, service);
      done:
	g_free (host);
	g_free (name);

	return SW_OKAY;
}

static gboolean
dmap_mdns_browser_resolve (DmapMdnsBrowser * browser, const char *name)
{
	sw_result result;
	sw_discovery_oid oid;

	result = sw_discovery_resolve (*browser->priv->discovery,
				       0,
				       name,
				       "_dmap._tcp",
				       "local", (sw_discovery_resolve_reply)
				       resolve_cb, (sw_opaque) browser,
				       (sw_discovery_oid *) & oid);

	return TRUE;
}

static void
browser_add_service (DmapMdnsBrowser * browser, const char *service_name)
{
	dmap_mdns_browser_resolve (browser, service_name);
}

static void
browser_remove_service (DmapMdnsBrowser * browser, const char *service_name)
{
	g_signal_emit (browser,
		       dmap_mdns_browser_signals[SERVICE_REMOVED],
		       0, service_name);
}

static sw_result
browse_cb (sw_discovery discovery,
	   sw_discovery_oid oid,
	   sw_discovery_browse_status status,
	   sw_uint32 interface_index,
	   sw_const_string name,
	   sw_const_string type,
	   sw_const_string domain, DmapMdnsBrowser * browser)
{
	if (status == SW_DISCOVERY_BROWSE_ADD_SERVICE) {
		browser_add_service (browser, name);
	} else if (status == SW_DISCOVERY_BROWSE_REMOVE_SERVICE) {
		browser_remove_service (browser, name);
	}

	return SW_OKAY;
}

DmapMdnsBrowser *
dmap_mdns_browser_new (DmapMdnsBrowserServiceType type)
{
	DmapMdnsBrowser *browser_object;

	g_return_val_if_fail (type >= DMAP_MDNS_BROWSER_SERVICE_TYPE_INVALID
			      && type <= DMAP_MDNS_BROWSER_SERVICE_TYPE_LAST,
			      NULL);

	browser_object =
		DMAP_MDNS_BROWSER (g_object_new
				   (DMAP_TYPE_MDNS_BROWSER, NULL));
	browser_object->priv->service_type = type;

	return browser_object;
}

gboolean
dmap_mdns_browser_start (DmapMdnsBrowser * browser, GError ** error)
{
	sw_result result;

	if (browser->priv->discovery == NULL) {
		g_set_error (error,
			     DMAP_MDNS_BROWSER_ERROR,
			     DMAP_MDNS_BROWSER_ERROR_NOT_RUNNING,
			     "%s", _("MDNS service is not running"));
		return FALSE;
	}

	if (browser->priv->oid != NULL) {
		g_debug ("Browser already active");
		return FALSE;
	}

	browser->priv->oid = g_new0 (sw_discovery_oid, 1);

	result = sw_discovery_browse (*browser->priv->discovery,
				      0,
				      "_dmap._tcp",
				      "local",
				      (sw_discovery_browse_reply) browse_cb,
				      (sw_opaque) browser,
				      (sw_discovery_oid *) browser->
				      priv->oid);

	if (result != SW_OKAY) {
		g_debug ("Error starting mDNS discovery using Howl");
		g_set_error (error,
			     DMAP_MDNS_BROWSER_ERROR,
			     DMAP_MDNS_BROWSER_ERROR_FAILED,
			     "%s", _("Unable to activate browser"));

		return FALSE;
	}

	return TRUE;
}

gboolean
dmap_mdns_browser_stop (DmapMdnsBrowser * browser, GError ** error)
{
	if (browser->priv->discovery == NULL) {
		g_set_error (error,
			     DMAP_MDNS_BROWSER_ERROR,
			     DMAP_MDNS_BROWSER_ERROR_NOT_RUNNING,
			     "%s", _("MDNS service is not running"));
		return FALSE;
	}
	if (browser->priv->oid == NULL) {
		g_set_error (error,
			     DMAP_MDNS_BROWSER_ERROR,
			     DMAP_MDNS_BROWSER_ERROR_FAILED,
			     "%s", _("Browser is not active"));
		return FALSE;

	}

	sw_discovery_cancel (*browser->priv->discovery, *browser->priv->oid);

	g_free (browser->priv->oid);
	browser->priv->oid = NULL;

	return TRUE;
}

DmapMdnsBrowserServiceType
dmap_mdns_browser_get_service_type (DmapMdnsBrowser * browser)
{
	g_return_val_if_fail (browser != NULL,
			      DMAP_MDNS_BROWSER_SERVICE_TYPE_INVALID);
	return browser->priv->service_type;
}

static void
dmap_mdns_browser_class_init (DmapMdnsBrowserClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = dmap_mdns_browser_dispose;
	object_class->finalize = dmap_mdns_browser_finalize;

	signals[SERVICE_ADDED] =
		g_signal_new ("service-added",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (DmapMdnsBrowserClass,
					       service_added), NULL, NULL,
			      NULL, G_TYPE_NONE,
			      1, DMAP_TYPE_MDNS_SERVICE);
	signals[SERVICE_REMOVED] =
		g_signal_new ("service-removed",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (DmapMdnsBrowserClass,
					       service_removed), NULL, NULL,
			      NULL, G_TYPE_NONE, 1,
			      G_TYPE_STRING);
}

static void
dmap_mdns_browser_init (DmapMdnsBrowser * browser)
{
	memset (browser->priv, 0, sizeof (DmapMdnsBrowserPrivate));

	browser->priv = dmap_mdns_browser_get_instance_private(browser);

	set_local_address (browser);

	howl_client_init (browser);
}

static void
resolver_free (sw_discovery_oid * oid, DmapMdnsBrowser * browser)
{
	sw_discovery_cancel (*browser->priv->discovery, *oid);
	g_free (oid);
}

static void
dmap_mdns_browser_dispose (GObject * object)
{
	DmapMdnsBrowser *browser = DMAP_MDNS_BROWSER (object);
	GSList *walk;
	DmapMdnsBrowserService *service;

	if (browser->priv->oid) {
		dmap_mdns_browser_stop (browser, NULL);
	}

	for (walk = browser->priv->services; walk; walk = walk->next) {
		service = (DmapMdnsBrowserService *) walk->data;
		g_object_unref (service);
	}
	g_slist_free (browser->priv->services);

	if (browser->priv->resolvers) {
		g_slist_foreach (browser->priv->resolvers,
				 (GFunc) resolver_free, browser);
		g_slist_free (browser->priv->resolvers);
	}

	if (browser->priv->discovery) {
		sw_discovery_fina (*browser->priv->discovery);
		g_free (browser->priv->discovery);
	}

	if (browser->priv->watch_id > 0) {
		g_source_remove (browser->priv->watch_id);
	}

	if (browser->priv->local_address) {
		gnome_vfs_address_free (browser->priv->local_address);
	}

	G_OBJECT_CLASS (dmap_mdns_browser_parent_class)->dispose (object);
}

static void
dmap_mdns_browser_finalize (GObject * object)
{
	g_signal_handlers_destroy (object);
	G_OBJECT_CLASS (dmap_mdns_browser_parent_class)->finalize (object);
}
