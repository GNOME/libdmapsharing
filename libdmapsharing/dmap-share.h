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

#ifndef __DMAP_SHARE_H
#define __DMAP_SHARE_H

#include <glib-object.h>

#include <libsoup/soup.h>
#include <libsoup/soup-address.h>
#include <libsoup/soup-message.h>
#include <libsoup/soup-uri.h>
#include <libsoup/soup-server.h>

#include <libdmapsharing/dmap-record.h>
#include <libdmapsharing/dmap-mdns-publisher.h>
#include <libdmapsharing/dmap-container-record.h>

G_BEGIN_DECLS
/**
 * DMAP_TYPE_SHARE:
 *
 * The type for #DMAPShare.
 */
#define DMAP_TYPE_SHARE         (dmap_share_get_type ())
/**
 * DMAP_SHARE:
 * @o: Object which is subject to casting.
 *
 * Casts a #DMAPShare or derived pointer into a (DMAPShare*) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DMAP_SHARE(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				 DMAP_TYPE_SHARE, DMAPShare))
/**
 * DMAP_SHARE_CLASS:
 * @k: a valid #DMAPShareClass
 *
 * Casts a derived #DMAPShareClass structure into a #DMAPShareClass structure.
 */
#define DMAP_SHARE_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), \
				 DMAP_TYPE_SHARE, DMAPShareClass))
/**
 * IS_DMAP_SHARE:
 * @o: Instance to check for being a %DMAP_TYPE_SHARE.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %DMAP_TYPE_SHARE.
 */
#define IS_DMAP_SHARE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				 DMAP_TYPE_SHARE))
/**
 * IS_DMAP_SHARE_CLASS:
 * @k: a #DMAPShareClass
 *
 * Checks whether @k "is a" valid #DMAPShareClass structure of type
 * %DMAP_SHARE or derived.
 */
#define IS_DMAP_SHARE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), \
				 DMAP_TYPE_SHARE))
/**
 * DMAP_SHARE_GET_CLASS:
 * @o: a #DMAPShare instance.
 *
 * Get the class structure associated to a #DMAPShare instance.
 *
 * Returns: pointer to object class structure.
 */
#define DMAP_SHARE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), \
				 DMAP_TYPE_SHARE, DMAPShareClass))
#define DMAP_STATUS_OK 200
typedef struct DMAPSharePrivate DMAPSharePrivate;

typedef struct
{
	GObject parent;
	DMAPSharePrivate *priv;
} DMAPShare;

typedef struct DMAPMetaDataMap DMAPMetaDataMap;

typedef struct
{
	GObjectClass parent;

	/* Pure virtual methods: */
	  guint (*get_desired_port) (DMAPShare * share);
	const char *(*get_type_of_service) (DMAPShare * share);
	void (*message_add_standard_headers) (DMAPShare * share,
					      SoupMessage * msg);
	struct DMAPMetaDataMap *(*get_meta_data_map) (DMAPShare * share);
	void (*add_entry_to_mlcl) (gpointer id,
				   DMAPRecord * record, gpointer mb);
	void (*databases_browse_xxx) (DMAPShare * share,
				      SoupServer * server,
				      SoupMessage * msg,
				      const char *path,
				      GHashTable * query,
				      SoupClientContext * context);
	void (*databases_items_xxx) (DMAPShare * share,
				     SoupServer * server,
				     SoupMessage * msg,
				     const char *path,
				     GHashTable * query,
				     SoupClientContext * context);

	/* Pure virtual methods: libsoup callbacks */
	void (*server_info) (DMAPShare * share, SoupServer * server,
			     SoupMessage * message, const char *path,
			     GHashTable * query, SoupClientContext * ctx);

	void (*content_codes) (DMAPShare * share, SoupServer * server,
			       SoupMessage * message, const char *path,
			       GHashTable * query, SoupClientContext * ctx);

	void (*login) (DMAPShare * share, SoupServer * server,
		       SoupMessage * message, const char *path,
		       GHashTable * query, SoupClientContext * ctx);

	void (*logout) (DMAPShare * share, SoupServer * server,
			SoupMessage * message, const char *path,
			GHashTable * query, SoupClientContext * ctx);

	void (*update) (DMAPShare * share, SoupServer * server,
			SoupMessage * message, const char *path,
			GHashTable * query, SoupClientContext * ctx);

	void (*ctrl_int) (DMAPShare * share, SoupServer * server,
			  SoupMessage * message, const char *path,
			  GHashTable * query, SoupClientContext * ctx);

	/* Virtual methods: MDNS callbacks */
	void (*published) (DMAPShare * share,
			   DMAPMdnsPublisher * publisher, const char *name);

	void (*name_collision) (DMAPShare * share,
				DMAPMdnsPublisher * publisher,
				const char *name);

	/* Virtual methods: */
	void (*databases) (DMAPShare * share,
			   SoupServer * server,
			   SoupMessage * message,
			   const char *path,
			   GHashTable * query, SoupClientContext * context);
} DMAPShareClass;

