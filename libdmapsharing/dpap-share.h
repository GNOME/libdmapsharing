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

#define TYPE_DPAP_SHARE         (dpap_share_get_type ())
#define DPAP_SHARE(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				 TYPE_DPAP_SHARE, DPAPShare))
#define DPAP_SHARE_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), \
				 TYPE_DPAP_SHARE, DPAPShareClass))
#define IS_DPAP_SHARE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				 TYPE_DPAP_SHARE))
#define IS_DPAP_SHARE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_DPAP_SHARE))
#define DPAP_SHARE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), \
				 TYPE_DPAP_SHARE, DPAPShareClass))

typedef struct DPAPSharePrivate DPAPSharePrivate;

typedef struct {
	DMAPShareClass dmap_share_class;
} DPAPShareClass;

typedef struct {
	DMAPShare dmap_share_instance;
	DPAPSharePrivate *priv;
} DPAPShare;

GType      dpap_share_get_type (void);

DPAPShare *dpap_share_new      (const char *name, const char *password,
			        gpointer db, gpointer container_db,
				gchar *transcode_mimetype);

#endif /* __DPAP_SHARE_H */

G_END_DECLS
