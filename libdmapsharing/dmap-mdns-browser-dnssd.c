/*
 * Copyright (C) 2009 W. Michael Petullo <mike@flyn.org>
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

#include <stdlib.h>
#include <stdio.h>
#include <dns_sd.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <glib.h>
#include <glib-object.h>
#include <arpa/inet.h>

#include "dmap-mdns-browser.h"

struct _DmapMdnsBrowserPrivate
{
	DmapMdnsServiceType service_type;
	DNSServiceRef sd_browse_ref;
	GSList *services;
	GSList *backlog;
};

typedef struct _ServiceContext
{
	DNSServiceRef service_discovery_ref;
	DNSServiceRef host_lookup_ref;
	DmapMdnsBrowser *browser;
	DNSServiceFlags flags;
	uint32_t interface_index;
	DmapMdnsService *service;
	gchar *domain;
} ServiceContext;

enum
{
	SERVICE_ADDED,
	SERVICE_REMOVED,
	LAST_SIGNAL
};

static guint _signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE_WITH_PRIVATE (DmapMdnsBrowser,
                            dmap_mdns_browser,
                            G_TYPE_OBJECT);

static void
dmap_mdns_browser_init (DmapMdnsBrowser * browser)
{
	g_assert (NULL != browser);

	browser->priv = dmap_mdns_browser_get_instance_private(browser);
}

static void
_service_context_free (ServiceContext *ctx)
{
	g_assert (NULL != ctx);
	g_assert (NULL != ctx->browser);

	g_object_unref (&ctx->service);
	g_object_unref (ctx->browser);

	g_free (ctx->domain);
	g_free (ctx);
}

static gboolean
_signal_service_added (ServiceContext *context)
{
	g_assert (NULL != context);

	gchar                           *service_name       = NULL;
	gchar                           *name               = NULL;
	gchar                           *host               = NULL;
	uint16_t                         port               = 0;
	gchar                           *pair               = NULL;
	DmapMdnsServiceTransportProtocol transport_protocol = DMAP_MDNS_SERVICE_TRANSPORT_PROTOCOL_TCP;
	gboolean                         password_protected = FALSE;

	DmapMdnsService *service;

	g_object_get(context->service,
	            "service-name", &service_name,
	            "name", &name,
	            "host", &host,
	            "port", &port,
	            "pair", &pair,
	            "transport-protocol", transport_protocol,
	            "password-protected", password_protected,
	             NULL);

	// FIXME: The name and service_name variables need to be renamed.
	// Wait until working on DACP because I think this is when
	// they are different. See Avahi code.
	service = g_object_new(DMAP_TYPE_MDNS_SERVICE,
	                      "service-name", service_name,
	                      "name", name,
	                      "host", host,
	                      "port", port,
	                      "pair", pair,
	                      "transport-protocol", transport_protocol,
	                      "password-protected", password_protected,
	                       NULL);

	service = g_new0 (DmapMdnsService, 1);

	// add to the services list
	context->browser->priv->services =
		g_slist_append (context->browser->priv->services, service);

	// notify all listeners
	g_signal_emit (context->browser,
		       _signals[SERVICE_ADDED], 0, service);

	return TRUE;
}

static char *
_extract_name (const char *dns_name)
{
        /* Turns "Children's\032Music._daap._tcp.local
	 * into "Children's Music"
	 */

	char *name = calloc(strlen(dns_name) + 1, sizeof(char));
	if (NULL == name) {
		goto done;
	}

	char *space;
	do {
		space = strstr(dns_name, "\\032");
		if (NULL != space) {
			strncat(name, dns_name, space - dns_name);
			strcat(name, " ");
		} else {
			strcat(name, dns_name);
		}

		dns_name = space + 4;
	} while (NULL != space);

	char *dot = strchr(name, '.');
	if (NULL != dot) {
		*dot = 0x00;
	}

done:
	return name;
}

