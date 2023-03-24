/* Implmentation of DPAP (e.g., iPhoto Picture) sharing
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
#include <glib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <libsoup/soup.h>

#include <libdmapsharing/dmap.h>
#include <libdmapsharing/dmap-share-private.h>
#include <libdmapsharing/dmap-private-utils.h>
#include <libdmapsharing/dmap-structure.h>

static guint _get_desired_port (DmapShare * share);
static const char *_get_type_of_service (DmapShare * share);
static void _server_info (DmapShare * share, SoupServerMessage * message, const char *path);
static void _message_add_standard_headers (DmapShare * share, SoupServerMessage * message);

#define DPAP_TYPE_OF_SERVICE "_dpap._tcp"
#define DPAP_PORT 8770

struct DmapImageSharePrivate
{
	gchar unused;
};

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

static struct DmapMetaDataMap _meta_data_map[] = {
	{"dmap.itemid", ITEM_ID},
	{"dmap.itemname", ITEM_NAME},
	{"dmap.itemkind", ITEM_KIND},
	{"dmap.persistentid", PERSISTENT_ID},
	{"dmap.containeritemid", CONTAINER_ITEM_ID},
	{"dpap.aspectratio", PHOTO_ASPECTRATIO},
	{"dpap.creationdate", PHOTO_CREATIONDATE},
	{"dpap.imagefilename", PHOTO_IMAGEFILENAME},
	{"dpap.imageformat", PHOTO_IMAGEFORMAT},
	{"dpap.imagefilesize", PHOTO_IMAGEFILESIZE},
	{"dpap.imagelargefilesize", PHOTO_IMAGELARGEFILESIZE},
	{"dpap.imagepixelheight", PHOTO_IMAGEPIXELHEIGHT},
	{"dpap.imagepixelwidth", PHOTO_IMAGEPIXELWIDTH},
	{"dpap.imagerating", PHOTO_IMAGERATING},
	{"dpap.thumb", PHOTO_THUMB},
	{"dpap.hires", PHOTO_HIRES},
	{"dpap.filedata", PHOTO_FILEDATA},
	{"dpap.imagecomments", PHOTO_IMAGECOMMENTS},
	{NULL, 0}
};

#define DPAP_ITEM_KIND_PHOTO 3	/* This is the constant that dpap-sharp uses. */

/* Mmap'ed full image file. Global so that it may be free'ed in a different
 * function call that the one that set it up.
 */
static GMappedFile *_mapped_file = NULL;

G_DEFINE_TYPE_WITH_PRIVATE (DmapImageShare,
                            dmap_image_share,
                            DMAP_TYPE_SHARE);

static struct DmapMetaDataMap *
_get_meta_data_map (G_GNUC_UNUSED DmapShare * share)
{
	return _meta_data_map;
}

static GMappedFile *
_file_to_mmap (const char *location)
{
	GFile *file;
	GMappedFile *mapped_file = NULL;
	char *path = NULL;
	GError *error = NULL;

	file = g_file_new_for_uri (location);
	/* NOTE: this is broken if original filename contains "%20" etc. This
	 * is because g_file_get_path() will translate this to " ", etc. But
	 * the filename really may have used "%20" (not " ").
	 */
	path = g_file_get_path (file);
	if (path == NULL) {
		g_warning ("Couldn't mmap %s: couldn't get path", path);
		g_object_unref (file);
		goto done;
	}
	g_object_unref (file);

	mapped_file = g_mapped_file_new (path, FALSE, &error);
	if (mapped_file == NULL) {
		g_warning ("Unable to map file %s: %s", path, error->message);
	}

done:
	g_free (path);

	return mapped_file;
}

