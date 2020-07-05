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

static void dmap_mdns_publisher_class_init (DmapMdnsPublisherClass * klass);
static void dmap_mdns_publisher_init (DmapMdnsPublisher * publisher);
static void _finalize (GObject * object);

struct DmapMdnsPublisherService
{
	char *name;
	guint port;
	char *type_of_service;
	gboolean password_required;
	gchar **txt_records;
};

struct DmapMdnsPublisherPrivate
{
	AvahiClient *client;
	AvahiEntryGroup *entry_group;
	GSList *service;
};

enum
{
	PUBLISHED,
	NAME_COLLISION,
	LAST_SIGNAL
};

static guint _signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE_WITH_PRIVATE (DmapMdnsPublisher,
                            dmap_mdns_publisher,
                            G_TYPE_OBJECT);

static gpointer _publisher_object = NULL;

GQuark
dmap_mdns_publisher_error_quark (void)
{
	static GQuark quark = 0;

	if (!quark)
		quark = g_quark_from_static_string
			("dmap_mdns_publisher_error");

	return quark;
}

static void
_emit_published (char *name, DmapMdnsPublisher * publisher)
{
	g_signal_emit (publisher, _signals[PUBLISHED], 0, name);
}

static void
_entry_group_cb (G_GNUC_UNUSED AvahiEntryGroup * group,
                 AvahiEntryGroupState state, DmapMdnsPublisher * publisher)
{
	if (state == AVAHI_ENTRY_GROUP_ESTABLISHED) {
		g_slist_foreach (publisher->priv->service,
				 (GFunc) _emit_published, publisher);
	} else if (state == AVAHI_ENTRY_GROUP_COLLISION) {
		g_warning ("MDNS name collision");

		/* FIXME: how to know which name collided?
		 * g_signal_emit (publisher, _signals [NAME_COLLISION], 0, publisher->priv->name);
		 */
		g_signal_emit (publisher, _signals[NAME_COLLISION], 0,
			       "unknown");
	}
}

static gboolean
_create_service (struct DmapMdnsPublisherService *service,
                 DmapMdnsPublisher * publisher, GError ** error)
{
	int ret;
	gboolean ok = FALSE;
	const char *password_record;
	AvahiStringList *txt_records;

	if (service->password_required) {
		password_record = "Password=true";
	} else {
		password_record = "Password=false";
	}

	txt_records = avahi_string_list_new (password_record, NULL);

	if (service->txt_records) {
		//g_debug("Number of txt records: %d", service->txt_records);
		gchar **txt_record = service->txt_records;

		while (*txt_record) {
			txt_records =
				avahi_string_list_add (txt_records,
						       *txt_record);
			txt_record++;
		}
	}

	ret = avahi_entry_group_add_service_strlst (publisher->
						    priv->entry_group,
						    AVAHI_IF_UNSPEC,
						    AVAHI_PROTO_UNSPEC,
	                                            0,
						    service->name,
						    service->type_of_service,
						    NULL, NULL, service->port,
						    txt_records);

	avahi_string_list_free (txt_records);

	if (ret < 0) {
		g_set_error (error,
			     DMAP_MDNS_PUBLISHER_ERROR,
			     DMAP_MDNS_PUBLISHER_ERROR_FAILED,
			     "%s: %s",
			     _("Could not add service"),
			     avahi_strerror (ret));
		goto done;
	}

	ok = TRUE;

done:
	return ok;
}

