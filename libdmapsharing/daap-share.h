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

#ifndef __DAAP_SHARE_H
#define __DAAP_SHARE_H

#include <glib-object.h>

#include <libdmapsharing/dmap-share.h>
#include <libdmapsharing/dmap-db.h>
#include <libdmapsharing/dmap-container-db.h>

G_BEGIN_DECLS
/**
 * DAAP_TYPE_SHARE:
 *
 * The type for #DAAPShare.
 */
#define DAAP_TYPE_SHARE         (daap_share_get_type ())
/**
 * DAAP_SHARE:
 * @o: Object which is subject to casting.
 * 
 * Casts a #DAAPShare or derived pointer into a (DAAPShare*) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DAAP_SHARE(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				 DAAP_TYPE_SHARE, DAAPShare))
/**
 * DAAP_SHARE_CLASS:
 * @k: a valid #DAAPShareClass
 *
 * Casts a derived #DAAPShareClass structure into a #DAAPShareClass structure.
 */
#define DAAP_SHARE_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), \
				 DAAP_TYPE_SHARE, DAAPShareClass))
/**
 * IS_DAAP_SHARE:
 * @o: Instance to check for being a %DAAP_TYPE_SHARE.
 * 
 * Checks whether a valid #GTypeInstance pointer is of type %DAAP_TYPE_SHARE.
 */
#define IS_DAAP_SHARE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				 DAAP_TYPE_SHARE))
/**
 * IS_DAAP_SHARE_CLASS:
 * @k: a #DAAPShareClass
 * 
 * Checks whether @k "is a" valid #DAAPShareClass structure of type
 * %DAAP_SHARE or derived.
 */
#define IS_DAAP_SHARE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), \
				 DAAP_TYPE_SHARE))
/**
 * DAAP_SHARE_GET_CLASS:
 * @o: a #DAAPShare instance.
 * 
 * Get the class structure associated to a #DAAPShare instance.
 *
 * Returns: pointer to object class structure.
 */
#define DAAP_SHARE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), \
				 DAAP_TYPE_SHARE, DAAPShareClass))
typedef struct DAAPSharePrivate DAAPSharePrivate;

typedef struct
{
	DMAPShareClass dmap_share_class;
} DAAPShareClass;

typedef struct
{
	DMAPShare dmap_share_instance;
	DAAPSharePrivate *priv;
} DAAPShare;

GType daap_share_get_type (void);

/**
 * daap_share_new:
 * @name: The name that will be published by mDNS.
 * @password: A share password or NULL.
 * @db: A media database.
 * @container_db: A container (album) database.
 * @transcode_mimetype: A transcode mimetype or NULL.
 * 
 * Creates a new DAAP share and publishes it using mDNS.
 *
 * Returns: a pointer to a DAAPShare.
 */
DAAPShare *daap_share_new (const char *name, const char *password,
			   DMAPDb * db, DMAPContainerDb * container_db,
			   gchar * transcode_mimetype);

#endif /* __DAAP_SHARE_H */

G_END_DECLS