static void
_add_entry_to_mlcl (guint id, DmapRecord * record, gpointer _mb)
{
	GNode *mlit;
	struct DmapMlclBits *mb = (struct DmapMlclBits *) _mb;

	mlit = dmap_structure_add (mb->mlcl, DMAP_CC_MLIT);

	if (dmap_share_client_requested (mb->bits, ITEM_KIND)) {
		dmap_structure_add (mlit, DMAP_CC_MIKD,
				    (gchar) DPAP_ITEM_KIND_PHOTO);
	}

	if (dmap_share_client_requested (mb->bits, ITEM_ID)) {
		dmap_structure_add (mlit, DMAP_CC_MIID, id);
	}

	if (dmap_share_client_requested (mb->bits, ITEM_NAME)) {
		gchar *filename = NULL;

		g_object_get (record, "filename", &filename, NULL);
		if (filename) {
			dmap_structure_add (mlit, DMAP_CC_MINM, filename);
			g_free (filename);
		} else {
			g_debug ("Filename requested but not available");
		}
	}

	if (dmap_share_client_requested (mb->bits, PERSISTENT_ID)) {
		dmap_structure_add (mlit, DMAP_CC_MPER, id);
	}

	if (TRUE) {
		/* dpap-sharp claims iPhoto '08 will not show thumbnails without PASP
		 * and this does seem to be the case when testing. */
		gchar *aspect_ratio = NULL;

		g_object_get (record, "aspect-ratio", &aspect_ratio, NULL);
		if (aspect_ratio) {
			dmap_structure_add (mlit, DMAP_CC_PASP, aspect_ratio);
			g_free (aspect_ratio);
		} else {
			g_debug
				("Aspect ratio requested but not available");
		}
	}

	if (dmap_share_client_requested (mb->bits, PHOTO_CREATIONDATE)) {
		gint creation_date = 0;

		g_object_get (record, "creation-date", &creation_date, NULL);
		dmap_structure_add (mlit, DMAP_CC_PICD, creation_date);
	}

	if (dmap_share_client_requested (mb->bits, PHOTO_IMAGEFILENAME)) {
		gchar *filename = NULL;

		g_object_get (record, "filename", &filename, NULL);
		if (filename) {
			dmap_structure_add (mlit, DMAP_CC_PIMF, filename);
			g_free (filename);
		} else {
			g_debug ("Filename requested but not available");
		}
	}

	if (dmap_share_client_requested (mb->bits, PHOTO_IMAGEFORMAT)) {
		gchar *format = NULL;

		g_object_get (record, "format", &format, NULL);
		if (format) {
			dmap_structure_add (mlit, DMAP_CC_PFMT, format);
			g_free (format);
		} else {
			g_debug ("Format requested but not available");
		}
	}

	if (dmap_share_client_requested (mb->bits, PHOTO_IMAGEFILESIZE)) {
		GArray *thumbnail = NULL;

		g_object_get (record, "thumbnail", &thumbnail, NULL);
		if (thumbnail) {
			dmap_structure_add (mlit, DMAP_CC_PIFS, thumbnail->len);
			g_array_unref(thumbnail);
		} else {
			dmap_structure_add (mlit, DMAP_CC_PIFS, 0);
		}
	}

	if (dmap_share_client_requested (mb->bits, PHOTO_IMAGELARGEFILESIZE)) {
		gint large_filesize = 0;

		g_object_get (record, "large-filesize", &large_filesize,
			      NULL);
		dmap_structure_add (mlit, DMAP_CC_PLSZ, large_filesize);
	}

	if (dmap_share_client_requested (mb->bits, PHOTO_IMAGEPIXELHEIGHT)) {
		gint pixel_height = 0;

		g_object_get (record, "pixel-height", &pixel_height, NULL);
		dmap_structure_add (mlit, DMAP_CC_PHGT, pixel_height);
	}

	if (dmap_share_client_requested (mb->bits, PHOTO_IMAGEPIXELWIDTH)) {
		gint pixel_width = 0;

		g_object_get (record, "pixel-width", &pixel_width, NULL);
		dmap_structure_add (mlit, DMAP_CC_PWTH, pixel_width);
	}

	if (dmap_share_client_requested (mb->bits, PHOTO_IMAGERATING)) {
		gint rating = 0;

		g_object_get (record, "rating", &rating, NULL);
		dmap_structure_add (mlit, DMAP_CC_PRAT, rating);
	}

	if (dmap_share_client_requested (mb->bits, PHOTO_IMAGECOMMENTS)) {
		gchar *comments = NULL;

		g_object_get (record, "comments", &comments, NULL);
		if (comments) {
			dmap_structure_add (mlit, DMAP_CC_PCMT, comments);
			g_free (comments);
		} else {
			g_debug ("Comments requested but not available");
		}
	}

	if (dmap_share_client_requested (mb->bits, PHOTO_FILEDATA)) {
		size_t size = 0;
		char *data = NULL;
		GArray *thumbnail = NULL;

		if (dmap_share_client_requested (mb->bits, PHOTO_THUMB)) {
			g_object_get (record, "thumbnail", &thumbnail, NULL);
			if (thumbnail) {
				data = thumbnail->data;
				size = thumbnail->len;
			} else {
				data = NULL;
				size = 0;
			}
		} else {
			/* Should be PHOTO_HIRES */
			char *location = NULL;

			g_object_get (record, "location", &location, NULL);
			if (_mapped_file) {
				/* Free any previously mapped image */
				g_mapped_file_unref (_mapped_file);
				_mapped_file = NULL;
			}

			_mapped_file = _file_to_mmap (location);
			if (_mapped_file == NULL) {
				g_warning ("Error opening %s", location);
				data = NULL;
				size = 0;
			} else {
				data = (char *)
					g_mapped_file_get_contents (_mapped_file);
				size = g_mapped_file_get_length (_mapped_file);
			}
			g_free (location);
		}
		dmap_structure_add (mlit, DMAP_CC_PFDT, data, size);
	}
}

