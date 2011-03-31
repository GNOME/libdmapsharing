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

#include "config.h"
#include "dmap-mdns-browser.h"

struct _DMAPMdnsBrowserPrivate
{
	DMAPMdnsBrowserServiceType service_type;

	DNSServiceFlags flags;

	uint16_t port;
	uint32_t interface_index;

	gchar *service_name;
	gchar *reg_type;
	gchar *full_name;
	gchar *host_target;
	gchar *domain;

	DNSServiceRef sd_service_ref;
	DNSServiceRef sd_browse_ref;

	GSList *services;
	GSList *resolvers;
};

enum
{
	SERVICE_ADDED,
	SERVICE_REMOVED,
	LAST_SIGNAL
};

static void print_browser (DMAPMdnsBrowser * browser);

static gboolean
add_sd_to_event_loop (DMAPMdnsBrowser * browser, DNSServiceRef sdRef);

static void dmap_mdns_browser_class_init (DMAPMdnsBrowserClass * klass);

static void dmap_mdns_browser_init (DMAPMdnsBrowser * browser);

static void dmap_mdns_browser_dispose (GObject * object);

static void dmap_mdns_browser_finalize (GObject * object);

static void dnssd_browser_init (DMAPMdnsBrowser * browser);

static void free_service (DMAPMdnsBrowserService * service);

static void
browser_add_service (DMAPMdnsBrowser * browser,
		     const gchar * service_name, const gchar * domain);

static gboolean
dmap_mdns_browser_resolve (DMAPMdnsBrowser * browser,
			   const gchar * name, const gchar * domain);

static void
dns_service_browse_reply (DNSServiceRef sdRef,
			  DNSServiceFlags flags,
			  uint32_t interfaceIndex,
			  DNSServiceErrorType errorCode,
			  const char *serviceName,
			  const char *regtype,
			  const char *replyDomain, void *context);

static void
dns_service_resolve_reply (DNSServiceRef sdRef,
			   DNSServiceFlags flags,
			   uint32_t interfaceIndex,
			   DNSServiceErrorType errorCode,
			   const char *fullname,
			   const char *hosttarget,
			   uint16_t port,
			   uint16_t txtLen,
			   const char *txtRecord, void *context);

gboolean dmap_mdns_browser_stop (DMAPMdnsBrowser * browser, GError ** error);

static const char *service_type_name[] = {
	NULL,
	"_daap._tcp",
	"_dpap._tcp",
	"_touch-remote._tcp"
};

#define DMAP_MDNS_BROWSER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), DMAP_TYPE_MDNS_BROWSER, DMAPMdnsBrowserPrivate))

static guint dmap_mdns_browser_signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (DMAPMdnsBrowser, dmap_mdns_browser, G_TYPE_OBJECT)

     static void dmap_mdns_browser_init (DMAPMdnsBrowser * browser)
{
	g_debug ("dmap_mdns_browser_init ()");

	browser->priv = DMAP_MDNS_BROWSER_GET_PRIVATE (browser);
	dnssd_browser_init (browser);
}

static gboolean
browse_result_available_cb (GIOChannel * gio,
			    GIOCondition condition, DMAPMdnsBrowser * browser)
{
	if (condition & G_IO_HUP) {
		g_error ("DNS-SD browser socket closed");
	}

	DNSServiceErrorType err =
		DNSServiceProcessResult (browser->priv->sd_browse_ref);

	if (!kDNSServiceErr_NoError == err) {
		g_error ("Error processing DNS-SD browse result");
	} else {
		err = DNSServiceResolve (&(browser->priv->sd_service_ref),
					 browser->priv->flags,
					 browser->priv->interface_index,
					 browser->priv->service_name,
					 browser->priv->reg_type,
					 browser->priv->domain,
					 (DNSServiceResolveReply)
					 dns_service_resolve_reply,
					 (void *) browser);
	}

	if (kDNSServiceErr_NoError == err) {
		g_debug ("Success processing DNS-SD browse result");
		add_sd_to_event_loop (browser, browser->priv->sd_service_ref);
	} else {
		g_error ("Error setting up DNS-SD resolve handler");
	}

	return TRUE;
}

static gboolean
service_result_available_cb (GIOChannel * gio,
			     GIOCondition condition,
			     DMAPMdnsBrowser * browser)
{
	if (condition & G_IO_HUP) {
		g_error ("DNS-SD service socket closed");
	}

	DNSServiceErrorType err =
		DNSServiceProcessResult (browser->priv->sd_service_ref);

	if (!kDNSServiceErr_NoError == err) {
		g_error ("Error processing DNS-SD service result");
	} else {
		browser_add_service (browser,
				     (gchar *) browser->priv->service_name,
				     (gchar *) browser->priv->domain);
	}

	return TRUE;
}

