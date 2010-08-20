/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2005 Charles Schmidt <cschmidt2@emich.edu>
 * Copyright (C) 2006 William Jon McCann <mccann@jhu.edu>
 *
 * Modifications Copyright (C) 2008 W. Michael Petullo <mike@flyn.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "config.h"

#include <stdlib.h>
#include <stdio.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <glib-object.h>

#include <avahi-client/lookup.h>
#include <avahi-client/publish.h>
#include <avahi-client/client.h>
#include <avahi-common/error.h>
#include <avahi-glib/glib-malloc.h>
#include <avahi-glib/glib-watch.h>

#include "dmap-mdns-avahi.h"
#include "dmap-mdns-publisher.h"

static void	dmap_mdns_publisher_class_init (DmapMdnsPublisherClass *klass);
static void	dmap_mdns_publisher_init	  (DmapMdnsPublisher	    *publisher);
static void	dmap_mdns_publisher_finalize   (GObject	            *object);

#define DMAP_MDNS_PUBLISHER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_DMAP_MDNS_PUBLISHER, DmapMdnsPublisherPrivate))

struct DmapMdnsPublisherPrivate
{
	AvahiClient     *client;
	AvahiEntryGroup *entry_group;

	char            *name;
	guint            port;
	char		*type_of_service;
	gboolean         password_required;
	gchar          **txt_records;
};

enum {
	PUBLISHED,
	NAME_COLLISION,
	LAST_SIGNAL
};

enum {
	PROP_0
};

