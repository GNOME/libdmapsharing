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

#include <arpa/inet.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>

#include <glib.h>
#include <glib-object.h>

#include "config.h"
#include "dmap-mdns-browser.h"

#define LONG_TIME 10000000



static volatile int stopNow = 0;
static volatile int timeOut = LONG_TIME;



struct 
_DMAPMdnsBrowserPrivate
{
	DMAPMdnsBrowserServiceType service_type;

	DNSServiceFlags flags; 

	uint16_t port;
	uint32_t interfaceIndex;

	char regtype[100];
	char domain[100];
	char serviceName[100];
	char fullname[100];
	char hosttarget[100];

	GSList *services;
	GSList *resolvers;
};

enum 
{
	SERVICE_ADDED,
	SERVICE_REMOVED,
	LAST_SIGNAL
};


static void 
print_browser (DMAPMdnsBrowser *browser);

static gboolean
dnssd_resolve_services (DMAPMdnsBrowser *browser, DNSServiceRef sdRef);

static void 
dmap_mdns_browser_class_init (DMAPMdnsBrowserClass *klass);

static void 
dmap_mdns_browser_init (DMAPMdnsBrowser *browser);

static void 
dmap_mdns_browser_dispose (GObject *object);

static void 
dmap_mdns_browser_finalize (GObject *object);

static void
dnssd_browser_init (DMAPMdnsBrowser *browser);

static void 
free_service (DMAPMdnsBrowserService *service);

static void
browser_add_service (
	DMAPMdnsBrowser *browser,
	const gchar *service_name,
	const gchar *domain);

static gboolean 
dmap_mdns_browser_resolve (
	DMAPMdnsBrowser *browser,
	const gchar *name,
	const gchar *domain);

void 
dnsServiceBrowseReply (
	DNSServiceRef sdRef,
	DNSServiceFlags flags,
	uint32_t interfaceIndex,
	DNSServiceErrorType errorCode,
	const char *serviceName,
	const char *regtype,
	const char *replyDomain,
	void *context);

void 
dnsServiceResolveReply (
	DNSServiceRef sdRef,
	DNSServiceFlags flags,
	uint32_t interfaceIndex,
	DNSServiceErrorType errorCode,
	const char *fullname,
	const char *hosttarget,
	uint16_t port,
	uint16_t txtLen,
	const char *txtRecord,
	void *context);

gboolean 
dmap_mdns_browser_stop (
	DMAPMdnsBrowser *browser,
	GError **error);

static const char*
service_type_name[] = 
{
	NULL,
	"_daap._tcp",
	"_dpap._tcp",
	"_touch-remote._tcp"
};


#define DMAP_MDNS_BROWSER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), DMAP_TYPE_MDNS_BROWSER, DMAPMdnsBrowserPrivate))

