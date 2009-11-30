/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Implmentation of DPAP (e.g., iPhoto Picture) sharing
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

#include "config.h"

#include <time.h>
#include <string.h>
#include <stdlib.h>

#include <glib/gi18n.h>
#include <glib/gslist.h>
#include <glib/gmappedfile.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <libsoup/soup.h>
#include <libsoup/soup-address.h>
#include <libsoup/soup-message.h>
#include <libsoup/soup-uri.h>
#include <libsoup/soup-server.h>

#include <libdmapsharing/dmap-db.h>
#include <libdmapsharing/dmap-container-db.h>
#include <libdmapsharing/dmap-container-record.h>
#include <libdmapsharing/dpap-record.h>
#include <libdmapsharing/dpap-share.h>
#include <libdmapsharing/dmap-structure.h>

static void dpap_share_set_property  (GObject *object,
					 guint prop_id,
					 const GValue *value,
					 GParamSpec *pspec);
static void dpap_share_get_property  (GObject *object,
					 guint prop_id,
					 GValue *value,
				 	 GParamSpec *pspec);
static void dpap_share_dispose	(GObject *object);
guint dpap_share_get_desired_port (DMAPShare *share);
const char *dpap_share_get_type_of_service (DMAPShare *share);
void dpap_share_server_info (DMAPShare         *share,
			     SoupServer        *server,
	  		     SoupMessage       *message,
			     const char        *path,
			     GHashTable        *query,
			     SoupClientContext *context);
void dpap_share_databases (DMAPShare         *share,
			     SoupServer        *server,
	  		     SoupMessage       *message,
			     const char        *path,
			     GHashTable        *query,
			     SoupClientContext *context);
void dpap_share_message_add_standard_headers (SoupMessage *message);

#define DPAP_TYPE_OF_SERVICE "_dpap._tcp"
#define DPAP_PORT 8770

struct DPAPSharePrivate {
	/* db things */
	DMAPDb *db;
	DMAPContainerDb *container_db;
};

/* Mmap'ed full image file. Global so that it may be free'ed in a different
 * function call that the one that set it up.
 */
static GMappedFile *mapped_file = NULL;

#define DPAP_SHARE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_DPAP_SHARE, DPAPSharePrivate))

enum {
	PROP_0,
	PROP_DB,
	PROP_CONTAINER_DB,
};

G_DEFINE_TYPE (DPAPShare, dpap_share, TYPE_DMAP_SHARE)

