/* Header for abstraction of Multicast DNS for DMAP sharing
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

#ifndef _DMAP_MDNS_PUBLISHER_H
#define _DMAP_MDNS_PUBLISHER_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS
#define DMAP_TYPE_MDNS_PUBLISHER         (dmap_mdns_publisher_get_type ())
#define DMAP_MDNS_PUBLISHER(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), DMAP_TYPE_MDNS_PUBLISHER, DmapMdnsPublisher))
#define DMAP_MDNS_PUBLISHER_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), DMAP_TYPE_MDNS_PUBLISHER, DmapMdnsPublisherClass))
#define DMAP_IS_MDNS_PUBLISHER(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), DMAP_TYPE_MDNS_PUBLISHER))
#define DMAP_IS_MDNS_PUBLISHER_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), DMAP_TYPE_MDNS_PUBLISHER))
#define DMAP_MDNS_PUBLISHER_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), DMAP_TYPE_MDNS_PUBLISHER, DmapMdnsPublisherClass))
typedef struct DmapMdnsPublisherPrivate DmapMdnsPublisherPrivate;

typedef struct {
	GObject object;

	DmapMdnsPublisherPrivate *priv;
} DmapMdnsPublisher;

typedef struct {
	GObjectClass parent_class;

	void (*published) (DmapMdnsPublisher * publisher, const char *name);
	void (*name_collision) (DmapMdnsPublisher * publisher,
				const char *name);
} DmapMdnsPublisherClass;

typedef enum {
	DMAP_MDNS_PUBLISHER_ERROR_NOT_RUNNING,
	DMAP_MDNS_PUBLISHER_ERROR_FAILED,
} DmapMdnsPublisherError;

#define DMAP_MDNS_PUBLISHER_ERROR dmap_mdns_publisher_error_quark ()

GQuark dmap_mdns_publisher_error_quark (void);

GType dmap_mdns_publisher_get_type (void);

DmapMdnsPublisher *dmap_mdns_publisher_new (void);
gboolean dmap_mdns_publisher_publish (DmapMdnsPublisher * publisher,
				      const char *name,
				      guint port,
				      const char *type_of_service,
				      gboolean password_required,
				      gchar ** txt_records, GError ** error);
gboolean dmap_mdns_publisher_rename_at_port (DmapMdnsPublisher * publisher,
					     guint port,
					     const char *name,
					     GError ** error);
gboolean dmap_mdns_publisher_withdraw (DmapMdnsPublisher * publisher,
				       guint port, GError ** error);

G_END_DECLS
#endif /* _DMAP_MDNS_PUBLISHER_H */
