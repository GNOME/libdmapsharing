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

#include "config.h"

#include <stdio.h>
#include <glib.h>
#include <dns_sd.h>
#include <arpa/inet.h>

#include "dmap-mdns-publisher.h"

#define DMAP_MDNS_PUBLISHER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_DMAP_MDNS_PUBLISHER, DmapMdnsPublisherPrivate))

struct DmapMdnsPublisherPrivate
{
	DNSServiceRef	 sdref;
        char            *name;
        guint16          port;
        char            *type_of_service;
        gboolean         password_required;
};

enum {
        PUBLISHED,
	NAME_COLLISION,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (DmapMdnsPublisher, dmap_mdns_publisher, G_TYPE_OBJECT)

static gpointer publisher_object = NULL;

GQuark
dmap_mdns_publisher_error_quark (void)
{        static GQuark quark = 0;
        if (!quark)
                quark = g_quark_from_static_string ("dmap_mdns_publisher_error");

        return quark;
}

static gboolean
publisher_set_name_internal (DmapMdnsPublisher *publisher,
                             const char          *name,
                             GError             **error)
{
        g_free (publisher->priv->name);
        publisher->priv->name = g_strdup (name);

        return TRUE;
}

static gboolean
publisher_set_port_internal (DmapMdnsPublisher *publisher,
                             gint               port,
                             GError             **error)
{
        publisher->priv->port = port;

	return TRUE;
}

gboolean
dmap_mdns_publisher_set_name (DmapMdnsPublisher *publisher,
                                 const char          *name,
                                 GError             **error)
{
        g_return_val_if_fail (publisher != NULL, FALSE);

        publisher_set_name_internal (publisher, name, error);

        return TRUE;
}

static gboolean
publisher_set_type_of_service_internal (DmapMdnsPublisher *publisher,
                             const char          *type_of_service,
                             GError             **error)
{
        g_free (publisher->priv->type_of_service);
        publisher->priv->type_of_service = g_strdup (type_of_service);

        return TRUE;
}

gboolean
dmap_mdns_publisher_set_type_of_service (DmapMdnsPublisher *publisher,
                                 const char          *type_of_service,
                                 GError             **error)
{
        g_return_val_if_fail (publisher != NULL, FALSE);

        publisher_set_type_of_service_internal (publisher, type_of_service, error);

        return TRUE;
}

static gboolean
publisher_set_password_required_internal (DmapMdnsPublisher *publisher,
                                          gboolean             required,
                                          GError             **error)
{
        publisher->priv->password_required = required;
        return TRUE;
}

gboolean
dmap_mdns_publisher_set_password_required (DmapMdnsPublisher *publisher,
                                              gboolean             required,
                                              GError             **error)
{
        g_return_val_if_fail (publisher != NULL, FALSE);

        publisher_set_password_required_internal (publisher, required, error);

        return TRUE;
}

gboolean
dmap_mdns_publisher_publish (DmapMdnsPublisher *publisher,
                                const char          *name,
                                guint                port,
                                const char          *type_of_service,
                                gboolean             password_required,
                                GError             **error)
{
	int dns_err;

        publisher_set_name_internal (publisher, name, NULL);
        publisher_set_port_internal (publisher, port, NULL);
        publisher_set_type_of_service_internal (publisher, type_of_service, NULL);
        publisher_set_password_required_internal (publisher, password_required, NULL);

	g_warning ("%s %s %d", publisher->priv->name, publisher->priv->type_of_service, publisher->priv->port);
	if ((dns_err = DNSServiceRegister (&publisher->priv->sdref,
		0,
		0,
		name,
		type_of_service,
		NULL,
		NULL,
		htons (port),
		0,
		NULL,
		NULL,
		NULL)) != kDNSServiceErr_NoError) {
                g_set_error (error,
                             DMAP_MDNS_PUBLISHER_ERROR,
                             DMAP_MDNS_PUBLISHER_ERROR_FAILED,
                             "%s: %d",
                             "Error publishing via DNSSD", dns_err);
		if (dns_err == kDNSServiceErr_NameConflict) {
			g_signal_emit (publisher, signals[NAME_COLLISION], 0, publisher->priv->name);
		}
                return FALSE;
        }

	g_signal_emit (publisher, signals[PUBLISHED], 0, publisher->priv->name);

        return TRUE;
}

gboolean
dmap_mdns_publisher_withdraw (DmapMdnsPublisher *publisher,
                                 GError             **error)
{
	g_error ("Not implemented");

        return TRUE;
}

static void
dmap_mdns_publisher_set_property (GObject         *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
        switch (prop_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                break;
        }
}

static void
dmap_mdns_publisher_get_property (GObject        *object,
                                     guint        prop_id,
                                     GValue      *value,
                                     GParamSpec  *pspec)
{
        switch (prop_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                break;
        }
}

static void
dmap_mdns_publisher_finalize (GObject *object)
{
        DmapMdnsPublisher *publisher;

        g_return_if_fail (object != NULL);
        g_return_if_fail (IS_DMAP_MDNS_PUBLISHER (object));

        publisher = DMAP_MDNS_PUBLISHER (object);

        g_return_if_fail (publisher->priv != NULL);

        g_free (publisher->priv->name);
        g_free (publisher->priv->type_of_service);

        G_OBJECT_CLASS (dmap_mdns_publisher_parent_class)->finalize (object);
}

static void
dmap_mdns_publisher_class_init (DmapMdnsPublisherClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->finalize     = dmap_mdns_publisher_finalize;
        object_class->get_property = dmap_mdns_publisher_get_property;
        object_class->set_property = dmap_mdns_publisher_set_property;

	signals [PUBLISHED] =
                g_signal_new ("published",
                              G_TYPE_FROM_CLASS (object_class),
                              G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (DmapMdnsPublisherClass, published),
                              NULL,
                              NULL,
                              g_cclosure_marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1, G_TYPE_STRING);
        signals [NAME_COLLISION] =
                g_signal_new ("name-collision",
                              G_TYPE_FROM_CLASS (object_class),
                              G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (DmapMdnsPublisherClass, name_collision),
                              NULL,
                              NULL,
                              g_cclosure_marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1, G_TYPE_STRING);

        g_type_class_add_private (klass, sizeof (DmapMdnsPublisherPrivate));
}

static void
dmap_mdns_publisher_init (DmapMdnsPublisher *publisher)
{
        publisher->priv = DMAP_MDNS_PUBLISHER_GET_PRIVATE (publisher);
}

DmapMdnsPublisher *
dmap_mdns_publisher_new (void)
{
        if (publisher_object) {
                g_object_ref (publisher_object);
        } else {
                publisher_object = g_object_new (TYPE_DMAP_MDNS_PUBLISHER, NULL);
                g_object_add_weak_pointer (publisher_object,
                                           (gpointer *) &publisher_object);
        }

        return DMAP_MDNS_PUBLISHER (publisher_object);
}
