/*
 * Header for DAAP (e.g., iTunes Music) sharing
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

#ifndef _DMAP_AV_SHARE_H
#define _DMAP_AV_SHARE_H

#include <glib-object.h>

#include <libdmapsharing/dmap-share.h>
#include <libdmapsharing/dmap-db.h>
#include <libdmapsharing/dmap-container-db.h>

G_BEGIN_DECLS
/**
 * SECTION: dmap-av-share
 * @short_description: A DAAP share.
 *
 * #DmapAvShare objects encapsulate a DAAP share.
 */

/**
 * DMAP_TYPE_AV_SHARE:
 *
 * The type for #DmapAvShare.
 */
#define DMAP_TYPE_AV_SHARE         (dmap_av_share_get_type ())
/**
 * DMAP_AV_SHARE:
 * @o: Object which is subject to casting.
 * 
 * Casts a #DmapAvShare or derived pointer into a (DmapAvShare*) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DMAP_AV_SHARE(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				 DMAP_TYPE_AV_SHARE, DmapAvShare))
/**
 * DMAP_AV_SHARE_CLASS:
 * @k: a valid #DmapAvShareClass
 *
 * Casts a derived #DmapAvShareClass structure into a #DmapAvShareClass structure.
 */
#define DMAP_AV_SHARE_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), \
				 DMAP_TYPE_AV_SHARE, DmapAvShareClass))
/**
 * DMAP_IS_AV_SHARE:
 * @o: Instance to check for being a %DMAP_TYPE_AV_SHARE.
 * 
 * Checks whether a valid #GTypeInstance pointer is of type %DMAP_TYPE_AV_SHARE.
 */
#define DMAP_IS_AV_SHARE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				 DMAP_TYPE_AV_SHARE))
/**
 * DMAP_IS_AV_SHARE_CLASS:
 * @k: a #DmapAvShareClass
 * 
 * Checks whether @k "is a" valid #DmapAvShareClass structure of type
 * %DMAP_AV_SHARE or derived.
 */
#define DMAP_IS_AV_SHARE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), \
				 DMAP_TYPE_AV_SHARE))
/**
 * DMAP_AV_SHARE_GET_CLASS:
 * @o: a #DmapAvShare instance.
 * 
 * Get the class structure associated to a #DmapAvShare instance.
 *
 * Returns: pointer to object class structure.
 */
#define DMAP_AV_SHARE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), \
				 DMAP_TYPE_AV_SHARE, DmapAvShareClass))
typedef struct DmapAvSharePrivate DmapAvSharePrivate;

typedef struct {
	DmapShareClass dmap_share_class;
} DmapAvShareClass;

typedef struct {
	DmapShare dmap_share_instance;
	DmapAvSharePrivate *priv;
} DmapAvShare;

GType dmap_av_share_get_type (void);

/**
 * dmap_av_share_new:
 * @name: The name that will be published by mDNS.
 * @password: (nullable): A share password or NULL.
 * @db: A media database.
 * @container_db: A container (album) database.
 * @transcode_mimetype: (nullable): A transcode mimetype or NULL.
 * 
 * Creates a new DAAP share and publishes it using mDNS.
 *
 * Returns: a pointer to a DmapAvShare.
 */
DmapAvShare *dmap_av_share_new (const char *name, const char *password,
			   DmapDb * db, DmapContainerDb * container_db,
			   gchar * transcode_mimetype);

#endif /* _DMAP_AV_SHARE_H */

G_END_DECLS
