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

#define TYPE_DAAP_SHARE         (daap_share_get_type ())
#define DAAP_SHARE(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				 TYPE_DAAP_SHARE, DAAPShare))
#define DAAP_SHARE_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), \
				 TYPE_DAAP_SHARE, DAAPShareClass))
#define IS_DAAP_SHARE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				 TYPE_DAAP_SHARE))
#define IS_DAAP_SHARE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), \
				 TYPE_DAAP_SHARE))
#define DAAP_SHARE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), \
				 TYPE_DAAP_SHARE, DAAPShareClass))

typedef struct DAAPSharePrivate DAAPSharePrivate;

typedef struct {
	DMAPShareClass dmap_share_class;
} DAAPShareClass;

typedef struct {
	DMAPShare dmap_share_instance;
	DAAPSharePrivate *priv;
} DAAPShare;

GType      daap_share_get_type (void);

DAAPShare *daap_share_new      (const char *name, const char *password,
			        DMAPDb *db, DMAPContainerDb *container_db,
				gchar *transcode_mimetype);

#endif /* __DAAP_SHARE_H */

G_END_DECLS
