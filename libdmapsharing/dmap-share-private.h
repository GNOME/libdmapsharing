/* Header for DMAP (e.g., iTunes Music or iPhoto Picture) sharing
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

#ifndef _DMAP_SHARE_PRIVATE_H
#define _DMAP_SHARE_PRIVATE_H

#include <glib-object.h>

#include <libsoup/soup.h>

#include <libdmapsharing/dmap-share.h>
#include <libdmapsharing/dmap-mdns-publisher.h>
#include <libdmapsharing/dmap-container-record.h>

G_BEGIN_DECLS

/* Non-virtual methods */
guint dmap_share_get_auth_method (DmapShare * share);

gboolean dmap_share_session_id_validate (DmapShare * share,
                                         SoupServerMessage * message,
                                         GHashTable * query,
                                         guint32 * id);

gboolean dmap_share_client_requested (DmapBits bits, gint field);

void dmap_share_message_set_from_dmap_structure (DmapShare * share,
						  SoupServerMessage * message,
						  GNode * structure);

GSList *dmap_share_build_filter (gchar * filterstr);

void dmap_share_login (DmapShare * share,
                       SoupServerMessage * message,
                       const char *path,
                       GHashTable * query);

/* Virtual methods: MDNS callbacks */
void dmap_share_name_collision (DmapShare * share,
				 DmapMdnsPublisher * publisher,
				 const char *name);

#endif /* _DMAP_SHARE_PRIVATE_H */

G_END_DECLS