static void
_databases_browse_xxx (G_GNUC_UNUSED DmapShare * share,
                       G_GNUC_UNUSED SoupServerMessage * msg,
                       const char *path,
                       G_GNUC_UNUSED GHashTable *query)
{
	g_warning ("Unhandled: %s", path);
}

static void
_send_chunked_file (SoupServer * server, SoupServerMessage * message,
                    DmapImageRecord * record, guint64 filesize)
{
	GInputStream *stream;
	char *location = NULL;
	GError *error = NULL;
	ChunkData *cd = g_new0 (ChunkData, 1);
	SoupMessageHeaders *headers = NULL;

	g_object_get (record, "location", &location, NULL);

	cd->server = server;

	stream = G_INPUT_STREAM (dmap_image_record_read (record, &error));

	if (error != NULL) {
		g_warning ("Couldn't open %s: %s.", location, error->message);
		g_error_free (error);
		soup_server_message_set_status (message,
					 SOUP_STATUS_INTERNAL_SERVER_ERROR, NULL);
		g_free (cd);
		goto done;
	}

	cd->stream = stream;

	if (cd->stream == NULL) {
		g_warning ("Could not set up input stream");
		g_free (cd);
		goto done;
	}

	headers = soup_server_message_get_response_headers(message);

	soup_message_headers_set_encoding (headers, SOUP_ENCODING_CONTENT_LENGTH);
	soup_message_headers_set_content_length (headers, filesize);

	soup_message_headers_append (headers, "Connection", "Close");
	soup_message_headers_append (headers, "Content-Type", "application/x-dmap-tagged");

	g_signal_connect (message, "wrote_headers",
			  G_CALLBACK (dmap_private_utils_write_next_chunk), cd);
	g_signal_connect (message, "wrote_chunk",
			  G_CALLBACK (dmap_private_utils_write_next_chunk), cd);
	g_signal_connect (message, "finished",
			  G_CALLBACK (dmap_private_utils_chunked_message_finished), cd);
	/* NOTE: cd g_free'd by chunked_message_finished(). */

done:
	g_free(location);
}

static void
_databases_items_xxx (DmapShare * share,
                      SoupServer * server,
                      SoupServerMessage * msg,
                      const char *path)
{
	DmapDb *db = NULL;
	const gchar *rest_of_path;
	const gchar *id_str;
	guint id;
	guint64 filesize = 0;
	DmapImageRecord *record;

	rest_of_path = strchr (path + 1, '/');
	id_str = rest_of_path + 9;
	id = strtoul (id_str, NULL, 10);

	g_object_get (share, "db", &db, NULL);
	record = DMAP_IMAGE_RECORD (dmap_db_lookup_by_id (db, id));
	g_object_get (record, "large-filesize", &filesize, NULL);

	DMAP_SHARE_GET_CLASS (share)->message_add_standard_headers
		(share, msg);
	soup_server_message_set_status (msg, SOUP_STATUS_OK, NULL);

	_send_chunked_file (server, msg, record, filesize);

	g_object_unref (record);
}

