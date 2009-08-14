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

G_BEGIN_DECLS

#define TYPE_DMAP_MDNS_BROWSER         (dmap_mdns_browser_get_type ())
#define DMAP_MDNS_BROWSER(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_DMAP_MDNS_BROWSER, DMAPMdnsBrowser))
#define DMAP_MDNS_BROWSER_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), TYPE_DMAP_MDNS_BROWSER, DMAPMdnsBrowserClass))
#define IS_DMAP_MDNS_BROWSER(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_DMAP_MDNS_BROWSER))
#define IS_DMAP_MDNS_BROWSER_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_DMAP_MDNS_BROWSER))
#define DMAP_MDNS_BROWSER_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_DMAP_MDNS_BROWSER, DMAPMdnsBrowserClass))

typedef struct _DMAPMdnsBrowser        DMAPMdnsBrowser;
typedef struct _DMAPMdnsBrowserClass   DMAPMdnsBrowserClass;
typedef struct _DMAPMdnsBrowserPrivate DMAPMdnsBrowserPrivate;
typedef struct _DMAPMdnsBrowserService DMAPMdnsBrowserService;

typedef enum
{
    DMAP_MDNS_BROWSER_SERVICE_TYPE_INVALID = 0,
    DMAP_MDNS_BROWSER_SERVICE_TYPE_DAAP,
    DMAP_MDNS_BROWSER_SERVICE_TYPE_DPAP,
    DMAP_MDNS_BROWSER_SERVICE_TYPE_LAST = DMAP_MDNS_BROWSER_SERVICE_TYPE_DPAP
} DMAPMdnsBrowserServiceType;

typedef enum
{
    DMAP_MDNS_BROWSER_ERROR_NOT_RUNNING = 0,
    DMAP_MDNS_BROWSER_ERROR_FAILED,
} DMAPMdnsBrowserError;

struct _DMAPMdnsBrowserService
{
    gchar *service_name;
    gchar *name;
    gchar *host;
    guint port;
    gboolean password_protected;
};

struct _DMAPMdnsBrowserClass
{
    GObjectClass parent_class;

    void (* service_added)    (DMAPMdnsBrowser *browser,
                               DMAPMdnsBrowserService *service);
    void (* service_removed ) (DMAPMdnsBrowser *browser,
                               DMAPMdnsBrowserService *service);
};

struct _DMAPMdnsBrowser
{
    GObject object;

    DMAPMdnsBrowserPrivate *priv;
};

#define DMAP_MDNS_BROWSER_ERROR dmap_mdns_browser_error_quark ()

GQuark                     dmap_mdns_browser_error_quark      (void);

GType                      dmap_mdns_browser_get_type         (void);

DMAPMdnsBrowser           *dmap_mdns_browser_new              (DMAPMdnsBrowserServiceType type);

gboolean                   dmap_mdns_browser_start            (DMAPMdnsBrowser *browser,
                                                               GError **error);
gboolean                   dmap_mdns_browser_stop             (DMAPMdnsBrowser *browser,
                                                               GError **error);

G_CONST_RETURN GSList     *dmap_mdns_browser_get_services     (DMAPMdnsBrowser *browser);
DMAPMdnsBrowserServiceType dmap_mdns_browser_get_service_type (DMAPMdnsBrowser *browser);

G_END_DECLS

#endif