static void
dpap_share_class_init (DPAPShareClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	DMAPShareClass *parent_class = DMAP_SHARE_CLASS (object_class);

	object_class->get_property = dpap_share_get_property;
	object_class->set_property = dpap_share_set_property;
	object_class->dispose = dpap_share_dispose;

	parent_class->get_desired_port    = dpap_share_get_desired_port;
	parent_class->get_type_of_service = dpap_share_get_type_of_service;
	parent_class->message_add_standard_headers = dpap_share_message_add_standard_headers;
	parent_class->server_info         = dpap_share_server_info;
	parent_class->databases           = dpap_share_databases;

	/* FIXME?: */
	g_object_class_install_property (object_class,
                                         PROP_DB,
                                         g_param_spec_pointer ("db",
                                                              "DB",
                                                              "DB object",
                                                               G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
                                         PROP_CONTAINER_DB,
                                         g_param_spec_pointer ("container-db",
                                                              "Container DB",
                                                              "Container DB object",
                                                               G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	g_type_class_add_private (klass, sizeof (DPAPSharePrivate));
}

static void
dpap_share_init (DPAPShare *share)
{
	share->priv = DPAP_SHARE_GET_PRIVATE (share);
	/* FIXME: do I need to manually call parent _init? */
}

static void
dpap_share_set_property (GObject *object,
			    guint prop_id,
			    const GValue *value,
			    GParamSpec *pspec)
{
	DPAPShare *share = DPAP_SHARE (object);

	switch (prop_id) {
	/* FIXME: */
	case PROP_DB:
		share->priv->db = (DMAPDb *) g_value_get_pointer (value);
		break;
	case PROP_CONTAINER_DB:
		share->priv->container_db = (DMAPContainerDb *) g_value_get_pointer (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
dpap_share_get_property (GObject *object,
			    guint prop_id,
			    GValue *value,
			    GParamSpec *pspec)
{
	DPAPShare *share = DPAP_SHARE (object);

	switch (prop_id) {
	case PROP_DB:
		g_value_set_pointer (value, share->priv->db);
		break;
	case PROP_CONTAINER_DB:
		g_value_set_pointer (value, share->priv->container_db);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
dpap_share_dispose (GObject *object)
{
	/* FIXME: implement in parent */
}

/* FIXME: trancode_mimetype currently not used for DPAP, only DAAP. 
 *        Threrfore, it is not passed to g_object_new.
 */
DPAPShare *
dpap_share_new (const char *name,
		const char *password,
		gpointer db,
		gpointer container_db,
		gchar *transcode_mimetype)
{
	DPAPShare *share;

	share = DPAP_SHARE (g_object_new (TYPE_DPAP_SHARE,
					     "name", name,
					     "password", password,
					     "db", db,
					     "container-db", container_db,
					     NULL));

	_dmap_share_server_start (DMAP_SHARE (share));
	_dmap_share_publish_start (DMAP_SHARE (share));

	return share;
}

void
dpap_share_message_add_standard_headers (SoupMessage *message)
{
	soup_message_headers_append (message->response_headers, "DPAP-Server", "libdmapsharing" VERSION);
}

#define DPAP_STATUS_OK 200

#define DMAP_VERSION 2.0
#define DPAP_VERSION 1.1
#define DPAP_TIMEOUT 1800

guint
dpap_share_get_desired_port (DMAPShare *share)
{
	return DPAP_PORT;
}

const char *
dpap_share_get_type_of_service (DMAPShare *share)
{
	return DPAP_TYPE_OF_SERVICE;
}

void
dpap_share_server_info (DMAPShare *share,
		SoupServer        *server,
	  	SoupMessage       *message,
		const char        *path,
		GHashTable        *query,
		SoupClientContext *context)
{
/* MSRV	server info response
 * 	MSTT status
 * 	MPRO dpap version
 * 	PPRO dpap version
 * 	MINM name
 * 	MSAU authentication method
 * 	MSLR login required
 * 	MSTM timeout interval
 * 	MSAL supports auto logout
 * 	MSUP supports update
 * 	MSPI supports persistent ids
 * 	MSEX supports extensions
 * 	MSBR supports browse
 * 	MSQY supports query
 * 	MSIX supports index
 * 	MSRS supports resolve
 * 	MSDC databases count
 */
	gchar *nameprop;
	GNode *msrv;

	g_debug ("Path is %s.", path);

	g_object_get ((gpointer) share, "name", &nameprop, NULL);

	msrv = dmap_structure_add (NULL, DMAP_CC_MSRV);
	dmap_structure_add (msrv, DMAP_CC_MSTT, (gint32) DPAP_STATUS_OK);
	dmap_structure_add (msrv, DMAP_CC_MPRO, (gdouble) DMAP_VERSION);
	dmap_structure_add (msrv, DMAP_CC_PPRO, (gdouble) DPAP_VERSION);
	dmap_structure_add (msrv, DMAP_CC_MINM, nameprop);
	/*dmap_structure_add (msrv, DMAP_CC_MSAU, _dmap_share_get_auth_method (share));*/
	/* authentication method
	 * 0 is nothing
	 * 1 is name & password
	 * 2 is password only
	 */
	dmap_structure_add (msrv, DMAP_CC_MSLR, 0);
	dmap_structure_add (msrv, DMAP_CC_MSTM, (gint32) DPAP_TIMEOUT);
	dmap_structure_add (msrv, DMAP_CC_MSAL, (gchar) 0);
	/*dmap_structure_add (msrv, DMAP_CC_MSUP, (gchar) 0);
	 *dmap_structure_add (msrv, DMAP_CC_MSPI, (gchar) 0);
	 *dmap_structure_add (msrv, DMAP_CC_MSEX, (gchar) 0);
	 *dmap_structure_add (msrv, DMAP_CC_MSBR, (gchar) 0);
	 *dmap_structure_add (msrv, DMAP_CC_MSQY, (gchar) 0); */
	dmap_structure_add (msrv, DMAP_CC_MSIX, (gchar) 0);
	/* dmap_structure_add (msrv, DMAP_CC_MSRS, (gchar) 0); */
	dmap_structure_add (msrv, DMAP_CC_MSDC, (gint32) 1);

	_dmap_share_message_set_from_dmap_structure (share, message, msrv);
	dmap_structure_destroy (msrv);

	g_free (nameprop);
}

typedef enum {
	ITEM_ID = 0,
	ITEM_NAME,
	ITEM_KIND,
	PERSISTENT_ID,
	CONTAINER_ITEM_ID,
	PHOTO_ASPECTRATIO,
	PHOTO_CREATIONDATE,
	PHOTO_IMAGEFILENAME,
	PHOTO_IMAGEFORMAT,
	PHOTO_IMAGEFILESIZE,
	PHOTO_IMAGELARGEFILESIZE,
	PHOTO_IMAGEPIXELHEIGHT,
	PHOTO_IMAGEPIXELWIDTH,
	PHOTO_IMAGERATING,
	PHOTO_HIRES,
	PHOTO_THUMB,
	PHOTO_FILEDATA,
	PHOTO_IMAGECOMMENTS
} DPAPMetaData;

static struct DMAPMetaDataMap meta_data_map[] = {
	{"dmap.itemid",			ITEM_ID},
    	{"dmap.itemname",		ITEM_NAME},
    	{"dmap.itemkind",		ITEM_KIND},
    	{"dmap.persistentid",		PERSISTENT_ID},
	{"dmap.containeritemid",	CONTAINER_ITEM_ID},
    	{"dpap.aspectratio",		PHOTO_ASPECTRATIO},
    	{"dpap.creationdate",		PHOTO_CREATIONDATE},
	{"dpap.imagefilename",		PHOTO_IMAGEFILENAME},
	{"dpap.imageformat",		PHOTO_IMAGEFORMAT},
	{"dpap.imagefilesize",		PHOTO_IMAGEFILESIZE},
	{"dpap.imagelargefilesize",	PHOTO_IMAGELARGEFILESIZE},
	{"dpap.imagepixelheight",	PHOTO_IMAGEPIXELHEIGHT},
	{"dpap.imagepixelwidth",	PHOTO_IMAGEPIXELWIDTH},
	{"dpap.imagerating",		PHOTO_IMAGERATING},
	{"dpap.thumb",			PHOTO_THUMB},
	{"dpap.hires",			PHOTO_HIRES},
	{"dpap.filedata",		PHOTO_FILEDATA},
	{"dpap.imagecomments",		PHOTO_IMAGECOMMENTS}};

#define DPAP_ITEM_KIND_PHOTO 3 /* This is the constant that dpap-sharp uses. */

static GMappedFile *
file_to_mmap (const char *location)
{
        GFile *file;
        GMappedFile *mapped_file = NULL;
        char *path;
        GError *error = NULL;

        file = g_file_new_for_uri (location);
        path = g_file_get_path (file);
        if (path == NULL) {
                g_warning ("Couldn't mmap %s: couldn't get path", path);
                g_object_unref (file);
                return mapped_file;
        }
        g_object_unref (file);

        mapped_file = g_mapped_file_new (path, FALSE, &error);
        if (mapped_file == NULL) {
                g_warning ("Unable to map file %s: %s", path, error->message);
        }

        g_free (path);
	return mapped_file;
}

static void
add_entry_to_mlcl (gpointer id, DMAPRecord *record, gpointer _mb)
{
	struct MLCL_Bits *mb = (struct MLCL_Bits *) _mb;

	GNode *mlit;
	gchar *aspect_ratio;

	mlit = dmap_structure_add (mb->mlcl, DMAP_CC_MLIT);

	if (_dmap_share_client_requested (mb->bits, ITEM_KIND))
		dmap_structure_add (mlit, DMAP_CC_MIKD, (gchar) DPAP_ITEM_KIND_PHOTO);
	if (_dmap_share_client_requested (mb->bits, ITEM_ID))
		dmap_structure_add (mlit, DMAP_CC_MIID, (gint32) GPOINTER_TO_UINT (id));
	if (_dmap_share_client_requested (mb->bits, ITEM_NAME)) {
		gchar *filename;
		g_object_get (record, "filename", &filename, NULL);
		dmap_structure_add (mlit, DMAP_CC_MINM, filename);
	}
	if (_dmap_share_client_requested (mb->bits, PERSISTENT_ID))
		dmap_structure_add (mlit, DMAP_CC_MPER, (gint64) GPOINTER_TO_UINT (id));
	/* dpap-sharp claims iPhoto '08 will not show thumbnails without PASP: */
	g_object_get (record, "aspect-ratio", &aspect_ratio, NULL);
	dmap_structure_add (mlit, DMAP_CC_PASP, aspect_ratio);
	if (_dmap_share_client_requested (mb->bits, PHOTO_CREATIONDATE)) {
		gint creation_date;
		g_object_get (record, "creation-date", &creation_date, NULL);
		dmap_structure_add (mlit, DMAP_CC_PICD, creation_date);
	}
	if (_dmap_share_client_requested (mb->bits, PHOTO_IMAGEFILENAME)) {
		gchar *filename;
		g_object_get (record, "filename", &filename, NULL);
		dmap_structure_add (mlit, DMAP_CC_PIMF, filename);
	}
	if (_dmap_share_client_requested (mb->bits, PHOTO_IMAGEFORMAT)) {
		gchar *format;
		g_object_get (record, "format", &format, NULL);
		dmap_structure_add (mlit, DMAP_CC_PFMT, format);
	}
	if (_dmap_share_client_requested (mb->bits, PHOTO_IMAGEFILESIZE)) {
		gint filesize;
		g_object_get (record, "filesize", &filesize, NULL);
		dmap_structure_add (mlit, DMAP_CC_PIFS, filesize);
	}
	if (_dmap_share_client_requested (mb->bits, PHOTO_IMAGELARGEFILESIZE)) {
		gint large_filesize;
		g_object_get (record, "large-filesize", &large_filesize, NULL);
		dmap_structure_add (mlit, DMAP_CC_PLSZ, large_filesize);
	}
	if (_dmap_share_client_requested (mb->bits, PHOTO_IMAGEPIXELHEIGHT)) {
		gint pixel_height;
		g_object_get (record, "pixel-height", &pixel_height, NULL);
		dmap_structure_add (mlit, DMAP_CC_PHGT, pixel_height);
	}
	if (_dmap_share_client_requested (mb->bits, PHOTO_IMAGEPIXELWIDTH)) {
		gint pixel_width;
		g_object_get (record, "pixel-width", &pixel_width, NULL);
		dmap_structure_add (mlit, DMAP_CC_PWTH, pixel_width);
	}
	if (_dmap_share_client_requested (mb->bits, PHOTO_IMAGERATING)) {
		gint rating;
		g_object_get (record, "rating", &rating, NULL);
		dmap_structure_add (mlit, DMAP_CC_PRAT, rating);
	}
	if (_dmap_share_client_requested (mb->bits, PHOTO_IMAGECOMMENTS)) {
		gchar *comments;
		g_object_get (record, "comments", &comments, NULL);
		dmap_structure_add (mlit, DMAP_CC_PCMT, comments);
	}
	if (_dmap_share_client_requested (mb->bits, PHOTO_FILEDATA)) {
		size_t size = 0;
		unsigned char *data = NULL;
		if (_dmap_share_client_requested (mb->bits, PHOTO_THUMB)) {
			g_object_get (record, "thumbnail", &data, NULL);
			g_object_get (record, "filesize", &size, NULL);
		} else {
			/* Should be PHOTO_HIRES */
			const char *location;
			g_object_get (record, "location", &location, NULL);
			if (mapped_file) {
				/* Free any previously mapped image */
				g_mapped_file_free (mapped_file);
				mapped_file = NULL;
			}

			mapped_file = file_to_mmap (location);
			if (mapped_file == NULL) {
				/* FIXME: Error out ! */
			} else {
				data = (unsigned char *) g_mapped_file_get_contents (mapped_file);
				size = g_mapped_file_get_length (mapped_file);
			}
		}
		dmap_structure_add (mlit, DMAP_CC_PFDT, data, size);
	}
	return;
}

/* FIXME: Handle ('...') and share code with DAAPShare. */
static GSList *
build_filter (gchar *filterstr)
{
	/* Produces a list of lists, each being a filter definition that may
	 * be one or more filter criteria.
	 */

	/* A filter string looks like (iTunes):
	 * 'daap.songgenre:Other'+'daap.songartist:Band'.
	 * or (Roku):
	 * 'daap.songgenre:Other' 'daap.songartist:Band'.
	 * or
         * 'dmap.itemid:1000'
	 * or
         * 'dmap.itemid:1000','dmap:itemid:1001'
	 * or
	 * 'daap.songgenre:Foo'+'daap.songartist:Bar'+'daap.songalbum:Baz'
         */

	GSList *list = NULL;

	g_debug ("Filter string is %s.", filterstr);

	if (filterstr != NULL) {
		int i;
		gchar **t1 = g_strsplit (filterstr, ",", 0);

		for (i = 0; t1[i]; i++) {
			int j;
			GSList *filter = NULL;
			gchar **t2;

			t2 = _dmap_db_strsplit_using_quotes (t1[i]);

			for (j = 0; t2[j]; j++) {
				FilterDefinition *def;
				gchar **t3;

				t3 = g_strsplit (t2[j], ":", 0);

				if (g_strcasecmp ("dmap.itemid", t3[0]) == 0) {
					def = g_new0 (FilterDefinition, 1);
					def->value = g_strdup (t3[1]);
					def->record_get_value = NULL;
				} else {
					g_warning ("Unknown category: %s", t3[0]);
					def = NULL;
				}

				if (def != NULL)
					filter = g_slist_append (filter, def);

				g_strfreev (t3);
			}

			list = g_slist_append (list, filter);

			g_strfreev (t2);
		}
		g_strfreev (t1);
	}

        return list;
}

static void
debug_param (gpointer key, gpointer val, gpointer user_data)
{
        g_debug ("%s %s", (char *) key, (char *) val);
}

void
dpap_share_databases (DMAPShare *share,
	      SoupServer        *server,
	      SoupMessage       *message,
	      const char        *path,
	      GHashTable        *query,
	      SoupClientContext *context)

{
	const char *rest_of_path;

	g_debug ("Path is %s.", path);
	g_hash_table_foreach (query, debug_param, NULL);

	if (! _dmap_share_session_id_validate (share, context, message, query, NULL)) {
		soup_message_set_status (message, SOUP_STATUS_FORBIDDEN);
		return;
	}

	rest_of_path = strchr (path + 1, '/');

	if (rest_of_path == NULL) {
	/* AVDB server databases
	 * 	MSTT status
	 * 	MUTY update type
	 * 	MTCO specified total count
	 * 	MRCO returned count
	 * 	MLCL listing
	 * 		MLIT listing item
	 * 			MIID item id
	 * 			MPER persistent id
	 * 			MINM item name
	 * 			MIMC item count
	 * 			MCTC container count
	 */
		gchar *nameprop;
		GNode *avdb;
		GNode *mlcl;
		GNode *mlit;

		g_object_get ((gpointer) share, "name", &nameprop, NULL);

		avdb = dmap_structure_add (NULL, DMAP_CC_AVDB);
		dmap_structure_add (avdb, DMAP_CC_MSTT, (gint32) DPAP_STATUS_OK);
		dmap_structure_add (avdb, DMAP_CC_MUTY, 0);
		dmap_structure_add (avdb, DMAP_CC_MTCO, (gint32) 1);
		dmap_structure_add (avdb, DMAP_CC_MRCO, (gint32) 1);
		mlcl = dmap_structure_add (avdb, DMAP_CC_MLCL);
		mlit = dmap_structure_add (mlcl, DMAP_CC_MLIT);
		dmap_structure_add (mlit, DMAP_CC_MIID, (gint32) 1);
		dmap_structure_add (mlit, DMAP_CC_MPER, (gint64) 1);
		dmap_structure_add (mlit, DMAP_CC_MINM, nameprop);
		dmap_structure_add (mlit, DMAP_CC_MIMC, dmap_db_count (DPAP_SHARE (share)->priv->db));
		dmap_structure_add (mlit, DMAP_CC_MCTC, (gint32) 1);

		_dmap_share_message_set_from_dmap_structure (share, message, avdb);
		dmap_structure_destroy (avdb);

		g_free (nameprop);
	} else if (g_ascii_strcasecmp ("/1/items", rest_of_path) == 0) {
	/* ADBS database songs
	 * 	MSTT status
	 * 	MUTY update type
	 * 	MTCO specified total count
	 * 	MRCO returned count
	 * 	MLCL listing
	 * 		MLIT
	 * 			attrs
	 * 		MLIT
	 * 		...
	 */
		GNode *adbs;
		gchar *record_query;
		gint32 num_songs = dmap_db_count (DPAP_SHARE (share)->priv->db);
		struct MLCL_Bits mb = {NULL,0};

		mb.bits = _dmap_share_parse_meta (query, meta_data_map, G_N_ELEMENTS (meta_data_map));

		adbs = dmap_structure_add (NULL, DMAP_CC_ADBS);
		dmap_structure_add (adbs, DMAP_CC_MSTT, (gint32) DPAP_STATUS_OK);
		dmap_structure_add (adbs, DMAP_CC_MUTY, 0);
		dmap_structure_add (adbs, DMAP_CC_MTCO, (gint32) num_songs);
		dmap_structure_add (adbs, DMAP_CC_MRCO, (gint32) num_songs);
		mb.mlcl = dmap_structure_add (adbs, DMAP_CC_MLCL);

		record_query = g_hash_table_lookup (query, "query");

		if (record_query) {
			GHashTable *records;
			GSList *filter_def;

			/* FIXME: fix memory leaks (DAAP too): */
			filter_def = build_filter (record_query);
			records = _dmap_db_apply_filter (DMAP_DB (DPAP_SHARE (share)->priv->db), filter_def);
			g_hash_table_foreach (records, (GHFunc) add_entry_to_mlcl, &mb);
			/* FIXME: need to free hash table keys but not records */
		} else {
			g_warning ("Missing query parameter");
		}

		_dmap_share_message_set_from_dmap_structure (share, message, adbs);
		dmap_structure_destroy (adbs);
		adbs = NULL;
	} else if (g_ascii_strcasecmp ("/1/containers", rest_of_path) == 0) {
	/* APLY database playlists
	 * 	MSTT status
	 * 	MUTY update type
	 * 	MTCO specified total count
	 * 	MRCO returned count
	 * 	MLCL listing
	 * 		MLIT listing item
	 * 			MIID item id
	 * 			MPER persistent item id
	 * 			MINM item name
	 * 			MIMC item count
	 * 			ABPL baseplaylist (only for base)
	 * 		MLIT
	 * 		...
	 */
		gchar *nameprop;
		GNode *aply;
		GNode *mlcl;
		GNode *mlit;

		g_object_get ((gpointer) share, "name", &nameprop, NULL);

		aply = dmap_structure_add (NULL, DMAP_CC_APLY);
		dmap_structure_add (aply, DMAP_CC_MSTT, (gint32) DPAP_STATUS_OK);
		dmap_structure_add (aply, DMAP_CC_MUTY, 0);
		dmap_structure_add (aply, DMAP_CC_MTCO, (gint32) 1);
		dmap_structure_add (aply, DMAP_CC_MRCO, (gint32) 1);
		mlcl = dmap_structure_add (aply, DMAP_CC_MLCL);

		mlit = dmap_structure_add (mlcl, DMAP_CC_MLIT);
		dmap_structure_add (mlit, DMAP_CC_MIID, (gint32) 1);
		dmap_structure_add (mlit, DMAP_CC_MPER, (gint64) 1);
		dmap_structure_add (mlit, DMAP_CC_MINM, nameprop);
		dmap_structure_add (mlit, DMAP_CC_MIMC, dmap_db_count (DPAP_SHARE (share)->priv->db));
		dmap_structure_add (mlit, DMAP_CC_ABPL, (gchar) 1); /* base album (AKA playlist) */

		dmap_container_db_foreach (DPAP_SHARE (share)->priv->container_db, (GHFunc) _dmap_share_add_playlist_to_mlcl, (gpointer) mlcl);

		_dmap_share_message_set_from_dmap_structure (share, message, aply);
		dmap_structure_destroy (aply);

		g_free (nameprop);
	} else if (g_ascii_strncasecmp ("/1/containers/", rest_of_path, 14) == 0) {
	/* APSO playlist songs
	 * 	MSTT status
	 * 	MUTY update type
	 * 	MTCO specified total count
	 * 	MRCO returned count
	 * 	MLCL listing
	 * 		MLIT listing item
	 * 			MIKD item kind
	 * 			MIID item id
	 * 			MCTI container item id
	 * 		MLIT
	 * 		...
	 */
		GNode *apso;
		struct MLCL_Bits mb = {NULL,0};
		gint pl_id = atoi (rest_of_path + 14);

		mb.bits = _dmap_share_parse_meta (query, meta_data_map, G_N_ELEMENTS (meta_data_map));

		apso = dmap_structure_add (NULL, DMAP_CC_APSO);
		dmap_structure_add (apso, DMAP_CC_MSTT, (gint32) DPAP_STATUS_OK);
		dmap_structure_add (apso, DMAP_CC_MUTY, 0);

		if (pl_id == 1) {
			gint32 num_songs = dmap_db_count (DPAP_SHARE (share)->priv->db);
			dmap_structure_add (apso, DMAP_CC_MTCO, (gint32) num_songs);
			dmap_structure_add (apso, DMAP_CC_MRCO, (gint32) num_songs);
			mb.mlcl = dmap_structure_add (apso, DMAP_CC_MLCL);

			dmap_db_foreach (DPAP_SHARE (share)->priv->db, (GHFunc) add_entry_to_mlcl, &mb);
		} else {
			DMAPContainerRecord *record;
                        const DMAPDb *entries;
                        guint num_songs;

                        record = dmap_container_db_lookup_by_id (DPAP_SHARE (share)->priv->container_db, pl_id);
			entries = dmap_container_record_get_entries (record);
			num_songs = dmap_db_count (entries);

                        dmap_structure_add (apso, DMAP_CC_MTCO, (gint32) num_songs);
                        dmap_structure_add (apso, DMAP_CC_MRCO, (gint32) num_songs);
                        mb.mlcl = dmap_structure_add (apso, DMAP_CC_MLCL);

			dmap_db_foreach (entries, (GHFunc) add_entry_to_mlcl, (gpointer) &mb);

			g_object_unref (record);
		}

		_dmap_share_message_set_from_dmap_structure (share, message, apso);
		dmap_structure_destroy (apso);
	} else {
		g_warning ("Unhandled: %s\n", path);
	}
}
