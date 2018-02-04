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
 * The type for #DmapShare.
 */
#define DMAP_TYPE_SHARE         (dmap_share_get_type ())
/**
 * DMAP_SHARE:
 * @o: Object which is subject to casting.
 *
 * Casts a #DmapShare or derived pointer into a (DmapShare*) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DMAP_SHARE(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				 DMAP_TYPE_SHARE, DmapShare))
/**
 * DMAP_SHARE_CLASS:
 * @k: a valid #DmapShareClass
 *
 * Casts a derived #DmapShareClass structure into a #DmapShareClass structure.
 */
#define DMAP_SHARE_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), \
				 DMAP_TYPE_SHARE, DmapShareClass))
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
 * @k: a #DmapShareClass
 *
 * Checks whether @k "is a" valid #DmapShareClass structure of type
 * %DMAP_SHARE or derived.
 */
#define IS_DMAP_SHARE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), \
				 DMAP_TYPE_SHARE))
/**
 * DMAP_SHARE_GET_CLASS:
 * @o: a #DmapShare instance.
 *
 * Get the class structure associated to a #DmapShare instance.
 *
 * Returns: pointer to object class structure.
 */
#define DMAP_SHARE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), \
				 DMAP_TYPE_SHARE, DmapShareClass))
#define DMAP_STATUS_OK 200
typedef struct DmapSharePrivate DmapSharePrivate;

typedef struct
{
	GObject parent;
	DmapSharePrivate *priv;
} DmapShare;

typedef struct DmapMetaDataMap DmapMetaDataMap;

typedef enum
{
	DMAP_SHARE_AUTH_METHOD_NONE = 0,
	DMAP_SHARE_AUTH_METHOD_NAME_AND_PASSWORD = 1,
	DMAP_SHARE_AUTH_METHOD_PASSWORD = 2
} DmapShareAuthMethod;

typedef struct
{
	GObjectClass parent;

	/* Pure virtual methods: */
	  guint (*get_desired_port) (DmapShare * share);
	const char *(*get_type_of_service) (DmapShare * share);
	void (*message_add_standard_headers) (DmapShare * share,
					      SoupMessage * msg);
	struct DmapMetaDataMap *(*get_meta_data_map) (DmapShare * share);
	void (*add_entry_to_mlcl) (guint id, DmapRecord * record, gpointer mb);
	void (*databases_browse_xxx) (DmapShare * share,
				      SoupServer * server,
				      SoupMessage * msg,
				      const char *path,
				      GHashTable * query,
				      SoupClientContext * context);
	void (*databases_items_xxx) (DmapShare * share,
				     SoupServer * server,
				     SoupMessage * msg,
				     const char *path,
				     GHashTable * query,
				     SoupClientContext * context);

	/* Pure virtual methods: libsoup callbacks */
	void (*server_info) (DmapShare * share, SoupServer * server,
			     SoupMessage * message, const char *path,
			     GHashTable * query, SoupClientContext * ctx);

	void (*content_codes) (DmapShare * share, SoupServer * server,
			       SoupMessage * message, const char *path,
			       GHashTable * query, SoupClientContext * ctx);

	void (*login) (DmapShare * share, SoupServer * server,
		       SoupMessage * message, const char *path,
		       GHashTable * query, SoupClientContext * ctx);

	void (*logout) (DmapShare * share, SoupServer * server,
			SoupMessage * message, const char *path,
			GHashTable * query, SoupClientContext * ctx);

	void (*update) (DmapShare * share, SoupServer * server,
			SoupMessage * message, const char *path,
			GHashTable * query, SoupClientContext * ctx);

	void (*ctrl_int) (DmapShare * share, SoupServer * server,
			  SoupMessage * message, const char *path,
			  GHashTable * query, SoupClientContext * ctx);

	/* Virtual methods: MDNS callbacks */
	void (*published) (DmapShare * share,
			   DmapMdnsPublisher * publisher, const char *name);

	void (*name_collision) (DmapShare * share,
				DmapMdnsPublisher * publisher,
				const char *name);

	/* Virtual methods: */
	void (*databases) (DmapShare * share,
			   SoupServer * server,
			   SoupMessage * message,
			   const char *path,
			   GHashTable * query, SoupClientContext * context);
} DmapShareClass;

