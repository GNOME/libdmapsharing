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

#ifndef _DMAP_SHARE_H
#define _DMAP_SHARE_H

#include <glib-object.h>

#include <libsoup/soup.h>

#include <libdmapsharing/dmap-record.h>
#include <libdmapsharing/dmap-mdns-publisher.h>
#include <libdmapsharing/dmap-container-record.h>

G_BEGIN_DECLS
/**
 * SECTION: dmap-share
 * @short_description: An abstract parent to the various share classes.
 *
 * #DmapShare provides an abstract parent to the #DmapAvShare, #DmapControlShare, and #DmapImageShare classes.
 */

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
 * DMAP_IS_SHARE:
 * @o: Instance to check for being a %DMAP_TYPE_SHARE.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %DMAP_TYPE_SHARE.
 */
#define DMAP_IS_SHARE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				 DMAP_TYPE_SHARE))
/**
 * DMAP_IS_SHARE_CLASS:
 * @k: a #DmapShareClass
 *
 * Checks whether @k "is a" valid #DmapShareClass structure of type
 * %DMAP_SHARE or derived.
 */
#define DMAP_IS_SHARE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), \
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
typedef struct DmapSharePrivate DmapSharePrivate;

typedef struct {
	GObject parent;
	DmapSharePrivate *priv;
} DmapShare;

typedef struct DmapMetaDataMap DmapMetaDataMap;

typedef enum {
	DMAP_SHARE_AUTH_METHOD_NONE = 0,
	DMAP_SHARE_AUTH_METHOD_NAME_AND_PASSWORD = 1,
	DMAP_SHARE_AUTH_METHOD_PASSWORD = 2
} DmapShareAuthMethod;

typedef struct {
	GObjectClass parent;

	/* Pure virtual methods: */
	  guint (*get_desired_port) (DmapShare * share);
	const char *(*get_type_of_service) (DmapShare * share);
	void (*message_add_standard_headers) (DmapShare * share,
					      SoupServerMessage * msg);
	struct DmapMetaDataMap *(*get_meta_data_map) (DmapShare * share);
	void (*add_entry_to_mlcl) (guint id, DmapRecord * record, gpointer mb);
	void (*databases_browse_xxx) (DmapShare * share,
				      SoupServerMessage * msg,
				      const char *path,
				      GHashTable * query);
	void (*databases_items_xxx) (DmapShare * share,
				     SoupServer * server,
				     SoupServerMessage * msg,
	                             const char *path);

	/* Pure virtual methods: libsoup callbacks */
	void (*server_info) (DmapShare * share,
			     SoupServerMessage * message,
	                     const char *path);

	void (*content_codes) (DmapShare * share, SoupServerMessage * message,
                               const char *path);

	void (*login) (DmapShare * share,
		       SoupServerMessage * message, const char *path,
                       GHashTable * query);

	void (*logout) (DmapShare * share, SoupServerMessage * message,
                        const char *path, GHashTable * query);

	void (*update) (DmapShare * share, SoupServerMessage * message,
	                const char *path, GHashTable * query);

	void (*ctrl_int) (DmapShare * share, SoupServerMessage * message,
	                  const char *path, GHashTable * query);

	/* Virtual methods: MDNS callbacks */
	void (*published) (DmapShare * share,
                           DmapMdnsPublisher * publisher,
                           const char *name);

	void (*name_collision) (DmapShare * share,
				DmapMdnsPublisher * publisher,
				const char *name);

	/* Virtual methods: */
	void (*databases) (DmapShare * share,
			   SoupServer * server,
			   SoupServerMessage * message,
			   const char *path,
			   GHashTable * query);
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

/**
 * dmap_share_serve:
 * @share: a #DmapShare instance.
 * @error: return location for a GError, or NULL.
 *
 * Begin serving the service defined by share. A program will normally also
 * call dmap_share_publish.
 *
 * Returns: TRUE if serving succeeds, else FALSE with error set.
 */
gboolean dmap_share_serve(DmapShare *share, GError **error);

/**
 * dmap_share_publish:
 * @share: a #DmapShare instance.
 * @error: return location for a GError, or NULL.
 *
 * Publish the availability of the given share using mDNS-SD.
 *
 * Returns: TRUE if publishing succeeds, else FALSE.
 */
gboolean dmap_share_publish(DmapShare *share, GError **error);

/**
 * dmap_share_free_filter:
 * @filter: (element-type GSList): The filter list to free.
 *
 * Free the given filter list.
 */
void dmap_share_free_filter (GSList * filter);

/**
 * dmap_share_emit_error:
 * @share: a #DmapShare instance.
 * @code: error code.
 * @format: printf()-style format for error message
 * @...: parameters for message format
 */
void dmap_share_emit_error(DmapShare *share, gint code, const gchar *format, ...);

#endif /* _DMAP_SHARE_H */

G_END_DECLS
