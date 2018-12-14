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

struct _DmapMdnsBrowserPrivate
{
	DmapMdnsServiceType service_type;
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

static void dmap_mdns_browser_class_init (DmapMdnsBrowserClass * klass);
static void dmap_mdns_browser_init (DmapMdnsBrowser * browser);
static void _dispose (GObject * object);
static void _finalize (GObject * object);
static void _client_init (DmapMdnsBrowser * browser);
static void _resolve_cb (AvahiServiceResolver * service_resolver,
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
			DmapMdnsBrowser * browser);
static gboolean _resolve (DmapMdnsBrowser * browser,
                          const gchar * name,
                          const gchar * domain);
static void _add_service (DmapMdnsBrowser * browser,
                          const gchar * service_name,
                          const gchar * domain);
static void _remove_service (DmapMdnsBrowser * browser,
                             const gchar * service_name);
static void _browse_cb (AvahiServiceBrowser * service_browser,
                        AvahiIfIndex interface,
                        AvahiProtocol protocol,
                        AvahiBrowserEvent event,
                        const gchar * name,
                        const gchar * type, const gchar * domain,
#ifdef HAVE_AVAHI_0_6
                        AvahiLookupResultFlags flags,
#endif
                        DmapMdnsBrowser * browser);

#define DMAP_MDNS_BROWSER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), DMAP_TYPE_MDNS_BROWSER, DmapMdnsBrowserPrivate))

static guint _signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE_WITH_CODE (DmapMdnsBrowser,
                         dmap_mdns_browser,
                         G_TYPE_OBJECT,
                         G_ADD_PRIVATE (DmapMdnsBrowser));

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
dmap_mdns_browser_class_init (DmapMdnsBrowserClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	dmap_mdns_browser_parent_class = g_type_class_peek_parent (klass);

	object_class->dispose  = _dispose;
	object_class->finalize = _finalize;

	_signals[SERVICE_ADDED] =
		g_signal_new ("service-added",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (DmapMdnsBrowserClass,
					       service_added), NULL, NULL,
			      g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE,
			      1, DMAP_TYPE_MDNS_SERVICE);
	_signals[SERVICE_REMOVED] =
		g_signal_new ("service-removed",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (DmapMdnsBrowserClass,
					       service_removed), NULL, NULL,
			      g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1,
			      G_TYPE_STRING);
}

static void
dmap_mdns_browser_init (DmapMdnsBrowser * browser)
{
	browser->priv = DMAP_MDNS_BROWSER_GET_PRIVATE (browser);
	_client_init (browser);
}

static void
_dispose (GObject * object)
{
	DmapMdnsBrowser *browser = DMAP_MDNS_BROWSER (object);
	GSList *walk;
	DmapMdnsService *service;

	for (walk = browser->priv->services; walk; walk = walk->next) {
		service = (DmapMdnsService *) walk->data;
		g_object_unref (service);
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
_finalize (GObject * object)
{
	g_signal_handlers_destroy (object);
	G_OBJECT_CLASS (dmap_mdns_browser_parent_class)->finalize (object);
}

DmapMdnsBrowser *
dmap_mdns_browser_new (DmapMdnsServiceType type)
{
	DmapMdnsBrowser *browser_object;

	g_assert(type >  DMAP_MDNS_SERVICE_TYPE_INVALID);
	g_assert(type <= DMAP_MDNS_SERVICE_TYPE_LAST);

	browser_object =
		DMAP_MDNS_BROWSER (g_object_new (DMAP_TYPE_MDNS_BROWSER, NULL));

	browser_object->priv->service_type = type;

	return browser_object;
}

gboolean
dmap_mdns_browser_start (DmapMdnsBrowser * browser, GError ** error)
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
					   _service_type_name[browser->
		                                              priv->service_type],
					   NULL,
#ifdef HAVE_AVAHI_0_6
					   0,
#endif
					   (AvahiServiceBrowserCallback)
					   _browse_cb, browser);
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
dmap_mdns_browser_stop (DmapMdnsBrowser * browser, GError ** error)
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
dmap_mdns_browser_get_services (DmapMdnsBrowser * browser)
{
	g_return_val_if_fail (browser != NULL, NULL);
	return browser->priv->services;
}

DmapMdnsServiceType
dmap_mdns_browser_get_service_type (DmapMdnsBrowser * browser)
{
	g_return_val_if_fail (browser != NULL,
			      DMAP_MDNS_SERVICE_TYPE_INVALID);
	return browser->priv->service_type;
}

static void
_client_cb (AvahiClient * client,
            AvahiClientState state, DmapMdnsBrowser * browser)
{
	/* Called whenever the client or server state changes */

	switch (state) {
#ifdef HAVE_AVAHI_0_6
	case AVAHI_CLIENT_FAILURE:
		g_warning ("Client failure: %s",
			   avahi_strerror (avahi_client_errno (client)));
		break;
#endif
	default:
		break;
	}
}

static void
_client_init (DmapMdnsBrowser * browser)
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
				  (AvahiClientCallback) _client_cb, browser,
				  &error);
#endif
#ifdef HAVE_AVAHI_0_6
	{
		AvahiClientFlags flags = 0;

		browser->priv->client =
			avahi_client_new (avahi_glib_poll_get
					  (browser->priv->poll), flags,
					  (AvahiClientCallback) _client_cb,
					  browser, &error);
	}
#endif
}