struct DmapMetaDataMap
{
	gchar *tag;
	guint md;
};

typedef guint64 DmapBits;

/* FIXME: this is passed as user_data to various functions; 
 * need to rename. Also, get rid of initializations elsewhere: { NULL, 0, NULL };
 * instead define a function to do this.
 */
struct DmapMlclBits
{
	GNode *mlcl;
	DmapBits bits;
	DmapShare *share;
};

GType dmap_share_get_type (void);

/* Non-virtual methods */
guint _dmap_share_get_auth_method (DmapShare * share);

guint _dmap_share_get_revision_number (DmapShare * share);

gboolean _dmap_share_get_revision_number_from_query (GHashTable * query,
						     guint * number);

gboolean _dmap_share_session_id_validate (DmapShare * share,
					  SoupClientContext * context,
					  SoupMessage * msg,
					  GHashTable * query, guint32 * id);

guint32 _dmap_share_session_id_create (DmapShare * share,
				       SoupClientContext * ctx);

void _dmap_share_session_id_remove (DmapShare * share,
				    SoupClientContext * ctx, guint32 id);

gboolean _dmap_share_client_requested (DmapBits bits, gint field);

gboolean _dmap_share_uri_is_local (const char *text_uri);

gboolean _dmap_share_soup_auth_filter (SoupAuthDomain * auth_domain,
				       SoupMessage * msg, gpointer user_data);

gboolean _dmap_share_server_start (DmapShare * share);

gboolean _dmap_share_publish_start (DmapShare * share);

void _dmap_share_message_set_from_dmap_structure (DmapShare * share,
						  SoupMessage * message,
						  GNode * structure);

DmapBits _dmap_share_parse_meta (GHashTable * query,
				struct DmapMetaDataMap *mdm);

DmapBits _dmap_share_parse_meta_str (const char *attrs,
				    struct DmapMetaDataMap *mdm);

void _dmap_share_add_playlist_to_mlcl (gpointer id,
				       DmapContainerRecord * record,
				       gpointer mb);

GSList *_dmap_share_build_filter (gchar * filterstr);

/**
 * dmap_share_free_filter:
 * @filter: (element-type GSList): The filter list to free.
 *
 * Free the given filter list.
 */
void dmap_share_free_filter (GSList * filter);

/* Virtual methods (libsoup callbacks with default implementation): */
void _dmap_share_content_codes (DmapShare * share,
				SoupServer * server,
				SoupMessage * message,
				const char *path,
				GHashTable * query,
				SoupClientContext * context);

void _dmap_share_login (DmapShare * share,
			SoupServer * server,
			SoupMessage * message,
			const char *path,
			GHashTable * query, SoupClientContext * context);

void _dmap_share_logout (DmapShare * share,
			 SoupServer * server,
			 SoupMessage * message,
			 const char *path,
			 GHashTable * query, SoupClientContext * context);

void _dmap_share_update (DmapShare * share,
			 SoupServer * server,
			 SoupMessage * message,
			 const char *path,
			 GHashTable * query, SoupClientContext * context);

void
_dmap_share_databases (DmapShare * share,
		       SoupServer * server,
		       SoupMessage * message,
		       const char *path,
		       GHashTable * query, SoupClientContext * context);

void _dmap_share_ctrl_int (DmapShare * share,
			   SoupServer * server,
			   SoupMessage * message,
			   const char *path,
			   GHashTable * query, SoupClientContext * context);

/* Virtual methods: MDNS callbacks */
void _dmap_share_published (DmapShare * share,
			    DmapMdnsPublisher * publisher, const char *name);

void _dmap_share_name_collision (DmapShare * share,
				 DmapMdnsPublisher * publisher,
				 const char *name);

#endif /* __DMAP_SHARE_H */

G_END_DECLS