struct DMAPMetaDataMap
{
	gchar *tag;
	guint md;
};

/* FIXME: this is passed as user_data to various functions; 
 * need to rename. Also, get rid of initializations elsewhere: { NULL, 0, NULL };
 * instead define a function to do this.
 */
struct MLCL_Bits
{
	GNode *mlcl;
	bitwise bits;
	DMAPShare *share;
};

GType dmap_share_get_type (void);

/* Non-virtual methods */
guint _dmap_share_get_auth_method (DMAPShare * share);

guint _dmap_share_get_revision_number (DMAPShare * share);

gboolean _dmap_share_get_revision_number_from_query (GHashTable * query,
						     guint * number);

gboolean _dmap_share_session_id_validate (DMAPShare * share,
					  SoupClientContext * context,
					  SoupMessage * msg,
					  GHashTable * query, guint32 * id);

guint32 _dmap_share_session_id_create (DMAPShare * share,
				       SoupClientContext * ctx);

void _dmap_share_session_id_remove (DMAPShare * share,
				    SoupClientContext * ctx, guint32 id);

gboolean _dmap_share_client_requested (bitwise bits, gint field);

gboolean _dmap_share_uri_is_local (const char *text_uri);

gboolean _dmap_share_soup_auth_filter (SoupAuthDomain * auth_domain,
				       SoupMessage * msg, gpointer user_data);

gboolean _dmap_share_server_start (DMAPShare * share);

gboolean _dmap_share_publish_start (DMAPShare * share);

void _dmap_share_message_set_from_dmap_structure (DMAPShare * share,
						  SoupMessage * message,
						  GNode * structure);

bitwise _dmap_share_parse_meta (GHashTable * query,
				struct DMAPMetaDataMap *mdm);

bitwise _dmap_share_parse_meta_str (const char *attrs,
				    struct DMAPMetaDataMap *mdm);

void _dmap_share_add_playlist_to_mlcl (gpointer id,
				       DMAPContainerRecord * record,
				       gpointer mb);

GSList *_dmap_share_build_filter (gchar * filterstr);

void dmap_share_free_filter (GSList * filter);

/* Virtual methods (libsoup callbacks with default implementation): */
void _dmap_share_content_codes (DMAPShare * share,
				SoupServer * server,
				SoupMessage * message,
				const char *path,
				GHashTable * query,
				SoupClientContext * context);

void _dmap_share_login (DMAPShare * share,
			SoupServer * server,
			SoupMessage * message,
			const char *path,
			GHashTable * query, SoupClientContext * context);

void _dmap_share_logout (DMAPShare * share,
			 SoupServer * server,
			 SoupMessage * message,
			 const char *path,
			 GHashTable * query, SoupClientContext * context);

void _dmap_share_update (DMAPShare * share,
			 SoupServer * server,
			 SoupMessage * message,
			 const char *path,
			 GHashTable * query, SoupClientContext * context);

void
_dmap_share_databases (DMAPShare * share,
		       SoupServer * server,
		       SoupMessage * message,
		       const char *path,
		       GHashTable * query, SoupClientContext * context);

void _dmap_share_ctrl_int (DMAPShare * share,
			   SoupServer * server,
			   SoupMessage * message,
			   const char *path,
			   GHashTable * query, SoupClientContext * context);

/* Virtual methods: MDNS callbacks */
void _dmap_share_published (DMAPShare * share,
			    DMAPMdnsPublisher * publisher, const char *name);

void _dmap_share_name_collision (DMAPShare * share,
				 DMAPMdnsPublisher * publisher,
				 const char *name);

#endif /* __DMAP_SHARE_H */

G_END_DECLS