static gboolean
_create_services (DmapMdnsPublisher * publisher, GError ** error)
{
	gboolean ok = FALSE;
	static int suffix = 0;
	gchar *name;
	GSList *ptr1, *ptr2;
	struct DmapMdnsPublisherService *service1, *service2;
	int ret;

	if (publisher->priv->entry_group == NULL) {
		publisher->priv->entry_group =
			avahi_entry_group_new (publisher->priv->client,
					       (AvahiEntryGroupCallback)
					       _entry_group_cb, publisher);

		if (publisher->priv->entry_group == NULL) {
			g_debug ("Could not create AvahiEntryGroup for publishing");
			g_set_error (error,
				     DMAP_MDNS_PUBLISHER_ERROR,
				     DMAP_MDNS_PUBLISHER_ERROR_FAILED,
				     "%s",
				     _
				     ("Could not create AvahiEntryGroup for publishing"));
			goto done;
		}

		dmap_mdns_avahi_set_entry_group (publisher->
						 priv->entry_group);
	} else {
		avahi_entry_group_reset (publisher->priv->entry_group);
	}

	for (ptr1 = publisher->priv->service; ptr1; ptr1 = g_slist_next (ptr1)) {
		service1 = ptr1->data;
		name = service1->name;
		for (ptr2 = publisher->priv->service; ptr2; ptr2 = g_slist_next (ptr2)) {
			if (ptr1 == ptr2) {
				continue;
			}
			service2 = ptr2->data;
			if (!strcmp(service1->name, service2->name)
			 && !strcmp(service1->type_of_service, service2->type_of_service)) {
				name = g_strdup_printf("%s-%d", service1->name, suffix++);
			}
		}
		if (strcmp(name, service1->name)) {
			g_free(service1->name);
			service1->name = name;
			g_signal_emit (publisher, _signals[NAME_COLLISION], 0, name);
		}
		if (!_create_service (service1, publisher, error)) {
			goto done;
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
		goto done;
	}

	ok = TRUE;

done:
	return ok;
}

static gboolean
_refresh_services (DmapMdnsPublisher * publisher, GError ** error)
{
	return _create_services (publisher, error);
}

static struct DmapMdnsPublisherService *
_find_service_by_port (GSList * list, guint port)
{
	GSList *ptr;

	for (ptr = list; ptr; ptr = g_slist_next (ptr)) {
		if (port ==
		    ((struct DmapMdnsPublisherService *) ptr->data)->port) {
			break;
		}
	}

	return ptr ? ptr->data : NULL;
}

gboolean
dmap_mdns_publisher_rename_at_port (DmapMdnsPublisher * publisher,
				    guint port,
				    const char *name, GError ** error)
{
	struct DmapMdnsPublisherService *ptr;

	g_return_val_if_fail (publisher != NULL, FALSE);

	ptr = _find_service_by_port (publisher->priv->service, port);

	if (ptr == NULL) {
		g_set_error (error,
			     DMAP_MDNS_PUBLISHER_ERROR,
			     DMAP_MDNS_PUBLISHER_ERROR_FAILED,
			     "%s", _("No service at port"));
		return FALSE;
	}

	g_free (ptr->name);
	ptr->name = g_strdup (name);

	if (publisher->priv->entry_group) {
		_refresh_services (publisher, error);
	}

	return TRUE;
}

gboolean
dmap_mdns_publisher_publish (DmapMdnsPublisher * publisher,
			     const char *name,
			     guint port,
			     const char *type_of_service,
			     gboolean password_required,
			     gchar ** txt_records, GError ** error)
{
	struct DmapMdnsPublisherService *service;

	if (publisher->priv->client == NULL) {
		g_set_error (error,
			     DMAP_MDNS_PUBLISHER_ERROR,
			     DMAP_MDNS_PUBLISHER_ERROR_NOT_RUNNING,
			     "%s",
			     _("The avahi MDNS service is not running"));
		return FALSE;
	}

	service = g_new0 (struct DmapMdnsPublisherService, 1);

	service->name = g_strdup (name);
	service->port = port;
	service->type_of_service = g_strdup (type_of_service);
	service->password_required = password_required;
	service->txt_records = g_strdupv (txt_records);

	publisher->priv->service =
		g_slist_append (publisher->priv->service, service);

	return _create_services (publisher, error);
}

static void
_free_service (struct DmapMdnsPublisherService *service, G_GNUC_UNUSED gpointer user_data)
{
	g_free (service->name);
	g_free (service->type_of_service);
	g_strfreev (service->txt_records);
	g_free (service);
}

gboolean
dmap_mdns_publisher_withdraw (DmapMdnsPublisher * publisher,
			      guint port, GError ** error)
{
	struct DmapMdnsPublisherService *ptr;

	if (publisher->priv->client == NULL) {
		g_set_error (error,
			     DMAP_MDNS_PUBLISHER_ERROR,
			     DMAP_MDNS_PUBLISHER_ERROR_NOT_RUNNING,
			     "%s",
			     _("The avahi MDNS service is not running"));
		return FALSE;
	}

	if (publisher->priv->entry_group == NULL
	    || !(ptr =
		 _find_service_by_port (publisher->priv->service, port))) {
		g_set_error (error, DMAP_MDNS_PUBLISHER_ERROR,
			     DMAP_MDNS_PUBLISHER_ERROR_FAILED, "%s",
			     _("The MDNS service is not published"));
		return FALSE;
	}

	publisher->priv->service =
		g_slist_remove (publisher->priv->service, ptr);
	_free_service (ptr, NULL);

	if (publisher->priv->service == NULL) {
		avahi_entry_group_reset (publisher->priv->entry_group);
		avahi_entry_group_free (publisher->priv->entry_group);
		publisher->priv->entry_group = NULL;
	} else {
		_create_services (publisher, error);
		if (error != NULL)
			return FALSE;
	}

	return TRUE;
}

static GObject *
_constructor (GType type,
              guint n_construct_params,
              GObjectConstructParam * construct_params)
{
	/* This class is a singleton. */
	static GObject *self = NULL;

	if (self == NULL) {
		self = G_OBJECT_CLASS
			(dmap_mdns_publisher_parent_class)->constructor (type,
									 n_construct_params,
									 construct_params);
		g_object_add_weak_pointer (self, (gpointer) & self);
		goto done;
	}

	self = g_object_ref (self);

done:
	return self;
}

static void
dmap_mdns_publisher_class_init (DmapMdnsPublisherClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->constructor  = _constructor;
	object_class->finalize     = _finalize;

	_signals[PUBLISHED] =
		g_signal_new ("published",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (DmapMdnsPublisherClass,
					       published), NULL, NULL,
			      NULL, G_TYPE_NONE, 1,
			      G_TYPE_STRING);
	_signals[NAME_COLLISION] =
		g_signal_new ("name-collision",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (DmapMdnsPublisherClass,
					       name_collision), NULL, NULL,
			      NULL, G_TYPE_NONE, 1,
			      G_TYPE_STRING);
}

static void
dmap_mdns_publisher_init (DmapMdnsPublisher * publisher)
{
	publisher->priv = dmap_mdns_publisher_get_instance_private(publisher);
	publisher->priv->client = dmap_mdns_avahi_get_client ();
	publisher->priv->entry_group = NULL;
	publisher->priv->service = NULL;
}

static void
_finalize (GObject * object)
{
	DmapMdnsPublisher *publisher;

	g_assert(NULL != object);
	g_assert(DMAP_IS_MDNS_PUBLISHER (object));

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

	g_slist_foreach (publisher->priv->service, (GFunc) _free_service,
			 NULL);
	g_slist_free (publisher->priv->service);

	_publisher_object = NULL;

	G_OBJECT_CLASS (dmap_mdns_publisher_parent_class)->finalize (object);
}

DmapMdnsPublisher *
dmap_mdns_publisher_new (void)
{
	if (_publisher_object) {
		g_object_ref (_publisher_object);
	} else {
		_publisher_object =
			g_object_new (DMAP_TYPE_MDNS_PUBLISHER, NULL);
		g_object_add_weak_pointer (_publisher_object,
					   (gpointer *) &_publisher_object);
	}

	return DMAP_MDNS_PUBLISHER (_publisher_object);
}