static guint	     signals [LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (DmapMdnsPublisher, dmap_mdns_publisher, G_TYPE_OBJECT)

static gpointer publisher_object = NULL;

GQuark
dmap_mdns_publisher_error_quark (void)
{
	static GQuark quark = 0;
	if (!quark)
		quark = g_quark_from_static_string ("dmap_mdns_publisher_error");

	return quark;
}

static void
entry_group_cb (AvahiEntryGroup     *group,
		AvahiEntryGroupState state,
		DmapMdnsPublisher *publisher)
{
	if (state == AVAHI_ENTRY_GROUP_ESTABLISHED) {

		g_signal_emit (publisher, signals [PUBLISHED], 0, publisher->priv->name);

	} else if (state == AVAHI_ENTRY_GROUP_COLLISION) {
		g_warning ("MDNS name collision");

		g_signal_emit (publisher, signals [NAME_COLLISION], 0, publisher->priv->name);
	}
}

static gboolean
create_service (DmapMdnsPublisher *publisher,
		GError             **error)
{
	int         ret;
	const char *password_record;
	AvahiStringList *txt_records;

	if (publisher->priv->entry_group == NULL) {
		publisher->priv->entry_group = avahi_entry_group_new (publisher->priv->client,
								      (AvahiEntryGroupCallback)entry_group_cb,
								      publisher);
		dmap_mdns_avahi_set_entry_group (publisher->priv->entry_group);
	} else {
		avahi_entry_group_reset (publisher->priv->entry_group);
	}

	if (publisher->priv->entry_group == NULL) {
		g_warning ("Could not create AvahiEntryGroup for publishing");
		g_set_error (error,
			     DMAP_MDNS_PUBLISHER_ERROR,
			     DMAP_MDNS_PUBLISHER_ERROR_FAILED,
			     "%s",
			     _("Could not create AvahiEntryGroup for publishing"));
		return FALSE;
	}

#if 0
	g_message ("Service name:%s port:%u password:%d",
		   publisher->priv->name,
		   publisher->priv->port,
		   publisher->priv->password_required);
#endif

	if (publisher->priv->password_required) {
		password_record = "Password=true";
	} else {
		password_record = "Password=false";
	}
	
	txt_records = avahi_string_list_new(password_record, NULL);
	
	if (publisher->priv->txt_records) {
		//g_debug("Number of txt records: %d", publisher->priv->txt_records);
		gchar **txt_record = publisher->priv->txt_records;
		while (*txt_record) {
			txt_records = avahi_string_list_add(txt_records, *txt_record);
			txt_record++;
		}
	}

	ret = avahi_entry_group_add_service_strlst (publisher->priv->entry_group,
					     AVAHI_IF_UNSPEC,
					     AVAHI_PROTO_UNSPEC,
					     0,
					     publisher->priv->name,
					     publisher->priv->type_of_service,
					     NULL,
					     NULL,
					     publisher->priv->port,
					     txt_records);
					     
	avahi_string_list_free(txt_records);

	if (ret < 0) {
		g_set_error (error,
			     DMAP_MDNS_PUBLISHER_ERROR,
			     DMAP_MDNS_PUBLISHER_ERROR_FAILED,
			     "%s: %s",
			     _("Could not add service"),
			     avahi_strerror (ret));
		return FALSE;
	}

	ret = avahi_entry_group_commit (publisher->priv->entry_group);

	if (ret < 0) {
		g_set_error (error,
			     DMAP_MDNS_PUBLISHER_ERROR,
			     DMAP_MDNS_PUBLISHER_ERROR_FAILED,
			     "%s: %s",
			     _("Could not commit service"),
			     avahi_strerror (ret));
		return FALSE;
	}

	return TRUE;
}

static gboolean
refresh_service (DmapMdnsPublisher *publisher,
		 GError             **error)
{
	return create_service (publisher, error);
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

gboolean
dmap_mdns_publisher_set_name (DmapMdnsPublisher *publisher,
				 const char          *name,
				 GError             **error)
{
        g_return_val_if_fail (publisher != NULL, FALSE);

	publisher_set_name_internal (publisher, name, error);

	if (publisher->priv->entry_group) {
		refresh_service (publisher, error);
	}

	return TRUE;
}

static gboolean
publisher_set_port_internal (DmapMdnsPublisher *publisher,
			     guint                port,
			     GError             **error)
{
	publisher->priv->port = port;

	return TRUE;
}

gboolean
dmap_mdns_publisher_set_port (DmapMdnsPublisher *publisher,
				 guint                port,
				 GError             **error)
{
        g_return_val_if_fail (publisher != NULL, FALSE);

	publisher_set_port_internal (publisher, port, error);

	if (publisher->priv->entry_group) {
		refresh_service (publisher, error);
	}

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

	if (publisher->priv->entry_group) {
		refresh_service (publisher, error);
	}

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

	if (publisher->priv->entry_group) {
		refresh_service (publisher, error);
	}

	return TRUE;
}


static gboolean
publisher_set_txt_records_internal (DmapMdnsPublisher *publisher,
					  gchar              **txt_records,
					  GError             **error)
{
	g_strfreev (publisher->priv->txt_records);
	publisher->priv->txt_records = g_strdupv (txt_records);
	return TRUE;
}

gboolean
dmap_mdns_publisher_set_txt_records (DmapMdnsPublisher *publisher,
					      gchar              **txt_records,
					      GError             **error)
{
        g_return_val_if_fail (publisher != NULL, FALSE);

	publisher_set_txt_records_internal (publisher, txt_records, error);

	if (publisher->priv->entry_group) {
		refresh_service (publisher, error);
	}

	return TRUE;
}

gboolean
dmap_mdns_publisher_publish (DmapMdnsPublisher *publisher,
				const char          *name,
				guint                port,
				const char          *type_of_service,
				gboolean             password_required,
				gchar              **txt_records,
				GError             **error)
{
	if (publisher->priv->client == NULL) {
                g_set_error (error,
			     DMAP_MDNS_PUBLISHER_ERROR,
			     DMAP_MDNS_PUBLISHER_ERROR_NOT_RUNNING,
                             "%s",
                             _("The avahi MDNS service is not running"));
		return FALSE;
	}

	publisher_set_name_internal (publisher, name, NULL);
	publisher_set_port_internal (publisher, port, NULL);
	publisher_set_type_of_service_internal (publisher, type_of_service, NULL);
	publisher_set_password_required_internal (publisher, password_required, NULL);
	publisher_set_txt_records_internal (publisher, txt_records, NULL);

	return create_service (publisher, error);
}

gboolean
dmap_mdns_publisher_withdraw (DmapMdnsPublisher *publisher,
				 GError             **error)
{
	if (publisher->priv->client == NULL) {
                g_set_error (error,
			     DMAP_MDNS_PUBLISHER_ERROR,
			     DMAP_MDNS_PUBLISHER_ERROR_NOT_RUNNING,
                             "%s",
                             _("The avahi MDNS service is not running"));
		return FALSE;
	}

	if (publisher->priv->entry_group == NULL) {
                g_set_error (error,
			     DMAP_MDNS_PUBLISHER_ERROR,
			     DMAP_MDNS_PUBLISHER_ERROR_FAILED,
                             "%s",
                             _("The MDNS service is not published"));
		return FALSE;
	}

	avahi_entry_group_reset (publisher->priv->entry_group);
	avahi_entry_group_free (publisher->priv->entry_group);
	publisher->priv->entry_group = NULL;

	return TRUE;
}

static void
dmap_mdns_publisher_set_property (GObject	  *object,
				     guint	   prop_id,
				     const GValue *value,
				     GParamSpec	  *pspec)
{
	switch (prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
dmap_mdns_publisher_get_property (GObject	 *object,
				     guint	  prop_id,
				     GValue	 *value,
				     GParamSpec	 *pspec)
{
	switch (prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
dmap_mdns_publisher_class_init (DmapMdnsPublisherClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize	   = dmap_mdns_publisher_finalize;
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

	publisher->priv->client = dmap_mdns_avahi_get_client ();
}

static void
dmap_mdns_publisher_finalize (GObject *object)
{
	DmapMdnsPublisher *publisher;

	g_return_if_fail (object != NULL);
	g_return_if_fail (IS_DMAP_MDNS_PUBLISHER (object));

	publisher = DMAP_MDNS_PUBLISHER (object);

	g_return_if_fail (publisher->priv != NULL);

	if (publisher->priv->entry_group) {
		avahi_entry_group_free (publisher->priv->entry_group);
		publisher->priv->entry_group = NULL;
	}

	avahi_client_free (publisher->priv->client);

	g_free (publisher->priv->name);
	g_free (publisher->priv->type_of_service);
	
	g_strfreev (publisher->priv->txt_records);

	G_OBJECT_CLASS (dmap_mdns_publisher_parent_class)->finalize (object);
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
