/* Copyright (C) 2005 Charles Schmidt <cschmidt2@emich.edu>
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

static void	dmap_mdns_publisher_class_init (DMAPMdnsPublisherClass *klass);
static void	dmap_mdns_publisher_init	  (DMAPMdnsPublisher	    *publisher);
static void	dmap_mdns_publisher_finalize   (GObject	            *object);

#define DMAP_MDNS_PUBLISHER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), DMAP_TYPE_MDNS_PUBLISHER, DMAPMdnsPublisherPrivate))

struct DMAPMdnsPublisherService
{
	char            *name;
	guint            port;
	char		*type_of_service;
	gboolean         password_required;
	gchar          **txt_records;
};

struct DMAPMdnsPublisherPrivate
{
	AvahiClient     *client;
	AvahiEntryGroup *entry_group;
	GSList		*service;
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

G_DEFINE_TYPE (DMAPMdnsPublisher, dmap_mdns_publisher, G_TYPE_OBJECT)

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
emit_published (char *name, DMAPMdnsPublisher *publisher)
{
	g_signal_emit (publisher, signals [PUBLISHED], 0, name);
}

static void
entry_group_cb (AvahiEntryGroup     *group,
		AvahiEntryGroupState state,
		DMAPMdnsPublisher *publisher)
{
	if (state == AVAHI_ENTRY_GROUP_ESTABLISHED) {
		g_slist_foreach (publisher->priv->service, (GFunc) emit_published, publisher);
	} else if (state == AVAHI_ENTRY_GROUP_COLLISION) {
		g_warning ("MDNS name collision");

		/* FIXME: how to know which name collided?
		g_signal_emit (publisher, signals [NAME_COLLISION], 0, publisher->priv->name);
		*/
		g_signal_emit (publisher, signals [NAME_COLLISION], 0, "unknown");
	}
}