static gboolean
add_sd_to_event_loop (DMAPMdnsBrowser * browser, DNSServiceRef sd_ref)
{
	int dns_sd_fd = DNSServiceRefSockFD (sd_ref);

	GIOChannel *dns_sd_chan = g_io_channel_unix_new (dns_sd_fd);
	GIOFunc result_func = NULL;

	if (browser->priv->sd_browse_ref == sd_ref) {
		result_func = (GIOFunc) browse_result_available_cb;
	} else if (browser->priv->sd_service_ref) {
		result_func = (GIOFunc) service_result_available_cb;
	}

	if (!g_io_add_watch (dns_sd_chan,
			     G_IO_IN | G_IO_HUP | G_IO_ERR,
			     result_func, browser)) {
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
			  const char *reply_domain, void *context)
{
	if (error_code == kDNSServiceErr_NoError) {
		g_debug ("dns_service_browse_reply ():  success");

		// Cast the context pointer to a DMAPMdnsBrowser
		DMAPMdnsBrowser *browser = (DMAPMdnsBrowser *) context;

		browser->priv->flags = flags;
		browser->priv->interface_index = interface_index;

		g_free (browser->priv->service_name);
		browser->priv->service_name = g_strdup (service_name);

		g_free (browser->priv->reg_type);
		browser->priv->reg_type = g_strdup (regtype);

		g_free (browser->priv->domain);
		browser->priv->domain = g_strdup (reply_domain);
	} else {
		g_debug ("dnsServiceBrowserReply ():  fail");
	}
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
			   const char *txt_record, void *context)
{
	g_debug ("dns_service_resolve_reply ()");

	// Cast the context pointer to a DMAPMdnsBrowser
	DMAPMdnsBrowser *browser = (DMAPMdnsBrowser *) context;

	if (kDNSServiceErr_NoError == error_code) {
		browser->priv->flags = flags;
		browser->priv->interface_index = interface_index;
		browser->priv->port = htons (port);

		g_free (browser->priv->full_name);
		browser->priv->full_name = g_strdup (fullname);

		g_free (browser->priv->host_target);
		browser->priv->host_target = g_strdup (hosttarget);
	}
}

static void
dnssd_browser_init (DMAPMdnsBrowser * browser)
{
	g_debug ("dnssd_browser_init()");

	browser->priv->flags = kDNSServiceFlagsDefault;
	browser->priv->port = 0;
	browser->priv->interface_index = 0;

	g_free (browser->priv->reg_type);
	browser->priv->reg_type =
		g_strdup (service_type_name
			  [DMAP_MDNS_BROWSER_SERVICE_TYPE_DAAP]);

	g_free (browser->priv->domain);
	browser->priv->domain = g_strdup ("");

	g_free (browser->priv->service_name);
	browser->priv->service_name = g_strdup ("");

	g_free (browser->priv->full_name);
	browser->priv->full_name = g_strdup ("");

	g_free (browser->priv->host_target);
	browser->priv->host_target = g_strdup ("");
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

static gboolean
dmap_mdns_browser_resolve (DMAPMdnsBrowser * browser,
			   const gchar * name, const gchar * domain)
{
	g_debug ("dmap_mdns_browser_resolve ()");

	DMAPMdnsBrowserService *service;

	service = g_new (DMAPMdnsBrowserService, 1);

	service->service_name = g_strdup (browser->priv->reg_type);
	service->name = name;
	service->host = g_strdup (browser->priv->host_target);
	service->port = browser->priv->port;
	service->pair = NULL;
	service->password_protected = FALSE;

	// add to the services list
	browser->priv->services =
		g_slist_append (browser->priv->services, service);

	// notify all listeners 
	g_signal_emit (browser,
		       dmap_mdns_browser_signals[SERVICE_ADDED], 0, service);

	return TRUE;
}

static void
browser_add_service (DMAPMdnsBrowser * browser,
		     const gchar * service_name, const gchar * domain)
{
	g_debug ("browser_add_service ()");

	dmap_mdns_browser_resolve (browser, service_name, domain);
}

gboolean
dmap_mdns_browser_start (DMAPMdnsBrowser * browser, GError ** error)
{
	g_debug ("dmap_mdns_browser_start ()");

	gboolean is_success = FALSE;

	DNSServiceErrorType browse_err = kDNSServiceErr_Unknown;

	browse_err = DNSServiceBrowse (&(browser->priv->sd_browse_ref),
				      browser->priv->flags,
				      browser->priv->interface_index,
				      browser->priv->reg_type,
				      browser->priv->domain,
				      (DNSServiceBrowseReply)
				      dns_service_browse_reply,
				      (void *) browser);

	if (kDNSServiceErr_NoError == browse_err) {
		g_debug ("*** Browse Success ****");

		add_sd_to_event_loop (browser, browser->priv->sd_browse_ref);
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
	DNSServiceRefDeallocate (browser->priv->sd_service_ref);

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
free_service (DMAPMdnsBrowserService * service)
{
	g_free (service->service_name);
	g_free (service->name);
	g_free (service->host);
	g_free (service->pair);
	g_free (service);
}

static void
print_browser (DMAPMdnsBrowser * browser)
{
	printf ("\n======= Browser Fields =======\n");

	printf ("     reg_type: %s\n", browser->priv->reg_type);
	printf (" service_name: %s\n", browser->priv->service_name);
	printf ("    full_name: %s\n", browser->priv->full_name);
	printf ("  host_target: %s\n", browser->priv->host_target);
	printf ("       domain: %s\n", browser->priv->domain);

	printf ("         port: %d\n", browser->priv->port);
	printf ("  iface index: %d\n", browser->priv->interface_index);
	printf ("        flags: %d\n", browser->priv->flags);
	printf ("==============================\n");

	printf ("\n");
}