static void
_dns_service_browse_reply (G_GNUC_UNUSED DNSServiceRef sd_ref,
                           DNSServiceFlags flags,
                           uint32_t interface_index,
                           DNSServiceErrorType error_code,
                           const char *service_name,
                           G_GNUC_UNUSED const char *regtype,
                           const char *domain, void *udata)
{
	if (error_code != kDNSServiceErr_NoError) {
		g_warning ("dnsServiceBrowserReply ():  fail");
		goto done;
	}

	if (!(flags & kDNSServiceFlagsAdd)) {
		goto done;
	}

	DmapMdnsBrowser *browser = (DmapMdnsBrowser *) udata;

	ServiceContext *context = g_new0 (ServiceContext, 1);
	context->browser = g_object_ref (browser);
	context->flags = flags;
	context->interface_index = interface_index;
	context->domain = g_strdup (domain);
	context->service = g_object_new(DMAP_TYPE_MDNS_SERVICE, NULL);
	g_object_set(context->service, "service-name", service_name, NULL);

	browser->priv->backlog = g_slist_prepend (browser->priv->backlog, context);

done:
	return;
}

/*
static void
_dns_host_resolve_reply (G_GNUC_UNUSED DNSServiceRef sd_ref,
                         G_GNUC_UNUSED DNSServiceFlags flags,
                         G_GNUC_UNUSED uint32_t interface_index,
                         DNSServiceErrorType error_code,
                         G_GNUC_UNUSED const char *hostname,
                         const struct sockaddr *address,
                         G_GNUC_UNUSED uint32_t ttl,
                         void *udata)
{
	ServiceContext *ctx = (ServiceContext *) udata;

	if (error_code != kDNSServiceErr_NoError) {
		g_warning ("_dns_host_resolve_reply ():  fail");
		return;
	}

	switch(address->sa_family) {
	case AF_INET: {
		gchar host[INET_ADDRSTRLEN];
		struct sockaddr_in *addr_in = (struct sockaddr_in *) address;
		inet_ntop(AF_INET, &(addr_in->sin_addr), host, INET_ADDRSTRLEN);
		g_object_set(&ctx->service, "host", host, NULL);
		break;
	}
	case AF_INET6: {
		gchar host[INET6_ADDRSTRLEN];
		struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *) address;
		inet_ntop(AF_INET6, &(addr_in6->sin6_addr), host, INET6_ADDRSTRLEN);
		g_object_set(&ctx->service, "host", host, NULL);
		break;
	}
	default:
		g_object_set(&ctx->service, "host", NULL, NULL);
		break;
	}
}
*/

static gboolean
_lookup_result_available_cb (G_GNUC_UNUSED GIOChannel * gio,
                             GIOCondition condition,
                             ServiceContext *context)
{
	gboolean fnval = FALSE;

	if (condition & (G_IO_HUP | G_IO_ERR)) {
		g_warning ("DNS-SD service socket closed");
		goto done;
	}

	/* Obtain IP address (_dns_host_resolve_reply) */
	DNSServiceErrorType err = DNSServiceProcessResult (context->host_lookup_ref);
	if (err != kDNSServiceErr_NoError) {
		g_warning ("Error processing DNS-SD service result");
		goto done;
	}

	_signal_service_added (context);

done:
	DNSServiceRefDeallocate (context->host_lookup_ref);

	if (NULL != context) {
		_service_context_free (context);
	}

	return fnval;
}

static gboolean
_add_host_lookup_to_event_loop (ServiceContext *context)
{
	int dns_sd_fd = DNSServiceRefSockFD (context->host_lookup_ref);

	GIOChannel *dns_sd_chan = g_io_channel_unix_new (dns_sd_fd);

	if (!g_io_add_watch (dns_sd_chan,
	                     G_IO_IN | G_IO_HUP | G_IO_ERR,
	                     (GIOFunc) _lookup_result_available_cb, context)) {
		g_warning ("Error adding host lookup to event loop");
	}

	g_io_channel_unref (dns_sd_chan);

	return TRUE;
}

