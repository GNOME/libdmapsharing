/*
 * Copyright (C) 2005 Charles Schmidt <cschmidt2@emich.edu>
 * Copyright (C) 2006 William Jon McCann <mccann@jhu.edu>
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

#include <libdmapsharing/dmap.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <glib-object.h>

#ifdef HAVE_AVAHI_0_6
#include <avahi-client/lookup.h>
#include <avahi-client/publish.h>
#endif
#include <avahi-client/client.h>
#include <avahi-common/error.h>
#include <avahi-glib/glib-malloc.h>
#include <avahi-glib/glib-watch.h>

struct _DMAPMdnsBrowserPrivate
{
	DMAPMdnsBrowserServiceType service_type;
	AvahiClient *client;
	AvahiGLibPoll *poll;
	AvahiServiceBrowser *service_browser;
	GSList *services;
	GSList *resolvers;
};

enum
{
	SERVICE_ADDED,
	SERVICE_REMOVED,
	LAST_SIGNAL
};

#ifdef HAVE_AVAHI_0_5
#define AVAHI_ADDRESS_STR_MAX (40)	/* IPv6 Max = 4*8 + 7 + 1 for NUL */
#endif

static void dmap_mdns_browser_class_init (DMAPMdnsBrowserClass * klass);
static void dmap_mdns_browser_init (DMAPMdnsBrowser * browser);
static void dmap_mdns_browser_dispose (GObject * object);
static void dmap_mdns_browser_finalize (GObject * object);
static void avahi_client_init (DMAPMdnsBrowser * browser);
static void resolve_cb (AvahiServiceResolver * service_resolver,
			AvahiIfIndex interface,
			AvahiProtocol protocol,
			AvahiResolverEvent event,
			const gchar * service_name,
			const gchar * type,
			const gchar * domain,
			const gchar * host_name,
			const AvahiAddress * address,
			uint16_t port, AvahiStringList * text,
#ifdef HAVE_AVAHI_0_6
			AvahiLookupResultFlags flags,
#endif
			DMAPMdnsBrowser * browser);
static gboolean dmap_mdns_browser_resolve (DMAPMdnsBrowser * browser,
					   const gchar * name,
					   const gchar * domain);
static void browser_add_service (DMAPMdnsBrowser * browser,
				 const gchar * service_name,
				 const gchar * domain);
static void browser_remove_service (DMAPMdnsBrowser * browser,
				    const gchar * service_name);
static void browse_cb (AvahiServiceBrowser * service_browser,
		       AvahiIfIndex interface,
		       AvahiProtocol protocol,
		       AvahiBrowserEvent event,
		       const gchar * name,
		       const gchar * type, const gchar * domain,
#ifdef HAVE_AVAHI_0_6
		       AvahiLookupResultFlags flags,
#endif
		       DMAPMdnsBrowser * browser);
static void free_service (DMAPMdnsBrowserService * service);

#define DMAP_MDNS_BROWSER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), DMAP_TYPE_MDNS_BROWSER, DMAPMdnsBrowserPrivate))

static guint dmap_mdns_browser_signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (DMAPMdnsBrowser, dmap_mdns_browser, G_TYPE_OBJECT);

static gchar *service_type_name[] = {
	NULL,
	"_daap._tcp",
	"_dpap._tcp",
	"_touch-remote._tcp"
};

GQuark
dmap_mdns_browser_error_quark (void)
{
	static GQuark quark = 0;

	if (!quark)
		quark = g_quark_from_static_string
			("dmap_mdns_browser_error");

	return quark;
}

