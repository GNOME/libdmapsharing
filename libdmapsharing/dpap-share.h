/*
 * Header for DPAP (e.g., iPhoto Picture) sharing
 *
 * Copyright (C) 2005 Charles Schmidt <cschmidt2@emich.edu>
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

#ifndef __DPAP_SHARE_H
#define __DPAP_SHARE_H

#include <glib-object.h>

#include <libdmapsharing/dmap-share.h>

G_BEGIN_DECLS
/**
 * DPAP_TYPE_SHARE:
 *
 * The type for #DPAPShare.
 */
#define DPAP_TYPE_SHARE         (dpap_share_get_type ())
/**
 * DPAP_SHARE:
 * @o: Object which is subject to casting.
 *
 * Casts a #DPAPShare or derived pointer into a (DPAPShare*) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DPAP_SHARE(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				 DPAP_TYPE_SHARE, DPAPShare))
/**
 * DPAP_SHARE_CLASS:
 * @k: a valid #DPAPShareClass
 *
 * Casts a derived #DPAPShareClass structure into a #DPAPShareClass structure.
 */
#define DPAP_SHARE_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), \
				 DPAP_TYPE_SHARE, DPAPShareClass))
/**
 * IS_DPAP_SHARE:
 * @o: Instance to check for being a %DPAP_TYPE_SHARE.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %DPAP_TYPE_SHARE.
 */
#define IS_DPAP_SHARE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				 DPAP_TYPE_SHARE))
/**
 * IS_DPAP_SHARE_CLASS:
 * @k: a #DPAPShareClass
 *
 * Checks whether @k "is a" valid #DPAPShareClass structure of type
 * %DPAP_SHARE or derived.
 */
#define IS_DPAP_SHARE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), DPAP_TYPE_SHARE))
/**
 * DPAP_SHARE_GET_CLASS:
 * @o: a #DPAPShare instance.
 *
 * Get the class structure associated to a #DPAPShare instance.
 *
 * Returns: pointer to object class structure.
 */
#define DPAP_SHARE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), \
				 DPAP_TYPE_SHARE, DPAPShareClass))
typedef struct DPAPSharePrivate DPAPSharePrivate;

typedef struct
{
	DMAPShareClass dmap_share_class;
} DPAPShareClass;

typedef struct
{
	DMAPShare dmap_share_instance;
	DPAPSharePrivate *priv;
} DPAPShare;

GType dpap_share_get_type (void);

/**
 * dpap_share_new:
 * @name: The name that will be published by mDNS.
 * @password: A share password or NULL.
 * @db: A media database.
 * @container_db: A container (album) database.
 * @transcode_mimetype: A transcode mimetype or NULL.
 *
 * Creates a new DPAP share and publishes it using mDNS.
 *
 * Returns: a pointer to a DPAPShare.
 */
DPAPShare *dpap_share_new (const char *name, const char *password,
			   gpointer db, gpointer container_db,
			   gchar * transcode_mimetype);

#endif /* __DPAP_SHARE_H */

G_END_DECLS