static guint 
dmap_mdns_browser_signals [LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (DMAPMdnsBrowser, dmap_mdns_browser, G_TYPE_OBJECT)

static void
dmap_mdns_browser_init (DMAPMdnsBrowser *browser)
{
	printf ("dmap_mdns_browser_init ()\n");
	
	browser->priv = DMAP_MDNS_BROWSER_GET_PRIVATE (browser);
	dnssd_browser_init (browser);
	
	printf ("\n");
}


gboolean
dnssd_resolve_services (DMAPMdnsBrowser *browser, DNSServiceRef sdRef)
{
	printf ("dnssd_resolve_services ()\n");
	
	int dns_sd_fd = DNSServiceRefSockFD (sdRef);
	int nFDs = dns_sd_fd + 1;
	int result = 0;
	
	struct timeval tv;
	fd_set readfds;
	
	stopNow = 0;
	
	gboolean isSuccess = FALSE;

	print_browser (browser);	

	while (!stopNow)
	{
		printf ("*** Loop:  processing ****\n");
		
		FD_ZERO (&readfds);
		FD_SET (dns_sd_fd, &readfds);

		tv.tv_sec = timeOut;
		tv.tv_usec = 0;

		result = select (nFDs, &readfds, (fd_set*) NULL, (fd_set*) NULL, &tv);

		if (result > 0)
		{
			printf ("*** Loop:  (result > 0) ****\n");

			DNSServiceErrorType err = kDNSServiceErr_NoError;

			if (FD_ISSET (dns_sd_fd, &readfds))
			{
				printf ("*** Loop:  reading daemon response ****\n");
				err = DNSServiceProcessResult (sdRef);
			}

			if (err)
			{
				printf ("*** Loop:  error reading daemon ****\n");
				stopNow = 1;
			}
			else
			{
				printf ("*** Loop:  success reading daemon ****\n");
				isSuccess = TRUE;
				stopNow = 1;
			}
		}
		else if (0 == result)
		{
			printf ("*** Loop:  waiting ****\n");
		}
		else
		{
			printf ("select () returned %d errno %d %s\n",
					result, errno, strerror (errno));

			if (errno == EBADF)
			{
				printf ("*** Loop - EBADF ****\n");
			}
			else if (errno == EINTR)
			{
				printf ("*** Loop - EINTR ****\n");
			}
			else if (errno == EISDIR)
			{
				printf ("*** Loop - EISDIR ****\n");
			}
			else if (errno == EINVAL)
			{
				printf ("*** Loop - EINVAL ****\n");
			}
			
			stopNow = 1;
		}
	}
	
	printf ("\n");

	FD_CLR (dns_sd_fd, &readfds);
	
	return isSuccess;
}



void 
dnsServiceBrowseReply (
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    DNSServiceErrorType errorCode,
    const char *serviceName,
    const char *regtype,
    const char *replyDomain,
    void *context)
{
	if (errorCode == kDNSServiceErr_NoError)
	{
		printf("dnsServiceBrowserReply ():  success\n");
		
		// Cast the context pointer to a DMAPMdnsBrowser
		DMAPMdnsBrowser *browser = (DMAPMdnsBrowser *) context;
	
		browser->priv->flags = flags;
		browser->priv->interfaceIndex = interfaceIndex;
		
		strcpy (browser->priv->serviceName, serviceName);	
		strcpy (browser->priv->regtype, regtype);	
		strcpy (browser->priv->domain, replyDomain);	
	}
	else
	{
		printf("dnsServiceBrowserReply ():  fail\n");
	}
	
	printf("\n");
}



void 
dnsServiceResolveReply (
	DNSServiceRef sdRef,
	DNSServiceFlags flags,
	uint32_t interfaceIndex,
	DNSServiceErrorType errorCode,
	const char *fullname,
	const char *hosttarget,
	uint16_t port,
	uint16_t txtLen,
	const char *txtRecord,
	void *context)
{
	printf ("dnsServiceResolveReply ()\n");
		
	// Cast the context pointer to a DMAPMdnsBrowser
	DMAPMdnsBrowser *browser = (DMAPMdnsBrowser *) context;
	
	if (kDNSServiceErr_NoError == errorCode)
	{

		browser->priv->flags = flags;
		browser->priv->interfaceIndex = interfaceIndex;
		browser->priv->port = htons (port);

		strcpy (browser->priv->fullname, fullname);	
		strcpy (browser->priv->hosttarget, hosttarget);	
	}
}



static void
dnssd_browser_init (DMAPMdnsBrowser *browser)
{
	printf ("dnssd_browser_init()\n");

	browser->priv->flags = kDNSServiceFlagsDefault;	
	browser->priv->port = 0;	
	browser->priv->interfaceIndex = 0;	
	
	strcpy (browser->priv->regtype, service_type_name[1]);	
	strcpy (browser->priv->domain, "");	
	strcpy (browser->priv->serviceName, "");	
	strcpy (browser->priv->fullname, "");	
	strcpy (browser->priv->hosttarget, "");	

	print_browser (browser);

	printf ("\n");
}



static void 
dmap_mdns_browser_class_init (DMAPMdnsBrowserClass *klass)
{
	printf ("dmap_mdns_browser_class_init()\n\n");
	
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	dmap_mdns_browser_parent_class = g_type_class_peek_parent (klass);

	object_class->dispose = dmap_mdns_browser_dispose;
	object_class->finalize = dmap_mdns_browser_finalize;

	g_type_class_add_private (klass, sizeof (DMAPMdnsBrowserPrivate));
   
	// Signal makeup
	dmap_mdns_browser_signals [SERVICE_ADDED] =
		g_signal_new ("service-added",
			  G_TYPE_FROM_CLASS (object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (DMAPMdnsBrowserClass, service_added),
			  NULL,
			  NULL,
			  g_cclosure_marshal_VOID__POINTER,
			  G_TYPE_NONE,
			  1, G_TYPE_POINTER);

	dmap_mdns_browser_signals [SERVICE_REMOVED] =
		g_signal_new ("service-removed",
			  G_TYPE_FROM_CLASS (object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (DMAPMdnsBrowserClass, service_removed),
			  NULL,
			  NULL,
			  g_cclosure_marshal_VOID__STRING,
			  G_TYPE_NONE,
			  1, G_TYPE_STRING);
}



DMAPMdnsBrowser* 
dmap_mdns_browser_new (DMAPMdnsBrowserServiceType type)
{
	printf ("dmap_mdns_browser_new ()\n\n");
	
	DMAPMdnsBrowser *browser_object = 0;

	g_return_val_if_fail (type >= DMAP_MDNS_BROWSER_SERVICE_TYPE_INVALID &&
				type <= DMAP_MDNS_BROWSER_SERVICE_TYPE_LAST, NULL);

	browser_object = 
		DMAP_MDNS_BROWSER (g_object_new (DMAP_TYPE_MDNS_BROWSER, NULL));
	browser_object->priv->service_type = type;

	return browser_object;
}



static gboolean 
dmap_mdns_browser_resolve (
	DMAPMdnsBrowser *browser,
	const gchar *name,
	const gchar *domain)
{
	printf ("dmap_mdns_browser_resolve ()\n");
	
	DMAPMdnsBrowserService *service;
	service = g_new (DMAPMdnsBrowserService, 1);
	
	service->service_name = g_strdup (browser->priv->regtype);
	service->name = name;
	service->host = g_strdup (browser->priv->hosttarget);
	service->port = browser->priv->port;
	service->pair = NULL;
	service->password_protected = FALSE;

	// add to the services list
	browser->priv->services = 
		g_slist_append (browser->priv->services, service);
	
	// notify all listeners	
	g_signal_emit (
		browser,
		dmap_mdns_browser_signals [SERVICE_ADDED], 
		0, 
		service);
	
	printf ("\n");
	
	return TRUE;	
}



static void
browser_add_service (
	DMAPMdnsBrowser *browser,
	const gchar *service_name,
	const gchar *domain)
{
	printf ("browser_add_service ()\n");

	dmap_mdns_browser_resolve (browser, service_name, domain);
	
	printf ("\n");
}



gboolean 
dmap_mdns_browser_start (DMAPMdnsBrowser *browser, GError **error)
{
	printf ("dmap_mdns_browser_start ()\n");

	gboolean isSuccess = FALSE;
	gboolean canBrowse = FALSE;
	gboolean canResolve = FALSE;
	
	DNSServiceRef sdBrowseRef;
	DNSServiceRef sdServiceRef;
	
	DNSServiceErrorType browseErr = kDNSServiceErr_Unknown;
	DNSServiceErrorType resolveErr = kDNSServiceErr_Unknown;

	print_browser (browser);

	browseErr = DNSServiceBrowse (
		&(sdBrowseRef),
		browser->priv->flags,
		browser->priv->interfaceIndex,
		browser->priv->regtype,
		browser->priv->domain,
		(DNSServiceBrowseReply) dnsServiceBrowseReply,
		(void *) browser);
	
	if (kDNSServiceErr_NoError == browseErr)
	{
		printf ("*** Browse Success ****\n");
		canBrowse = dnssd_resolve_services (browser, sdBrowseRef);
		DNSServiceRefDeallocate (sdBrowseRef);
	}

	if (canBrowse)
	{
		printf ("*** Browser read from daemon ****\n");
		resolveErr = DNSServiceResolve (
				&(sdServiceRef),
				browser->priv->flags,
				browser->priv->interfaceIndex,
				browser->priv->serviceName,
				browser->priv->regtype,
				browser->priv->domain,
				(DNSServiceResolveReply) dnsServiceResolveReply,
				(void *) browser);
	}

	if (kDNSServiceErr_NoError == resolveErr)
	{
		printf ("*** Service Resolve Success ****\n");
		canResolve = dnssd_resolve_services (browser, sdServiceRef);
		DNSServiceRefDeallocate (sdServiceRef);
	}
	
	if (canResolve)
	{
		printf ("*** Service read from daemon ****\n");

		print_browser (browser);

		// Add the successful browsed service
		browser_add_service (
			browser, 
			(gchar *) browser->priv->serviceName, 
			(gchar *) browser->priv->domain);
		
		isSuccess = TRUE;
	}	

	printf ("\n");
	
	return isSuccess;
}



/**
 *  Note that you must terminate the connection with the daemon and 
 *  free any memory associated with DNSServiceRef
 */
gboolean 
dmap_mdns_browser_stop (DMAPMdnsBrowser *browser, GError **error)
{
	return TRUE;
}



GQuark 
dmap_mdns_browser_error_quark (void)
{
	static GQuark quark = 0;

	if (!quark)
	{
		// Create a unique quark from the specified string
		quark = g_quark_from_static_string 
			("DNS-SD:  dmap_mdns_browser_error_quark"); 
	}

	return quark;
}



G_CONST_RETURN GSList*
dmap_mdns_browser_get_services (DMAPMdnsBrowser *browser)
{
	g_return_val_if_fail (browser != NULL, NULL);
	return browser->priv->services;
}



DMAPMdnsBrowserServiceType 
dmap_mdns_browser_get_service_type (DMAPMdnsBrowser *browser)
{
	g_return_val_if_fail (
		browser != NULL, 
		DMAP_MDNS_BROWSER_SERVICE_TYPE_INVALID);
	return browser->priv->service_type;
}



static void 
dmap_mdns_browser_dispose (GObject *object)
{
	DMAPMdnsBrowser *browser = DMAP_MDNS_BROWSER (object);
	GSList *walk;
	DMAPMdnsBrowserService *service;

	for (walk = browser->priv->services; walk; walk = walk->next) 
	{
		service = (DMAPMdnsBrowserService *) walk->data;
		free_service (service);
	}

	g_slist_free (browser->priv->services);

	/*
	if (browser->priv->resolvers) 
	{
		g_slist_foreach (
			browser->priv->resolvers,
			DNSServiceRefDeallocate,
			NULL);
		
		g_slist_free (browser->priv->resolvers);
	}
	*/	
    
	G_OBJECT_CLASS (dmap_mdns_browser_parent_class)->dispose (object);
}



static void 
dmap_mdns_browser_finalize (GObject *object)
{
    g_signal_handlers_destroy (object);
    G_OBJECT_CLASS (dmap_mdns_browser_parent_class)->finalize (object);
}



static void
free_service (DMAPMdnsBrowserService *service)
{
	g_free (service->service_name);
	g_free (service->name);
	g_free (service->host);
	g_free (service->pair);
	g_free (service);
}


static void
print_browser (DMAPMdnsBrowser *browser)
{
	printf ("\n======= Browser Fields =======\n");
	
	printf ("     regtype: %s\n", browser->priv->regtype);
	printf ("service_name: %s\n", browser->priv->serviceName);
	printf ("    fullname: %s\n", browser->priv->fullname);
	printf (" host target: %s\n", browser->priv->hosttarget);
	printf ("      domain: %s\n", browser->priv->domain);

	printf ("        port: %d\n", browser->priv->port);
	printf (" iface index: %d\n", browser->priv->interfaceIndex);
	printf ("       flags: %d\n", browser->priv->flags);
	printf ("==============================\n");
	
	printf ("\n");
}