static void
dmap_mdns_browser_class_init (DMAPMdnsBrowserClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	dmap_mdns_browser_parent_class = g_type_class_peek_parent (klass);

	object_class->dispose = dmap_mdns_browser_dispose;
	object_class->finalize = dmap_mdns_browser_finalize;

	g_type_class_add_private (klass, sizeof (DMAPMdnsBrowserPrivate));

	dmap_mdns_browser_signals[SERVICE_ADDED] =
		g_signal_new ("service-added",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (DMAPMdnsBrowserClass,
					       service_added), NULL, NULL,
			      g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE,
			      1, G_TYPE_POINTER);
	dmap_mdns_browser_signals[SERVICE_REMOVED] =
		g_signal_new ("service-removed",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (DMAPMdnsBrowserClass,
					       service_removed), NULL, NULL,
			      g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1,
			      G_TYPE_STRING);
}

static void
dmap_mdns_browser_init (DMAPMdnsBrowser * browser)
{
	browser->priv = DMAP_MDNS_BROWSER_GET_PRIVATE (browser);
	avahi_client_init (browser);
}

static void
dmap_mdns_browser_dispose (GObject * object)
{
	DMAPMdnsBrowser *browser = DMAP_MDNS_BROWSER (object);
	GSList *walk;
	DMAPMdnsBrowserService *service;

	for (walk = browser->priv->services; walk; walk = walk->next) {
		service = (DMAPMdnsBrowserService *) walk->data;
		free_service (service);
	}
	g_slist_free (browser->priv->services);

	if (browser->priv->resolvers) {
		g_slist_foreach (browser->priv->resolvers,
				 (GFunc) avahi_service_resolver_free, NULL);
		g_slist_free (browser->priv->resolvers);
	}

	if (browser->priv->service_browser) {
		avahi_service_browser_free (browser->priv->service_browser);
	}

	if (browser->priv->client) {
		avahi_client_free (browser->priv->client);
	}

	if (browser->priv->poll) {
		avahi_glib_poll_free (browser->priv->poll);
	}

	G_OBJECT_CLASS (dmap_mdns_browser_parent_class)->dispose (object);
}

static void
dmap_mdns_browser_finalize (GObject * object)
{
	g_signal_handlers_destroy (object);
	G_OBJECT_CLASS (dmap_mdns_browser_parent_class)->finalize (object);
}