static void
_resolve_cb (AvahiServiceResolver * service_resolver,
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
             DmapMdnsBrowser * browser)
{
	gchar *name = NULL;
	gchar *pair = NULL;	/* FIXME: extract DACP-specific items into sub-class. Ensure in Howl and dns-sd code too. */
	DmapMdnsServiceTransportProtocol transport_protocol = DMAP_MDNS_SERVICE_TRANSPORT_PROTOCOL_TCP; // FIXME: subclass
	gchar host[AVAHI_ADDRESS_STR_MAX];
	gboolean pp = FALSE;
	DmapMdnsService *service;

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
					if (name == NULL) {
						name = g_strdup (value);
					}
				} else if (strcmp (key, "DvNm") == 0) {
					if (name != NULL) {
						g_free (name);
					}
					/* Remote's name is presented as DvNm in DACP */
					name = g_strdup (value);
				} else if (strcmp (key, "Pair") == 0) {
					/* Pair is used when first connecting to a DACP remote */
					pair = g_strdup (value);
				} else if (strcmp (key, "tp") == 0) {
					/* RAOP transport protocol */
					transport_protocol = strstr (value, "UDP")
					                   ? DMAP_MDNS_SERVICE_TRANSPORT_PROTOCOL_UDP
							   : DMAP_MDNS_SERVICE_TRANSPORT_PROTOCOL_TCP;
				}

				g_free (key);
				g_free (value);
			}
		}

		if (name == NULL) {
			name = g_strdup(service_name);
		}

		avahi_address_snprint (host, AVAHI_ADDRESS_STR_MAX, address);

		service = g_object_new (DMAP_TYPE_MDNS_SERVICE,
		                       "service-name", service_name,
		                       "name", name,
		                       "host", host,
		                       "port", port,
		                       "pair", pair, // FIXME: subclass.
		                       "transport-protocol", transport_protocol, // FIXME: subclass.
		                       "password-protected", pp,
		                        NULL);

		g_free(name);
		g_free(pair);

		browser->priv->services =
			g_slist_append (browser->priv->services, service);
		g_signal_emit (browser,
			       _signals[SERVICE_ADDED], 0,
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
_resolve (DmapMdnsBrowser * browser,
          const gchar * name, const gchar * domain)
{
	gboolean ok = FALSE;
	AvahiServiceResolver *service_resolver;

	service_resolver = avahi_service_resolver_new (browser->priv->client,
						       AVAHI_IF_UNSPEC,
						       AVAHI_PROTO_INET,
						       name,
						       _service_type_name
						       [browser->priv->service_type],
						       domain,
						       AVAHI_PROTO_UNSPEC,
#ifdef HAVE_AVAHI_0_6
						       0,
#endif
						       (AvahiServiceResolverCallback) _resolve_cb, browser);
	if (service_resolver == NULL) {
		g_debug ("Error starting mDNS resolving using AvahiServiceResolver");
		goto done;
	}

	browser->priv->resolvers =
		g_slist_prepend (browser->priv->resolvers, service_resolver);

	ok = TRUE;

done:
	return ok;
}

static void
_add_service (DmapMdnsBrowser * browser,
		     const gchar * service_name, const gchar * domain)
{
	_resolve (browser, service_name, domain);
}

static void
_remove_service (DmapMdnsBrowser * browser, const gchar * service_name)
{
	g_signal_emit (browser,
		       _signals[SERVICE_REMOVED],
		       0, service_name);
}

static void
_browse_cb (AvahiServiceBrowser * service_browser,
            AvahiIfIndex interface,
            AvahiProtocol protocol,
            AvahiBrowserEvent event,
            const gchar * name, const gchar * type, const gchar * domain,
#ifdef HAVE_AVAHI_0_6
            AvahiLookupResultFlags flags,
#endif
            DmapMdnsBrowser * browser)
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
		g_debug ("Ignoring local service %s", name);
		return;
	}

	if (event == AVAHI_BROWSER_NEW) {
		_add_service (browser, name, domain);
	} else if (event == AVAHI_BROWSER_REMOVE) {
		_remove_service (browser, name);
	}
}
