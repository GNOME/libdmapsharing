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

#ifndef _DMAP_MDNS_BROWSER_H
#define _DMAP_MDNS_BROWSER_H

#include <glib.h>
#include <glib-object.h>

#include <libdmapsharing/dmap-mdns-service.h>

G_BEGIN_DECLS
/**
 * SECTION: dmap-mdns-browser
 * @short_description: An mDNS browser.
 *
 * #DmapMdnsBrowser objects watch for DMAP shares.
 */

/**
 * DMAP_TYPE_MDNS_BROWSER:
 *
 * The type for #DmapMdnsBrowser.
 */
#define DMAP_TYPE_MDNS_BROWSER         (dmap_mdns_browser_get_type ())
/**
 * DMAP_MDNS_BROWSER:
 * @o: Object which is subject to casting.
 *
 * Casts a #DmapMdnsBrowser or derived pointer into a (DmapMdnsBrowser *) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DMAP_MDNS_BROWSER(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), DMAP_TYPE_MDNS_BROWSER, DmapMdnsBrowser))
/**
 * DMAP_MDNS_BROWSER_CLASS:
 * @k: a valid #DmapMdnsBrowserClass
 *
 * Casts a derived #DmapMdnsBrowserClass structure into a #DmapMdnsBrowserClass structure.
 */
#define DMAP_MDNS_BROWSER_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), DMAP_TYPE_MDNS_BROWSER, DmapMdnsBrowserClass))
/**
 * DMAP_IS_MDNS_BROWSER:
 * @o: Instance to check for being a %DMAP_TYPE_MDNS_BROWSER.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %DAAP_TYPE_MDNS_BROWSER.
 */
#define DMAP_IS_MDNS_BROWSER(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), DMAP_TYPE_MDNS_BROWSER))
/**
 * DMAP_IS_MDNS_BROWSER_CLASS:
 * @k: a #DmapMdnsBrowserClass
 *
 * Checks whether @k "is a" valid #DmapMdnsBrowserClass structure of type
 * %DMAP_MDNS_BROWSER or derived.
 */
#define DMAP_IS_MDNS_BROWSER_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), DMAP_TYPE_MDNS_BROWSER))
/**
 * DMAP_MDNS_BROWSER_GET_CLASS:
 * @o: a #DmapMdnsBrowser instance.
 *
 * Get the class structure associated to a #DmapMdnsBrowser instance.
 *
 * Returns: pointer to object class structure.
 */
#define DMAP_MDNS_BROWSER_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), DMAP_TYPE_MDNS_BROWSER, DmapMdnsBrowserClass))

typedef struct _DmapMdnsBrowserPrivate DmapMdnsBrowserPrivate;

typedef enum {
	DMAP_MDNS_BROWSER_ERROR_NOT_RUNNING = 0,
	DMAP_MDNS_BROWSER_ERROR_FAILED,
} DmapMdnsBrowserError;

typedef struct {
	GObject object;

	DmapMdnsBrowserPrivate *priv;
} DmapMdnsBrowser;

typedef struct {
	GObjectClass parent_class;

	void (*service_added) (DmapMdnsBrowser *browser,
			       DmapMdnsService *service);
	void (*service_removed) (DmapMdnsBrowser *browser,
				 DmapMdnsService *service);
} DmapMdnsBrowserClass;

#define DMAP_MDNS_BROWSER_ERROR dmap_mdns_browser_error_quark ()

GQuark dmap_mdns_browser_error_quark (void);

GType dmap_mdns_browser_get_type (void);

/**
 * dmap_mdns_browser_new:
 * @type: The type of service to browse.
 *
 * Creates a new mDNS browser.
 *
 * Returns: a pointer to a DmapMdnsBrowser.
 */
DmapMdnsBrowser *dmap_mdns_browser_new (DmapMdnsServiceType type);

/**
 * dmap_mdns_browser_start:
 * @browser: A DmapMdnsBrowser.
 * @error: A GError.
 *
 * Starts a DmapMdnsBrowser.
 *
 * Returns: TRUE on success, else FALSE.
 */
gboolean dmap_mdns_browser_start (DmapMdnsBrowser * browser, GError ** error);

/**
 * dmap_mdns_browser_stop:
 * @browser: A DmapMdnsBrowser.
 * @error: A GError.
 *
 * Stops a DmapMdnsBrowser.
 *
 * Returns: TRUE on success, else FALSE.
 */
gboolean dmap_mdns_browser_stop (DmapMdnsBrowser * browser, GError ** error);

/**
 * dmap_mdns_browser_get_services:
 * @browser: A DmapMdnsBrowser.
 *
 * Returns: (element-type DmapMdnsService) (transfer none): services available to @browser.
 */
const GSList *dmap_mdns_browser_get_services (DmapMdnsBrowser *
						       browser);
DmapMdnsServiceType dmap_mdns_browser_get_service_type (DmapMdnsBrowser
							       * browser);

/**
 * DmapMdnsBrowser::service-added:
 * @browser: the #DmapMdnsBrowser which received the signal.
 * @service: #DmapMdnsService
 *
 * Emitted each time a service becomes available to @browser
 */

G_END_DECLS
#endif