DMAPMdnsBrowser *
dmap_mdns_browser_new (DMAPMdnsBrowserServiceType type)
{
	DMAPMdnsBrowser *browser_object;

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
dmap_mdns_browser_start (DMAPMdnsBrowser * browser, GError ** error)
{
	if (browser->priv->client == NULL) {
		g_set_error (error,
			     DMAP_MDNS_BROWSER_ERROR,
			     DMAP_MDNS_BROWSER_ERROR_NOT_RUNNING,
			     "%s", _("MDNS service is not running"));
		return FALSE;
	}
	if (browser->priv->service_browser != NULL) {
		g_debug ("Browser already active");
		return TRUE;
	}

	browser->priv->service_browser =
		avahi_service_browser_new (browser->priv->client,
					   AVAHI_IF_UNSPEC,
					   AVAHI_PROTO_UNSPEC,
					   service_type_name[browser->
							     priv->service_type],
					   NULL,
#ifdef HAVE_AVAHI_0_6
					   0,
#endif
					   (AvahiServiceBrowserCallback)
					   browse_cb, browser);
	if (browser->priv->service_browser == NULL) {
		g_debug ("Error starting mDNS discovery using AvahiServiceBrowser");
		g_set_error (error,
			     DMAP_MDNS_BROWSER_ERROR,
			     DMAP_MDNS_BROWSER_ERROR_FAILED,
			     "%s", _("Unable to activate browser"));

		return FALSE;
	}

	return TRUE;
}

gboolean
dmap_mdns_browser_stop (DMAPMdnsBrowser * browser, GError ** error)
{
	if (browser->priv->client == NULL) {
		g_set_error (error,
			     DMAP_MDNS_BROWSER_ERROR,
			     DMAP_MDNS_BROWSER_ERROR_NOT_RUNNING,
			     "%s", _("MDNS service is not running"));
		return FALSE;
	}
	if (browser->priv->service_browser == NULL) {
		g_set_error (error,
			     DMAP_MDNS_BROWSER_ERROR,
			     DMAP_MDNS_BROWSER_ERROR_FAILED,
			     "%s", _("Browser is not active"));
		return FALSE;
	}
	avahi_service_browser_free (browser->priv->service_browser);
	browser->priv->service_browser = NULL;

	return TRUE;
}

G_CONST_RETURN GSList *
dmap_mdns_browser_get_services (DMAPMdnsBrowser * browser)
{
	g_return_val_if_fail (browser != NULL, NULL);
	return browser->priv->services;
}

DMAPMdnsBrowserServiceType
dmap_mdns_browser_get_service_type (DMAPMdnsBrowser * browser)
{
	g_return_val_if_fail (browser != NULL,
			      DMAP_MDNS_BROWSER_SERVICE_TYPE_INVALID);
	return browser->priv->service_type;
}

static void
client_cb (AvahiClient * client,
	   AvahiClientState state, DMAPMdnsBrowser * browser)
{
	/* Called whenever the client or server state changes */

	switch (state) {
#ifdef HAVE_AVAHI_0_6
	case AVAHI_CLIENT_FAILURE:
		g_warning ("Client failure: %s\n",
			   avahi_strerror (avahi_client_errno (client)));
		break;
#endif
	default:
		break;
	}
}

static void
avahi_client_init (DMAPMdnsBrowser * browser)
{
	gint error = 0;

	avahi_set_allocator (avahi_glib_allocator ());

	browser->priv->poll = avahi_glib_poll_new (NULL, G_PRIORITY_DEFAULT);

	if (!browser->priv->poll) {
		g_debug ("Unable to create AvahiGlibPoll object for mDNS");
	}
#ifdef HAVE_AVAHI_0_5
	browser->priv->client =
		avahi_client_new (avahi_glib_poll_get (browser->priv->poll),
				  (AvahiClientCallback) client_cb, browser,
				  &error);
#endif
#ifdef HAVE_AVAHI_0_6
	{
		AvahiClientFlags flags = 0;

		browser->priv->client =
			avahi_client_new (avahi_glib_poll_get
					  (browser->priv->poll), flags,
					  (AvahiClientCallback) client_cb,
					  browser, &error);
	}
#endif
}

static void
resolve_cb (AvahiServiceResolver * service_resolver,
	    AvahiIfIndex interface,
	    AvahiProtocol protocol,
	    AvahiResolverEvent event,
	    const gchar * service_name,
	    const gchar * type,
	    const gchar * domain,
	    const gchar * host_name,
	    const AvahiAddress * address,
	    uint16_t port, AvahiStringList * text,
#ifdef HAVE_AVAHI_0_6
	    AvahiLookupResultFlags flags,
#endif
	    DMAPMdnsBrowser * browser)
{
	gchar *name = NULL;
	gchar *pair = NULL;	/* FIXME: extract DACP-specific items into sub-class? See also howl code. */
	gchar host[AVAHI_ADDRESS_STR_MAX];
	gboolean pp = FALSE;
	DMAPMdnsBrowserService *service;

	switch (event) {
	case AVAHI_RESOLVER_FAILURE:
		g_warning
			("Failed to resolve service '%s' of type '%s' in domain '%s': %s\n",
			 service_name, type, domain,
			 avahi_strerror (avahi_client_errno
					 (avahi_service_resolver_get_client
					  (service_resolver))));
		break;
	case AVAHI_RESOLVER_FOUND:

		if (text) {
			AvahiStringList *l;

			for (l = text; l != NULL; l = l->next) {
				size_t size;
				gchar *key;
				gchar *value;
				gint ret;

				ret = avahi_string_list_get_pair (l, &key,
								  &value,
								  &size);
				if (ret != 0 || key == NULL) {
					continue;
				}

				if (strcmp (key, "Password") == 0) {
					if (size >= 4
					    && strncmp (value, "true",
							4) == 0) {
						pp = TRUE;
					} else if (size >= 1
						   && strncmp (value, "1",
							       1) == 0) {
						pp = TRUE;
					}
				} else if (strcmp (key, "Machine Name") == 0) {
					if (name == NULL)
						name = g_strdup (value);
				} else if (strcmp (key, "DvNm") == 0) {
					if (name != NULL)
						g_free (name);
					/* Remote's name is presented as DvNm in DACP */
					name = g_strdup (value);
				} else if (strcmp (key, "Pair") == 0) {
					/* Pair is used when first connecting to a DACP remote */
					pair = g_strdup (value);
				}

				g_free (key);
				g_free (value);
			}
		}

		if (name == NULL) {
			name = g_strdup (service_name);
		}

		avahi_address_snprint (host, AVAHI_ADDRESS_STR_MAX, address);

		service = g_new (DMAPMdnsBrowserService, 1);
		service->service_name = g_strdup (service_name);
		service->name = name;
		service->host = g_strdup (host);
		service->port = port;
		service->pair = pair;
		service->password_protected = pp;
		browser->priv->services =
			g_slist_append (browser->priv->services, service);
		g_signal_emit (browser,
			       dmap_mdns_browser_signals[SERVICE_ADDED], 0,
			       service);
		break;
	default:
		g_warning ("Unhandled event");
		break;
	}

	browser->priv->resolvers =
		g_slist_remove (browser->priv->resolvers, service_resolver);
	avahi_service_resolver_free (service_resolver);
}

static gboolean
dmap_mdns_browser_resolve (DMAPMdnsBrowser * browser,
			   const gchar * name, const gchar * domain)
{
	AvahiServiceResolver *service_resolver;

	service_resolver = avahi_service_resolver_new (browser->priv->client,
						       AVAHI_IF_UNSPEC,
						       AVAHI_PROTO_INET,
						       name,
						       service_type_name
						       [browser->
							priv->service_type],
						       domain,
						       AVAHI_PROTO_UNSPEC,
#ifdef HAVE_AVAHI_0_6
						       0,
#endif
						       (AvahiServiceResolverCallback) resolve_cb, browser);
	if (service_resolver == NULL) {
		g_debug ("Error starting mDNS resolving using AvahiServiceResolver");
		return FALSE;
	}

	browser->priv->resolvers =
		g_slist_prepend (browser->priv->resolvers, service_resolver);

	return TRUE;
}

static void
browser_add_service (DMAPMdnsBrowser * browser,
		     const gchar * service_name, const gchar * domain)
{
	dmap_mdns_browser_resolve (browser, service_name, domain);
}

static void
browser_remove_service (DMAPMdnsBrowser * browser, const gchar * service_name)
{
	g_signal_emit (browser,
		       dmap_mdns_browser_signals[SERVICE_REMOVED],
		       0, service_name);
}

static void
browse_cb (AvahiServiceBrowser * service_browser,
	   AvahiIfIndex interface,
	   AvahiProtocol protocol,
	   AvahiBrowserEvent event,
	   const gchar * name, const gchar * type, const gchar * domain,
#ifdef HAVE_AVAHI_0_6
	   AvahiLookupResultFlags flags,
#endif
	   DMAPMdnsBrowser * browser)
{
	gboolean local;

#ifdef HAVE_AVAHI_0_5
	local = avahi_client_is_service_local (browser->priv->client,
					       interface, protocol, name,
					       type, domain);
#endif
#ifdef HAVE_AVAHI_0_6
	local = ((flags & AVAHI_LOOKUP_RESULT_LOCAL) != 0);
#endif
	if (local && getenv ("LIBDMAPSHARING_ENABLE_LOCAL") == NULL) {
		g_warning ("Ignoring local service %s", name);
		return;
	}

	if (event == AVAHI_BROWSER_NEW) {
		browser_add_service (browser, name, domain);
	} else if (event == AVAHI_BROWSER_REMOVE) {
		browser_remove_service (browser, name);
	}
}

static void
free_service (DMAPMdnsBrowserService * service)
{
	g_free (service->service_name);
	g_free (service->name);
	g_free (service->host);
	g_free (service->pair);
	g_free (service);
}
