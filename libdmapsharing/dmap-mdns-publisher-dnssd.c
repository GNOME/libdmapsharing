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
#include <string.h>
#include <glib.h>
#include <dns_sd.h>
#include <arpa/inet.h>

#include "dmap-mdns-publisher.h"

#define DMAP_MDNS_PUBLISHER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), DMAP_TYPE_MDNS_PUBLISHER, DMAPMdnsPublisherPrivate))

struct DMAPMdnsPublisherPrivate
{
	DNSServiceRef	 sdref;
        char            *name;
};

enum {
        PUBLISHED,
	NAME_COLLISION,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (DMAPMdnsPublisher, dmap_mdns_publisher, G_TYPE_OBJECT)

static gpointer publisher_object = NULL;

GQuark
dmap_mdns_publisher_error_quark (void)
{        static GQuark quark = 0;
        if (!quark)
                quark = g_quark_from_static_string ("dmap_mdns_publisher_error");

        return quark;
}

gboolean
dmap_mdns_publisher_rename_at_port (DMAPMdnsPublisher *publisher,
				    guint	       port,
                                    const char        *name,
                                    GError           **error)
{
	g_error ("Not implemented");

        return TRUE;
}

static gchar *
_build_txt_record(gboolean password_required, gchar **txt_records, uint16_t *txt_len)
{
	*txt_len = 0;

	gchar **_txt_records;
	for (_txt_records = txt_records; _txt_records && *_txt_records; _txt_records++) {
		*txt_len += strlen(*_txt_records) + 1; // + 1 for req. len.
		_txt_records++;
	}

	char password_size = 0;
	if(TRUE == password_required) {
		password_size = (char) strlen("Password=true") + 1;
	} else {
		password_size = (char) strlen("Password=false") + 1;
	}
	*txt_len += password_size;

	size_t i = 0;
	gchar *txt_record = g_malloc(*txt_len);

	for (; txt_records && *txt_records; txt_records++) {
		size_t len = strlen(*txt_records);

		g_assert(len <= ~(char)0);

		txt_record[i++] = (char)len;

		memcpy(txt_record + i, *txt_records, len);
		i += len;
	}

	txt_record[i++] = password_size - 1;
	if(TRUE == password_required) {
		strcpy(txt_record + i, "Password=true");
	} else {
		strcpy(txt_record + i, "Password=false");
	}

	return txt_record;
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
	gboolean fnval = TRUE;
	uint16_t txt_len = 0;
	char *txt_record = NULL;
	int dns_err;

	/* TODO: Unify txt_records and password_required to simplify build_txt_...? */
	txt_record = _build_txt_record(password_required, txt_records, &txt_len);

	g_warning ("%s %s %d", name, type_of_service, port);
	if ((dns_err = DNSServiceRegister (&publisher->priv->sdref,
		0,
		0,
		name,
		type_of_service,
		NULL,
		NULL,
		htons (port),
		txt_len,
		txt_record,
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
		fnval = FALSE;
		goto done;
        }

	g_signal_emit (publisher, signals[PUBLISHED], 0, publisher->priv->name);

done:
	g_free(txt_record);

        return fnval;
}

gboolean
dmap_mdns_publisher_withdraw (DMAPMdnsPublisher *publisher,
			      guint port,
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
        DMAPMdnsPublisher *publisher;

        g_return_if_fail (object != NULL);
        g_return_if_fail (IS_DMAP_MDNS_PUBLISHER (object));

        publisher = DMAP_MDNS_PUBLISHER (object);

        g_return_if_fail (publisher->priv != NULL);

        g_free (publisher->priv->name);

        G_OBJECT_CLASS (dmap_mdns_publisher_parent_class)->finalize (object);
}

static void
dmap_mdns_publisher_class_init (DMAPMdnsPublisherClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->finalize     = dmap_mdns_publisher_finalize;
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
