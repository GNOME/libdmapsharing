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

#include "dmap-mdns-browser.h"

struct _DMAPMdnsBrowserPrivate
{
	DMAPMdnsBrowserServiceType service_type;

	DNSServiceRef sd_browse_ref;

	GSList *services;
	GSList *resolvers;
	GSList *backlog;
};

typedef struct _ServiceContext
{
	DNSServiceRef ref;

	DMAPMdnsBrowser *browser;

	DNSServiceFlags flags;

	uint16_t port;
	uint32_t interface_index;

	gchar *service_name;
	gchar *full_name;
	gchar *host_target;
	gchar *domain;
} ServiceContext;

enum
{
	SERVICE_ADDED,
	SERVICE_REMOVED,
	LAST_SIGNAL
};

#define DMAP_MDNS_BROWSER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), DMAP_TYPE_MDNS_BROWSER, DMAPMdnsBrowserPrivate))

static guint dmap_mdns_browser_signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (DMAPMdnsBrowser, dmap_mdns_browser, G_TYPE_OBJECT);

static void
dnssd_browser_init (DMAPMdnsBrowser * browser)
{
	g_debug ("dnssd_browser_init()");
}

static void
dmap_mdns_browser_init (DMAPMdnsBrowser * browser)
{
	g_debug ("dmap_mdns_browser_init ()");

	browser->priv = DMAP_MDNS_BROWSER_GET_PRIVATE (browser);
	dnssd_browser_init (browser);
}

static void
service_context_free (ServiceContext *ctx)
{
	g_debug ("service_context_free ()");

	DNSServiceRefDeallocate (ctx->ref);
	g_object_unref (ctx->browser);
	g_free (ctx->service_name);
	g_free (ctx->full_name);
	g_free (ctx->host_target);
	g_free (ctx->domain);
}

static gboolean
dmap_mdns_browser_resolve (ServiceContext *context)
{
	g_debug ("dmap_mdns_browser_resolve ()");

	DMAPMdnsBrowserService *service;

	service = g_new (DMAPMdnsBrowserService, 1);

	// FIXME: The name and service_name variables need to be renamed.
	// Wait until working on DACP because I think this is when
	// they are different. See Avahi code.
	service->service_name = g_strdup (context->service_name);
	service->name = g_strdup (context->service_name);
	service->host = g_strdup (context->host_target);
	service->port = context->port;
	service->pair = NULL;
	service->password_protected = FALSE;

	// add to the services list
	context->browser->priv->services =
		g_slist_append (context->browser->priv->services, service);

	// notify all listeners
	g_signal_emit (context->browser,
		       dmap_mdns_browser_signals[SERVICE_ADDED], 0, service);

	return TRUE;
}

static gboolean
service_result_available_cb (GIOChannel * gio, GIOCondition condition,
                             ServiceContext *context)
{
	if (condition & (G_IO_HUP | G_IO_ERR)) {
		g_warning ("DNS-SD service socket closed");
		service_context_free (context);
		return FALSE;
	}

	DNSServiceErrorType err = DNSServiceProcessResult (context->ref);

	if (err != kDNSServiceErr_NoError) {
		g_warning ("Error processing DNS-SD service result");
		return FALSE;
	}

	dmap_mdns_browser_resolve (context);

	return FALSE;
}

static gboolean
add_resolve_to_event_loop (ServiceContext *context, DNSServiceRef sd_ref)
{
	int dns_sd_fd = DNSServiceRefSockFD (sd_ref);

	GIOChannel *dns_sd_chan = g_io_channel_unix_new (dns_sd_fd);

	if (!g_io_add_watch (dns_sd_chan,
	                     G_IO_IN | G_IO_HUP | G_IO_ERR,
	                     (GIOFunc) service_result_available_cb, context)) {
		g_warning ("Error adding SD to event loop");
	}

	return TRUE;
}

static void
dns_service_resolve_reply (DNSServiceRef sd_ref,
			   DNSServiceFlags flags,
			   uint32_t interface_index,
			   DNSServiceErrorType error_code,
			   const char *fullname,
			   const char *hosttarget,
			   uint16_t port,
			   uint16_t txt_len,
			   const char *txt_record, void *udata)
{
	ServiceContext *ctx = (ServiceContext *) udata;

	if (error_code != kDNSServiceErr_NoError) {
		g_debug ("dns_service_resolve_reply ():  fail");
		return;
	}

	g_debug ("dns_service_resolve_reply ():  success");

	ctx->flags = flags;
	ctx->interface_index = interface_index;
	ctx->port = htons (port);
	ctx->full_name = g_strdup (fullname);
	ctx->host_target = g_strdup (hosttarget);
}

