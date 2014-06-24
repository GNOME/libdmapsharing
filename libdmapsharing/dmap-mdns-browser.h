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

#ifndef __DMAP_MDNS_BROWSER_H__
#define __DMAP_MDNS_BROWSER_H__

#include <glib.h>
#include <glib-object.h>

#include <libdmapsharing/dmap-mdns-service.h>

G_BEGIN_DECLS
/**
 * DMAP_TYPE_MDNS_BROWSER:
 *
 * The type for #DMAPMdnsBrowser.
 */
#define DMAP_TYPE_MDNS_BROWSER         (dmap_mdns_browser_get_type ())
/**
 * DMAP_MDNS_BROWSER:
 * @o: Object which is subject to casting.
 *
 * Casts a #DMAPMdnsBrowser or derived pointer into a (DMAPMdnsBrowser *) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DMAP_MDNS_BROWSER(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), DMAP_TYPE_MDNS_BROWSER, DMAPMdnsBrowser))
/**
 * DMAP_MDNS_BROWSER_CLASS:
 * @k: a valid #DMAPMdnsBrowserClass
 *
 * Casts a derived #DMAPMdnsBrowserClass structure into a #DMAPMdnsBrowserClass structure.
 */
#define DMAP_MDNS_BROWSER_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), DMAP_TYPE_MDNS_BROWSER, DMAPMdnsBrowserClass))
/**
 * IS_DMAP_MDNS_BROWSER:
 * @o: Instance to check for being a %DMAP_TYPE_MDNS_BROWSER.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %DAAP_TYPE_MDNS_BROWSER.
 */
#define IS_DMAP_MDNS_BROWSER(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), DMAP_TYPE_MDNS_BROWSER))
/**
 * IS_DMAP_MDNS_BROWSER_CLASS:
 * @k: a #DMAPMdnsBrowserClass
 *
 * Checks whether @k "is a" valid #DMAPMdnsBrowserClass structure of type
 * %DMAP_MDNS_BROWSER or derived.
 */
#define IS_DMAP_MDNS_BROWSER_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), DMAP_TYPE_MDNS_BROWSER))
/**
 * DMAP_MDNS_BROWSER_GET_CLASS:
 * @o: a #DMAPMdnsBrowser instance.
 *
 * Get the class structure associated to a #DMAPMdnsBrowser instance.
 *
 * Returns: pointer to object class structure.
 */
#define DMAP_MDNS_BROWSER_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), DMAP_TYPE_MDNS_BROWSER, DMAPMdnsBrowserClass))
typedef struct _DMAPMdnsBrowser DMAPMdnsBrowser;
typedef struct _DMAPMdnsBrowserClass DMAPMdnsBrowserClass;
typedef struct _DMAPMdnsBrowserPrivate DMAPMdnsBrowserPrivate;

typedef enum
{
	DMAP_MDNS_BROWSER_ERROR_NOT_RUNNING = 0,
	DMAP_MDNS_BROWSER_ERROR_FAILED,
} DMAPMdnsBrowserError;

struct _DMAPMdnsBrowserClass
{
	GObjectClass parent_class;

	void (*service_added) (DMAPMdnsBrowser * browser,
			       DMAPMdnsService * service);
	void (*service_removed) (DMAPMdnsBrowser * browser,
				 DMAPMdnsService * service);
};

struct _DMAPMdnsBrowser
{
	GObject object;

	DMAPMdnsBrowserPrivate *priv;
};

#define DMAP_MDNS_BROWSER_ERROR dmap_mdns_browser_error_quark ()

GQuark dmap_mdns_browser_error_quark (void);

GType dmap_mdns_browser_get_type (void);

/**
 * dmap_mdns_browser_new:
 * @type: The type of service to browse.
 *
 * Creates a new mDNS browser.
 *
 * Returns: a pointer to a DMAPMdnsBrowser.
 */
DMAPMdnsBrowser *dmap_mdns_browser_new (DMAPMdnsServiceType type);

/**
 * dmap_mdns_browser_start:
 * @browser: A DMAPMdnsBrowser.
 * @error: A GError.
 *
 * Starts a DMAPMdnsBrowser.
 *
 * Returns: TRUE on success, else FALSE.
 */
gboolean dmap_mdns_browser_start (DMAPMdnsBrowser * browser, GError ** error);

/**
 * dmap_mdns_browser_stop:
 * @browser: A DMAPMdnsBrowser.
 * @error: A GError.
 *
 * Stops a DMAPMdnsBrowser.
 *
 * Returns: TRUE on success, else FALSE.
 */
gboolean dmap_mdns_browser_stop (DMAPMdnsBrowser * browser, GError ** error);

G_CONST_RETURN GSList *dmap_mdns_browser_get_services (DMAPMdnsBrowser *
						       browser);
DMAPMdnsServiceType dmap_mdns_browser_get_service_type (DMAPMdnsBrowser
							       * browser);

/**
 * DMAPMdnsBrowser::service-added:
 * @browser: the #DMAPMdnsBrowser which received the signal.
 * @service: #DMAPMdnsService
 *
 * Emitted each time a service becomes available to @browser
 */

G_END_DECLS
#endif