static gboolean
create_service (struct DMAPMdnsPublisherService *service,
		DMAPMdnsPublisher *publisher,
		GError **error)
{
	int ret;
	const char *password_record;
	AvahiStringList *txt_records;

	if (service->password_required) {
		password_record = "Password=true";
	} else {
		password_record = "Password=false";
	}
	
	txt_records = avahi_string_list_new(password_record, NULL);
	
	if (service->txt_records) {
		//g_debug("Number of txt records: %d", service->txt_records);
		gchar **txt_record = service->txt_records;
		while (*txt_record) {
			txt_records = avahi_string_list_add(txt_records, *txt_record);
			txt_record++;
		}
	}

	ret = avahi_entry_group_add_service_strlst (publisher->priv->entry_group,
					     AVAHI_IF_UNSPEC,
					     AVAHI_PROTO_UNSPEC,
					     0,
					     service->name,
					     service->type_of_service,
					     NULL,
					     NULL,
					     service->port,
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

	return TRUE;
}

static gboolean
create_services (DMAPMdnsPublisher *publisher,
		 GError             **error)
{
	GSList     *ptr;
	int         ret;

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

	for (ptr = publisher->priv->service; ptr; ptr = g_slist_next (ptr)) {
		if (! create_service (ptr->data, publisher, error)) {
			return FALSE;
		}
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
refresh_services (DMAPMdnsPublisher *publisher,
		  GError             **error)
{
	return create_services (publisher, error);
}

static struct DMAPMdnsPublisherService *
find_service_by_port (GSList *list, guint port)
{
	GSList *ptr;

	for (ptr = list; ptr; ptr = g_slist_next (ptr)) {
		if (port == ((struct DMAPMdnsPublisherService *) ptr->data)->port)
			break;
	}

	return ptr ? ptr->data : NULL;
}

gboolean
dmap_mdns_publisher_rename_at_port (DMAPMdnsPublisher *publisher,
				    guint	       port,
				    const char        *name,
				    GError           **error)
{
	struct DMAPMdnsPublisherService *ptr;

        g_return_val_if_fail (publisher != NULL, FALSE);

	ptr = find_service_by_port (publisher->priv->service, port);

	if (ptr == NULL) {
                g_set_error (error,
			     DMAP_MDNS_PUBLISHER_ERROR,
			     DMAP_MDNS_PUBLISHER_ERROR_FAILED,
                             "%s",
                             _("No service at port"));
		return FALSE;
	}

	g_free (ptr->name);
	ptr->name = g_strdup (name);

	if (publisher->priv->entry_group) {
		refresh_services (publisher, error);
	}

	return TRUE;
}

gboolean
dmap_mdns_publisher_publish (DMAPMdnsPublisher *publisher,
				const char          *name,
				guint                port,
				const char          *type_of_service,
				gboolean             password_required,
				gchar              **txt_records,
				GError             **error)
{
	struct DMAPMdnsPublisherService *service;

	if (publisher->priv->client == NULL) {
                g_set_error (error,
			     DMAP_MDNS_PUBLISHER_ERROR,
			     DMAP_MDNS_PUBLISHER_ERROR_NOT_RUNNING,
                             "%s",
                             _("The avahi MDNS service is not running"));
		return FALSE;
	}

	
	service = g_new (struct DMAPMdnsPublisherService, 1);

	service->name = g_strdup(name);
	service->port = port;
	service->type_of_service = g_strdup (type_of_service);
	service->password_required = password_required;
	service->txt_records = g_strdupv (txt_records);

	publisher->priv->service = g_slist_append (publisher->priv->service, service);

	return create_services (publisher, error);
}

static void
free_service (struct DMAPMdnsPublisherService *service, gpointer user_data)
{
	g_free (service->name);
	g_free (service->type_of_service);
	g_strfreev (service->txt_records);
}

gboolean
dmap_mdns_publisher_withdraw (DMAPMdnsPublisher *publisher,
			      guint port,
			      GError             **error)
{
	struct DMAPMdnsPublisherService *ptr;

	if (publisher->priv->client == NULL) {
                g_set_error (error,
			     DMAP_MDNS_PUBLISHER_ERROR,
			     DMAP_MDNS_PUBLISHER_ERROR_NOT_RUNNING,
                             "%s",
                             _("The avahi MDNS service is not running"));
		return FALSE;
	}

	if (publisher->priv->entry_group == NULL
	    || ! (ptr = find_service_by_port (publisher->priv->service, port))) {
                g_set_error (error,
			     DMAP_MDNS_PUBLISHER_ERROR,
			     DMAP_MDNS_PUBLISHER_ERROR_FAILED,
                             "%s",
                             _("The MDNS service is not published"));
		return FALSE;
	}

	free_service (ptr, NULL);
	publisher->priv->service = g_slist_remove (publisher->priv->service, ptr);

	if (publisher->priv->service == NULL) {
		avahi_entry_group_reset (publisher->priv->entry_group);
		avahi_entry_group_free (publisher->priv->entry_group);
		publisher->priv->entry_group = NULL;
	} else {
		create_services (publisher, error);
		if (error != NULL)
			return FALSE;
	}

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

static GObject *
dmap_mdns_publisher_constructor (GType type,
				 guint n_construct_params,
				 GObjectConstructParam *construct_params)
{
	/* This class is a singleton. */
	static GObject *self = NULL;

	if (self == NULL) {
		self = G_OBJECT_CLASS (dmap_mdns_publisher_parent_class)->constructor (
			type, n_construct_params, construct_params);
		g_object_add_weak_pointer (self, (gpointer) &self);
		return self;
	}

	return g_object_ref (self);
}

static void
dmap_mdns_publisher_class_init (DMAPMdnsPublisherClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->constructor  = dmap_mdns_publisher_constructor;
	object_class->finalize	   = dmap_mdns_publisher_finalize;
	object_class->get_property = dmap_mdns_publisher_get_property;
	object_class->set_property = dmap_mdns_publisher_set_property;

	signals [PUBLISHED] =
		g_signal_new ("published",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (DMAPMdnsPublisherClass, published),
			      NULL,
			      NULL,
			      g_cclosure_marshal_VOID__STRING,
			      G_TYPE_NONE,
			      1, G_TYPE_STRING);
	signals [NAME_COLLISION] =
		g_signal_new ("name-collision",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (DMAPMdnsPublisherClass, name_collision),
			      NULL,
			      NULL,
			      g_cclosure_marshal_VOID__STRING,
			      G_TYPE_NONE,
			      1, G_TYPE_STRING);

	g_type_class_add_private (klass, sizeof (DMAPMdnsPublisherPrivate));
}

static void
dmap_mdns_publisher_init (DMAPMdnsPublisher *publisher)
{
	publisher->priv = DMAP_MDNS_PUBLISHER_GET_PRIVATE (publisher);

	publisher->priv->client      = dmap_mdns_avahi_get_client ();
	publisher->priv->entry_group = NULL;
	publisher->priv->service     = NULL;
}

static void
dmap_mdns_publisher_finalize (GObject *object)
{
	DMAPMdnsPublisher *publisher;

	g_return_if_fail (object != NULL);
	g_return_if_fail (IS_DMAP_MDNS_PUBLISHER (object));

	publisher = DMAP_MDNS_PUBLISHER (object);

	g_return_if_fail (publisher->priv != NULL);

	if (publisher->priv->entry_group) {
		avahi_entry_group_free (publisher->priv->entry_group);
		publisher->priv->entry_group = NULL;
	}

	/* FIXME: dmap_mdns_avahi_get_client() ensures that a client is initialized only
	 * once during the lifetime of a program. This needs to be changed so that the
	 * following call works. Otherwise, an application like Rhythmbox will crash if 
	 * a user deactivates and then activates the DAAP plugin. In this case,
	 * publisher->priv->client will be free'd but will not be allocated in a
	 * successive call to dmap_mdns_avahi_get_client().
	 *
	 * avahi_client_free (publisher->priv->client);
	 */

	g_slist_foreach (publisher->priv->service, (GFunc) free_service, NULL);
	g_slist_free (publisher->priv->service);

	publisher_object = NULL;

	G_OBJECT_CLASS (dmap_mdns_publisher_parent_class)->finalize (object);
}

DMAPMdnsPublisher *
dmap_mdns_publisher_new (void)
{
	if (publisher_object) {
		g_object_ref (publisher_object);
	} else {
		publisher_object = g_object_new (DMAP_TYPE_MDNS_PUBLISHER, NULL);
		g_object_add_weak_pointer (publisher_object,
					   (gpointer *) &publisher_object);
	}

	return DMAP_MDNS_PUBLISHER (publisher_object);
}