static gboolean
browse_result_available_cb (GIOChannel * gio,
			    GIOCondition condition, DMAPMdnsBrowser * browser)
{
	if (condition & (G_IO_HUP | G_IO_ERR )) {
		g_warning ("DNS-SD browser socket closed");
		return FALSE;
	}

	DNSServiceErrorType err =
		DNSServiceProcessResult (browser->priv->sd_browse_ref);

	if (err != kDNSServiceErr_NoError) {
		g_warning ("Error processing DNS-SD browse result");
		return FALSE;
	}

	for (; browser->priv->backlog; browser->priv->backlog = g_slist_remove_link (browser->priv->backlog, browser->priv->backlog)) {
		ServiceContext *ctx = (ServiceContext *) browser->priv->backlog->data;
		DNSServiceRef ref;

		err = DNSServiceResolve (&ref,
		                         ctx->flags,
		                         ctx->interface_index,
		                         ctx->service_name,
		                         service_type_name[browser->priv->service_type],
		                         ctx->domain,
		                         (DNSServiceResolveReply)
		                         dns_service_resolve_reply,
		                         (void *) ctx);

		if (err != kDNSServiceErr_NoError) {
			g_warning ("Error setting up DNS-SD resolve handler");
			service_context_free (ctx);
			continue;
		}

		ctx->ref = ref;

		g_debug ("Success processing DNS-SD browse result");
		add_resolve_to_event_loop (ctx, ref);
	}

	return TRUE;
}

static gboolean
add_browse_to_event_loop (DMAPMdnsBrowser *browser)
{
	int dns_sd_fd = DNSServiceRefSockFD (browser->priv->sd_browse_ref);

	GIOChannel *dns_sd_chan = g_io_channel_unix_new (dns_sd_fd);

	if (!g_io_add_watch (dns_sd_chan,
	                     G_IO_IN | G_IO_HUP | G_IO_ERR,
	                     (GIOFunc) browse_result_available_cb, browser)) {
		g_error ("Error adding SD to event loop");
	}

	return TRUE;
}


static void
dns_service_browse_reply (DNSServiceRef sd_ref,
			  DNSServiceFlags flags,
			  uint32_t interface_index,
			  DNSServiceErrorType error_code,
			  const char *service_name,
			  const char *regtype,
			  const char *reply_domain, void *udata)
{
	if (error_code != kDNSServiceErr_NoError) {
		g_debug ("dnsServiceBrowserReply ():  fail");
		return;
	}

	if (!(flags & kDNSServiceFlagsAdd)) {
		return;
	}

	g_debug ("adding a service: %s", service_name);

	// Cast the context pointer to a DMAPMdnsBrowser
	DMAPMdnsBrowser *browser = (DMAPMdnsBrowser *) udata;

	ServiceContext *context = g_new0 (ServiceContext, 1);
	context->browser = g_object_ref (browser);
	context->flags = flags;
	context->interface_index = interface_index;
	context->service_name = g_strdup (service_name);
	context->domain = g_strdup (reply_domain);

	browser->priv->backlog = g_slist_prepend (browser->priv->backlog, context);
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

	G_OBJECT_CLASS (dmap_mdns_browser_parent_class)->dispose (object);
}

static void
dmap_mdns_browser_finalize (GObject * object)
{
	g_signal_handlers_destroy (object);
	G_OBJECT_CLASS (dmap_mdns_browser_parent_class)->finalize (object);
}

static void
dmap_mdns_browser_class_init (DMAPMdnsBrowserClass * klass)
{
	g_debug ("dmap_mdns_browser_class_init()");

	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	dmap_mdns_browser_parent_class = g_type_class_peek_parent (klass);

	object_class->dispose = dmap_mdns_browser_dispose;
	object_class->finalize = dmap_mdns_browser_finalize;

	g_type_class_add_private (klass, sizeof (DMAPMdnsBrowserPrivate));

	// Signal makeup
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

DMAPMdnsBrowser *
dmap_mdns_browser_new (DMAPMdnsBrowserServiceType type)
{
	g_debug ("dmap_mdns_browser_new ()");

	DMAPMdnsBrowser *browser_object = 0;

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
	g_debug ("dmap_mdns_browser_start ()");

	gboolean is_success = FALSE;

	DNSServiceErrorType browse_err = kDNSServiceErr_Unknown;

	browse_err = DNSServiceBrowse (&(browser->priv->sd_browse_ref), 0,
	                               kDNSServiceInterfaceIndexAny,
	                               service_type_name[browser->priv->service_type], 0,
	                               (DNSServiceBrowseReply) dns_service_browse_reply,
	                               (void *) browser);

	if (kDNSServiceErr_NoError == browse_err) {
		g_debug ("*** Browse Success ****");

		add_browse_to_event_loop (browser);
	}

	return is_success;
}

/**
 *  Note that you must terminate the connection with the daemon and
 *  free any memory associated with DNSServiceRef
 */
gboolean
dmap_mdns_browser_stop (DMAPMdnsBrowser * browser, GError ** error)
{
	DNSServiceRefDeallocate (browser->priv->sd_browse_ref);
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