static void
dmap_image_share_class_init (DmapImageShareClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	DmapShareClass *parent_class = DMAP_SHARE_CLASS (object_class);

	parent_class->get_desired_port = _get_desired_port;
	parent_class->get_type_of_service = _get_type_of_service;
	parent_class->message_add_standard_headers = _message_add_standard_headers;
	parent_class->get_meta_data_map = _get_meta_data_map;
	parent_class->add_entry_to_mlcl = _add_entry_to_mlcl;
	parent_class->databases_browse_xxx = _databases_browse_xxx;
	parent_class->databases_items_xxx = _databases_items_xxx;
	parent_class->server_info = _server_info;
}

static void
dmap_image_share_init (DmapImageShare * share)
{
	/* FIXME: do I need to manually call parent _init? */
	share->priv = dmap_image_share_get_instance_private(share);
}

/* FIXME: trancode_mimetype currently not used for DPAP, only DAAP. 
 *        Threrfore, it is not passed to g_object_new.
 */
DmapImageShare *
dmap_image_share_new (const char *name,
                      const char *password,
                      gpointer db,
                      gpointer container_db,
                      G_GNUC_UNUSED gchar * transcode_mimetype)
{
	g_object_ref (db);
	g_object_ref (container_db);

	return DMAP_IMAGE_SHARE (g_object_new (DMAP_TYPE_IMAGE_SHARE,
	                                      "name", name,
	                                      "password", password,
	                                      "db", db,
	                                      "container-db", container_db,
	                                       NULL));
}

static void
_message_add_standard_headers (G_GNUC_UNUSED DmapShare * share, SoupServerMessage * message)
{
	soup_message_headers_append (
		soup_server_message_get_response_headers(message),
		"DPAP-Server",
		"libdmapsharing" VERSION
	);
}

#define DMAP_VERSION 2.0
#define DPAP_VERSION 1.1
#define DPAP_TIMEOUT 1800

static guint
_get_desired_port (G_GNUC_UNUSED DmapShare * share)
{
	return DPAP_PORT;
}

static const char *
_get_type_of_service (G_GNUC_UNUSED DmapShare * share)
{
	return DPAP_TYPE_OF_SERVICE;
}

static void
_server_info (DmapShare * share, SoupServerMessage * message, const char *path)
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
	dmap_structure_add (msrv, DMAP_CC_MSTT, (gint32) SOUP_STATUS_OK);
	dmap_structure_add (msrv, DMAP_CC_MPRO, (gdouble) DMAP_VERSION);
	dmap_structure_add (msrv, DMAP_CC_PPRO, (gdouble) DPAP_VERSION);
	dmap_structure_add (msrv, DMAP_CC_MINM, nameprop);
	/*dmap_structure_add (msrv, DMAP_CC_MSAU, dmap_share_get_auth_method (share)); */
	/* authentication method
	 * 0 is nothing
	 * 1 is name & password
	 * 2 is password only
	 */
	dmap_structure_add (msrv, DMAP_CC_MSLR, 0);
	dmap_structure_add (msrv, DMAP_CC_MSTM, (gint32) DPAP_TIMEOUT);
	dmap_structure_add (msrv, DMAP_CC_MSAL, (gchar) 0);
	/*dmap_structure_add (msrv, DMAP_CC_MSUP, (gchar) 1);
	 *dmap_structure_add (msrv, DMAP_CC_MSPI, (gchar) 0);
	 *dmap_structure_add (msrv, DMAP_CC_MSEX, (gchar) 0);
	 *dmap_structure_add (msrv, DMAP_CC_MSBR, (gchar) 0);
	 *dmap_structure_add (msrv, DMAP_CC_MSQY, (gchar) 0); */
	dmap_structure_add (msrv, DMAP_CC_MSIX, (gchar) 0);
	/* dmap_structure_add (msrv, DMAP_CC_MSRS, (gchar) 0); */
	dmap_structure_add (msrv, DMAP_CC_MSDC, (gint32) 1);

	dmap_share_message_set_from_dmap_structure (share, message, msrv);
	dmap_structure_destroy (msrv);

	g_free (nameprop);
}