static void
_dns_service_resolve_reply (G_GNUC_UNUSED DNSServiceRef sd_ref,
                            DNSServiceFlags flags,
                            uint32_t interface_index,
                            DNSServiceErrorType error_code,
                            const char *name,
                            G_GNUC_UNUSED const char *host,
                            uint16_t port,
                            G_GNUC_UNUSED uint16_t txt_len,
                            G_GNUC_UNUSED const char *txt_record,
                            void *udata)
{
	DNSServiceRef ref;
	ServiceContext *ctx = (ServiceContext *) udata;

	if (error_code != kDNSServiceErr_NoError) {
		g_warning ("_dns_service_resolve_reply ():  fail");
		return;
	}

	ctx->flags = flags;
	ctx->interface_index = interface_index;
	g_object_set(ctx->service,
	            "port", htons(port),
	            "name", _extract_name (name),
	            "pair", NULL,
	            "password-protected", FALSE,
	             NULL);

	g_error("FIXME: Not implemented");
/*
	DNSServiceErrorType err = DNSServiceGetAddrInfo (&ref,
	                              0,
	                              ctx->interface_index,
	                              kDNSServiceProtocol_IPv4,
	                              host,
	                             (DNSServiceGetAddrInfoReply) _dns_host_resolve_reply,
	                             (void *) ctx);
	if (err != kDNSServiceErr_NoError) {
		g_warning ("Error setting up DNS-SD address info handler");
		_service_context_free (ctx);
	}
*/

	ctx->host_lookup_ref = ref;

	_add_host_lookup_to_event_loop (ctx);
}

static gboolean
_service_result_available_cb (G_GNUC_UNUSED GIOChannel * gio,
                              GIOCondition condition,
                              ServiceContext *context)
{
	gboolean fnval = FALSE;

	if (condition & (G_IO_HUP | G_IO_ERR)) {
		g_warning ("DNS-SD service socket closed");
		goto done;
	}

	/* Obtain service description (_dns_service_resolve_reply) */
	DNSServiceErrorType err = DNSServiceProcessResult (context->service_discovery_ref);
	if (err != kDNSServiceErr_NoError) {
		g_warning ("Error processing DNS-SD service result");
		goto done;
	}

done:
	DNSServiceRefDeallocate (context->service_discovery_ref);

	return fnval;
}

static gboolean
_add_service_discovery_to_event_loop (ServiceContext *context)
{
	int dns_sd_fd = DNSServiceRefSockFD (context->service_discovery_ref);

	GIOChannel *dns_sd_chan = g_io_channel_unix_new (dns_sd_fd);

	if (!g_io_add_watch (dns_sd_chan,
	                     G_IO_IN | G_IO_HUP | G_IO_ERR,
	                     (GIOFunc) _service_result_available_cb, context)) {
		g_warning ("Error adding SD to event loop");
	}

	g_io_channel_unref (dns_sd_chan);

	return TRUE;
}

static gboolean
_browse_result_available_cb (G_GNUC_UNUSED GIOChannel * gio,
                             GIOCondition condition,
                             DmapMdnsBrowser * browser)
{
	gboolean fnval = FALSE;

	if (condition & (G_IO_HUP | G_IO_ERR )) {
		g_warning ("DNS-SD browser socket closed");
		goto done;
	}

	DNSServiceErrorType err = DNSServiceProcessResult (browser->priv->sd_browse_ref);
	if (err != kDNSServiceErr_NoError) {
		g_warning ("Error processing DNS-SD browse result");
		goto done;
	}

	while (browser->priv->backlog) {
		gchar *service_name;
		DNSServiceRef ref;
		ServiceContext *ctx = (ServiceContext *) browser->priv->backlog->data;

		g_object_get(ctx->service, "service-name", &service_name, NULL);

		err = DNSServiceResolve (&ref,
		                         ctx->flags,
		                         ctx->interface_index,
		                         service_name,
		                         _service_type_name[browser->priv->service_type],
		                         ctx->domain,
		                         (DNSServiceResolveReply)
		                         _dns_service_resolve_reply,
		                         (void *) ctx);

		if (err != kDNSServiceErr_NoError) {
			g_warning ("Error setting up DNS-SD resolve handler");
			_service_context_free (ctx);
			continue;
		}

		ctx->service_discovery_ref = ref;

		_add_service_discovery_to_event_loop (ctx);

		browser->priv->backlog = g_slist_delete_link (browser->priv->backlog, browser->priv->backlog);

		fnval = TRUE;
	}

done:
	return fnval;
}

