/*
 * Copyright (C) 2014 W. Michael Petullo <mike@flyn.org>
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

struct _DmapMdnsServicePrivate {
	gchar *service_name;
        gchar *name;
        gchar *host;
        guint port;
        gboolean password_protected;
        gchar *pair;                                         // FIXME: subclass
        DmapMdnsServiceTransportProtocol transport_protocol; // FIXME: subclass
};

enum {
        PROP_0,
        PROP_SERVICE_NAME,
        PROP_NAME,
        PROP_HOST,
        PROP_PORT,
        PROP_PASSWORD_PROTECTED,
        PROP_PAIR,
        PROP_TRANSPORT_PROTOCOL
};

static void
_set_property (GObject *object,
               guint prop_id,
               const GValue *value,
               GParamSpec *pspec)
{
	DmapMdnsService *service = DMAP_MDNS_SERVICE (object);

	switch (prop_id) {
	case PROP_SERVICE_NAME:
		g_free (service->priv->service_name);
		service->priv->service_name = g_value_dup_string (value);
		break;
	case PROP_NAME:
		g_free (service->priv->name);
		service->priv->name = g_value_dup_string (value);
		break;
	case PROP_HOST:
		g_free (service->priv->host);
		service->priv->host = g_value_dup_string (value);
		break;
	case PROP_PORT:
		service->priv->port = g_value_get_uint (value);
		break;
	case PROP_PASSWORD_PROTECTED:
		service->priv->password_protected = g_value_get_boolean (value);
		break;
	case PROP_PAIR:
		g_free (service->priv->pair);
		service->priv->pair = g_value_dup_string (value);
		break;
	case PROP_TRANSPORT_PROTOCOL:
		service->priv->transport_protocol = g_value_get_uint (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object,
						   prop_id,
						   pspec);
		break;
	}
}

static void
_get_property (GObject *object,
               guint prop_id,
               GValue *value,
               GParamSpec *pspec)
{
	DmapMdnsService *service = DMAP_MDNS_SERVICE (object);

	switch (prop_id) {
	case PROP_SERVICE_NAME:
		g_value_set_string (value, service->priv->service_name);
		break;
	case PROP_NAME:
		g_value_set_string (value, service->priv->name);
		break;
	case PROP_HOST:
		g_value_set_string (value, service->priv->host);
		break;
	case PROP_PORT:
		g_value_set_uint (value, service->priv->port);
		break;
	case PROP_PASSWORD_PROTECTED:
		g_value_set_boolean (value, service->priv->password_protected);
		break;
	case PROP_PAIR:
		g_value_set_string (value, service->priv->pair);
		break;
	case PROP_TRANSPORT_PROTOCOL:
		g_value_set_uint (value, service->priv->transport_protocol);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object,
						   prop_id,
						   pspec);
		break;
	}
}

static void _dispose (GObject * object);
static void _finalize (GObject * object);

G_DEFINE_TYPE_WITH_PRIVATE (DmapMdnsService,
                            dmap_mdns_service,
                            G_TYPE_OBJECT);

static void
dmap_mdns_service_class_init (DmapMdnsServiceClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = _dispose;
	object_class->finalize = _finalize;
	object_class->set_property = _set_property;
	object_class->get_property = _get_property;

	dmap_mdns_service_parent_class = g_type_class_peek_parent (klass);

	g_object_class_install_property (object_class,
                                         PROP_SERVICE_NAME,
                                         g_param_spec_string ("service-name",
                                                              "Service Name",
                                                              "Service Name",
                                                              NULL,
                                                              G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
                                         PROP_NAME,
                                         g_param_spec_string ("name",
                                                              "Name",
                                                              "Name",
                                                              NULL,
                                                              G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
                                         PROP_HOST,
                                         g_param_spec_string ("host",
                                                              "Host",
                                                              "Host",
                                                              NULL,
                                                              G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
                                         PROP_PORT,
                                         g_param_spec_uint ("port",
                                                            "Port",
                                                            "Port",
                                                            0,
	                                                    G_MAXINT,
                                                            0,
                                                            G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
                                         PROP_PASSWORD_PROTECTED,
                                         g_param_spec_boolean ("password-protected",
                                                               "Password Protected",
                                                               "Password Protected",
                                                               FALSE,
                                                               G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
                                         PROP_PAIR,
                                         g_param_spec_string ("pair",
                                                              "Pair",
                                                              "Pair",
                                                              NULL,
                                                              G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
                                         PROP_TRANSPORT_PROTOCOL,
                                         g_param_spec_uint ("transport-protocol",
                                                            "Transport Protocol",
                                                            "Transport Protocol",
	                                                    DMAP_MDNS_SERVICE_TRANSPORT_PROTOCOL_TCP,
	                                                    DMAP_MDNS_SERVICE_TRANSPORT_PROTOCOL_LAST,
	                                                    DMAP_MDNS_SERVICE_TRANSPORT_PROTOCOL_TCP,
                                                            G_PARAM_READWRITE));
}

static void
dmap_mdns_service_init (DmapMdnsService * service)
{
	service->priv = dmap_mdns_service_get_instance_private(service);
}

static void
_dispose (GObject * object)
{
	G_OBJECT_CLASS (dmap_mdns_service_parent_class)->dispose (object);
}

static void
_finalize (GObject * object)
{
	DmapMdnsService *service = DMAP_MDNS_SERVICE (object);

	g_signal_handlers_destroy (object);

	if (service->priv->service_name) {
		g_free (service->priv->service_name);
	}

	if (service->priv->name) {
		g_free (service->priv->name);
	}

	if (service->priv->host) {
		g_free (service->priv->host);
	}

	if (service->priv->pair) {
		g_free (service->priv->pair);
	}

	G_OBJECT_CLASS (dmap_mdns_service_parent_class)->finalize (object);
}
