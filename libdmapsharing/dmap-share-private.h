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
#include <libsoup/soup-address.h>
#include <libsoup/soup-message.h>
#include <libsoup/soup-uri.h>
#include <libsoup/soup-server.h>

#include <libdmapsharing/dmap-share.h>
#include <libdmapsharing/dmap-mdns-publisher.h>
#include <libdmapsharing/dmap-container-record.h>

G_BEGIN_DECLS

/* Non-virtual methods */
guint dmap_share_get_auth_method (DmapShare * share);

guint dmap_share_get_revision_number (DmapShare * share);

gboolean dmap_share_get_revision_number_from_query (GHashTable * query,
						     guint * number);

gboolean dmap_share_session_id_validate (DmapShare * share,
                                         SoupClientContext * context,
                                         GHashTable * query,
                                         guint32 * id);

guint32 dmap_share_session_id_create (DmapShare * share,
				       SoupClientContext * ctx);

gboolean dmap_share_client_requested (DmapBits bits, gint field);

gboolean dmap_share_uri_is_local (const char *text_uri);

gboolean dmap_share_soup_auth_filter (SoupAuthDomain * auth_domain,
				       SoupMessage * msg, gpointer user_data);

void dmap_share_message_set_from_dmap_structure (DmapShare * share,
						  SoupMessage * message,
						  GNode * structure);

DmapBits dmap_share_parse_meta (GHashTable * query,
				struct DmapMetaDataMap *mdm);

DmapBits dmap_share_parse_meta_str (const char *attrs,
				    struct DmapMetaDataMap *mdm);

void dmap_share_add_playlist_to_mlcl (guint id,
                                      DmapContainerRecord * record,
                                      gpointer mb);

GSList *dmap_share_build_filter (gchar * filterstr);

/* Virtual methods (libsoup callbacks with default implementation): */
void dmap_share_content_codes (DmapShare * share,
				SoupMessage * message,
				const char *path,
				SoupClientContext * context);

void dmap_share_login (DmapShare * share,
                       SoupMessage * message,
                       const char *path,
                       GHashTable * query,
                       SoupClientContext * context);

void dmap_share_logout (DmapShare * share,
                        SoupMessage * message,
                        const char *path,
                        GHashTable * query,
                        SoupClientContext * context);

void dmap_share_update (DmapShare * share,
			 SoupServer * server,
			 SoupMessage * message,
			 const char *path,
			 GHashTable * query);

void dmap_share_databases (DmapShare * share,
		       SoupServer * server,
		       SoupMessage * message,
		       const char *path,
		       GHashTable * query, SoupClientContext * context);

void dmap_share_ctrl_int (DmapShare * share,
			   SoupServer * server,
			   SoupMessage * message,
			   const char *path,
			   GHashTable * query, SoupClientContext * context);

/* Virtual methods: MDNS callbacks */
void dmap_share_name_collision (DmapShare * share,
				 DmapMdnsPublisher * publisher,
				 const char *name);

#endif /* _DMAP_SHARE_PRIVATE_H */

G_END_DECLS