static gboolean
_add_browse_to_event_loop (DmapMdnsBrowser *browser)
{
	gboolean fnval = FALSE;

	int dns_sd_fd = DNSServiceRefSockFD (browser->priv->sd_browse_ref);

	GIOChannel *dns_sd_chan = g_io_channel_unix_new (dns_sd_fd);

	if (!g_io_add_watch (dns_sd_chan,
	                     G_IO_IN | G_IO_HUP | G_IO_ERR,
	                     (GIOFunc) _browse_result_available_cb, browser)) {
		g_warning ("Error adding SD to event loop");
		goto done;
	}

	g_io_channel_unref (dns_sd_chan);

	fnval = TRUE;

done:
	return fnval;
}

static void
_dispose (GObject * object)
{
	DmapMdnsBrowser *browser = DMAP_MDNS_BROWSER (object);
	GSList *walk;
	DmapMdnsService *service;

	for (walk = browser->priv->services; NULL != walk; walk = walk->next) {
		service = (DmapMdnsService *) walk->data;
		g_object_unref (service);
	}

	g_slist_free (browser->priv->services);

	G_OBJECT_CLASS (dmap_mdns_browser_parent_class)->dispose (object);
}

static void
_finalize (GObject * object)
{
	g_signal_handlers_destroy (object);
	G_OBJECT_CLASS (dmap_mdns_browser_parent_class)->finalize (object);
}

static void
dmap_mdns_browser_class_init (DmapMdnsBrowserClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	dmap_mdns_browser_parent_class = g_type_class_peek_parent (klass);

	object_class->dispose  = _dispose;
	object_class->finalize = _finalize;

	// Signal makeup
	_signals[SERVICE_ADDED] =
		g_signal_new ("service-added",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (DmapMdnsBrowserClass,
					       service_added), NULL, NULL,
			      NULL, G_TYPE_NONE,
			      1, DMAP_TYPE_MDNS_SERVICE);

	_signals[SERVICE_REMOVED] =
		g_signal_new ("service-removed",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (DmapMdnsBrowserClass,
					       service_removed), NULL, NULL,
			      NULL, G_TYPE_NONE, 1,
			      G_TYPE_STRING);
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
	gboolean fnval = FALSE;

	DNSServiceErrorType browse_err = kDNSServiceErr_Unknown;

	browse_err = DNSServiceBrowse (&(browser->priv->sd_browse_ref),
	                                 0,
	                                 kDNSServiceInterfaceIndexAny,
	                                 _service_type_name[browser->priv->service_type],
	                                "",
	                                (DNSServiceBrowseReply) _dns_service_browse_reply,
	                                (void *) browser);

	if (kDNSServiceErr_NoError == browse_err) {
		fnval = TRUE;
		_add_browse_to_event_loop (browser);
	} else {
		g_warning ("Error starting mDNS discovery using DNS-SD");
                g_set_error (error,
                             DMAP_MDNS_BROWSER_ERROR,
                             DMAP_MDNS_BROWSER_ERROR_FAILED,
                             "%s", "Unable to activate browser");
	}

	return fnval;
}

gboolean
dmap_mdns_browser_stop (DmapMdnsBrowser * browser,
                        G_GNUC_UNUSED GError ** error)
{
	if (NULL != browser->priv->sd_browse_ref) {
		DNSServiceRefDeallocate (browser->priv->sd_browse_ref);
	}

	return TRUE;
}

GQuark
dmap_mdns_browser_error_quark (void)
{
	static GQuark quark = 0;

	if (!quark) {
		// Create a unique quark from the specified string
		quark = g_quark_from_static_string
			("DNS-SD:  dmap_mdns_browser_error_quark");
	}

	return quark;
}

const GSList *
dmap_mdns_browser_get_services (DmapMdnsBrowser * browser)
{
	g_assert(NULL != browser);

	return browser->priv->services;
}

DmapMdnsServiceType
dmap_mdns_browser_get_service_type (DmapMdnsBrowser * browser)
{
	g_assert(NULL != browser);

	return browser->priv->service_type;
}
